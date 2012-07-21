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
    void validate(SourceResult *r) {
        WCHAR buff[4096];
        char *zErrMsg = 0;
        wsprintf(buff, L"UPDATE %s SET uses = uses+1 WHERE key=\"%s\"\n", CStringW(m_dbname).GetString(), r->object()->key);        
        int z=sqlite3_exec(db, CStringA(buff), 0, 0, &zErrMsg);
    }
};
