
// those kind of sources could have a simplified load and save ?
struct WebsiteSource : DBSource {
    WebsiteSource() : DBSource(L"websites", L"WEBSITE") {
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE websites(key TEXT PRIMARY KEY ASC, display TEXT, href TEXT, searchHref TEXT, icon TEXT, bonus INTEGER)", 0, 0, &zErrMsg);        
        
        sqlite3_exec(db, "INSERT OR REPLACE INTO websites (key, display, href, searchHref, icon, bonus) VALUES('Google', 'Google', 'http://google.com', 'https://www.google.com/#q=%q', 'google', coalesce((SELECT bonus FROM websites WHERE key=\"Google\"), 0));\n\
                          INSERT OR REPLACE INTO websites (key, display, href, searchHref, icon, bonus) VALUES('Amazon', 'Amazon', 'http://amazon.com', 'http://www.amazon.com/s/ref=nb_sb_noss?url=search-alias%3Daps&field-keywords=%q', 'amazon', coalesce((SELECT bonus FROM websites WHERE key=\"Amazon\"), 0));\n", 0, 0, &zErrMsg);
    }
};

struct SearchWithVerbSource : Source {
    SearchWithVerbSource() : Source(L"SEARCHWITHVERB") {        
        m_index[L"Search With"]=SourceResult(L"Search With", L"Search With", L"Search With", this, 0, 0, m_index[L"Search With"].bonus);
    }
};

struct WebSearchRule : Rule {
    WebSearchRule() : Rule(L"TEXT", L"SEARCHWITHVERB", L"WEBSITE") {}
    virtual bool execute(std::vector<SourceResult> &args) {
        CString searchURL=args[2].source->getString(args[2].key+"/searchHref");        
        searchURL.Replace(L"%q", args[0].display);            
        ShellExecute(0, 0, searchURL, 0, 0, SW_SHOWDEFAULT);
        return true;
    }
};


