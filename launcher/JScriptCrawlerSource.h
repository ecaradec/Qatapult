#include "Source.h"
#include "

struct JScriptCrawlerSource : Source {
    ActiveScriptHost host;
    sqlite3         *db;
    CStringA         m_dbname;
    
    sqlite3_stmt    *getusesstmt;
    sqlite3_stmt    *validatestmt;

    JScriptCrawlerSource(Qatapult *pUI, const TCHAR *pluginname, const TCHAR *scriptpath):Source(L"JScript",CString(pluginname)+L" (Catalog )") {
        host.ParseScriptText(getFileContent(scriptpath),scriptpath);

        m_dbname="databases\\"+CStringA(pluginname)+".db";

        int rc = sqlite3_open(m_dbname, &db);

        // as the pluginname is the name of it's folder use a default name for the table to allow moving 
        // the plugin without losing history
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE main(key TEXT PRIMARY KEY ASC, bonus INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        UpgradeTable(db,"main");

        const char *unused=0;                    
        rc = sqlite3_prepare_v2(db,"SELECT uses FROM main WHERE key = ?;",-1, &getusesstmt, &unused);

        rc = sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO main (key, uses, lastUse) VALUES(?, coalesce((SELECT uses FROM main WHERE key=?), 0)+1);",-1, &validatestmt, &unused);
    }
    ~JScriptCrawlerSource() {
        sqlite3_finalize(getusesstmt);
        sqlite3_finalize(validatestmt);
        m_pQatapultScript->Release();
        sqlite3_close(db);
    }
    void validate(SourceResult *r) {
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;
        sqlite3_reset(validatestmt);
        rc = sqlite3_bind_text16(validatestmt, 1, r->object->key, -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(validatestmt, 2, r->object->key, -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(validatestmt, 3, r->object->key, -1, SQLITE_STATIC);
        sqlite3_step(validatestmt);
        const char *errmsg=sqlite3_errmsg(db);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query);
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        rc = sqlite3_prepare_v2(db,L"SELECT * FROM "+m_dbname+L" WHERE display LIKE ?;",-1, &stmt, &unused);
        /*rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
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
        }*/

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(stmt);         
    }
    virtual void crawl() {
        

    }    
};
