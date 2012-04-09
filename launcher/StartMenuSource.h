#pragma once
#include "FindFileRecursively.h"
#include "ShellLink.h"
#include "Source.h"
#include "sqlite3/sqlite3.h"
#include "Utility.h"
#include "resource.h"

CString GetSpecialFolder(int csidl);
int SaveSearchFolders(HWND hListView);

extern int uselev;

// might be useful
// http://stackoverflow.com/questions/1744902/how-might-i-obtain-the-icontextmenu-that-is-displayed-in-an-ishellview-context-m
struct StartMenuSource : Source {
    sqlite3 *db;
    StartMenuSource(HWND hwnd) : Source(L"FILE", L"Indexed files (Catalog )"), m_hwnd(hwnd) {        
        m_ignoreemptyquery=true;
        
        int rc = sqlite3_open("databases\\startmenu.db", &db);
        
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE startmenu(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);        

        UpgradeTable(db,"startmenu");
        // deleted tables : startmenu_verbs, startmenu_verbs_index
    }
    ~StartMenuSource() {
        sqlite3_close(db);
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        // could probably be done in subclass as well as the callback since sourceresult will not change 
        CString q(query);
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        rc = sqlite3_prepare_v2(db,"SELECT key, display, expand, path, uses FROM startmenu WHERE display LIKE ?;",-1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
        int i=0;
        while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
            results.push_back(SourceResult(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),     // key
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),    // display
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),    // expand
                                            this,                                               // source
                                            0,                                                  // id
                                            0,                                                  // data                                            
                                            sqlite3_column_int(stmt,4)));                       // Use

            results.back().object=new FileObject(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),
                                                 this,
                                                 UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),
                                                 UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),
                                                 UTF8toUTF16((char*)sqlite3_column_text(stmt,3)));
        }

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(stmt);
    }
    void crawl() {
        std::vector<CString> lnks;

        FindFilesRecursively(GetSpecialFolder(CSIDL_COMMON_STARTMENU), L"*.lnk", lnks, 999);
        FindFilesRecursively(GetSpecialFolder(CSIDL_STARTMENU), L"*.lnk", lnks, 999);

        WCHAR curDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, curDir);

        pugi::xpath_node_set ns=settingsWT.select_nodes("/settings/searchFolders");
        for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++) {
            lnks.push_back(UTF8toUTF16(it->node().child_value("folder")));
            FindFilesRecursively(lnks.back(), L"*.*", lnks, 3);
        }

        CStringA q; // 82 bug
        q+="BEGIN;\n";
        char *zErrMsg=0;

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        // there can only exists one single mark
        int mark;
        sqlite3_exec(db, "SELECT mark FROM startmenu LIMIT 1;", getIntCB, &mark, &zErrMsg);
        sqlite3_free(zErrMsg);

        rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        for(uint i=0;i<lnks.size();i++) {
            CString str(lnks[i]);
            if(str.Right(4)==L".lnk") {
                PathRemoveExtension(str.GetBuffer()); str.ReleaseBuffer();
            }
            str=PathFindFileName(str.GetBuffer()); str.ReleaseBuffer();            

            CString d=lnks[i].Left(lnks[i].ReverseFind(L'\\'));
            CString f=lnks[i].Mid(lnks[i].ReverseFind(L'\\')+1);

            CString startmenu_key=md5(lnks[i]);
            
            rc = sqlite3_prepare_v2(db,
                                    "INSERT OR REPLACE INTO startmenu(key,display,expand,path,uses,mark) VALUES(?, ?, ?, ?, coalesce((SELECT uses FROM startmenu WHERE key=?), 0), ?);\n",
                                    -1, &stmt, &unused);
            rc = sqlite3_bind_text16(stmt, 1, startmenu_key.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 2, str.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 3, str.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 4, lnks[i].GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 5, startmenu_key.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_int(stmt, 6, mark+1);
            rc = sqlite3_step(stmt);
            const char *errmsg=sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
        }

        rc = sqlite3_exec(db, "END;", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        // delete disappeared results
        sqlite3_prepare_v2(db, "DELETE FROM startmenu WHERE mark != ?", -1, &stmt, &unused);
        rc = sqlite3_bind_int(stmt, 1, mark+1);
        rc = sqlite3_step(stmt);
        //const char *errmsg=sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
    }
    // validate
    void validate(SourceResult *r) {
        WCHAR buff[4096];
        char *zErrMsg = 0;
        // FIXME : bonus should be nb_use to allow fixing various coefficient to it
        wsprintf(buff, L"UPDATE startmenu SET uses = uses + 1 WHERE key=\"%s\"\n", r->object->key);        
        int z=sqlite3_exec(db, CStringA(buff), 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }

    // getvalue name, buff, bufflen
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return r->object->getIcon(flags);
    }
    // itemkey/name : itemkey/verb/open/icon < get subresults ???
    virtual CString getString(SourceResult &sr,const TCHAR *val_) {
        return sr.object->getString(val_);
    }
    virtual int getInt(const TCHAR *itemquery) {
        return false; 
    }

    HWND m_hwnd;
};



// dumb code to scan the control panel, leaks included
// ugly but works
// I have an issue considering how to store various kinds of data for reusing later
// here for icons it's ok if I store a itemidlist but how to persist it ??? and such others
// issues
// saving, loading, deleting, getting icons has to pass through the source. Should I get
// args from the source as well ???? its possible via a getarg(id, value*) ???
// where value is anything
/*CComPtr<IShellFolder> pSF;
SHGetDesktopFolder(&pSF);
ULONG eaten=0;
LPITEMIDLIST pidl=0;
DWORD attr=0;
pSF->ParseDisplayName(0, 0, L"::{26EE0668-A00A-44D7-9371-BEB064C98683}", &eaten, &pidl, &attr);
CComPtr<IShellFolder> pCPSF;
//pSF->GetUIObjectOf(0, 1, (LPCITEMIDLIST*)&pidl, IID_IShellFolder, 0, (void**)&pCPSF);
pSF->BindToObject(pidl, 0, IID_IShellFolder, (void**)&pCPSF);

CComPtr<IEnumIDList> pEIDL;
pCPSF->EnumObjects(0, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS, &pEIDL);

LPITEMIDLIST pidl2;
ULONG fetched=0;
STRRET name;
while(pEIDL->Next(1, &pidl2, &fetched)==S_OK) {         
    // TODO leak
    // TODO make absolute pidl 
    pCPSF->GetDisplayNameOf(pidl2, SHGDN_NORMAL, &name);
            
    CComPtr<IPersistFolder2> pSF2;
    pCPSF->BindToObject(pidl2, 0, IID_IPersistFolder2, (void**)&pSF2);
            
    LPITEMIDLIST pidlCF=0;
    pSF2->GetCurFolder(&pidlCF);

    m_results.push_back(SourceResult(CStringW(name.pOleStr), L"", this, 0, 0));
    m_results.back().icon=::getIcon(pidlCF);
}*/

class SearchFoldersDlg : public CDialogImpl<SearchFoldersDlg>
{
public:
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(SearchFoldersDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        //MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        //MESSAGE_HANDLER(WM_SAVESETTINGS, OnSaveSettings)        
    END_MSG_MAP()

    HWND hListView;

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        CRect r;
        INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        RECT rcClient;                       // The parent window's client area.

        GetClientRect(&rcClient); 

        // Create the list-view window in report view with label editing enabled.
        hListView = ::CreateWindow(WC_LISTVIEW, 
                                        L"",
                                        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE | WS_BORDER,
                                        0, 0,
                                        rcClient.right - rcClient.left,
                                        rcClient.bottom - rcClient.top,
                                        m_hWnd,
                                        0,
                                        0,
                                        NULL); 

        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT);

        // allows for full row selection
        //ListView_SetExtendedListViewStyle(hListView, ListView_GetExtendedListViewStyle(hListView)| LVS_EX_FULLROWSELECT);

        ::GetClientRect(hListView, &r);

        LVCOLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText=L"Path";
        lvc.cx=r.Width()-60;
        ListView_InsertColumn(hListView, 0, &lvc);

        lvc.pszText=L"Depth";
        lvc.cx=60;
        lvc.fmt = LVCFMT_CENTER;
        ListView_InsertColumn(hListView, 1, &lvc);

        //lvc.pszText=L"Bonus";
        //lvc.cx=40;
        //ListView_InsertColumn(GetDlgItem(hWnd, IDC_LIST3), 0, &lvc);
            
        WCHAR curDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, curDir);

        int i=0;

        pugi::xpath_node_set ns=settings.select_nodes("settings/searchFolders/folder");
        for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++,i++) {
            CStringW path=UTF8toUTF16(it->node().child_value());
            if(path==L"")
                break;
            LVITEM lvi;
            memset(&lvi, 0, sizeof(lvi));
            lvi.pszText=(LPWSTR)path.GetString();
            lvi.mask=LVFIF_TEXT;
            lvi.iItem=i;
            int iitem=ListView_InsertItem(hListView, &lvi);
            ListView_SetItemText(hListView, iitem, 1, L"3");
        }

        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=L"";
        lvi.mask=LVFIF_TEXT;
        lvi.iItem=i;
        int res=ListView_InsertItem(hListView, &lvi);

        //int ii=ListView_GetItemCount(hListView);
        return TRUE;
    }
    LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if(wParam==10000) {
            int iitem=ListView_GetSelectionMark(hListView);
            ListView_DeleteItem(hListView, iitem);

            SaveSearchFolders(hListView);
            return TRUE;
        }
        return FALSE;
    }
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if(((NMHDR*)lParam)->code==LVN_ENDLABELEDIT) {
            HWND hListView=((NMHDR*)lParam)->hwndFrom;

            int ii=ListView_GetItemCount(hListView);

            NMLVDISPINFO *nmdi=(NMLVDISPINFO*)lParam;
            INT i=ListView_SetItem(hListView, &nmdi->item);

            ListView_SetItemText(hListView, nmdi->item.iItem, 1, L"3");

            int itemcount=SaveSearchFolders(hListView);

            // if the edited item was the last, add an empty one
            if(nmdi->item.iItem==(itemcount-1)) {
                LVITEM lvi;
                memset(&lvi, 0, sizeof(lvi));
                lvi.pszText=L"";
                lvi.mask=LVFIF_TEXT;
                lvi.iItem=itemcount;
                int res=ListView_InsertItem(hListView, &lvi);
            }
            return TRUE;
        } else if(((NMHDR*)lParam)->code==NM_RCLICK) {
            HMENU hmenu=CreatePopupMenu();
            AppendMenu(hmenu, MF_STRING, 10000, L"Delete");
            DWORD pos=GetMessagePos();
            TrackPopupMenu(hmenu, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, m_hWnd, 0);
            return TRUE;
        }
        return FALSE;
    }
};
