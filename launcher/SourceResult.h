struct Source;

struct SourceResult {
    SourceResult() { rank=0; bonus=0; icon=0; smallicon=0; source=0; data=0; dirty=false; }
    SourceResult(const CString &_key, const CString &_display, const CString &_expand, Source *_s, int _id, void *_data, int _bonus) { 
        key=_key;
        rank=0;
        bonus=_bonus;
        icon=0;
        smallicon=0;
        display=_display;
        expand=_expand;
        source=_s;
        id=_id;
        data=_data;        
        dirty=false;
    }
    SourceResult(const SourceResult &s) {
        *this=s; // copy all fields stupidly
    }

    // must save
    Source  *source;
    CString  key;    
    CString  display;        
    CString  expand;
    int      bonus;
    int      id;

    // temporary for automatic
    CString  iconname;

    // no saving required (and no copy )
    int      rank;
    bool     dirty;    
    Gdiplus::Bitmap *icon;
    Gdiplus::Bitmap *smallicon;
    // never use data from sourceresult outside the source, 
    // don't even copy it, should really create 2 class one with data* the other without
    // it should always be null outside the source
    void    *data;
};


