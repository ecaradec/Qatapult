
struct DBSource : Source {
    sqlite3 *db;
    CStringA  m_dbname;
    DBSource(const CString &dbname, const CString &type) : Source(type), m_dbname(dbname) {
        int rc = sqlite3_open(m_dbname+".db", &db);
    }    
    ~DBSource() {
        sqlite3_close(db);
    }
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r) { 
        return Gdiplus::Bitmap::FromFile(L"icons\\"+r->source->getString(r->key+L"/icon")+".png");
    }
    void validate(SourceResult *r) {
        WCHAR buff[4096];
        char *zErrMsg = 0;
        wsprintf(buff, L"UPDATE %s SET bonus = MIN(bonus + 10, 100) WHERE key=\"%s\"\n", CStringW(m_dbname).GetString(), r->key);        
        int z=sqlite3_exec(db, CStringA(buff), 0, 0, &zErrMsg);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        Info info;
        info.results=&results;
        info.source=this;
        char *zErrMsg = 0;
        WCHAR buff[4096];
        wsprintf(buff, L"SELECT key, display, display, 0, bonus FROM %s WHERE display LIKE \"%%%s%%\";", CStringW(m_dbname).GetString(), query); // display twice for expand
        sqlite3_exec(db, CStringA(buff), getResultsCB, &info, &zErrMsg);
    }
    CString getString(const TCHAR *itemquery) {
        CString q(itemquery);
        CString key=q.Left(q.ReverseFind('/'));
        CString val=q.Mid(q.ReverseFind('/')+1);

        char *zErrMsg = 0;
        WCHAR buff[4096];
        wsprintf(buff, L"SELECT %s FROM %s WHERE key='%s'", val, CStringW(m_dbname).GetString(), key);

        CString str;
        sqlite3_exec(db, CStringA(buff), getStringCB, &str, 0);
        return str;
    }
};
