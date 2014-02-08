#pragma once

struct Source;
struct SourceResult;
struct Record;

// only stores object specifics into the object, then store sources commons in source
// derive and gives an correctly typed pointer if in need for extra data
struct Object {
    Object() {
        source=0; 
        m_bonus=0;
        m_uses=0;
        m_rank=0;
        subsource=0;
    }
    Object(const CString &k, const CString &t, Source *s, const CString &text) {
        m_bonus=0;
        m_uses=0;
        m_rank=0;
        subsource=0;
        type=t;
        key=k;
        source=s;        
        values[L"text"]=text;
    }    
    
    virtual ~Object();
    virtual CString toJSON();
    virtual CString toXML();
    virtual CString getString(const TCHAR *val_);
    //virtual Gdiplus::Bitmap *getIcon(long flags);
    virtual void drawIcon(Graphics &g, RectF &r);
    virtual void drawListItem(Graphics &g, SourceResult *sr, RectF &r, float fontsize, bool b, DWORD textcolor, DWORD bgcolor, DWORD focuscolor);

    std::shared_ptr<Gdiplus::Bitmap> m_icon;
    std::shared_ptr<Gdiplus::Bitmap> m_smallicon;
    
    CString                          type;
    Source                          *source;
    Source                          *subsource;
    CString                          key;
    std::map<CString,CString>        values;
    std::map<CString,__int64>        ivalues;
    
    int                              m_bonus;
    int                              m_uses;
    CString                          m_iconname;
    int                              m_rank;

private:
    Object(const Object& c); // disable copy constructor
};

struct FileObject : Object {
    FileObject(const CString &k, Source *s, const CString &text, const CString &expand, const CString &path);
    FileObject(const FileObject& f);
    FileObject(Record &r,Source *s);
    FileObject *clone();
    CString getString(const TCHAR *val_);
    void drawIcon(Graphics &g, RectF &r);

    int m_jumboDrawMethod;
};
