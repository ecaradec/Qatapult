#pragma once

// those kind of sources could have a simplified load and save ?
struct WebsiteSource : DBSource {
    WebsiteSource() : DBSource(L"WEBSITE",L"Websites (Catalog )", L"websites") {
        m_icon=L"icons\\website.png";
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE websites(key TEXT PRIMARY KEY ASC, display TEXT, href TEXT, searchHref TEXT, icon TEXT, bonus INTEGER)", 0, 0, &zErrMsg);        
        sqlite3_free(zErrMsg);

        UpgradeTable(db,"websites");

        sqlite3_exec(db, "INSERT OR REPLACE INTO websites (key, display, href, searchHref, icon, uses) VALUES('Google', 'Google', 'http://google.com', 'https://www.google.com/#q=%q', 'icons\\google.png', coalesce((SELECT bonus FROM websites WHERE key=\"Google\"), 0));\n\
                          INSERT OR REPLACE INTO websites (key, display, href, searchHref, icon, uses) VALUES('Amazon', 'Amazon', 'http://amazon.com', 'http://www.amazon.com/s/ref=nb_sb_noss?url=search-alias%3Daps&field-keywords=%q', 'icons\\amazon.png', coalesce((SELECT uses FROM websites WHERE key=\"Amazon\"), 0));\n", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query);
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        rc = sqlite3_prepare_v2(db,"SELECT key, display, href, searchHref, icon, uses FROM websites WHERE display LIKE ?;",-1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
        int i=0;
        while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
            results.push_back(SourceResult(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),     // key
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),    // display
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),    // expand
                                            this,                                               // source
                                            0,                                                  // id
                                            0,                                                  // data                                            
                                            sqlite3_column_int(stmt,5)));                       // uses

            Object *fo=new Object(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),
                                  type,
                                  this,
                                  UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));
            fo->values[L"href"]=UTF8toUTF16((char*)sqlite3_column_text(stmt,2));
            fo->values[L"searchHref"]=UTF8toUTF16((char*)sqlite3_column_text(stmt,3));
            fo->values[L"icon"]=UTF8toUTF16((char*)sqlite3_column_text(stmt,4));
            results.back().object().reset(fo);
        }

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(stmt);
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


