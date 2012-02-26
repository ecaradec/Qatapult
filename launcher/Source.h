#pragma once

#include "md5.h"
#include "Object.h"
#include "SourceResult.h"
#include "pugixml.hpp"
#include "simpleini.h"
#include "geticon.h"

struct Source;

struct Info {
    Source                    *source;
    std::vector<SourceResult> *results;
};


int getResultsCB(void *NotUsed, int argc, char **argv, char **azColName);
int getStringCB(void *NotUsed, int argc, char **argv, char **azColName);
int getIntCB(void *NotUsed, int argc, char **argv, char **azColName);

CString md5(const CString &data);
CStringA sqlEscapeString(const CStringA &args);
CStringW sqlEscapeStringW(const CStringW &args);

struct UI {
    virtual ~UI() {}
    virtual HWND getHWND() = 0;

    // plugins
    virtual CString getQuery() = 0;
    virtual void setStatus(const CString &s) {}

    // settings dialog    
    virtual void InvalidateIndex() = 0;
    virtual void Reload() = 0;
    virtual void SetCurrentSource(int pane, Source *s,CString &q) = 0;
    virtual void Show() = 0;
    virtual int  GetCurPane() = 0;
};


#define DE_UNDERLINE 0
#define DE_COLOR 1
// this is probably very inefficient as as way to draw highlighted text because it handle proper hyphen in all cases
// gdiplus doesn't draw hyphen if the bbox is smaller than 2 hyphen => this is a workaround
void drawEmphased(Graphics &g, CString text, CString query, RectF &rect, int flags=DE_UNDERLINE, StringAlignment align=StringAlignmentCenter, float fontSize=10.0f);

extern UI *g_pUI; // very lazy way to give access to the ui to the ui window proc

bool FuzzyMatch(const CString &w_,const CString &q_);

struct Source {
    Source(const CString& t)
        :itemlistFont(GetSettingsString(L"general",L"font",L"Arial"), 8.0f, FontStyleBold, UnitPoint),
         itemscoreFont(GetSettingsString(L"general",L"font",L"Arial"), 8.0f) {
        m_refreshPeriod=0;
        m_name=t;
        type=t;
        m_ignoreemptyquery=false;
        m_prefix=0;
        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }
    Source(const CString& t, const CString &n)
    :itemlistFont(GetSettingsString(L"general",L"font",L"Arial"), 8.0f, FontStyleBold, UnitPoint),
     itemscoreFont(GetSettingsString(L"general",L"font",L"Arial"), 8.0f){
        m_refreshPeriod=0;
        m_name=n;
        type=t;
        m_ignoreemptyquery=false;
        m_prefix=0;
        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }

    virtual ~Source() {}
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r, long flags=ICON_SIZE_LARGE) { 
        return r->object->getIcon(flags);
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        // empty result may not have an object
        if(sr->object)
            sr->object->drawItem(g,sr,r);
    }
    virtual void drawListItem(Graphics &g, DRAWITEMSTRUCT *dis, RectF &r) {
        if(!dis->itemData)
            return;

        SourceResult *sr=(SourceResult*)dis->itemData;
        if(sr->object)
            sr->object->drawListItem(g,dis,r);
    }
    // get results
    // fuse index and bonus from the db
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(FuzzyMatch(it->second.display,q)) {
                results.push_back(it->second);
                Object *o=new Object(it->first,type,this,it->second.display);
                o->values[L"expand"]=it->second.expand;
                o->values[L"iconname"]=it->second.iconname;
                results.back().object=o;
            }
        }
    }    
    virtual void validate(SourceResult *r)  {}
    virtual void crawl() {}
    // copy makes a deep copy
    virtual void copy(const SourceResult &r, SourceResult *out) {
        *out=r;
        if(r.icon)
            out->icon=r.icon->Clone(0,0,r.icon->GetWidth(),r.icon->GetHeight(),r.icon->GetPixelFormat());
        if(r.smallicon)
            out->smallicon=r.smallicon->Clone(0,0,r.smallicon->GetWidth(),r.smallicon->GetHeight(),r.smallicon->GetPixelFormat());
        if(r.object)
            out->object=r.object->clone();
    }
    virtual void clear(SourceResult &r) {
        delete r.icon; r.icon=0;
        delete r.smallicon; r.smallicon=0;
        delete r.object;
    }

    // unused yet
    // get named data of various types
    virtual Source *getSource(SourceResult &sr, CString &q) { return 0; }
    virtual CString getString(SourceResult &sr, const TCHAR *val) { 
        return sr.object->getString(val);
    }
    virtual int getInt(const TCHAR *itemquery) { return false; }

    virtual void rate(SourceResult *r) {}

    TCHAR                           m_prefix;
    StringFormat                    sfitemlist;
    Gdiplus::Font                   itemlistFont;
    Gdiplus::Font                   itemscoreFont;
    bool                            m_ignoreemptyquery;
    CString                         m_name;
    CString                         type;
    std::map<CString, SourceResult> m_index;
    std::vector<SourceResult>      *m_pArgs;
    UI                             *m_pUI;
    int                             m_refreshPeriod;
    CString                         m_icon;
};

