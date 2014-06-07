
struct TextItemSource : Source {
    sqlite3 *db;
    sqlite3_stmt *getusesstmt;
    sqlite3_stmt *validatestmt;
    
    TextItemSource(const TCHAR *name) : Source(name) {
        type=name;
        int rc = sqlite3_open("databases\\textitems.db", &db);

        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE main(key TEXT PRIMARY KEY ASC, uses INTEGER DEFAULT 0)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        const char *unused=0;
        rc = sqlite3_prepare_v2(db,"SELECT uses FROM main WHERE key = ?;",-1, &getusesstmt, &unused);
        rc = sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO main (key, uses) VALUES(?, coalesce((SELECT uses FROM main WHERE key=?), 0)+1);",-1, &validatestmt, &unused);
    }
    ~TextItemSource() {
        sqlite3_finalize(getusesstmt);
        sqlite3_finalize(validatestmt);        
        sqlite3_close(db);
    }
    void addItem(const TCHAR *str,const TCHAR *iconname) {
        m_index.push_back(std::pair<CString,CString>(str,iconname));
    }
    virtual void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::vector<std::pair<CString,CString> >::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(FuzzyMatch(it->first,q)) {

                pack.begin(KV_Map);;
                    pack.writePairString(L"type",type);
                    pack.writePairUint32(L"source",(uint32)this);
                    pack.writePairString(L"text",it->first);
                    pack.writePairString(L"icon",it->second);                
                    pack.writePairUint32(L"bonus",20);
                    int rc = sqlite3_bind_text16(getusesstmt, 1, it->first, -1, SQLITE_STATIC);                       
                    if(sqlite3_step(getusesstmt)==SQLITE_ROW) {
                        pack.writePairUint32(L"uses",sqlite3_column_int(getusesstmt,0));
                    } else {
                        pack.writePairUint32(L"uses",(uint32)0);
                    }
                    sqlite3_reset(getusesstmt);
                pack.end();
            }
        }
    }
    void validate(Object *o) {
        int rc = sqlite3_bind_text16(validatestmt, 1, o->key, -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(validatestmt, 2, o->key, -1, SQLITE_STATIC);
        sqlite3_step(validatestmt);
        sqlite3_reset(validatestmt);
        const char *err=sqlite3_errmsg(db);
    }

    CString                                  type;
    std::vector<std::pair<CString,CString> > m_index;
};
