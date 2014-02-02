/*struct DllPlugin {
    DllPlugin(TCHAR *path) {
        hmod=LoadLibrary(path);
    }
    Source *getSource(int i) {
        return new DllSource(hmod);
    }
    //Rule *getRule(int i) {
    //}
    //HWND getSettings() {
    //}
    HANDLE hmod;
};*/

/*
struct DllSource : Source {

    //FARPROC _drawItem;
    void (* m_collect)(DWORD *);
    //FARPROC _beginCollect;
    void (* m_validate)();
    WCHAR *(* m_getString)(WCHAR *);
    //FARPROC _crawl;
    //FARPROC _getString;
    //FARPROC _getInt;
    //FARPROC _rate;

    struct DllObject : Object {
        DllObject(DllSource *dllsource,DWORD hobject) {
            m_hobject=hobject;
            m_dllsource=dllsource;
        }
        CString toJSON() {
        }
        CString toXML() {
        }
        CString getString(const TCHAR *val_) {
            m_dllsource->getObjectString(m_hobject, val_);
        }
        Gdiplus::Bitmap *getIcon(long flags) {
            // FIX THIS
            //m_dllsource->drawObjectIcon(m_hobject, flags);
        }
        // Let the future drawIcon handle customization first
        //void drawItem(Graphics &g, SourceResult *sr, RectF &r);
        // Let the future drawIcon handle customization first
        //void drawListItem(Graphics &g, SourceResult *sr, RectF &r, float fontsize, bool b, DWORD textcolor, DWORD bgcolor, DWORD focuscolor)

        DllSource  *m_dllsource;
        DWORD       m_hobject;
    };

    DllSource(HMODULE h) : Source(L"Unknown") {
        hmod=h;
        //hmod=LoadLibrary(path);

        //_drawItem=GetProcAddress(hmod, "drawItem");
        //_beginCollect=GetProcAddress(hmod, "beginCollect");
        m_collect=(bool (*)(DWORD *))GetProcAddress(hmod, "collect");
        m_validate=(void (*)())GetProcAddress(hmod, "validate");
        //_crawl=GetProcAddress(hmod, "crawl");
        //_getString=GetProcAddress(hmod, "getString");
        //_getInt=GetProcAddress(hmod, "getInt");
        //_rate=GetProcAddress(hmod, "rate");
    }
    // get results
    // fuse index and bonus from the db
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        DWORD hobject;
        //while( _collect(&hobject) ) {
        //    results.push_back(new DllObject(this,hobject));
        //}
    }    
    virtual void validate(SourceResult *r)  {
        //_validate(r);
    }
    virtual void crawl() {
    }
    // unused yet
    // get named data of various types
    virtual int getInt(const TCHAR *itemquery) { 
        return false;
    }

    virtual void rate(const CString &q, SourceResult *r) {
    }

    TCHAR *getObjectString(TCHAR *) {

    }

    HMODULE hmod;
};*/