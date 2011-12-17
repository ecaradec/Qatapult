struct TextSource : Source {
    TextSource() : Source(L"TEXT") {
        m_ignoreemptyquery=true;
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &r, int def) {
        if(CString(query).Find(L'\'')==0 || CString(query).Find(L'.')==0) {
            r.push_back(SourceResult(L"", CString(query).Mid(1), CString(query).Mid(1), this, 1, 0, 0));            
        } else if(m_pArgs->size()!=0) {
            r.push_back(SourceResult(L"", query, query, this, 0, 0, 0));
        }
    }
    void rate(SourceResult *r) {
        if(r->id==1)
            r->rank=100;
        else
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
        Gdiplus::Font f(L"Arial", 12.0f); 

        g.DrawString(sr->display, sr->display.GetLength(), &f, r1, &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
    }
};