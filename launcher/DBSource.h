#pragma once
#include "Source.h"
#include "Utility.h"

struct DBSource : Source {
    sqlite3 *db;
    CStringA  m_dbname;
    CString   m_icon;    
    DBSource(const CString &type,const CString &name, const CString &dbname) : Source(type,name), m_dbname(dbname) {
        int rc = sqlite3_open(L"databases\\"+m_dbname+".db", &db);        
    }    
    ~DBSource() {        
        sqlite3_close(db);
    }
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) { 
        return r->object->getIcon(flags);
        //return Gdiplus::Bitmap::FromFile(L"icons\\"+r->source->getString(*r,L"icon")+".png");
    }
    void validate(SourceResult *r) {
        WCHAR buff[4096];
        char *zErrMsg = 0;
        wsprintf(buff, L"UPDATE %s SET uses = uses+1 WHERE key=\"%s\"\n", CStringW(m_dbname).GetString(), r->object->key);        
        int z=sqlite3_exec(db, CStringA(buff), 0, 0, &zErrMsg);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query);
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        rc = sqlite3_prepare_v2(db,L"SELECT key, display, expand, path, icon, uses FROM "+m_dbname+L" WHERE display LIKE ?;",-1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
        int i=0;
        while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
            results.push_back(SourceResult(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),     // key
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),    // display
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),    // expand
                                            this,                                               // source
                                            0,                                                  // id
                                            0,                                                  // data
                                            sqlite3_column_int(stmt,4)));                       // uses

            results.back().object=new FileObject(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),
                                                 this,
                                                 UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),
                                                 UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),
                                                 UTF8toUTF16((char*)sqlite3_column_text(stmt,3)));
        }

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(stmt);         
    }
    CString getString(SourceResult &sr,const TCHAR *val) {
        return sr.object->getString(val);
    }
};
