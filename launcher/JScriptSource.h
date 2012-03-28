#pragma once
#include "QatapultScript.h"



struct JScriptSource : Source {
    ActiveScriptHost host;
    sqlite3         *db;
    CStringA         m_dbname;
    
    sqlite3_stmt    *getusesstmt;
    sqlite3_stmt    *validatestmt;

    JScriptSource(Qatapult *pUI, const TCHAR *pluginname, const TCHAR *scriptpath):Source(L"JScript",CString(pluginname)+L" (Catalog )") {
        host.Initialize(L"Qatapult",L"JScript");
                
        m_pQatapultScript=QatapultScript::Make(pUI);
        m_pQatapultScript->AddRef();
        host.AddObject(L"qatapult",(IDispatch*)m_pQatapultScript);
        
        host.Require(scriptpath);

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
    ~JScriptSource() {
        sqlite3_finalize(getusesstmt);
        sqlite3_finalize(validatestmt);
        m_pQatapultScript->Release();
        sqlite3_close(db);
    }
    void validate(SourceResult *r) {
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;        
        rc = sqlite3_bind_text16(validatestmt, 1, r->object->key, -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(validatestmt, 2, r->object->key, -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(validatestmt, 3, r->object->key, -1, SQLITE_STATIC);
        sqlite3_step(validatestmt);
        const char *errmsg=sqlite3_errmsg(db);
        sqlite3_reset(validatestmt);
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        CComVariant ret;
        CComSafeArray<VARIANT> ary;
        //ary.Resize(0,0);        
        CollecterScript *rr=CollecterScript::Make(this,&results,&activetypes);
        CComQIPtr<IDispatch> pdisp(rr);

        CComVariant v;
        ary.Add(CComVariant(CComVariant(query)));
        ary.Add(CComVariant(CComVariant(pdisp)));

        int nbresults=results.size();

        host.Run(CComBSTR(L"collect"),ary.GetSafeArrayPtr(),&ret);

        for(int i=nbresults;i<results.size();i++) {
            SourceResult *r=&results[i];

            CString q(query);            
            int rc = sqlite3_bind_text16(getusesstmt, 1, r->object->key, -1, SQLITE_STATIC);                       
            if(sqlite3_step(getusesstmt)==SQLITE_ROW) {
                r->uses=sqlite3_column_int(getusesstmt,0);                
            }
            sqlite3_reset(getusesstmt);
        }        
    }

    QatapultScript *m_pQatapultScript;
};


