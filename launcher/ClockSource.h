/*
struct ClockSource : Source {
    ClockSource() : Source(L"CLOCK",L"Clock (Catalog )") {
        m_icon=L"icons\\clock.png";

        m_ignoreemptyquery=true;
        m_index[L"Clock"]=SourceResult(L"Clock", L"Clock", L"Clock", this, 0, 0, 0);
        m_refreshPeriod=1000;
        angle=0;
        hours=Gdiplus::Bitmap::FromFile(L"icons\\hours.png");
        minutes=Gdiplus::Bitmap::FromFile(L"icons\\minutes.png");
        seconds=Gdiplus::Bitmap::FromFile(L"icons\\seconds.png");

        
        HMODULE gdiplus=GetModuleHandle(L"gdiplus.dll");
        m_hasGdipDrawImageFX = !!GetProcAddress(gdiplus, "GdipDrawImageFX");
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        bool h=(float(r.Width)/r.Height)>2;
        RectF ricon=getStdIconPos(r,h);
        
        if(sr->icon)
            g.DrawImage(sr->icon, ricon);

        SYSTEMTIME st;
        GetSystemTime(&st);
        
        Gdiplus::Matrix m;
        
        if(m_hasGdipDrawImageFX) {
            int w=int(ricon.Width/10); // thickness of needles

            m.Reset();
            m.Translate(ricon.X+ricon.Width/2,ricon.Y+ricon.Height/2);
            m.Rotate(((float)st.wHour+(float)st.wMinute/60)/24*360+180);
            m.Translate(float(-w/2),float(-w/2));
            g.DrawImage(hours, &RectF(0.0f, 0.0f, float(w), float(ricon.Width/2)), &m, 0, 0, Gdiplus::UnitPixel);

            m.Reset();
            m.Translate(ricon.X+ricon.Width/2,r.Y+ricon.Height/2);
            m.Rotate(((float)st.wMinute+(float)st.wSecond/60)/60*360+180);
            m.Translate(float(-w/2),float(-w/2));
            g.DrawImage(minutes, &RectF(0.0f, 0.0f, float(w), float(ricon.Width/2)), &m, 0, 0, Gdiplus::UnitPixel);

            m.Reset();
            m.Translate(float(ricon.X+ricon.Width/2),float(ricon.Y+ricon.Height/2));
            m.Rotate((float)st.wSecond/60*360+180);
            m.Translate(float(-w/4),float(-w/2));
            g.DrawImage(seconds, &RectF(0.0f, 0.0f, float(w), float(ricon.Width/2)), &m, 0, 0, Gdiplus::UnitPixel);
        }

        StringFormat sfcenter;
        sfcenter.SetAlignment(getStdAlignment(h));    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(GetSettingsString(L"general",L"font",L"Arial"), 8.0f);
        //g.DrawString(sr->display, sr->display.GetLength(), &f, RectF(r.X, r.Y+r.Height-15, r.Width, 20), &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
        drawEmphased(g, sr->display, m_pUI->getQuery(), getStdTextPos(r,h,f.GetHeight(&g)),DE_UNDERLINE,getStdAlignment(h));
    }

    bool    m_hasGdipDrawImageFX;
    int     angle;
    Bitmap *hours;
    Bitmap *minutes;
    Bitmap *seconds;
};

struct ClockRule : Rule {
    ClockRule() {}
    bool execute(std::vector<RuleArg> &args) {
        return true;
    }
};
*/