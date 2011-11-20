#include "launcherdlg.h"
#include "Rule.h"
#include "FileRule.h"
#include "StartMenuRule.h"
#include "FileVerbRule.h"
#include "IWindowlessGUI.h"
#include "WindowlessInput.h"
#include <map>

using namespace Gdiplus;

struct Rule {
    Rule(const CString &arg1type, const CString &verbtype) {
        m_types.push_back(arg1type);
        m_types.push_back(verbtype);
    }
    Rule(const CString &arg1type, const CString &verbtype, const CString &arg2type) {
        m_types.push_back(arg1type);
        m_types.push_back(verbtype);
        m_types.push_back(arg2type);
    }
    bool match(std::vector<SourceResult> &args, int l) {
        if(args.size()>m_types.size())
            return false;
        for(int i=0;i<min(l, args.size());i++)
            if(args[i].source && args[i].source->type!=m_types[i])
                return false;
        return true;
    }
    virtual bool execute(std::vector<SourceResult> &args) { return true; }

    std::vector<CString> m_types;
};

struct FileVerbRule : Rule {
    FileVerbRule() : Rule(L"FILE",L"FILEVERB") {}
    bool execute(std::vector<SourceResult> &args) {
        ProcessCMCommand((IContextMenu*)args[1].data, args[1].id);        
        return true;
    }
    HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, UINT idCmdOffset) {
       CMINVOKECOMMANDINFO ici;
       ZeroMemory(&ici, sizeof(ici));
       ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
       ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCmdOffset);
       ici.nShow = SW_SHOWNORMAL;

       return pCM->InvokeCommand(&ici);
    }
};

struct TextSource : Source {
    TextSource() {
        type=L"TEXT";
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &args, std::vector<SourceResult> &r, int def) {
        if(CString(query).Find(L'\'')==0) {
            r.push_back(SourceResult());
            r.back().expandStr=CString(query).Mid(1,99);
            r.back().display=CString(query).Mid(1,99);
            r.back().source=this;
        }
    }
};

std::map<CString, std::vector<SourceResult> > g_history;

// SHGetImageList
struct AlphaGUI : IWindowlessGUI, KeyHook {
    AlphaGUI():m_input(this),m_dlg(this) {
        //WNDCLASS wnd={0};
        //wnd.style=0;
        //wnd.lpfnWndProc=::DefWindowProc;
        //wnd.lpfnWndProc=(WNDPROC)AlphaGUI::_WndProc;
        //wnd.lpszClassName=L"GUI";            
        //ATOM clss=RegisterClass(&wnd);
        m_pane=0;

        m_dlg.m_pKH=this;
        m_dlg.Create(ClauncherDlg::IDD);
        m_dlg.ShowWindow(SW_HIDE);

        m_hwnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"", WS_VISIBLE|WS_POPUP|WS_CHILD, 0, 0, 0, 0, m_dlg.GetSafeHwnd(), 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);

        m_background.Load(L"..\\background.png");
        PremultAlpha(m_background);

        m_focus.Load(L"..\\focus.png");
        PremultAlpha(m_focus);

        premult.Create(m_background.GetWidth(), m_background.GetHeight(), 32, CImage::createAlphaChannel);

        Update();        

        //CRect r;
        //GetWindowRect(m_hwnd, &r);
        //m_dlg.SetWindowPos(0, r.left, r.bottom, 0, 0, SWP_NOSIZE);

        RegisterHotKey(m_hwnd, 1, MOD_SHIFT, VK_SPACE);

        // sources        
        m_sources[L"FILE"].push_back(new FileSource);
        m_sources[L"FILE"].push_back(new StartMenuSource);
        m_sources[L"FILEVERB"].push_back(new FileVerbSource);        
        m_sources[L"TEXT"].push_back(new TextSource);
        
        // rules
        m_rules.push_back(new Rule(L"TEXT", L"EMAILVERB"));
        m_rules.push_back(new FileVerbRule);        
        

        OnQueryChange(L"");
        
        // verb sources
        //m_sources[L"FILEVERB"]=new FileVerbSource;
        //m_sources[L"HISTORY"]=new HistorySource;

        //SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOSIZE);
    }
    void Invalidate() {
        Update();
    }
    void CollectItems(const CString &q, const int pane, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int def) {
        // collect all active rules (match could have an args that tell how much to match )
        // i should probably ignore the current pane for the match or just match until pane-1 ?
        std::vector<Rule *> activerules;
        for(int i=0;i<m_rules.size();i++)
            if(m_rules[i]->match(args, pane))
               activerules.push_back(m_rules[i]); 

        // collect all active sources at this level
        std::map<CString,bool> activesources;
        for(int i=0;i<activerules.size();i++)
            if(activerules[i]->m_types.size()>pane)
                activesources[activerules[i]->m_types[pane]]=true;

        // collect displayable items
        results.clear();        
        for(std::map<CString,bool>::iterator it=activesources.begin(); it!=activesources.end(); it++)
            for(int i=0;i<m_sources[it->first].size();i++)
                m_sources[it->first][i]->collect(q, args, results, def);    
    }
    void OnQueryChange(const CString &q) {
        m_dlg.m_resultsWnd.ResetContent();
        CollectItems(q, m_pane, m_args, m_results, 0);        
        for(int i=0;i<m_results.size();i++) {
            int id=m_dlg.m_resultsWnd.AddString(m_results[i].display);
            m_dlg.m_resultsWnd.SetItemDataPtr(id, &m_results[i]); // ok if we don't add anything to results later
        }

        if(m_results.size()>0) {
            OnSelChange(&m_results.front());
        }

        ShowNextArg();
    }
    void ShowNextArg() {
        // check if there is extra args     
        if(m_args.size()>0) {
            std::vector<SourceResult> results;
            CollectItems(L"", m_pane+1, m_args, results, 1);
            if(results.size()!=0) {
                if(m_pane+1==m_args.size())
                    m_args.push_back(results.front());
                else
                    SetArg(m_pane+1,results.front()); 
            }
            
            if(results.size()==0) {
                while(m_args.size()>m_pane+1)
                    m_args.pop_back();
            }
        }
    }
    void SetArg(int pane, SourceResult &r) {
        if(m_args.size() < pane && m_args[pane].icon!=0) {
            delete m_args[pane].icon;
            m_args[pane].icon=0;
        }
        m_args[pane]=r;
    }
    void OnSelChange(SourceResult *r) {        
        if(m_args.size()==0)
            m_args.push_back(SourceResult());
        
        // a copy is not enough if there is deep data because the results are cleaned after the query
        SetArg(m_pane, *r);

        ShowNextArg();

        // history is nice but add too much complexity right now
        /*std::map<CString, std::vector<SourceResult> >::iterator itH=g_history.find(r->expandStr);
        if(itH!=g_history.end())
            m_args=itH->second;*/
    }
    SourceResult *GetSelectedItem() {        
        /*int sel=m_dlg.m_resultsWnd.GetCaretIndex();
        if(sel!=-1)
            return (SourceResult*)m_dlg.m_resultsWnd.GetItemDataPtr(sel);

        if(m_dlg.m_results.size()!=0) {            
            return &m_dlg.m_results.front();
        }*/
        
        return 0;
    }
    void Update() {        
        // load icons if they aren't 
        // that means that the only element that may have an icon are in m_args
        for(int i=0;i<m_args.size();i++) {
            SourceResult *r=&m_args[i];
            if(r->icon==0 && r->source) {
                r->icon=r->source->getIcon(r);
                if(r->icon)
                    PremultAlpha(*r->icon);
            }
        }

        // get a buffer
        HDC hdc=premult.GetDC();

        Graphics g(hdc);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.SetCompositingQuality(CompositingQualityHighQuality);

        g.Clear(Gdiplus::Color(0,0,0,0));

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(L"Arial", 8.0f);

        m_background.AlphaBlend(hdc, 0, 0);
        m_input.Draw(hdc);
        
        // draw icon on screen        
        m_focus.AlphaBlend(hdc, 22+157*0, 22);

        m_focus.AlphaBlend(hdc, 22+157*1, 22);

        m_focus.AlphaBlend(hdc, 22+157*m_pane, 22);

        for(int i=0;i<m_args.size(); i++) {            
            if(m_args[0].icon)
                g.DrawImage(m_args[i].icon, RectF(33+157*i,28,128,128));
           
            Gdiplus::PointF positions[256];
            Gdiplus::Matrix m;
            Gdiplus::RectF bbox;
            g.DrawString(m_args[i].display, m_args[i].display.GetLength(), &f, RectF(22+157*i, 154, 150, 20), &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
        }

        POINT p1={1680/2-350/2,200};
        POINT p2={0};
        SIZE s={m_background.GetWidth(), m_background.GetHeight()};
        BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, hdc, &p2, 0, &bf, ULW_ALPHA);
        
        premult.ReleaseDC();


        /*CRect r;
        GetWindowRect(m_hwnd, &r);
        m_dlg.ShowWindow(SW_SHOW);
        m_dlg.SetWindowPos(0, r.left+22+157*m_pane, r.bottom, 0, 0, SWP_NOSIZE);*/
    }
    LRESULT OnKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if(msg == WM_KEYDOWN && wParam == VK_RETURN)
        {
            SourceResult *r=m_dlg.GetSelectedItem();
            SetArg(m_pane, *r);

            // collect all active rules
            // might need to disambiguate here ?
            for(int i=0;i<m_rules.size();i++)
                if(m_args.size()==m_rules[i]->m_types.size() && m_rules[i]->match(m_args, m_args.size()))
                    if(m_rules[i]->execute(m_args)) {
                        g_history[m_args.front().expandStr]=m_args;

                        // found one rule
                        ShowWindow(m_hwnd, SW_HIDE);
                        m_dlg.ShowWindow(SW_HIDE);

                        // there is a contextmenu leak here
                        // ask the sources to clean up their items here
                        /*while(m_results.size()>1) { // keep the root rule
                            delete m_results.back().icon;
                            m_results.pop_back();                    
                            //return FALSE;
                        }*/
                        m_results.clear();
                        m_args.clear();
                        m_pane=0;
                        m_input.SetText(L"");
                        m_queries.clear();
                        return FALSE;
                    }

            m_args.push_back(SourceResult());
            m_queries.push_back(m_input.m_text);
            if(m_pane<m_args.size())
                m_pane++;

            m_input.SetText(L"");
            return FALSE;
        }	
        else if(msg == WM_KEYDOWN && wParam == VK_ESCAPE)
        {                 
            if(m_pane>0) {                                
                m_input.SetText(m_queries.size()==0?L"":m_queries.back());
                m_pane--;
                m_args.pop_back();       
                m_queries.pop_back();

                ShowNextArg();
            }
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_TAB)
        {        
            SourceResult *r=m_dlg.GetSelectedItem();
            m_input.SetText(r->expandStr);            
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_RIGHT)
        {        
            if(m_args.size()==0)
                return FALSE;

            if(m_pane<m_args.size()-1)
                m_pane++;
            if(m_pane<m_args.size()) {
                m_queries.push_back(m_input.m_text);
                m_input.SetText(L"");
            } else {
                m_queries.push_back(L"");
                m_input.SetText(L"");
            }
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_LEFT)
        {        
            if(m_args.size()==0)
                return FALSE;

            if(m_pane>0)
                m_pane--;
            m_input.SetText(m_queries[m_pane]);
            
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
                SetFocus(m_hwnd);
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
    // old behavior
    WindowlessInput    m_input;
    HWND               m_hwnd;
    CImage             m_background;
    CImage             m_focus;

    CString            lastResultExpandStr;

    // new behavior
    CImage                     premult;
    int                        m_pane;
    std::map<CString, std::vector<Source*> > m_sources;
    std::vector<Rule*>         m_rules;
    std::vector<SourceResult>  m_args;     // validated results
    std::vector<SourceResult>  m_results;  // currently displayed results
    std::vector<CString>       m_queries;

    ClauncherDlg       m_dlg;
};