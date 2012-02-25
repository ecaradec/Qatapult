struct TextObject : Object {
    TextObject(const CString &text,Source *s):Object(text,s,text) {
    }
    TextObject *clone() {
        return new TextObject(*this);
    }
    void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        RectF r1(r);
        
        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentNear);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);
        Gdiplus::Font f(GetSettingsString(L"general",L"font",L"Arial"), 10.0f); 

        CString str(sr->display);
        if(str[0]==source->m_prefix)
            str=str.Mid(1);

        r1.Height = int(r1.Height/f.GetHeight(&g))*f.GetHeight(&g);

        g.DrawString(str, -1, &f, r1, &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
    }
};

struct TextSource : Source {
    TextSource() : Source(L"TEXT",L"Text (Catalog )") {
        m_ignoreemptyquery=true;
        m_prefix=L'\'';
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &r, int def) {
        int id=CString(query)[0]==_T('\''); // clear the ranking if this is prefixed
        r.push_back(SourceResult(query, query, query, this, id, 0, 0));
        r.back().object=new TextObject(query,this);
    }
    void rate(SourceResult *r) {
        if(r->id==0)
            r->rank=0;
    }
};