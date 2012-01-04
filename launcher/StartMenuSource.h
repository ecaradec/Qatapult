#pragma once
#include "FindFileRecursively.h"
#include "sqlite3/sqlite3.h"
#include "md5.h"

inline CString GetSpecialFolder(int csidl) {
    CString tmp;
    SHGetFolderPath(0, csidl, 0, SHGFP_TYPE_CURRENT, tmp.GetBufferSetLength(MAX_PATH)); tmp.ReleaseBuffer();
    return tmp;
}


CStringW UTF8toUTF16(const CStringA& utf8)
{
    CStringW utf16;
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    if (len>1)
    { 
        wchar_t *ptr = utf16.GetBuffer(len-1);
        if (ptr) MultiByteToWideChar(CP_UTF8, 0, utf8, -1, ptr, len);
        utf16.ReleaseBuffer();
    }
    return utf16;
}
CStringA UTF16toUTF8(const CStringW& utf16)
{
    CStringA utf8;
    int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
    if (len>1)
    { 
        char *ptr = utf8.GetBuffer(len-1);
        if (ptr) WideCharToMultiByte(CP_UTF8, 0, utf16, -1, ptr, len, 0, 0);
        utf8.ReleaseBuffer();
    }
    return utf8;
}

 static int getResultsCB(void *NotUsed, int argc, char **argv, char **azColName) {
     Info *pinfo=(Info*)NotUsed;
     pinfo->results->push_back(SourceResult(UTF8toUTF16(argv[0]),         // key
                                            UTF8toUTF16(argv[1]),         // display
                                            UTF8toUTF16(argv[2]),          // expand
                                            pinfo->source,   // source
                                            atoi(argv[3]?argv[3]:"0"),               // id
                                            0,               // data
                                            atoi(argv[4]?argv[4]:"0"))); // bonus
    return 0;
}


static int getStringCB(void *NotUsed, int argc, char **argv, char **azColName) {
     *((CString*)NotUsed)=argv[0];
     //*((CString*)NotUsed)=UTF8toUTF16(argv[0]);
    return 0;
}

static int getIntCB(void *NotUsed, int argc, char **argv, char **azColName) {
     *((int*)NotUsed)=atoi(argv[0]?argv[0]:"0");
    return 0;
}

int SaveSearchFolders(HWND hListView) {
    WCHAR curDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, curDir);

    int itemcount=ListView_GetItemCount(hListView);
    for(int i=0;i<itemcount;i++) {
        TCHAR path[MAX_PATH]={0};
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=path;
        lvi.cchTextMax=sizeof(path);
        lvi.iItem=i;
        lvi.mask=LVFIF_TEXT;
        ListView_GetItem(hListView, &lvi);

        WritePrivateProfileString(L"SearchFolders", ItoS(i), path, CString(curDir)+L"\\settings.ini");
    }
    return itemcount;
}

BOOL CALLBACK SearchFolderDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int res;
    HWND hListView=GetDlgItem(hWnd, IDC_LIST3);
    CRect r;
    switch(msg)
    {
        case WM_INITDIALOG:
            // allows for full row selection
            ListView_SetExtendedListViewStyle(hListView, ListView_GetExtendedListViewStyle(hListView)| LVS_EX_FULLROWSELECT);

            GetClientRect(hListView, &r);

            WCHAR szText[256];     // Temporary buffer.
            LVCOLUMN lvc;
            int iCol;
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

            int i;
            for(i=0;;i++) {
                TCHAR path[MAX_PATH];
                GetPrivateProfileString(L"SearchFolders", ItoS(i), L"", path, sizeof(path), CString(curDir)+L"\\settings.ini");
                if(_tcslen(path)==0)
                    break;
                LVITEM lvi;
                memset(&lvi, 0, sizeof(lvi));
                lvi.pszText=path;
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
            res=ListView_InsertItem(hListView, &lvi);

        return TRUE;
        case WM_COMMAND:
            if(wParam==10000) {
                int iitem=ListView_GetSelectionMark(hListView);
                ListView_DeleteItem(hListView, iitem);

                SaveSearchFolders(hListView);
            }
        return TRUE;        
        case WM_NOTIFY:
            if(((NMHDR*)lParam)->code==LVN_ENDLABELEDIT) {

                NMLVDISPINFO *nmdi=(NMLVDISPINFO*)lParam;                
                ListView_SetItem(hListView, &nmdi->item);

                int itemcount=SaveSearchFolders(hListView);

                // if the edited item was the last, add an empty one
                if(nmdi->item.iItem==(itemcount-1)) {
                    LVITEM lvi;
                    memset(&lvi, 0, sizeof(lvi));
                    lvi.pszText=L"";
                    lvi.mask=LVFIF_TEXT;
                    lvi.iItem=i;
                    res=ListView_InsertItem(hListView, &lvi);
                }
            } else if(((NMHDR*)lParam)->code==NM_RCLICK) {
                HMENU hmenu=CreatePopupMenu();
                AppendMenu(hmenu, MF_STRING, 10000, L"Delete");
                DWORD pos=GetMessagePos();
                TrackPopupMenu(hmenu, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, hWnd, 0);
            }
            return TRUE;
    }
    return FALSE;
}

int uselev;

// might be useful
// http://stackoverflow.com/questions/1744902/how-might-i-obtain-the-icontextmenu-that-is-displayed-in-an-ishellview-context-m
struct StartMenuSource : Source {
    sqlite3 *db;
    StartMenuSource(HWND hwnd) : Source(L"FILE", L"STARTMENU"), m_hwnd(hwnd) {        
        m_ignoreemptyquery=true;
        
        int rc = sqlite3_open("startmenu.db", &db);
        
        char *zErrMsg = 0;

        sqlite3_exec(db, "CREATE TABLE startmenu(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);        
        sqlite3_exec(db, "CREATE TABLE startmenu_verbs(key TEXT PRIMARY KEY ASC, startmenu_key TEXT KEY, label TEXT, icon TEXT, id INTEGER, bonus INTEGER)", 0, 0, &zErrMsg);        
        sqlite3_exec(db, "CREATE INDEX startmenu_verbs_index ON startmenu_verbs(startmenu_key)", 0, 0, &zErrMsg);
    }
    ~StartMenuSource() {
        sqlite3_close(db);
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        // could probably be done in subclass as well as the callback since sourceresult will not change 
        Info info;
        info.results=&results;
        info.source=this;
        char *zErrMsg = 0;
        WCHAR buff[4096];
        if(uselev)
            wsprintf(buff, L"SELECT key, display, expand, 0, 0 FROM startmenu;", query);
        else
            wsprintf(buff, L"SELECT key, display, expand, 0, bonus FROM startmenu WHERE display LIKE \"%%%s%%\";", query);
        
        /*sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;        
        rc = sqlite3_prepare_v2(db,
                                "SELECT key, display, expand, 0, bonus FROM startmenu WHERE display LIKE ?;\n",
                                -1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 5, query, -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);*/

        sqlite3_exec(db, CStringA(buff), getResultsCB, &info, &zErrMsg);
    }
    void crawl() {
        std::vector<CString> lnks;

        FindFilesRecursively(GetSpecialFolder(CSIDL_COMMON_STARTMENU), L"*.lnk", lnks, 999);
        FindFilesRecursively(GetSpecialFolder(CSIDL_STARTMENU), L"*.lnk", lnks, 999);

        WCHAR curDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, curDir);
        int i;
        for(i=0;;i++) {
            TCHAR path[MAX_PATH];
            GetPrivateProfileString(L"SearchFolders", ItoS(i), L"", path, sizeof(path), CString(curDir)+L"\\settings.ini");
            if(_tcslen(path)==0)
                break;

            FindFilesRecursively(path, L"*.*", lnks, 3);
        }

        CStringA q; // 82 bug
        q+="BEGIN;\n";
        WCHAR buff[0xFFFF];
        char *zErrMsg=0;

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        // there can only exists one single mark
        int mark;
        sqlite3_exec(db, "SELECT mark FROM startmenu LIMIT 1;", getIntCB, &mark, &zErrMsg);

        rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);

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
                                    "INSERT OR REPLACE INTO startmenu(key,display,expand,path,bonus,mark) VALUES(?, ?, ?, ?, coalesce((SELECT bonus FROM startmenu WHERE key=?), 0), ?);\n",
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

            //CString progress;
            //progress.Format(L"%d/%d\n", i, lnks.size());
            //OutputDebugString(progress);
        }

        rc = sqlite3_exec(db, "END;", 0, 0, &zErrMsg);

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
        wsprintf(buff, L"UPDATE startmenu SET bonus = MIN(bonus + 5,40) WHERE key=\"%s\"\n", r->key);        
        int z=sqlite3_exec(db, CStringA(buff), 0, 0, &zErrMsg);
    }

    // getvalue name, buff, bufflen
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return ::getIcon(getString(r->key+L"/path"),flags);
    }
    // may be I should just have threaded the results ???
    virtual bool getSubResults(const TCHAR *query, const TCHAR *itemquery, std::vector<SourceResult> &results) {
        CString q(itemquery);
        CString key=q.Left(q.ReverseFind('/'));
        CString val=q.Mid(q.ReverseFind('/')+1);

        Info info;
        info.results=&results;
        info.source=this;
        char *zErrMsg = 0;
        WCHAR buff[4096];            
        wsprintf(buff, L"SELECT key, label, label, id, bonus FROM startmenu_verbs WHERE startmenu_key = \"%s\" AND label LIKE \"%%%s%%\";", key, query);
        sqlite3_exec(db, CStringA(buff), getResultsCB, &info, &zErrMsg);

        return true;
    }
    // itemkey/name : itemkey/verb/open/icon < get subresults ???
    virtual CString getString(const TCHAR *itemquery) {
        CString str;

        CString q(itemquery);
        CString key=q.Left(q.ReverseFind('/'));
        CString val=q.Mid(q.ReverseFind('/')+1);
        
        if(key.Find(L"/verb/")!=-1) {            
            WCHAR buff[4096];
            char *zErrMsg = 0;
            wsprintf(buff, L"SELECT %s FROM startmenu_verbs WHERE key = \"%s\";", val, key);
            sqlite3_exec(db, CStringA(buff), getStringCB, &str, &zErrMsg);
        } else {
            WCHAR buff[4096];
            char *zErrMsg = 0;
            wsprintf(buff, L"SELECT %s FROM startmenu WHERE key = \"%s\";", val, key);
            sqlite3_exec(db, CStringA(buff), getStringCB, &str, &zErrMsg);
        }

        return str; 
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