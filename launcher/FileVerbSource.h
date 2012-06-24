#pragma once
#include "Source.h"
#include "getItemVerbs.h"

struct FileVerbSource : Source {
    sqlite3 *db;
    sqlite3_stmt *getusesstmt;
    sqlite3_stmt *validatestmt;
    int bonus;

    FileVerbSource() : Source(L"FILEVERB", L"File verbs (Catalog )") {
        int rc = sqlite3_open("databases\\textitems.db", &db);

        bonus = 50;
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE main(key TEXT PRIMARY KEY ASC, uses INTEGER DEFAULT 0)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        const char *unused=0;
        rc = sqlite3_prepare_v2(db,"SELECT uses FROM main WHERE key = ?;",-1, &getusesstmt, &unused);
        rc = sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO main (key, uses) VALUES(?, coalesce((SELECT uses FROM main WHERE key=?), 0)+1);",-1, &validatestmt, &unused);

        int itemcount=GetSettingsInt(L"FileVerbs", L"count", 0);

        int b=itemcount+10;
        m_index[L"Open"]=SourceResult(L"Open", L"Open", L"Open", this, 0, 0, 0);
        m_index[L"Edit"]=SourceResult(L"Edit", L"Edit", L"Edit", this, 0, 0, 0);
        m_index[L"RunAs"]=SourceResult(L"RunAs", L"RunAs", L"RunAs", this, 0, 0, 0);
        m_index[L"Delete"]=SourceResult(L"Delete", L"Delete", L"Delete", this, 0, 0, 0);
        m_index[L"Properties"]=SourceResult(L"Properties", L"Properties", L"Properties", this, 0, 0, 0);
    }
    ~FileVerbSource() {
        sqlite3_finalize(getusesstmt);
        sqlite3_finalize(validatestmt);        
        sqlite3_close(db);
    }
    int getItemUses(const TCHAR *key) {
        int uses=0;
        int rc = sqlite3_bind_text16(getusesstmt, 1, key, -1, SQLITE_STATIC);
        if(sqlite3_step(getusesstmt)==SQLITE_ROW) {
            uses=sqlite3_column_int(getusesstmt,0);                
        }
        sqlite3_reset(getusesstmt);
        return uses;
    }
    void conditionalAddObject(std::vector<SourceResult> &results, const CString &name, const CString &q) {
        if(FuzzyMatch(name,q)) {
            results.push_back(SourceResult(name, name, name, this, 0, 0, 0));
            results.back().object().reset(new Object(name,L"FILEVERB",this,name));
            results.back().uses()=getItemUses(name);
            results.back().bonus()=20;
        }        
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        conditionalAddObject(results,L"Open",q);
        conditionalAddObject(results,L"Edit",q);
        conditionalAddObject(results,L"RunAs",q);
        conditionalAddObject(results,L"Delete",q);
        conditionalAddObject(results,L"Properties",q);
    }
    void validate(SourceResult *r) {        
        int rc = sqlite3_bind_text16(validatestmt, 1, r->object()->key, -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(validatestmt, 2, r->object()->key, -1, SQLITE_STATIC);
        sqlite3_step(validatestmt);
        sqlite3_reset(validatestmt);
        const char *err=sqlite3_errmsg(db);
    }

    struct Extra {
        CString command;
        CString args;
        CString workdir;
    };
    std::map<CString, Extra> m_extras;
};
