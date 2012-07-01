#pragma once
#include "Record.h"

extern DB websites;

// those kind of sources could have a simplified load and save ?
struct WebsiteSource : DBSource {
    WebsiteSource() : DBSource(L"WEBSITE",L"Websites (Catalog )", L"websites") {
        m_icon=L"icons\\website.png";
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query);
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        std::vector<Record> records;
        websites.findBy(records, "display", UTF16toUTF8(fuzzyfyArg(q)) );
        for(int i=0;i<records.size();i++) {
            Record &r=records[i];
            results.push_back(SourceResult(ItoS(r.ivalues[L"key"]),         // key
                                           r.values[L"display"],            // display
                                           r.values[L"display"],            // expand
                                           this,                            // source
                                           0,                               // id
                                           0,                               // data                                            
                                           r.ivalues[L"uses"]));            // uses

            Object *fo=new Object(ItoS(r.ivalues[L"key"]),
                                  type,
                                  this,
                                  r.values[L"display"]);
            
            fo->values=records[i].values;
            fo->values[L"text"]=fo->values[L"display"];
            fo->values[L"icon"]=L"icons\\"+fo->values[L"display"]+L".png";
            results.back().object().reset(fo);
        }
    }
};

struct SearchWithVerbSource : Source {
    SearchWithVerbSource() : Source(L"SEARCHWITHVERB") {        
        m_index[L"Search With"]=SourceResult(L"Search With", L"Search With", L"Search With", this, 0, 0, m_index[L"Search With"].bonus());
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


