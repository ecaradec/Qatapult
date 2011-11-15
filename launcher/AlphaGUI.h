#include "launcherdlg.h"
#include "Rule.h"

using namespace Gdiplus;

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
        Graphics g(hdc);
        Gdiplus::Font f(L"Arial", 10.0f);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        g.DrawString(m_text, m_text.GetLength(), &f, RectF(10,176, 350, 20), &sf, &SolidBrush(Color(0xFFFFFFFF)));
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

// handle all context menu action
// split in *sources* and *rules*
/*struct Sources {
    collect(TCHAR *q);
};

struct ContextMenuRule {
    bool collect(TCHAR *query);
};
struct EmailToRule {
    bool collect(TCHAR *query);
};
struct EmailTextRule {
    bool collect(TCHAR *query);
};

for(r in rules)
    if(r.collect(q))
        saverule(r);

*/

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
        objecticon=0;
        actionicon=0;
        m_dlg.Create(ClauncherDlg::IDD);
        m_dlg.ShowWindow(SW_HIDE);
        m_hwnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"", WS_VISIBLE|WS_POPUP|WS_CHILD, 0, 0, 0, 0, m_dlg.GetSafeHwnd(), 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);

        m_background.Load(L"..\\background.png");
        PremultAlpha(m_background);

        m_focus.Load(L"..\\focus.png");
        PremultAlpha(m_focus);

        Update();        

        SetWindowPos(m_dlg.GetSafeHwnd(), 0, 0, 200, 0, 0, SWP_NOSIZE);

        RegisterHotKey(m_hwnd, 1, MOD_SHIFT, VK_SPACE);

        m_ruleresults.push_back(RuleResult());

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
        // TODO : should we detect the change from the focus or from the rules ????
        // TODO : should we exchange the current ruleresult on the top of the stack and validate it on the validation step ?
        // TODO : comment anticiper l'action ???

        if(r->expandStr==lastResultExpandStr)
            return;
        lastResultExpandStr=r->expandStr;
        r->icon=r->rule->getIcon(r);
        PremultAlpha(*r->icon);
        //if(m_ruleresults.size()==0)
        //    m_ruleresults.push_back(RuleResult());
        m_ruleresults.back()=*r;
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
        premult.Create(m_background.GetWidth(), m_background.GetHeight(), 32, CImage::createAlphaChannel);

        HDC hdc=premult.GetDC();

        Graphics g(hdc);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.SetCompositingQuality(CompositingQualityHighQuality);

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(L"Arial", 8.0f);

        m_background.AlphaBlend(hdc, 0, 0);
        m_input.Draw(hdc);
        
        // draw icon on screen
        if(m_ruleresults.size()>=1) {
            m_focus.AlphaBlend(hdc, 22, 22);
            if(m_ruleresults[0].icon)
                g.DrawImage(m_ruleresults[0].icon, RectF(33,28,128,128));
           
            g.DrawString(m_ruleresults[0].display, m_ruleresults[0].display.GetLength(), &f, RectF(22, 154, 150, 20), &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
        }

        // draw action icon on screen        
        if(m_ruleresults.size()>=2) {
            m_focus.AlphaBlend(hdc, 179, 22);
            if(m_ruleresults[1].icon)
                g.DrawImage(m_ruleresults[1].icon, RectF(179+11,28,128,128));

            g.DrawString(m_ruleresults[1].display, m_ruleresults[1].display.GetLength(), &f, RectF(179, 154, 150, 20), &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
        }


        POINT p1={1680/2-350/2,200};
        POINT p2={0};
        SIZE s={m_background.GetWidth(), m_background.GetHeight()};
        BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, hdc, &p2, 0, &bf, ULW_ALPHA);
        
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
                while(m_ruleresults.size()>1) { // keep the root rule
                    delete m_ruleresults.back().icon;
                    m_ruleresults.pop_back();                    
                    //return FALSE;
                }
                delete m_ruleresults[0].icon;
                m_ruleresults[0]=RuleResult();

                m_input.SetText(L"");

                return FALSE;
            }

            m_dlg.m_rules.push_back(r);
            m_ruleresults.push_back(RuleResult());
            m_input.SetText(r->defaultQuery);
            
            return FALSE;
        }	
        else if(msg == WM_KEYDOWN && wParam == VK_ESCAPE)
        {
            if(m_dlg.IsWindowVisible()) {
                m_dlg.ShowWindow(SW_HIDE);
                return FALSE;
            } else if(m_dlg.m_rules.size()>1) { // keep the root rule
                delete m_dlg.m_rules.back();
                m_dlg.m_rules.pop_back();
                m_ruleresults.pop_back();
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
    void PremultAlpha(Bitmap &img) {
        for(int y=0;y<img.GetHeight(); y++) 
            for(int x=0;x<img.GetWidth(); x++) {
                DWORD c;
                img.GetPixel(x,y,(Color*)&c);
                
                DWORD r=((c&0xFF));
                DWORD g=((c&0xFF00)>>8);
                DWORD b=((c&0xFF0000)>>16);
                DWORD a=((c&0xFF000000)>>24);
            
                DWORD R=int(r*a)>>8;
                DWORD G=int(g*a)>>8;
                DWORD B=int(b*a)>>8;
                DWORD A=int(a);

                DWORD cm=R + (G<<8) + (B<<16) + (A<<24);
                img.SetPixel(x,y,Color(cm));
                //*(DWORD*)img.GetPixelAddress(x,y) = cm;
            }
    }

    WindowlessInput    m_input;
    HWND               m_hwnd;
    CImage             m_background;
    CImage             m_focus;

    CString            lastResultExpandStr;

    Bitmap   *actionicon;
    CString            actiontext;

    Bitmap   *objecticon;
    CString            objecttext;

    std::vector<RuleResult> m_ruleresults;

    ClauncherDlg       m_dlg;
};