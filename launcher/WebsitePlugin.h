#pragma once
#include "DBSource.h"
#include "Rule.h"
#include "Record.h"

struct WebsitesDB : DB {
    WebsitesDB() : DB("websites",
                      Array(std::make_pair("text",       TEXT),
                            std::make_pair("href",       TEXT),
                            std::make_pair("searchHref", TEXT),
                            std::make_pair("uses",       INTEGER)),1) {
    }
    void migrate(int currentversion) {
        switch(currentversion) {
        case 0:
            sqlite3_exec(db, "BEGIN;", 0, 0, 0);
            sqlite3_exec(db, "ALTER TABLE websites RENAME TO old_websites;", 0, 0, 0);
            sqlite3_exec(db, "CREATE TABLE websites (key INTEGER PRIMARY KEY,text TEXT,href TEXT,searchHref TEXT,uses INTEGER);", 0, 0, 0);
            sqlite3_exec(db, "INSERT INTO websites (text,href,searchHref,uses) SELECT text,href,searchHref,uses FROM old_websites;",0,0,0);
            sqlite3_exec(db, "DROP TABLE old_websites;", 0, 0, 0);
            sqlite3_exec(db, "END;", 0, 0, 0);                
            setVersion(1);
            break;
        }
    }
};

extern WebsitesDB websites;

// those kind of sources could have a simplified load and save ?
struct WebsiteSource : DBSource {
    WebsiteSource() : DBSource(L"WEBSITE",L"Websites (Catalog )", L"websites") {
        m_icon=L"icons\\website.png";
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"WEBSITE")==activetypes.end())
            return;

        CString q(query);
        sqlite3_stmt *stmt=0;
        const char *unused=0;

        std::vector<Record> records;
        websites.findBy(records, "text", UTF16toUTF8(fuzzyfyArg(q)) );
        for(int i=0;i<records.size();i++) {
            Record &r=records[i];
            results.push_back(new Object(ItoS(r.ivalues[L"key"]),
                                                      L"WEBSITE",
                                                      this,
                                                      r.values[L"text"]));
            results.back().uses()=r.ivalues[L"uses"];

            results.back().object()->values=records[i].values;
            results.back().object()->values[L"icon"]=L"icons\\"+results.back().object()->values[L"text"]+L".png";
        }
    }
};

struct SearchWithVerbSource : Source {
    SearchWithVerbSource() : Source(L"SEARCHWITHVERB") {        
        m_index[L"Search With"]=SourceResult(new Object(L"Search With", L"Search With", this, L"Search With"));
        m_index[L"Search With"].bonus()=m_index[L"Search With"].bonus();
    }
};

struct WebSearchRule : Rule {
    WebSearchRule() /*: Rule(Type(L"TEXT"), Type(L"Search With",L"icons\\searchwith.png"), Type(L"WEBSITE"))*/ {}
    virtual bool execute(std::vector<RuleArg> &args) {
        CString searchURL=args[2].object()->getString(L"searchHref");        
        searchURL.Replace(L"%q", args[0].object()->getString(L"text"));            
        ShellExecute(0, 0, searchURL, 0, 0, SW_SHOWDEFAULT);
        return true;
    }
};


