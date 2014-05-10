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

/*
Object o(uint8 *store, std::map<Type*> &types);
o.source=this;
o.type=types["FILE"]; // we need to register types somewhere
o.addString("filename","test");
o.addString("expand","c:\\test");
results.push_back(o);

Then all the methods are on the type ?

o.type->drawItem(o);

or 

o.source->drawItem(o)


extra values could be handled inside the shell command ? like extra arguments are.

command could have a "only show when" option

putty > connect > server

would be better handled as a custom application with a keyword putty instead of having to look for the putty.exe file which is way too complex
*/