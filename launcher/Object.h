#pragma once

struct Source;
struct SourceResult;

// draw
inline RectF getStdIconPos(RectF &r, bool h, float textheight=0) {
    if(h)
        return RectF(r.X, r.Y, r.Height, r.Height);
    return RectF(r.X+textheight/2, r.Y, r.Width-textheight, r.Width-textheight);
}
inline RectF getStdTextPos(RectF &r, bool h, float textheight) {
    if(h)
        return RectF(r.X+r.Height+r.Height/4, r.Y+r.Height/2-textheight/2, r.Width-r.Height, r.Height);
    return RectF(r.X, r.Y+r.Height-textheight, r.Width, 0);
}
inline RectF getStdTitlePos(RectF &r, bool h, float textheight) {
    if(h)
        return RectF(r.X+r.Height+r.Height/4, r.Y+0.1f*textheight, r.Width-r.Height-r.Height/4, r.Height);
    return RectF(r.X, r.Y+r.Height-textheight, r.Width, 0);
}
inline RectF getStdSubTitlePos(RectF &r, bool h, float textheight) {
    if(h)
        return RectF(r.X+r.Height+r.Height/4, r.Y+r.Height-1.1f*textheight, r.Width-r.Height-r.Height/4, r.Height);
    return RectF(r.X, r.Y+r.Height-textheight, r.Width, 0);
}
inline StringAlignment getStdAlignment(bool h) {
    return h?StringAlignmentNear:StringAlignmentCenter;
}

// only stores object specifics into the object, then store sources commons in source
// derive and gives an correctly typed pointer if in need for extra data
extern int objects;
struct Object {
    Object(const CString &k, const CString &t, Source *s, const CString &text) {
        type=t;
        key=k;
        source=s;        
        values[L"text"]=text;
        objects++;
    }
    Object(const Object& c) {
        source=c.source;
        key=c.key;
        values=c.values;
        type=c.type;
        objects++;
    }
    virtual ~Object();
    virtual CString getString(const TCHAR *val_);
    virtual Gdiplus::Bitmap *getIcon(long flags);
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r);
    virtual void drawListItem(Graphics &g, SourceResult *sr, RectF &r, float fontsize, bool b, DWORD textcolor, DWORD bgcolor, DWORD focuscolor);

    CString                   type;
    Source                   *source;
    CString                   key;
    std::map<CString,CString> values;
};

struct FileObject : Object {
    FileObject(const CString &k, Source *s, const CString &text, const CString &expand, const CString &path);
    FileObject(const FileObject& f);
    FileObject *clone();
    CString getString(const TCHAR *val_);
    Gdiplus::Bitmap *getIcon(long flags);
};

