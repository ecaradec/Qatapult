#pragma once

struct TextObject : Object {
    TextObject(const CString &text,Source *s):Object(text,L"TEXT",s,text) {
    }
    TextObject *clone() {
        return new TextObject(*this);
    }
    void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        RectF r1(r);
        
        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentNear);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);
        Gdiplus::Font f(g_fontfamily, 10.0f); 

        CString str(sr->display);
        if(str[0]==source->m_prefix)
            str=str.Mid(1);

        r1.Height = int(r1.Height/f.GetHeight(&g))*f.GetHeight(&g);

        g.DrawString(str, -1, &f, r1, &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
    }
};