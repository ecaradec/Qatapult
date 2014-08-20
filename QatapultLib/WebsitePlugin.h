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
    void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"WEBSITE")==activetypes.end())
            return;

        CString q(query);
        sqlite3_stmt *stmt=0;
        const char *unused=0;

        std::vector<Record> records;
        websites.findBy(records, "text", UTF16toUTF8(fuzzyfyArg(q)) );
        for(int i=0;i<records.size();i++) {
            Record &r=records[i];

            pack.begin(KV_Map);
                pack.writePairString(L"type",L"WEBSITE");
                pack.writePairUint32(L"source",(uint32)this);
            
                for(std::map<CString, CString>::iterator it=records[i].values.begin(); it!=records[i].values.end(); it++) {                
                    pack.writePairString(it->first, it->second);
                    if(it->first==L"text")
                        pack.writePairString(L"icon",L"icons\\"+it->second+L".png");
                }
                for(std::map<CString, __int64>::iterator it=records[i].ivalues.begin(); it!=records[i].ivalues.end(); it++) {
                    pack.writePairUint32(it->first, (uint32)it->second);
                }
            pack.end();
        }
    }
};
