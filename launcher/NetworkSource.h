#pragma once
#include "Source.h"
#include "Utility.h"

BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr, std::vector<CString> &lnks);

struct NetworkSource : Source {
    NetworkSource() : Source(L"FILE",L"Network (Catalog )") {
        m_icon=L"icons\\networklocal.png";
        m_ignoreemptyquery=true;
        int rc = sqlite3_open("databases\\network.db", &db);
        
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE files(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    ~NetworkSource() {
        sqlite3_close(db);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int flags, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        // could probably be done in subclass as well as the callback since sourceresult will not change 
        CString q(query);

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        rc = sqlite3_prepare_v2(db,"SELECT key, display, expand, path, bonus FROM files WHERE display LIKE ?;",-1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
        int i=0;
        while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
            results.push_back(SourceResult(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),        // key
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),        // display
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),        // expand
                                            this,                         // source
                                            sqlite3_column_int(stmt,3),   // id
                                            0,                            // data
                                            sqlite3_column_int(stmt,4))); // bonus
            
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
        EnumerateFunc(0, lnks);

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;
        char *zErrMsg=0;

        // there can only exists one single mark
        int mark;
        sqlite3_exec(db, "SELECT mark FROM files LIMIT 1;", getIntCB, &mark, &zErrMsg);
        sqlite3_free(zErrMsg);

        rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        for(uint i=0;i<lnks.size();i++) {
            CString str(lnks[i]);

            CString startmenu_key=md5(lnks[i]);
            
            rc = sqlite3_prepare_v2(db,
                                    "INSERT OR REPLACE INTO files(key,display,expand,path,bonus,mark) VALUES(?, ?, ?, ?, coalesce((SELECT bonus FROM files WHERE key=?), 0), ?);\n",
                                    -1, &stmt, &unused);
            rc = sqlite3_bind_text16(stmt, 1, startmenu_key.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 2, str.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 3, str+L"\\", -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 4, str.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 5, startmenu_key.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_int(stmt, 6, mark+1);
            rc = sqlite3_step(stmt);
            const char *errmsg=sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
        }

        rc = sqlite3_exec(db, "END;", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return r->object->getIcon(flags);
    }
    CString getString(SourceResult &sr,const TCHAR *val_) {
        return sr.object->getString(val_);
    }
    Source *getSource(SourceResult &r, CString &query) {
        query=r.display+L"\\";
        return (Source*)-1;
    }
    sqlite3 *db;
};

#pragma comment(lib, "mpr.lib")

#include <winnetwk.h>

void DisplayStruct(int i, LPNETRESOURCE lpnrLocal);

