#pragma once
#include "FindFileRecursively.h"
#include "sqlite3/sqlite3.h"

inline CString GetSpecialFolder(int csidl) {
    CString tmp;
    SHGetFolderPath(0, csidl, 0, SHGFP_TYPE_CURRENT, tmp.GetBufferSetLength(MAX_PATH)); tmp.ReleaseBuffer();
    return tmp;
}

struct Info {
    Source                    *source;
    std::vector<SourceResult> *results;
};

 static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
     Info *pinfo=(Info*)NotUsed;
     pinfo->results->push_back(SourceResult(argv[0],  //key
                                            argv[2],        //display
                                            argv[3],        //expand
                                            pinfo->source,  //source
                                            atoi(argv[4]),  // bonus
                                            0,
                                            atoi(argv[5]))); // id
    return 0;
}
  

// might be useful
// http://stackoverflow.com/questions/1744902/how-might-i-obtain-the-icontextmenu-that-is-displayed-in-an-ishellview-context-m
struct StartMenuSource : Source {
    sqlite3 *db;
    StartMenuSource(HWND hwnd) : Source(L"FILE", L"STARTMENU"), m_hwnd(hwnd) {        
        m_ignoreemptyquery=true;


        int rc = sqlite3_open("startmenu.db", &db);
        /*
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE startmenu(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, bonus INTEGER, id INTEGER)", callback, 0, &zErrMsg);        
        */
        // this would be an easy way to collect result, just select and push to result // ?
        //std::vector<SourceResult> results;
        //


        //crawl(&m_index);
        load();
    }
    ~StartMenuSource() {
        //sqlite3_close(db);
    }

    struct X {
        std::vector<Command> m_commands;
        CString              m_path;
    };

    virtual bool loadItem(int i) {
        if(!Source::loadItem(i))
            return false;
        CString key;
        GetPrivateProfileString(m_name, ItoS(i)+L"_key", L"", key.GetBufferSetLength(4096), 4096, m_db); key.ReleaseBuffer();
        X *x=new X;
        
        GetPrivateProfileString(m_name, ItoS(i)+L"_path", L"", x->m_path.GetBufferSetLength(4096), 4096, m_db); x->m_path.ReleaseBuffer();

        m_index[key].data=x;
        for(int c=0;; c++) {
            CString nb; nb.Format(L"%d_%d",i,c);
            
            Command cmd;
            GetPrivateProfileString(m_name, nb+L"_verbName", L"", cmd.display.GetBufferSetLength(256), 256, m_db); cmd.display.ReleaseBuffer();            
            if(cmd.display==L"")
                break;
            GetPrivateProfileString(m_name, nb+L"_verbCan", L"", cmd.verb.GetBufferSetLength(256), 256, m_db); cmd.verb.ReleaseBuffer();
            cmd.id=GetPrivateProfileInt(m_name, nb+L"_verbId", 0, m_db);
            x->m_commands.push_back(cmd);
        }
        return true;
    }
    bool saveItem(int i, SourceResult *r) {
        if(!Source::saveItem(i,r))
            return false;        

        X *x=(X*)m_index[r->key].data;
        
        CString nb; nb.Format(L"%d",i);
        WritePrivateProfileString(m_name,nb+L"_path",x->m_path, m_db);

        for(int c=0;c<x->m_commands.size(); c++) {
            CString nb; nb.Format(L"%d_%d",i,c);
            WritePrivateProfileString(m_name,nb+L"_verbName",x->m_commands[c].display, m_db);
            WritePrivateProfileString(m_name,nb+L"_verbCan",x->m_commands[c].verb, m_db);
            WritePrivateProfileString(m_name,nb+L"_verbId",ItoS(x->m_commands[c].id), m_db);
        }
        return true;
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        // could probably be done in subclass as well as the callback since sourceresult will not change 
        /*Info info;
        info.results=&results;
        info.source=this;
        char *zErrMsg = 0;
        sqlite3_exec(db, "SELECT * FROM startmenu", callback, &info, &zErrMsg);*/

        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(CString(it->second.display).MakeUpper().Find(q)!=-1) {
                results.push_back(it->second);
            }
        }
    }
    void crawl(std::map<CString,SourceResult> *index) {
        std::vector<CString> lnks;
                
        FindFilesRecursively(GetSpecialFolder(CSIDL_COMMON_STARTMENU), L"*.lnk", lnks);
        FindFilesRecursively(GetSpecialFolder(CSIDL_STARTMENU), L"*.lnk", lnks);

                
        // all files from the desktop
        //FindFilesRecursively(GetSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY), L"*.*", lnks);
        //FindFilesRecursively(GetSpecialFolder(CSIDL_DESKTOPDIRECTORY), L"*.*", lnks);        
        CStringA q;
        for(uint i=0;i<lnks.size();i++) {
            CString str(lnks[i]);
            PathRemoveExtension(str.GetBuffer()); str.ReleaseBuffer();
            str=PathFindFileName(str.GetBuffer()); str.ReleaseBuffer();            

            CString d=lnks[i].Left(lnks[i].ReverseFind(L'\\'));
            CString f=lnks[i].Mid(lnks[i].ReverseFind(L'\\')+1);

            X *x=new X;
            x->m_path=lnks[i];
            getItemVerbs(d, f, x->m_commands);
                        
            (*index)[lnks[i]] = SourceResult(lnks[i], str, str, this, i, x, 0);

            // this is enough for inserting once in the table, improve it later
            /*WCHAR buff[4096];
            wsprintf(buff, L"INSERT INTO startmenu VALUES ('%s', '%s', '%s', '%s', %d, %d);", lnks[i], m_name, str, str, 0, i);
            q+=buff;

            CString progress;
            progress.Format(L"%d/%d\n", i, lnks.size());
            OutputDebugString(progress);*/
        }

        //char *zErrMsg = 0;
        //int z=sqlite3_exec(db, q, callback, 0, &zErrMsg);                
    }
    // getvalue name, buff, bufflen
    Gdiplus::Bitmap *getIcon(SourceResult *r) {
        CString path;
        (*m_pArgs)[0].source->getData(r->key, L"PATH", (char*)path.GetBufferSetLength(MAX_PATH), MAX_PATH); path.ReleaseBuffer();

        return ::getIcon(path);
    }
    virtual bool getData(const TCHAR *itemkey, const TCHAR *name, char *buff, int len) {

        /*Info info;
        info.results=&results;
        info.source=this;
        char *zErrMsg = 0;
        sqlite3_exec(db, "SELECT * FROM startmenu_verbs", callback, &info, &zErrMsg);*/


        X *x=(X*)m_index[itemkey].data;
        if(CString(name)==L"VERBS") {
             *((std::vector<Command>**)buff)=&((X*)m_index[itemkey].data)->m_commands;
        }else if(CString(name)==L"PATH") {
            wcscpy((TCHAR*)buff, ((X*)m_index[itemkey].data)->m_path.GetString());
        }
        return false; 
    }
    virtual bool getSubResults(const TCHAR *itemkey, std::vector<SourceResult> &results) {        
        return false; 
    }
    virtual bool getString(const TCHAR *itemkey, CString &str) { 
        return false; 
    }
    virtual bool getInt(const TCHAR *itemkey, int &i) { 
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