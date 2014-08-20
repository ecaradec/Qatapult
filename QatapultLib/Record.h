#pragma once
#include "utf8.h"
#include "simpleini.h"


/*
struct RValue {

    union {
        CString strval;
        __int64 ival;
    }
};*/

struct Record {
    Record() {
        ivalues["key"]=-1;
    }
    std::map<CString, CString> values;
    std::map<CString, __int64> ivalues;    
};

extern int getIntCB(void *NotUsed, int argc, char **argv, char **azColName);
extern int getStringCB(void *NotUsed, int argc, char **argv, char **azColName);

enum RecordType {
    TEXT,
    INTEGER
};

struct IDB {
    virtual ~IDB() {};
    virtual void save(Record &r) {}
    virtual bool del(Record &r) { return true; }
};

typedef std::vector<std::pair<const char*, RecordType> > DBStructure;
// the structure of the db should decide the insert or replace, find, etc... not the record
struct DB : IDB {
    DBStructure m_types;

    sqlite3_stmt *m_insertOrReplaceObject;
    sqlite3_stmt *m_deleteObject;
    sqlite3_stmt *m_findObject;

    DB(char *tbname, char* strct):m_name(tbname), m_struct(strct) {
        int rc = sqlite3_open("databases\\"+CStringA(m_name)+".db", &db);

        char *zErrMsg = 0;
        int ok=sqlite3_exec(db, "CREATE TABLE "+CStringA(m_name)+"("+CStringA(m_struct)+")", 0, 0, &zErrMsg);        
        sqlite3_free(zErrMsg);
    }
    void setVersion(int version) {
        char *zErrMsg = 0;
        sqlite3_exec(db, "PRAGMA user_version="+CStringA(ItoS(version)), 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    void upgrade() {
        CString s;
        sqlite3_exec(db, "PRAGMA user_version", getStringCB, &s,0);
        int version=_ttoi(s);
        migrate(version);
    }
    virtual void migrate(int currentversion) {
    }
    DB(char *tbname, DBStructure &types, int currentversion):m_name(tbname) {
        m_types=types;

        static CStringA typesStr[]={"TEXT", "INTEGER"};

        int rc = sqlite3_open("databases\\"+CStringA(m_name)+".db", &db);
        CStringA str;        
        for(int i=0;i<types.size();i++) {
            CStringA n=CStringA(types[i].first);
            CStringA t=typesStr[types[i].second];
            str+=CStringA(",") + n + " " + t;
        }

        char *zErrMsg = 0;
        int ok=sqlite3_exec(db, "CREATE TABLE "+CStringA(m_name)+"(key INTEGER PRIMARY KEY"+str+")", 0, 0, &zErrMsg);        
        sqlite3_free(zErrMsg);
        if(ok==SQLITE_OK) {
            setVersion(currentversion);
        }
        
        CStringA strNames;
        CStringA strArgs;
        for(int i=0;i<types.size();i++) {
            strNames+=CStringA(",")+CStringA(types[i].first);
            strArgs+=L",?";
        }
        
        rc = sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO "+CStringA(m_name)+" (key"+strNames+") VALUES(?"+strArgs+");",-1, &m_insertOrReplaceObject, &unused);    

        rc = sqlite3_prepare_v2(db,"DELETE FROM "+CStringA(m_name)+" WHERE key=?",-1, &m_deleteObject, &unused);

        rc = sqlite3_prepare_v2(db,"SELECT * FROM "+CStringA(m_name)+" WHERE key = ?;",-1, &m_findObject, &unused);
    }
    ~DB() {
        sqlite3_finalize(m_insertOrReplaceObject);
        sqlite3_finalize(m_deleteObject);
        sqlite3_finalize(m_findObject);
        sqlite3_close(db);
    }
    void save(Record &r) {
        CStringA str;
        CStringA args;        

        if(r.ivalues["key"]==-1)
            sqlite3_bind_null(m_insertOrReplaceObject, 1);
        else
            sqlite3_bind_int64(m_insertOrReplaceObject, 1, r.ivalues["key"]);

        for(int i=0;i<m_types.size();i++) {
            CString keyname=m_types[i].first;
            if(m_types[i].second==TEXT) {
                CString v=r.values[keyname];
                sqlite3_bind_text16(m_insertOrReplaceObject, i+2, v, -1, SQLITE_STATIC);
            } else if(m_types[i].second==INTEGER) {
                __int64 v=r.ivalues[keyname];
                sqlite3_bind_int64(m_insertOrReplaceObject, i+2, v);
            }
        }
        
        rc=sqlite3_step(m_insertOrReplaceObject);        
        const char *errmsg=sqlite3_errmsg(db) ;        
        sqlite3_reset(m_insertOrReplaceObject);

        if(rc==SQLITE_ROW ||rc==SQLITE_DONE) {
            if(r.ivalues[L"key"]==-1) {
                r.ivalues[L"key"]=sqlite3_last_insert_rowid(db);
            }
        }
    }
    bool del(Record &r) {
        rc = sqlite3_bind_int64(m_deleteObject, 1, r.ivalues[L"key"]);
        rc=sqlite3_step(m_deleteObject);        
        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_reset(m_deleteObject);
        return rc==SQLITE_DONE;
    }

    // could as well be a macro
    int query(std::vector<Record> &records) {
        sqlite3_stmt *req;
        int rc = sqlite3_prepare_v2(db,"SELECT * FROM "+CStringA(m_name)+";",-1, &req, &unused);
        
        fetchRecords(req,records);

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(req);
        return records.size();
    }

    int findBy(std::vector<Record> &records, const char *name, const char *value) {
        sqlite3_stmt *req;
        //OutputDebugStringA("SELECT * FROM "+CStringA(m_name)+" WHERE "+CStringA(name)+" LIKE ?;\n");
        int rc = sqlite3_prepare_v2(db,"SELECT * FROM "+CStringA(m_name)+" WHERE "+CStringA(name)+" LIKE ?;",-1, &req, &unused);
        sqlite3_bind_text(req, 1, value, -1, SQLITE_STATIC);

        fetchRecords(req,records);

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(req);
        return records.size();
    }

    int find(Record &record, __int64 key) {
        rc = sqlite3_bind_int64(m_findObject, 1, key);
        rc=sqlite3_step(m_findObject);        
        const char *errmsg=sqlite3_errmsg(db) ;        

        if(rc!=SQLITE_ROW) {
            sqlite3_reset(m_findObject);
            return 0;
        }

        fetchRecord(m_findObject,record);
        sqlite3_reset(m_findObject);
        return 1;
    }

    void fetchRecords(sqlite3_stmt *req, std::vector<Record> &records) {
        int i=0;
        while((rc=sqlite3_step(req))==SQLITE_ROW) {
            records.push_back(Record());
            fetchRecord(req,records.back());
            i++;
        }
    }

    void fetchRecord(sqlite3_stmt *req, Record &r) {
        int columns=sqlite3_column_count(req);
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
    }

    int sqlExec(char *query) {
        sqlite3_exec(db,query,0,0,0);
        return 0;
    }

    int sqlGetInt(char *name) {
        int mark=0;
        sqlite3_exec(db, "SELECT "+CStringA(name)+" FROM "+CStringA(m_name)+" LIMIT 1;", getIntCB, &mark, 0);
        return mark;
    }

    const char   *m_struct;
    const char   *m_name;
    const char   *unused;
    int           rc;
    sqlite3      *db;
};