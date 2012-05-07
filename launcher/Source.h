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

#include "Upgrade.h"

CString md5(const CString &data);
CStringA sqlEscapeString(const CStringA &args);
CStringW sqlEscapeStringW(const CStringW &args);

struct UI {
    virtual ~UI() {}
    virtual HWND getHWND() = 0;

    // plugins
    virtual int getFocus() = 0;
    virtual CString getQuery(int p) = 0;
    // settings dialog    
    virtual void invalidateIndex() = 0;
    virtual void reload() = 0;
    virtual void setCurrentSource(int pane, Source *s,CString &q) = 0;
    virtual void show() = 0;
    virtual int  getCurPane() = 0;
    virtual CString getArgString(int c, const TCHAR *name) = 0;
    virtual int  getArgsCount() = 0;
};

#define DE_UNDERLINE 0
#define DE_COLOR 1
// this is probably very inefficient as as way to draw highlighted text because it handle proper hyphen in all cases
// gdiplus doesn't draw hyphen if the bbox is smaller than 2 hyphen => this is a workaround
void drawEmphased(Graphics &g, CString text, CString query, RectF &rect, int flags=DE_UNDERLINE, StringAlignment align=StringAlignmentCenter, float fontSize=10.0f, DWORD color=0xFFFFFFFF);

extern UI *g_pUI; // very lazy way to give access to the ui to the ui window proc
extern CString g_fontfamily;
extern DWORD g_textcolor;

bool FuzzyMatch(const CString &w_,const CString &q_);
inline int levenshtein_distance(const char *s,const char*t);
// value of a match is the word length
// -1 if the match is not consecutive 
// ponderate by word length to get somthing that's between 0-1
inline float evalMatch(const CString &w_,const CString &q_) {
    CString W(w_); W.MakeUpper();
    CString Q(q_); Q.MakeUpper();

    if(Q.GetLength()==0 || W.GetLength()==0)
        return 0;
    int q=0;
    float score=0;
    int cbonus=0;//W.GetLength()/2; // consecutive match bonus    
    int i=0;
    int longuestMatch=0;
    bool cmatch=false;
    for(;i<W.GetLength()&&q!=Q.GetLength();i++) {
        if(Q[q]==W[i]) {
            if(cmatch)
                longuestMatch++;
            q+=1;
            score+=1;
            cmatch=true;
        } else {
            cmatch=false;
        }        
    }
    float f=float(score + longuestMatch)/(2*W.GetLength());    
    return f;
    
    /*if(Q.GetLength()==0)
        return 0;
    int d=levenshtein_distance(CStringA(W),CStringA(Q));
    return float( W.GetLength() - d ) / W.GetLength();*/
}


struct Source {
    Source(const TCHAR* t) {
        def=false;
        m_refreshPeriod=0;
        m_name=t;
        type=t;
        m_ignoreemptyquery=false;
        m_prefix=0;
        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }
    Source(const TCHAR* t, const TCHAR *n) {
        def=false;
        m_refreshPeriod=0;
        m_name=n;
        type=t;
        m_ignoreemptyquery=false;
        m_prefix=0;
        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }

    virtual ~Source() {
        //itemlistFont;
        //itemscoreFont;
    }
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r, long flags=ICON_SIZE_LARGE) { 
        return r->object()->getIcon(flags);
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        // empty result may not have an object
        if(sr->object())
            sr->object()->drawItem(g,sr,r);
    }
    // get results
    // fuse index and bonus from the db
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(FuzzyMatch(it->second.display(),q)) {
                results.push_back(it->second);
                Object *o=new Object(it->first,type,this,it->second.display());
                o->values[L"expand"]=it->second.expand();
                o->values[L"iconname"]=it->second.iconname();
                results.back().object()=o;
            }
        }
    }    
    virtual void validate(SourceResult *r)  {}
    virtual void crawl() {}
    // copy makes a deep copy
    virtual void copy(SourceResult &r, RuleArg *out) {
        //assert(r.m_results.size()==1);
        out->m_results.back()=r;
        
        if(r.icon())
            out->m_results.back().icon()=r.icon()->Clone(0,0,r.icon()->GetWidth(),r.icon()->GetHeight(),r.icon()->GetPixelFormat());
        if(r.smallicon())
            out->smallicon()=r.smallicon()->Clone(0,0,r.smallicon()->GetWidth(),r.smallicon()->GetHeight(),r.smallicon()->GetPixelFormat());
        if(r.object())
            out->object()=r.object()->clone();
    }
    virtual void clear(SourceResult &r) {
        // FIXME : we need a clearItem and a clear
        delete r.object(); r.object()=0;
        delete r.icon(); r.icon()=0;
        delete r.smallicon(); r.smallicon()=0;
    }

    // unused yet
    // get named data of various types
    virtual Source *getSource(SourceResult &sr, CString &q) { return 0; }
    virtual CString getString(SourceResult &sr, const TCHAR *val) { 
        return sr.object()->getString(val);
    }
    virtual int getInt(const TCHAR *itemquery) { return false; }

    virtual void rate(const CString &q, SourceResult *r) {
        r->rank()=0;
        if(m_prefix!=0 && r->display()[0]==m_prefix)
            r->rank()+=100;

        CString T(r->object()->getString(L"text"));
        r->rank()=min(100,r->uses()*5) + r->bonus() + r->rank()+100*evalMatch(T,q);
    }
    
    int                             def;
    TCHAR                           m_prefix;
    StringFormat                    sfitemlist;
    bool                            m_ignoreemptyquery;
    CString                         m_name;
    CString                         type;
    std::map<CString, SourceResult> m_index;
    std::vector<SourceResult>      *m_pArgs;
    UI                             *m_pUI;
    int                             m_refreshPeriod;
    CString                         m_icon;
};

