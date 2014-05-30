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
    virtual void onQueryChange(const CString &q, bool select) = 0;
    // settings dialog    
    virtual void invalidateIndex() = 0;
    virtual void reload() = 0;
    virtual void setCurrentSource(int pane, Source *s,CString &q) = 0;
    virtual void show() = 0;
    virtual int  getCurPane() = 0;
    virtual CString getArgString(int c, int e,const TCHAR *name) = 0;
    virtual int  getArgsCount() = 0;
};

#define DE_UNDERLINE 0
#define DE_COLOR 1
// this is probably very inefficient as as way to draw highlighted text because it handle proper hyphen in all cases
// gdiplus doesn't draw hyphen if the bbox is smaller than 2 hyphen => this is a workaround
void drawEmphased(Graphics &g, CString text, CString query, RectF &rect, int flags=DE_UNDERLINE, int from=0, StringAlignment align=StringAlignmentCenter, float fontSize=10.0f, DWORD color=0xFFFFFFFF);

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

struct KVPack;
struct Source {
    Source(const TCHAR* t) {
        m_refreshPeriod=0;
        m_name=t;
        m_ignoreemptyquery=false;
        m_prefix=0;
        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }
    Source(const TCHAR* t, const TCHAR *n) {
        //def=false;
        m_refreshPeriod=0;
        m_name=n;
        m_ignoreemptyquery=false;
        m_prefix=0;
        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }

    virtual ~Source() {
    }

    // get results
    // fuse index and bonus from the db
    virtual void collect(const TCHAR *query, KVPack &results, int def, std::map<CString,bool> &activetypes) {}
    virtual void validate(SourceResult *r)  {}
    virtual void crawl() {}

    // unused yet
    // get named data of various types
    virtual Source *getSubSource(Object *o, CString &q) {         
        return (Source*)_ttoi(o->getString(L"subsource"));
    }
    virtual int getInt(const TCHAR *itemquery) { return false; }

    virtual void rate(const CString &q, Object *r) {
        r->m_rank=0;
        if(m_prefix!=0 && r->getString(L"text")[0]==m_prefix)
            r->m_rank+=100;

        CString T(r->getString(L"text"));
        r->m_rank=min(100.0f,r->m_uses*5.0f) + r->m_bonus + r->m_rank+100.0f*evalMatch(T,q);
    }
    
    //int                             def;
    TCHAR                           m_prefix;
    StringFormat                    sfitemlist;
    bool                            m_ignoreemptyquery;
    CString                         m_name;
    std::map<CString, SourceResult> m_index;
    std::vector<SourceResult>      *m_pArgs;
    UI                             *m_pUI;
    int                             m_refreshPeriod;
    CString                         m_icon;
};

