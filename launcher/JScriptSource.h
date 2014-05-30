#pragma once
#include "QatapultScript.h"
#include "Collecter.h"

struct JScriptSource : Source {
    JScriptSource(Qatapult *pUI, const TCHAR *pluginname, const TCHAR *scriptpath);
    ~JScriptSource();
    void validate(SourceResult *r);
    void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes);
    int getUses(const CString &key);

    ActiveScriptHost host;
    sqlite3         *db;
    CStringA         m_dbname;
    sqlite3_stmt    *getusesstmt;
    sqlite3_stmt    *validatestmt;
    QatapultScript  *m_pQatapultScript;
};