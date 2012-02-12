struct TextSource : Source {
    TextSource() : Source(L"TEXT") {
        m_ignoreemptyquery=true;
        m_prefix=L'\'';
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &r, int def) {
        int id=CString(query)[0]==_T('\''); // clear the ranking if this is prefixed
        r.push_back(SourceResult(query, query, query, this, id, 0, 0));
    }
    void rate(SourceResult *r) {
        if(r->id==0)
            r->rank=0;
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        RectF r1(r);
        r1.Y+=5;
        r1.X+=5;
        r1.Width-=10;
        r1.Height-=10;
        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentNear);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);
        Gdiplus::Font f(GetSettingsString(L"general",L"font",L"Arial"), 10.0f); 

        CString str(sr->display);
        if(str[0]==m_prefix)
            str=str.Mid(1);

        g.DrawString(str, -1, &f, r1, &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
    }
    CString getString(SourceResult &sr,const TCHAR *val_) {
        CString val(val_);
        CString str(sr.display);
        if(val==L"text") {            
            if(str[0]==m_prefix)
                str=str.Mid(1);
            return str;
        }
        return L"";
    }
};