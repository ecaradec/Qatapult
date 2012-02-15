
struct DBSource : Source {
    sqlite3 *db;
    CStringA  m_dbname;
    DBSource(const CString &type,const CString &name, const CString &dbname) : Source(type,name), m_dbname(dbname) {
        int rc = sqlite3_open(L"databases\\"+m_dbname+".db", &db);
    }    
    ~DBSource() {
        sqlite3_close(db);
    }
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) { 
        return Gdiplus::Bitmap::FromFile(L"icons\\"+r->source->getString(*r,L"icon")+".png");
    }
    void validate(SourceResult *r) {
        WCHAR buff[4096];
        char *zErrMsg = 0;
        wsprintf(buff, L"UPDATE %s SET bonus = MIN(bonus + 10, 100) WHERE key=\"%s\"\n", CStringW(m_dbname).GetString(), r->key);        
        int z=sqlite3_exec(db, CStringA(buff), 0, 0, &zErrMsg);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString q(query);
        q.Replace(L"_",L"\\_");
        q.Replace(L"%",L"\\%");
        q.Replace(L"'",L"\\'");
        q.Replace(L"\"",L"\\\"");

        Info info;
        info.results=&results;
        info.source=this;
        char *zErrMsg = 0;
        WCHAR buff[4096];
        wsprintf(buff, L"SELECT key, display, display, 0, bonus FROM %s WHERE display LIKE \"%%%s%%\";", CStringW(m_dbname).GetString(), q); // display twice for expand
        sqlite3_exec(db, CStringA(buff), getResultsCB, &info, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    CString getString(SourceResult &sr,const TCHAR *val) {
        char *zErrMsg = 0;
        WCHAR buff[4096];
        wsprintf(buff, L"SELECT %s FROM %s WHERE key='%s'", val, CStringW(m_dbname).GetString(), sr.key);

        CString str;
        sqlite3_exec(db, CStringA(buff), getStringCB, &str, 0);
        return str;
    }
};
