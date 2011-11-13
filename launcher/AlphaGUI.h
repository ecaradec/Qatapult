#include "launcherdlg.h"
#include "Rule.h"

struct IWindowlessGUI {
    virtual ~IWindowlessGUI() {}
    virtual void Invalidate() = 0;
    virtual void OnQueryChange(const CString &text) = 0;    
};

struct WindowlessInput {
    WindowlessInput(IWindowlessGUI *p):m_pParent(p) {
        m_text=L"";
    }
    void Draw(HDC hdc) {
        Gdiplus::Graphics g(hdc);
        Gdiplus::Font f(L"Arial", 10.0f);
        Gdiplus::StringFormat sf;
        sf.SetAlignment(Gdiplus::StringAlignmentCenter);
        g.DrawString(m_text, m_text.GetLength(), &f, Gdiplus::RectF(10,176, 350, 20), &sf, &Gdiplus::SolidBrush(Gdiplus::Color(0xFFFFFFFF)));
        // g.DrawDriverString((UINT16*)CStringW(txt).GetBuffer(),txt.GetLength(),&m_gdipFont,&b,pos,DriverStringOptionsCmapLookup,&m);
    }
    void OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam) {
        if(msg==WM_CHAR) {            
            if(wparam==8)
                m_text=m_text.Left(m_text.GetLength() -1 );
            else if(wparam<VK_SPACE)
                return;
            else
                m_text.AppendChar(wparam);

            m_pParent->OnQueryChange(m_text);
        }

        m_pParent->Invalidate();
    }
    void SetText(const CString &txt) {
        m_text=txt;
        m_pParent->OnQueryChange(m_text);
        m_pParent->Invalidate();
    }
    IWindowlessGUI  *m_pParent;
    CString          m_text;
};

// SHGetImageList
struct AlphaGUI : IWindowlessGUI, KeyHook {
    AlphaGUI():m_input(this),m_dlg(this) {
        //WNDCLASS wnd={0};
        //wnd.style=0;
        //wnd.lpfnWndProc=::DefWindowProc;
        //wnd.lpfnWndProc=(WNDPROC)AlphaGUI::_WndProc;
        //wnd.lpszClassName=L"GUI";            
        //ATOM clss=RegisterClass(&wnd);
        m_dlg.m_pKH=this;
        icon=0;
        actionicon=0;
        m_dlg.Create(ClauncherDlg::IDD);
        m_dlg.ShowWindow(SW_HIDE);
        m_hwnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"", WS_VISIBLE|WS_POPUP|WS_CHILD, 0, 0, 0, 0, m_dlg.GetSafeHwnd(), 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);

        m_img.Load(L"..\\background.png");
        PremultAlpha(m_img);

        m_focus.Load(L"..\\focus.png");
        PremultAlpha(m_focus);

        Update();        

        SetWindowPos(m_dlg.GetSafeHwnd(), 0, 0, 200, 0, 0, SWP_NOSIZE);

        RegisterHotKey(m_hwnd, 1, MOD_SHIFT, VK_SPACE);

        //SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOSIZE);
    }
    void Invalidate() {
        Update();
    }
    void OnQueryChange(const CString &q) {            
        m_dlg.m_resultsWnd.ResetContent();
        m_dlg.m_results.clear();
        m_dlg.m_rules.back()->collect(q, m_dlg.m_results);
        for(int i=0;i<m_dlg.m_results.size();i++) {
            int id=m_dlg.m_resultsWnd.AddString(m_dlg.m_results[i].display);
            m_dlg.m_resultsWnd.SetItemDataPtr(id, &m_dlg.m_results[i]); // ok if we don't add anything to results later
        }
        
        if(m_dlg.m_results.size()>0) {
            OnSelChange(&m_dlg.m_results.front());
        }
    }
    void OnSelChange(RuleResult *r) {
        // should we detect the change from the focus or from the rules ????
        if(r->expandStr==lastResultExpandStr)
            return;
        lastResultExpandStr=r->expandStr;
        Gdiplus::Bitmap *bmp=r->rule->getIcon(r);
        if(m_dlg.m_rules.size()==1) {
            objecttext=r->display;
            if(bmp) {
                delete icon;
                icon=bmp;
                PremultAlpha(*icon);
            }

        } if(m_dlg.m_rules.size()==2) {
            actiontext=r->display;
            if(bmp) {                
                delete actionicon;
                actionicon=bmp;
                PremultAlpha(*actionicon);
            }
        }        
    }
    void PremultAlpha(CImage &img) {
        for(int y=0;y<img.GetHeight(); y++) 
            for(int x=0;x<img.GetWidth(); x++) {
                DWORD c=*(DWORD*)img.GetPixelAddress(x,y);            
                DWORD r=((c&0xFF));
                DWORD g=((c&0xFF00)>>8);
                DWORD b=((c&0xFF0000)>>16);
                DWORD a=((c&0xFF000000)>>24);
            
                DWORD R=int(r*a)>>8;
                DWORD G=int(g*a)>>8;
                DWORD B=int(b*a)>>8;
                DWORD A=int(a);

                DWORD cm=R + (G<<8) + (B<<16) + (A<<24);
                *(DWORD*)img.GetPixelAddress(x,y) = cm;
            }
    }
    void PremultAlpha(Gdiplus::Bitmap &img) {
        for(int y=0;y<img.GetHeight(); y++) 
            for(int x=0;x<img.GetWidth(); x++) {
                DWORD c;
                img.GetPixel(x,y,(Gdiplus::Color*)&c);
                
                DWORD r=((c&0xFF));
                DWORD g=((c&0xFF00)>>8);
                DWORD b=((c&0xFF0000)>>16);
                DWORD a=((c&0xFF000000)>>24);
            
                DWORD R=int(r*a)>>8;
                DWORD G=int(g*a)>>8;
                DWORD B=int(b*a)>>8;
                DWORD A=int(a);

                DWORD cm=R + (G<<8) + (B<<16) + (A<<24);
                img.SetPixel(x,y,Gdiplus::Color(cm));
                //*(DWORD*)img.GetPixelAddress(x,y) = cm;
            }
    }
    RuleResult *GetSelectedItem() {        
        int sel=m_dlg.m_resultsWnd.GetCaretIndex();
        if(sel!=-1)
            return (RuleResult*)m_dlg.m_resultsWnd.GetItemDataPtr(sel);

        if(m_dlg.m_results.size()!=0) {            
            return &m_dlg.m_results.front();
        }
        
        return 0;
    }
    void Update() {
        CImage premult;
        premult.Create(m_img.GetWidth(), m_img.GetHeight(), 32, CImage::createAlphaChannel);

        HDC hdc=premult.GetDC();

        Gdiplus::Graphics g(hdc);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);

        Gdiplus::StringFormat sfcenter;
        sfcenter.SetAlignment(Gdiplus::StringAlignmentCenter);    
        sfcenter.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(L"Arial", 8.0f);

        m_img.AlphaBlend(hdc, 0, 0);
        m_input.Draw(hdc);
        
        // draw icon on screen
        m_focus.AlphaBlend(hdc, 22, 22);
        if(icon)
            g.DrawImage(icon, Gdiplus::RectF(33,28,128,128));
           
        g.DrawString(objecttext, objecttext.GetLength(), &f, Gdiplus::RectF(22, 154, 150, 20), &sfcenter, &Gdiplus::SolidBrush(Gdiplus::Color(0xFFFFFFFF)));

        // draw action icon on screen        
        m_focus.AlphaBlend(hdc, 179, 22);
        if(actionicon)
            g.DrawImage(actionicon, Gdiplus::RectF(179+11,28,128,128));

        g.DrawString(actiontext, actiontext.GetLength(), &f, Gdiplus::RectF(179, 154, 150, 20), &sfcenter, &Gdiplus::SolidBrush(Gdiplus::Color(0xFFFFFFFF)));

        premult.ReleaseDC();

        hdc=premult.GetDC();

        POINT p1={1680/2-350/2,200};
        POINT p2={0};
        SIZE s={m_img.GetWidth(), m_img.GetHeight()};
        BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, hdc, &p2, 0, &bf, ULW_ALPHA);
        //g.ReleaseHDC(hdc);
        premult.ReleaseDC();
    }
    LRESULT OnKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if(msg == WM_KEYDOWN && wParam == VK_RETURN)
        {	
            Rule *r=m_dlg.m_rules.back()->validate();
            if(r==0) {
                ShowWindow(m_hwnd, SW_HIDE);
                m_dlg.ShowWindow(SW_HIDE);                
                
                while(m_dlg.m_rules.size()>1) { // keep the root rule
                    delete m_dlg.m_rules.back();
                    m_dlg.m_rules.pop_back();
                    //return FALSE;
                }

                m_input.SetText(L"");

                return FALSE;
            }

            m_dlg.m_rules.push_back(r);
            m_input.SetText(r->defaultQuery);
            
            return FALSE;
        }	
        else if(msg == WM_KEYDOWN && wParam == VK_ESCAPE)
        {
            if(m_dlg.m_rules.size()>1) { // keep the root rule
                delete m_dlg.m_rules.back();
                m_dlg.m_rules.pop_back();
                actiontext=L"";
                delete actionicon;
                actionicon=0;
                //return FALSE;
            }
        
            Rule *r=m_dlg.m_rules.back();
            m_input.SetText(r->defaultQuery);
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_TAB)
        {        
            RuleResult *r=m_dlg.GetSelectedItem();
            m_input.SetText(r->expandStr);

            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_DOWN)
        {	
            CRect r;
            GetWindowRect(m_hwnd, &r);
            m_dlg.ShowWindow(SW_SHOW);
            m_dlg.SetWindowPos(0, r.left, r.bottom, 0, 0, SWP_NOSIZE);
            m_dlg.SetFocus();
            
            return FALSE;
        }

        m_input.OnWindowMessage(msg,wParam,lParam);
        return TRUE;
    }
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {   
        if(!OnKeyboardMessage(msg,wParam,lParam))
            return FALSE;

        if(msg==WM_HOTKEY && wParam==1) {
            if(IsWindowVisible(m_hwnd)) {
                ShowWindow(m_hwnd, SW_HIDE);
                m_dlg.ShowWindow(SW_HIDE);
            } else {
                ShowWindow(m_hwnd, SW_SHOW);
                //m_dlg.ShowWindow(SW_SHOW);
            }
        }
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    static LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
       
        return ((AlphaGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->WndProc(hwnd,msg,wParam,lParam);
    }

    WindowlessInput    m_input;
    HWND               m_hwnd;
    CImage             m_img;
    CImage             m_focus;
    CImage             m_runicon;

    CString            lastResultExpandStr;

    Gdiplus::Bitmap   *actionicon;
    CString            actiontext;

    Gdiplus::Bitmap   *icon;
    CString            objecttext;

    HICON              hicon;

    ClauncherDlg       m_dlg;
};