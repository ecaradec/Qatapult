#pragma once

#include "md5.h"
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
    virtual CString getQuery() = 0;
    virtual void InvalidateIndex() = 0;
};

UI *g_pUI; // very lazy way to give access to the ui to the ui window proc

struct Source {
    Source(const CString& t)
        :itemlistFont(L"Arial", 8.0f, FontStyleBold, UnitPoint),
         itemscoreFont(L"Arial", 8.0f) {
        m_name=t;
        type=t;
        m_ignoreemptyquery=false;


        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }
    Source(const CString& t, const CString &n)
    :itemlistFont(L"Arial", 8.0f, FontStyleBold, UnitPoint),
    itemscoreFont(L"Arial", 8.0f){
        m_name=n;
        type=t;
        m_ignoreemptyquery=false;

        sfitemlist.SetTrimming(StringTrimmingEllipsisCharacter);
    }

    virtual ~Source() =0 {}
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r, long flags=ICON_SIZE_LARGE) { return 0; }
    // draw
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        if(sr->icon)
            g.DrawImage(sr->icon, RectF(r.X+10, r.Y+10, 128, 128));

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(L"Arial", 8.0f);
        g.DrawString(sr->display, sr->display.GetLength(), &f, RectF(r.X, r.Y+r.Height-15, r.Width, 20), &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
    }
    virtual void drawListItem(Graphics &g, SourceResult *sr, RectF &r) {
        g.FillRectangle(&SolidBrush(Color(0xFFFFFFFF)), r);

        if(!sr->smallicon)
            sr->smallicon=getIcon(sr,ICON_SIZE_SMALL);
        
        if(sr->smallicon)
            g.DrawImage(sr->smallicon, RectF(r.X+10, r.Y, r.Height, r.Height)); // height not a bug, think a minute
        
        g.DrawString(sr->display, sr->display.GetLength(), &itemlistFont, RectF(r.X+r.Height+5+10, r.Y+5, r.Width, r.Height), &sfitemlist, &SolidBrush(Color(0xFF000000)));
        g.DrawString(ItoS(sr->rank), -1, &itemscoreFont, RectF(r.X+r.Height+5+10, r.Y+25, r.Width, r.Height), &sfitemlist, &SolidBrush(Color(0xFF000000)));
    }
    // get results
    // fuse index and bonus from the db
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(CString(it->second.display).MakeUpper().Find(q)!=-1) {
                results.push_back(it->second);
            }
        }
    }
    
    void drawUnderlined(Graphics &g, CString text, CString query, RectF &rect, StringAlignment align=StringAlignmentCenter, float fontSize=10.0f) {
        int p=CString(text).MakeUpper().Find(CString(query).MakeUpper());
        int len=query.GetLength();

        Gdiplus::Font arial(L"Arial", fontSize);
        Gdiplus::Font arialUn(L"Arial", fontSize, Gdiplus::FontStyleUnderline);
        Gdiplus::StringFormat sfmt;  
        sfmt.SetFormatFlags(StringFormatFlagsNoClip);
        
        int             charRangeCount=3;

        Gdiplus::CharacterRange charRange[3] = { CharacterRange(0, p), CharacterRange(p, len), CharacterRange(p+len, text.GetLength() - (p + len)) };
        Gdiplus::Region         regions[3];
        Gdiplus::Font*          fonts[3]={&arial,&arialUn,&arial};        

        sfmt.SetMeasurableCharacterRanges(charRangeCount, charRange);        
        RectF rtmp(rect);
        rtmp.X=0;
        rtmp.Width=9999;
        g.MeasureCharacterRanges(text, -1, &arial, rtmp, &sfmt, charRangeCount, regions);
        
        PointF origin(0,0);
        RectF bbox;
        sfmt.SetAlignment(align);
        g.MeasureString(text, -1, &arial, rect, &sfmt, &bbox);

        Gdiplus::StringFormat sfmtDisp;
        sfmtDisp.SetTrimming(StringTrimmingEllipsisCharacter);
        
        float begin=bbox.X;
        float pos=begin;
        int y=0;
        const WCHAR *text2=text;
        for(int i=0;i<charRangeCount;i++) {
            Gdiplus::RectF r1;
            regions[i].GetBounds(&r1, &g);
            r1.X=pos;
            pos+=r1.Width;
            r1.Width+=20;
            r1.Width=min(r1.Width+r1.X, rect.X+rect.Width)-r1.X;
            //y+=20;
            //r1.Y=y;

            g.DrawString(text2, charRange[i].Length, fonts[i], r1, &sfmtDisp, &Gdiplus::SolidBrush(Gdiplus::Color(255,255,255)));            
            text2+=charRange[i].Length;
        }
    }

    virtual void validate(SourceResult *r)  {}
    virtual void crawl() {}

    // unused yet
    // get named data of various types
    virtual bool getSubResults(const TCHAR *query, const TCHAR *itemquery, std::vector<SourceResult> &results) { return false; }
    virtual CString getString(const TCHAR *itemquery) { return L""; }
    virtual int getInt(const TCHAR *itemquery) { return false; }

    virtual void release(SourceResult *r) {}
    virtual void rate(SourceResult *r) {}

    StringFormat                    sfitemlist;
    Gdiplus::Font                   itemlistFont;
    Gdiplus::Font                   itemscoreFont;
    bool                            m_ignoreemptyquery;
    CString                         m_name;
    CString                         type;
    std::map<CString, SourceResult> m_index;
    std::vector<SourceResult>      *m_pArgs;
};
