#pragma once
#include "utf8.h"

struct Record {
    Record() {
        ivalues["key"]=-1;
    }
    std::map<CString, CString>       values;
    std::map<CString, sqlite3_int64> ivalues;
};

struct DB {
    DB(char *tbname, char* strct):m_name(tbname), m_struct(strct) {
    }

    void save(Record &r) {
        CStringA str;
        CStringA args;
        
        // there is always a least one item, not need to test
        str+=r.ivalues.begin()->first;
        args+="?";
        std::map<CString,sqlite3_int64>::iterator first=r.ivalues.begin(); first++;
        for(std::map<CString,sqlite3_int64>::iterator it=first; it!=r.ivalues.end(); it++) {            
            str+=","+it->first;
            args+=",?";
        }
        for(std::map<CString,CString>::iterator it=r.values.begin(); it!=r.values.end(); it++) {
            str+=","+it->first;
            args+=",?";
        }

        //OutputDebugStringA("REPLACE INTO websites "+str+" VALUES("+args+");");
        static sqlite3_stmt *req;
        int rc = sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO "+CStringA(m_name)+" ("+str+") VALUES("+args+");",-1, &req, &unused);                

        int j=1;
        for(std::map<CString,sqlite3_int64>::iterator it=r.ivalues.begin(); it!=r.ivalues.end(); it++) {
            if(it->first==L"key") {
                if(it->second==-1) {
                    rc = sqlite3_bind_null(req, 1);
                    continue;
                }
            }

            rc = sqlite3_bind_int64(req, j, it->second); j++;
        }    
        for(std::map<CString,CString>::iterator it=r.values.begin(); it!=r.values.end(); it++) {
            rc = sqlite3_bind_text16(req, j, it->second, -1, SQLITE_STATIC); j++;
        }    

        int i=0;
        rc=sqlite3_step(req);

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(req);

        if(r.ivalues[L"key"]==-1) {
            int rc = sqlite3_prepare_v2(db,"SELECT last_insert_rowid();",-1, &req, &unused);
            rc=sqlite3_step(req);
            r.ivalues[L"key"]=sqlite3_column_int64(req,0);
            sqlite3_finalize(req);            
        }
    }
    bool del(Record &r) {
        static sqlite3_stmt *req;
        int rc = sqlite3_prepare_v2(db,"DELETE FROM "+CStringA(m_name)+" WHERE key=?",-1, &req, &unused);
        rc = sqlite3_bind_int64(req, 1, r.ivalues[L"key"]);
        rc=sqlite3_step(req);
        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(req);
        return rc==SQLITE_DONE;
    }

    void initialize() {
        int rc = sqlite3_open("databases\\"+CStringA(m_name)+".db", &db);

        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE "+CStringA(m_name)+"("+CStringA(m_struct)+")", 0, 0, &zErrMsg);        
        sqlite3_free(zErrMsg);
    }

    // could as well be a macro
    int query(std::vector<Record> &records) {
        static bool initialized=false;
        if(!initialized) {
            initialize();
            initialized=true;
        }
        
        sqlite3_stmt *req;
        int rc = sqlite3_prepare_v2(db,"SELECT * FROM "+CStringA(m_name)+";",-1, &req, &unused);
        
        fetchRecords(req,records);

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(req);
        return records.size();
    }

    int findBy(std::vector<Record> &records, const char *name, const char *value) {
        static bool initialized=false;
        if(!initialized) {
            initialize();
            initialized=true;
        }
    
        sqlite3_stmt *req;
        OutputDebugStringA("SELECT * FROM "+CStringA(m_name)+" WHERE "+CStringA(name)+" LIKE ?;\n");
        int rc = sqlite3_prepare_v2(db,"SELECT * FROM "+CStringA(m_name)+" WHERE "+CStringA(name)+" LIKE ?;",-1, &req, &unused);
        sqlite3_bind_text(req, 1, value, -1, SQLITE_STATIC);

        fetchRecords(req,records);

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(req);
        return records.size();
    }

    void fetchRecords(sqlite3_stmt *req, std::vector<Record> &records) {
        int i=0;
        while((rc=sqlite3_step(req))==SQLITE_ROW) {

            int columns=sqlite3_column_count(req);

            records.push_back(Record());
            Record &r=records.back();
            for(int j=0;j<columns;j++) {
                int                  type  = sqlite3_column_type(req,j);
                const char          *name  = sqlite3_column_name(req,j);                
                if(type==SQLITE_TEXT) {                    
                    const unsigned char *value = sqlite3_column_text(req,j);
                    r.values[UTF8toUTF16(name)] = UTF8toUTF16(value);
                } else if(type==SQLITE_INTEGER) {
                    r.ivalues[UTF8toUTF16(name)] = sqlite3_column_int64(req,j);
                }
            }

            i++;
        }
    }

    void sqlExec(char *sql, sqlite3_callback xCallback, void *pArg) {
        char *zErrMsg=0;
        sqlite3_exec(db, "SELECT mark FROM startmenu LIMIT 1;", xCallback, &pArg, &zErrMsg);
        sqlite3_free(zErrMsg);
    }

    const char   *m_struct;
    const char   *m_name;
    const char   *unused;
    int           rc;
    sqlite3      *db;
};