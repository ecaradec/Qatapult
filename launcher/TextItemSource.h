
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
        m_index[str]=SourceResult(new Object(str,str,this,str));
        m_index[str].iconname()=iconname;
    }
    virtual void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(FuzzyMatch(it->second.display(),q)) {

                uint8 *obj=pack.beginBlock();
                pack.pack(L"type",type);
                pack.pack(L"source",(uint32)this);
                pack.pack(L"text",it->second.expand());
                pack.pack(L"icon",it->second.iconname());                
                pack.pack(L"bonus",20);
                int rc = sqlite3_bind_text16(getusesstmt, 1, it->second.expand(), -1, SQLITE_STATIC);                       
                if(sqlite3_step(getusesstmt)==SQLITE_ROW) {
                    pack.pack(L"uses",sqlite3_column_int(getusesstmt,0));
                } else {
                    pack.pack(L"uses",(uint32)0);
                }
                sqlite3_reset(getusesstmt);
                pack.endBlock(obj);
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

    CString                         type;
    std::map<CString, SourceResult> m_index;
};
