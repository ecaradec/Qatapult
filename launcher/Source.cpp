#include "stdafx.h"
#include "Source.h"

 /*int getResultsCB(void *NotUsed, int argc, char **argv, char **azColName) {
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
*/

int getStringCB(void *NotUsed, int argc, char **argv, char **azColName) {
     *((CString*)NotUsed)=argv[0];
     //*((CString*)NotUsed)=UTF8toUTF16(argv[0]);
    return 0;
}

int getIntCB(void *NotUsed, int argc, char **argv, char **azColName) {
     *((int*)NotUsed)=atoi(argv[0]?argv[0]:"0");
    return 0;
}


CString md5(const CString &data) {
    std::string data_str=CStringA(data);
    return CString(md5(data_str).c_str());
}

CStringA sqlEscapeString(const CStringA &args) {
    CStringA tmp(args);
    tmp.Replace("_","\\_");
    tmp.Replace("%","\\%");
    tmp.Replace("'","\\'");
    tmp.Replace("\"","\\\"");
    return tmp;
}

CStringW sqlEscapeStringW(const CStringW &args) {
    CStringW tmp(args);
    tmp.Replace(L"_",L"\\_");
    tmp.Replace(L"%",L"\\%");
    tmp.Replace(L"'",L"\\'");
    tmp.Replace(L"\"",L"\\\"");
    return tmp;
}

void drawEmphased(Graphics &g, CString text, CString query, RectF &rect, int flags, StringAlignment align, float fontSize, DWORD color) {       
    Gdiplus::Font hifont(g_fontfamily, fontSize, Gdiplus::FontStyleUnderline);
    Gdiplus::Font lofont(g_fontfamily, fontSize);
    Gdiplus::SolidBrush hibrush(Gdiplus::Color(color|0xFF000000));
    Gdiplus::SolidBrush lobrush(Gdiplus::Color((color&0x00FFFFFF)|0x88000000));

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
