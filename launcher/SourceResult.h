#pragma once

struct Source;
struct Object;

struct SourceResult {
    SourceResult() { object=0; rank=0; bonus=0; uses=0; icon=0; smallicon=0; source=0; data=0; dirty=false; }
    SourceResult(const CString &_key, const CString &_display, const CString &_expand, Source *_s, int _id=0, void *_data=0, int _uses=0) { 
        //key=_key;
        object=0; 
        rank=0;
        bonus=0;
        uses=_uses;
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
    CString  display;
    CString  expand;
    int      bonus;
    int      uses;
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


