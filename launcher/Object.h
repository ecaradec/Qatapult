#pragma once
#include "KVPack.h"

struct Source;
struct SourceResult;
struct Record;

// only stores object specifics into the object, then store sources commons in source
// derive and gives an correctly typed pointer if in need for extra data
struct Object {
    Object(uint8 *pObj, bool ownData=false) :m_obj(pObj) {
        source=0; 
        m_bonus=0;
        m_uses=0;
        m_rank=0;
        m_ownData=ownData;

        // File object
        m_jumboDrawMethod=0;

        CString text=m_obj.getString(L"text");
        type=m_obj.getString(L"type");
        key=m_obj.getString(L"key");
        m_uses=m_obj.getInt(L"uses");
        m_bonus=m_obj.getInt(L"bonus");
        source=(Source*)m_obj.getInt(L"source");
    }

    // clone only works with m_Obj type object
    // it's purpose is to disassociate the object from the big data pack
    Object *clone() {
        return new Object(m_obj.clone().pobj,true);
    }
    
    virtual ~Object();
    virtual CString toJSON();
    virtual CString toXML();
    virtual int getInt(const TCHAR *val_);
    virtual CString getString(const TCHAR *val_);
    CString getFileString(const TCHAR *val_);
    //virtual Gdiplus::Bitmap *getIcon(long flags);
    virtual void drawIcon(Graphics &g, RectF &r);
    virtual void drawFileIcon(Graphics &g, RectF &r);
    virtual void drawListItem(Graphics &g, RectF &r, float fontsize, bool b, DWORD textcolor, DWORD bgcolor, DWORD focuscolor);
    
    bool                             m_ownData;
    KVObject                         m_obj;
    
    std::shared_ptr<Gdiplus::Bitmap> m_icon;
    std::shared_ptr<Gdiplus::Bitmap> m_smallicon;    

    CString                          type;
    Source                          *source;
    CString                          key;
    
    int                              m_bonus;
    int                              m_uses;
    CString                          m_iconname;
    int                              m_rank;

    // File object
    int                              m_jumboDrawMethod;

private:
    //Object(const Object& c); // disable copy constructor
};

