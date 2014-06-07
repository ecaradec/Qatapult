#pragma once
#include "FindFileRecursively.h"
#include "ShellLink.h"
#include "Source.h"
#include "sqlite3/sqlite3.h"
#include "Utility.h"
#include "resource.h"
#include "FileObject.h"

struct xml_string_writer: pugi::xml_writer
{
    std::string result;

    virtual void write(const void* data, size_t size)
    {
        result += std::string(static_cast<const char*>(data), size);
    }
};


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
    virtual void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"FILE")==activetypes.end())
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
            pack.begin(KV_Map);
                pack.writePairString(L"type",L"FILE");
                pack.writePairUint32(L"source",(uint32)this);
                pack.writePairString(L"key",UTF8toUTF16((char*)sqlite3_column_text(stmt,0)));
                pack.writePairString(L"text",UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));
                pack.writePairString(L"expand",UTF8toUTF16((char*)sqlite3_column_text(stmt,2)));
                pack.writePairString(L"path",UTF8toUTF16((char*)sqlite3_column_text(stmt,3)));
                pack.writePairUint32(L"uses",sqlite3_column_int(stmt,4));
                pack.writePairUint32(L"bonus",(uint32)0);
            pack.end();
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
            CString folder=UTF8toUTF16(it->node().child("folder").child_value("path"));
            if(folder==L"")
                continue;
            lnks.push_back(folder);
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
    void validate(Object *o) {
        WCHAR buff[4096];
        char *zErrMsg = 0;
        wsprintf(buff, L"UPDATE startmenu SET uses = uses + 1 WHERE key=\"%s\"\n", o->key);
        int z=sqlite3_exec(db, CStringA(buff), 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    virtual int getInt(const TCHAR *itemquery) {
        return false; 
    }
    void rate(const CString &q, Object *r) {
        Source::rate(q,r);
        CString P(r->getString(L"path"));
        if(P.Right(4)==L".lnk")
            r->m_rank+=25;
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
DWORD attr=0;,
pSF->ParseDisplayName(0, 0, L"::{26EE0668-A00A-44D7-9371-BEB064C98683}", &eaten, &pidl, &attr);
CComPtr<IShellFolder> pCPSF;
//pSF->GetUIObjectOf(0, 1, (LPCITEMIDLIST*)&pidl, IID_IShellFolder 0, (void**)&pCPSF);
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

//#include "SimpleOptDialog.h"
//#include "Qatapult.h"

#include "Layout.h"
#define WM_SAVESETTINGS (WM_USER+10)
class SearchFoldersDlg : public CDialogImpl<SearchFoldersDlg>
{
public:
    enum { IDD = IDD_EMPTY };

    enum {
        ID_PATH,
        ID_PICKFOLDER,
        ID_FOLDERLIST,
        ID_NEW,
        ID_SAVE,
        ID_DEL
    };
 
    CStatic         folderLbl; 
    CButton         newBtn, delBtn, saveBtn;
    CStatic         pathLbl;
    CEdit           pathEdit;
    CButton         pickFolderBtn;
    CListViewCtrl   foldersLVC;

    BEGIN_MSG_MAP(SearchFoldersDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SAVESETTINGS, OnSaveSettings)
        NOTIFY_HANDLER(ID_FOLDERLIST, LVN_ITEMCHANGING, OnRuleSelecting)
        NOTIFY_HANDLER(ID_FOLDERLIST, LVN_ITEMCHANGED, OnRuleSelected)
        
        COMMAND_HANDLER(ID_NEW, BN_CLICKED, OnNew)
        COMMAND_HANDLER(ID_DEL, BN_CLICKED, OnDel)
        COMMAND_HANDLER(ID_SAVE, BN_CLICKED, OnSave)
        COMMAND_HANDLER(ID_PICKFOLDER, BN_CLICKED, OnPickFolder)
        
        //MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        //CHAIN_MSG_MAP(SimpleOptDialog)      
    END_MSG_MAP()

    void OnFinalMessage(HWND) {
        folderLbl.Detach(); 
        newBtn.Detach(); 
        delBtn.Detach();
        saveBtn.Detach();
        pathLbl.Detach();
        pathEdit.Detach();
        pickFolderBtn.Detach();
        foldersLVC.Detach();
    }

    LRESULT OnNew(SHORT id, SHORT code, HWND lParam, BOOL& bHandled) {
        saveEdition(); // save current        

        CFolderDialog dlg(*this);
        dlg.DoModal();
        
        // save to xml
        addItem(dlg.m_szFolderPath);

        // show in view
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=L"";
        lvi.mask=LVFIF_TEXT|LVIF_NORECOMPUTE;
        lvi.iItem=foldersLVC.GetItemCount();
        int i=foldersLVC.InsertItem(&lvi);

        pathEdit.SetWindowTextW(dlg.m_szFolderPath);
        foldersLVC.SetItemText(i, 0, dlg.m_szFolderPath);
        foldersLVC.SetItemText(i, 1, L"3");
        foldersLVC.SetItemText(i, 2, L"0");

        foldersLVC.SelectItem( i );
        foldersLVC.SetFocus();

        enableEdition(true);
        
        return S_OK;
    }

    LRESULT OnDel(SHORT id, SHORT code, HWND lParam, BOOL& bHandled) {
        pugi::xpath_node_set ns=settings.select_nodes("/settings/searchFolders/folder");
        int sel=foldersLVC.GetSelectedIndex();
        foldersLVC.DeleteItem(sel);
        if(sel==-1)
            return S_OK;
        settings.select_single_node("/settings/searchFolders").node().remove_child(ns[sel].node());
        
        if(foldersLVC.GetItemCount()>0) {
            if(sel-1>=0)
                sel--;
            foldersLVC.SelectItem( sel );
            foldersLVC.SetFocus();
        } else {
            //clearEdition();
            enableEdition(false);
        }

        return S_OK;
    }

    LRESULT OnSave(SHORT id, SHORT code, HWND lParam, BOOL& bHandled) {
        saveEdition(); 
        return S_OK;
    }

    LRESULT OnPickFolder(SHORT id, SHORT code, HWND lParam, BOOL& bHandled) {
        CFolderDialog dlg(*this);
        dlg.DoModal();
        pathEdit.SetWindowTextW(dlg.m_szFolderPath);
        return S_OK;
    }

    void saveEdition() {
        int sel=foldersLVC.GetSelectedIndex();
        if(sel==-1)
            return;

        CString path;
        pathEdit.GetWindowTextW(path);

        foldersLVC.SetItemText(sel, 0, path);
        foldersLVC.SetItemText(sel, 1, L"3");
        foldersLVC.SetItemText(sel, 2, L"0");

        saveItem(sel,path);
    }

    void addItem(const CString &path) {
        // ensure the searchfolders node exists
        if(settings.select_single_node("/settings/searchFolders").node().empty()) {
            settings.select_single_node("/settings").node().append_child("searchFolders");
        }
        
        pugi::xml_node r = settings.select_single_node("/settings/searchFolders").node().append_child("folder");
        r.append_child("path").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(path));
        r.append_child("depth").append_child(pugi::node_pcdata).set_value("3");
        r.append_child("bonus").append_child(pugi::node_pcdata).set_value("0");
    }

    void saveItem(int sel, const CString &path) {
        // ensure the searchfolders node exists
        if(settings.select_single_node("/settings/searchFolders").node().empty()) {
            settings.select_single_node("/settings").node().append_child("searchFolders");
        }

        // find the selected node and empty it
        pugi::xpath_node_set ns=settings.select_nodes("/settings/searchFolders/folder");
        pugi::xml_node r=ns[sel].node();
        while(r.first_child()) {
            r.remove_child(r.first_child());
        }

        r.append_child("path").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(path));
        r.append_child("depth").append_child(pugi::node_pcdata).set_value("3");
        r.append_child("bonus").append_child(pugi::node_pcdata).set_value("0");
    }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    { 
        Layout l(*this);

        int x=0;
        int y=0;

        folderLbl.Create(*this, l.getSpace(80,25), L"Folders : ", WS_CHILD|WS_VISIBLE);

        newBtn.Create(*this, l.getSpace(100,25), L"New", WS_CHILD|WS_VISIBLE, 0, ID_NEW);
        delBtn.Create(*this, l.getSpace(100,25), L"Delete", WS_CHILD|WS_VISIBLE, 0, ID_DEL);
        //saveBtn.Create(*this, l.getSpace(100,25), L"Save", WS_CHILD|WS_VISIBLE, 0, ID_SAVE);
        
        l.clearRow();
        l.pos.x+=85;
        int uiWidth=l.r.right-l.pos.x;
        foldersLVC.Create(*this, l.getSpace(uiWidth, 275), L"", WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,WS_EX_CLIENTEDGE,ID_FOLDERLIST);        
        foldersLVC.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
        foldersLVC.AddColumn(L"Path",1);
        foldersLVC.SetColumnWidth(0, uiWidth*0.79);
        foldersLVC.AddColumn(L"Depth",1);
        foldersLVC.SetColumnWidth(1, uiWidth*0.09);
        foldersLVC.AddColumn(L"Bonus",1);
        foldersLVC.SetColumnWidth(1, uiWidth*0.09);
        

        pugi::xpath_node_set ns=settings.select_nodes("settings/searchFolders/folder");
        for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++) {
            LVITEM lvi;
            memset(&lvi, 0, sizeof(lvi));
            lvi.pszText=L"";
            lvi.mask=LVFIF_TEXT|LVIF_NORECOMPUTE;
            lvi.iItem=foldersLVC.GetItemCount();
            int r=foldersLVC.InsertItem(&lvi);

            foldersLVC.SetItemText(r, 0, UTF8toUTF16(it->node().child_value("path")));
            foldersLVC.SetItemText(r, 1, UTF8toUTF16(it->node().child_value("depth")));
            foldersLVC.SetItemText(r, 2, UTF8toUTF16(it->node().child_value("bonus")));
        }

        l.clearRow();

        pathLbl.Create(*this, l.getSpace(80,25), L"Path : ", WS_CHILD|WS_VISIBLE);
        pathEdit.Create(*this, l.getSpace(uiWidth-28,25), L"", WS_CHILD|WS_VISIBLE, WS_EX_CLIENTEDGE, ID_PATH);        

        pickFolderBtn.Create(*this, l.getSpace(25,25), L"...", WS_CHILD|WS_VISIBLE, 0, ID_PICKFOLDER);

        l.clearRow();

        enableEdition(false);
        
        SetStdFontOfDescendants(*this);
        return TRUE;
    }


    LRESULT OnRuleSelecting(int wParam, LPNMHDR lParam, BOOL& bHandled) {
        //saveEdition();
        return S_OK;
    }

    LRESULT OnRuleSelected(int wParam, LPNMHDR lParam, BOOL& bHandled) {

        LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

        if((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_SELECTED) != (pnmv->uOldState & LVIS_SELECTED))
        {
            if (pnmv->uNewState & LVIS_SELECTED) {
                pugi::xpath_node_set ns=settings.select_nodes("/settings/searchFolders/folder");                
                if(pnmv->iItem>=ns.size())
                    return S_OK;
                
                CString path=UTF8toUTF16(ns[pnmv->iItem].node().child_value("path"));
                pathEdit.SetWindowTextW(path);
                enableEdition(true);
            } else {
                pathEdit.SetWindowTextW(L"");
                //clearEdition();
                enableEdition(false);
            }
        }

        Invalidate(TRUE);
        return S_OK;
    }
    
    void enableEdition(bool b) {
        delBtn.EnableWindow(b);
        pathEdit.EnableWindow(b);
        pickFolderBtn.EnableWindow(b);
    }
    
    LRESULT OnSaveSettings(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        saveEdition();
        return S_OK;
    }
};
