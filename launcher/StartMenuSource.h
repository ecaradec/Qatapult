#pragma once
#include "FindFileRecursively.h"
#include "sqlite3/sqlite3.h"

inline CString GetSpecialFolder(int csidl) {
    CString tmp;
    SHGetFolderPath(0, csidl, 0, SHGFP_TYPE_CURRENT, tmp.GetBufferSetLength(MAX_PATH)); tmp.ReleaseBuffer();
    return tmp;
}
 static int getResultsCB(void *NotUsed, int argc, char **argv, char **azColName) {
     Info *pinfo=(Info*)NotUsed;
     pinfo->results->push_back(SourceResult(argv[0],         // key
                                            argv[1],         // display
                                            argv[2],         // expand
                                            pinfo->source,   // source
                                            0,               // id
                                            0,               // data
                                            (int)argv[3])); // bonus
    return 0;
}


static int getStringCB(void *NotUsed, int argc, char **argv, char **azColName) {
     *((CString*)NotUsed)=argv[0];
    return 0;
}

// might be useful
// http://stackoverflow.com/questions/1744902/how-might-i-obtain-the-icontextmenu-that-is-displayed-in-an-ishellview-context-m
struct StartMenuSource : Source {
    sqlite3 *db;
    StartMenuSource(HWND hwnd) : Source(L"FILE", L"STARTMENU"), m_hwnd(hwnd) {        
        m_ignoreemptyquery=true;

        int rc = sqlite3_open("startmenu.db", &db);
        
        char *zErrMsg = 0;

        sqlite3_exec(db, "CREATE TABLE startmenu(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, bonus INTEGER)", 0, 0, &zErrMsg);        
        sqlite3_exec(db, "CREATE TABLE startmenu_verbs(key TEXT PRIMARY KEY ASC, startmenu_key TEXT SECONDARY KEY, label TEXT, icon TEXT, id INTEGER, bonus INTEGER)", 0, 0, &zErrMsg);        
    }
    ~StartMenuSource() {
        //sqlite3_close(db);
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        // could probably be done in subclass as well as the callback since sourceresult will not change 
        Info info;
        info.results=&results;
        info.source=this;
        char *zErrMsg = 0;
        WCHAR buff[4096];
        wsprintf(buff, L"SELECT key, display, expand, bonus FROM startmenu WHERE display LIKE \"%%%s%%\";", query);
        sqlite3_exec(db, CStringA(buff), getResultsCB, &info, &zErrMsg);
    }
    void crawl(std::map<CString,SourceResult> *index) {
        std::vector<CString> lnks;
                
        FindFilesRecursively(GetSpecialFolder(CSIDL_COMMON_STARTMENU), L"*.lnk", lnks);
        FindFilesRecursively(GetSpecialFolder(CSIDL_STARTMENU), L"*.lnk", lnks);

                
        // all files from the desktop
        //FindFilesRecursively(GetSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY), L"*.*", lnks);
        //FindFilesRecursively(GetSpecialFolder(CSIDL_DESKTOPDIRECTORY), L"*.*", lnks);        
        CStringA q; // 82 bug
        q+="BEGIN;\n";
        WCHAR buff[0xFFFF];
        for(uint i=0;i<lnks.size();i++) {
            CString str(lnks[i]);
            PathRemoveExtension(str.GetBuffer()); str.ReleaseBuffer();
            str=PathFindFileName(str.GetBuffer()); str.ReleaseBuffer();            

            CString d=lnks[i].Left(lnks[i].ReverseFind(L'\\'));
            CString f=lnks[i].Mid(lnks[i].ReverseFind(L'\\')+1);

            X *x=new X;
            x->m_path=lnks[i];
            getItemVerbs(d, f, x->m_commands);
                        
            // this is enough for inserting once in the table, improve it later            
            wsprintf(buff, L"INSERT OR REPLACE INTO startmenu(key,display,expand,path) VALUES(\"%s\", \"%s\", \"%s\", \"%s\");\n", lnks[i], str, str, lnks[i]);
            q+=buff;
            for(int j=0;j<x->m_commands.size();j++) {
                wsprintf(buff, L"INSERT OR REPLACE INTO startmenu_verbs(key, startmenu_key, label, icon, id) VALUES(\"%d_%s\",    \"%s\",    \"%s\",                     \"%s\",                  %d);\n",
                                                                                                                    j, lnks[i], lnks[i], x->m_commands[j].display, x->m_commands[j].verb, x->m_commands[j].id);
                q+=buff;
            }
            CString progress;
            progress.Format(L"%d/%d\n", i, lnks.size());
            OutputDebugString(progress);
        }
        q+="END;";
             
        char *zErrMsg = 0;
        int z=sqlite3_exec(db, q, 0, 0, &zErrMsg);      
        if(z!=0) {
            DebugBreak();
        }
        q="";
//        OutputDebugStringA(q);        
    }
    // getvalue name, buff, bufflen
    Gdiplus::Bitmap *getIcon(SourceResult *r) {
        return ::getIcon(getString(r->key, L"path"));
    }
    virtual bool getSubResults(const TCHAR *itemkey, const TCHAR *name, std::vector<SourceResult> &results) {
        if(CString(name)==L"VERBS") {
            Info info;
            info.results=&results;
            info.source=this;
            char *zErrMsg = 0;
            WCHAR buff[4096];            
            wsprintf(buff, L"SELECT key, label, label, bonus FROM startmenu_verbs WHERE startmenu_key = \"%s\";", itemkey);
            sqlite3_exec(db, CStringA(buff), getResultsCB, &info, &zErrMsg);
            int test=0;
        }
        return false; 
    }
    virtual CString getString(const TCHAR *itemkey, const TCHAR *name) {
        WCHAR buff[4096];
        char *zErrMsg = 0;
        CString str;
        wsprintf(buff, L"SELECT %s FROM startmenu WHERE key = \"%s\";", name, itemkey);
        sqlite3_exec(db, CStringA(buff), getStringCB, &str, &zErrMsg);
        return str; 
    }
    virtual int getInt(const TCHAR *itemkey, const TCHAR *name) { 
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