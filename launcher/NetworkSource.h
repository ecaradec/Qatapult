#pragma once
#include "Source.h"
#include "Utility.h"
#include "Record.h"
#include "KVPack.h"

BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr, std::vector<CString> &lnks);

struct NetworkShares : DB {
    NetworkShares() : DB("networkshares",
                 Array(std::make_pair("text",   TEXT),
                       std::make_pair("expand", TEXT),
                       std::make_pair("path",   TEXT),
                       std::make_pair("verb",   TEXT),
                       std::make_pair("bonus",  INTEGER),
                       std::make_pair("mark",   INTEGER),
                       std::make_pair("uses",   INTEGER)),
                       1) {
    }
};
extern NetworkShares networkshares;

struct NetworkSource : Source {
    NetworkSource() : Source(L"FILE",L"Network (Catalog )") {
        m_icon=L"icons\\networklocal.png";
        m_ignoreemptyquery=true;
    }
    ~NetworkSource() {
        //sqlite3_close(db);
    }
    void collect(const TCHAR *query, KVPack &pack, int flags, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"FILE")==activetypes.end())
            return;

        // could probably be done in subclass as well as the callback since sourceresult will not change 
        CString q(query);

        std::vector<Record> records;
        networkshares.findBy(records, "text", UTF16toUTF8(fuzzyfyArg(q)));
        
        for(int i=0;i<records.size();i++) {

            uint8 *pobj=pack.beginBlock();
            pack.pack(L"type",L"FILE");
            pack.pack(L"source",(uint32)this);
            pack.pack(L"icon",L"icons\\networklocal.png");
            
            for(std::map<CString, CString>::iterator it=records[i].values.begin(); it!=records[i].values.end(); it++) {
                pack.pack(it->first, it->second);
            }
            for(std::map<CString, __int64>::iterator it=records[i].ivalues.begin(); it!=records[i].ivalues.end(); it++) {
                pack.pack(it->first, (uint32)it->second);
            }

            pack.endBlock(pobj);
        }
    }
    void crawl() {
        std::vector<CString> lnks;
        EnumerateFunc(0, lnks);

        int mark=networkshares.sqlGetInt("mark");

        std::vector<Record> records;
        for(uint i=0;i<lnks.size();i++) {
            Record r;
            networkshares.findBy(records,"path",CStringA(lnks[i]));
            if(records.size()>0) {
                r=records[0];                                
            }
            
            r.ivalues[L"mark"]=mark+1;
            r.values[L"text"]=lnks[i];
            r.values[L"expand"]=lnks[i]+L"\\";
            r.values[L"path"]=lnks[i];
            networkshares.save(r);
        }

        networkshares.sqlExec((char*)(CStringA("DELETE FROM networkshares WHERE mark=")+CStringA(ItoS(mark))).GetString());
    }
    void validate(SourceResult *r)  {
        networkshares.sqlExec((char*)(CStringA("UPDATE networkshares SET uses=uses+1 WHERE key=")+CStringA(r->object()->key)).GetString());
    }
    sqlite3 *db;
};

#pragma comment(lib, "mpr.lib")

#include <winnetwk.h>

void DisplayStruct(int i, LPNETRESOURCE lpnrLocal);

