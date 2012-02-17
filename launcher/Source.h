#pragma once

#include "md5.h"

struct Source;

struct Info {
    Source                    *source;
    std::vector<SourceResult> *results;
};


 static int getResultsCB(void *NotUsed, int argc, char **argv, char **azColName) {
     Info *pinfo=(Info*)NotUsed;
     pinfo->results->push_back(SourceResult(UTF8toUTF16(argv[0]),         // key
                                            UTF8toUTF16(argv[1]),         // display
                                            UTF8toUTF16(argv[2]),          // expand
                                            pinfo->source,   // source
                                            atoi(argv[3]?argv[3]:"0"),               // id
                                            0,               // data
                                            atoi(argv[4]?argv[4]:"0"))); // bonus
    return 0;
}


static int getStringCB(void *NotUsed, int argc, char **argv, char **azColName) {
     *((CString*)NotUsed)=argv[0];
     //*((CString*)NotUsed)=UTF8toUTF16(argv[0]);
    return 0;
}

static int getIntCB(void *NotUsed, int argc, char **argv, char **azColName) {
     *((int*)NotUsed)=atoi(argv[0]?argv[0]:"0");
    return 0;
}


inline CString md5(const CString &data) {
    std::string data_str=CStringA(data);
    return CString(md5(data_str).c_str());
}

CStringA sqlEscapeString(const CStringA &args) {
    CStringA tmp(args);
    tmp.Replace("\"", "");
    return tmp;
}

CStringW sqlEscapeStringW(const CStringW &args) {
    CStringW tmp(args);
    tmp.Replace(L"\"", L"");
    return tmp;
}

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
};


#define DE_UNDERLINE 0
#define DE_COLOR 1
// this is probably very inefficient as as way to draw highlighted text because it handle proper hyphen in all cases
// gdiplus doesn't draw hyphen if the bbox is smaller than 2 hyphen => this is a workaround
void drawEmphased(Graphics &g, CString text, CString query, RectF &rect, int flags=DE_UNDERLINE, StringAlignment align=StringAlignmentCenter, float fontSize=10.0f) {       
    Gdiplus::Font hifont(GetSettingsString(L"general",L"font",L"Arial"), fontSize, Gdiplus::FontStyleUnderline);
    Gdiplus::Font lofont(GetSettingsString(L"general",L"font",L"Arial"), fontSize);
    Gdiplus::SolidBrush hibrush(Gdiplus::Color(255,255,255,255));
    Gdiplus::SolidBrush lobrush(Gdiplus::Color(128,255,255,255));


    Gdiplus::StringFormat sfmt(Gdiplus::StringFormat::GenericTypographic());  
    sfmt.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces|StringFormatFlagsNoWrap|StringFormatFlagsNoClip|StringFormatFlagsNoFitBlackBox);

    PointF origin(0,0);
    RectF strbbox;
    RectF rtmp(rect);
    rtmp.X=0;
    rtmp.Width=9999;    
    
    g.MeasureString(text, -1, &hifont, rtmp, &sfmt, &strbbox);
    
    RectF r1(rect);
    //r1.X+=10;
    //r1.Width-=20;

    float maxwidth = min(strbbox.Width, r1.Width);        

    CString cur;
    CString T(text); T.MakeUpper();
    CString Q(query); Q.MakeUpper();
    
    Gdiplus::StringFormat sfmtdraw(&sfmt);
    sfmtdraw.SetTrimming(StringTrimmingEllipsisCharacter);

    int y=0;
    if(align==StringAlignmentCenter)
        r1.X=r1.X+r1.Width/2-maxwidth/2;

    r1.Width=maxwidth;
    float maxright = r1.X+maxwidth;

    RectF hyphenbbox;
    g.MeasureString(L"…", -1, &hifont, PointF(0,0), &sfmt, &hyphenbbox);
    // gives a little bit of space so that the exact box used to fit the text doesn't cause an hyphen when there is still space
    // I should really measure one by one then center the resulting text
    maxright+=hyphenbbox.Width/2;
    int q=0;
    for(int i=0;i<text.GetLength();i++) {
        CString cur=text[i];

        RectF bbox;
        g.MeasureString(cur, -1, &hifont, PointF(0,0), &sfmtdraw, &bbox);
        if((r1.X+hyphenbbox.Width)>(rect.X+rect.Width) ) {
            g.DrawString(L"…", -1, &lofont, r1, &sfmtdraw, &hibrush);
            break;
        }
        
        bool same=T[i]==Q[q];
        if(same)
            q++;

        Brush *b=&hibrush;
        Font *f=&lofont;
        if(flags==DE_UNDERLINE)
            f=same?&hifont:&lofont;
        else
            b=same?&hibrush:&lobrush;
        
        g.DrawString(cur, -1, f, r1, &sfmtdraw, b);
        r1.X+=bbox.Width;
    }
}



UI *g_pUI; // very lazy way to give access to the ui to the ui window proc


bool FuzzyMatch(const CString &w_,const CString &q_) {
    CString W(w_); W.MakeUpper();
    CString Q(q_); Q.MakeUpper();
    int q=0;
    for(int i=0;i<W.GetLength()&&q!=Q.GetLength();i++) {
        if(Q[q]==W[i])
            q++;
    }
    return q==Q.GetLength();
}

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
        return Gdiplus::Bitmap::FromFile(r->iconname);
    }
    // draw
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        if(sr->icon)
            g.DrawImage(sr->icon, RectF(r.X+10, r.Y+10, 128, 128));

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(GetSettingsString(L"general",L"font",L"Arial"), 8.0f);
        drawEmphased(g, sr->display, m_pUI->getQuery(), RectF(r.X+10, r.Y+r.Height-17, r.Width-20, 20));        
    }
    virtual void drawListItem(Graphics &g, DRAWITEMSTRUCT *dis, RectF &r) {        
        if(dis->itemData==0)
            return;

        //if(dis->itemState&ODS_SELECTED)
        //    g.FillRectangle(&SolidBrush(Color(0xFFDDDDFF)), r);
        //else
        g.FillRectangle(&SolidBrush(Color(0xFFFFFFFF)), r);

        SourceResult *sr=(SourceResult*)dis->itemData;
        if(!sr->smallicon)
            sr->smallicon=getIcon(sr,ICON_SIZE_SMALL);
        
        if(sr->smallicon)
            g.DrawImage(sr->smallicon, RectF(r.X+10, r.Y, r.Height, r.Height)); // height not a bug, think a minute
        
        REAL x=r.X+r.Height+5+10;
        
        
        CString str(sr->display);
        if(str[0]==m_prefix)
            str=str.Mid(1);

        g.DrawString(str, -1, &itemlistFont, RectF(x, r.Y+5.0f, r.Width-x, 14.0f), &sfitemlist, &SolidBrush(Color(0xFF000000)));
        
        StringFormat sfscore;
        sfscore.SetAlignment(StringAlignmentNear);
        g.DrawString(ItoS(sr->rank), -1, &itemscoreFont, RectF(r.X+r.Height+5+10, r.Y+25, r.Width, r.Height), &sfscore, &SolidBrush(Color(0xFF000000)));

        Font pathfont(GetSettingsString(L"general",L"font",L"Arial"), 8.0f);
        StringFormat sfpath;
        sfpath.SetTrimming(StringTrimmingEllipsisPath);
        CString path(sr->source->getString(*sr,L"path"));
        path.TrimRight(L'\\');
        g.DrawString(path.Left(path.ReverseFind(L'\\')), -1, &pathfont, RectF(r.X+r.Height+5+40, r.Y+25, r.Width-(r.X+r.Height+5+40), 14), &sfpath, &SolidBrush(Color(0xBB000000)));
    }
    // get results
    // fuse index and bonus from the db
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(FuzzyMatch(it->second.display,q)) {
                results.push_back(it->second);
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
    }
    virtual void clear(SourceResult &r) {
        delete r.icon; r.icon=0;
        delete r.smallicon; r.smallicon=0;
    }

    // unused yet
    // get named data of various types
    virtual Source *getSource(SourceResult &sr, CString &q) { return 0; }
    virtual CString getString(SourceResult &sr, const TCHAR *val) { return L""; }
    virtual int getInt(const TCHAR *itemquery) { return false; }

    virtual void release(SourceResult *r) {}
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
};
