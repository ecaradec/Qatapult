struct Source;
struct SourceResult;
struct Object {
    virtual ~Object() = 0 {};
    virtual Object *clone() = 0;
    virtual Gdiplus::Bitmap *getIcon(long flags) = 0;
    virtual CString getString(const TCHAR *val) = 0;
};

struct SourceResult {
    SourceResult() { object=0; rank=0; bonus=0; icon=0; smallicon=0; source=0; data=0; dirty=false; }
    SourceResult(const CString &_key, const CString &_display, const CString &_expand, Source *_s, int _id, void *_data, int _bonus) { 
        key=_key;
        object=0; 
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
    Object  *object;
    CString  key;    
    CString  display;        
    CString  expand;
    int      bonus;
    int      id;
    void    *data; // must be cloned if needed

    // temporary for automatic
    CString  iconname;

    // no saving required (and no copy )
    int      rank;
    bool     dirty;    
    Gdiplus::Bitmap *icon;
    Gdiplus::Bitmap *smallicon;
};


