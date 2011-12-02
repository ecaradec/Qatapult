inline CString ItoS(int i) {
    CString tmp; tmp.Format(L"%d",i);
    return tmp;
}
struct Source;

struct SourceResult {
    SourceResult() { rank=0; bonus=0; icon=0; source=0; data=0; dirty=false; }
    SourceResult(const CString &_key, const CString &_display, const CString &_expand, Source *_s, int _id, void *_data, int _bonus) { 
        key=_key;
        rank=0;
        bonus=_bonus;
        icon=0;
        display=_display;
        expand=_expand;
        source=_s;
        id=_id;
        data=_data;        
        dirty=false;
    }

    // must save
    Source  *source;
    CString  key;    
    CString  display;        
    CString  expand;
    int      bonus;
    int      id;

    // no saving required (and no copy )
    int      rank;
    bool     dirty;    
    Gdiplus::Bitmap *icon;
    // never use data from sourceresult outside the source, 
    // don't even copy it, should really create 2 class one with data* the other without
    // it should always be null outside the source
    void    *data;
};

// comments on the serialisation of the member 'data'
// ==================================================
//
// data is problematic to save because it is an opaque pointer
// but the serialisation could easily defered to the source
// a more annoying issue is the one of the ownership of the data
// especially with the case of m_args in alphagui
// there is 3 way to solve this :
// the clone way, the smartptr way, and the know the ownership way
// I usually prefer the 3rd way but I can't find a way to make it
// work simply in that case
//
// or may be data is not the correct storage for it, we could
// maintain a second index and query for it details through the 
// source ?
//
// may be there should only be source, key, rank, dirty and icon
// and the real index would be only accessed from the source with the key ???
// this would effectively split sourceresult in two one class internal
// to sources and one class internal to the alphagui
//
// => temporarily moved to the non serialisation area