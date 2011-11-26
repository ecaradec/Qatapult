#include "launcherdlg.h"
#include "Source.h"
#include "FileSource.h"
#include "StartMenuSource.h"
#include "FileVerbSource.h"
#include "EmailVerbSource.h"
#include "IWindowlessGUI.h"
#include "WindowlessInput.h"

// TODO
// [_] filtering outside of commands (not sure this is a good thing, doesn't work nicely for files
// [_] shortcuts are considered files (this is annoying with the tab completion because it expand to whole path )
// [_] add an index for the file source with a set of default folders
// [_] add an index for the startmenu source
// [_] add usage info in the index for ranking purpose
// [_] SourceResult should contains an map to store it's data probably, would solve most problems

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
    int match(std::vector<SourceResult> &args, int l) {
        uint i; 
        for(i=0;i<args.size();i++) {
            if(args[i].source && args[i].source->type!=m_types[i])
                break;
        }

        // if the command match everything in the buffer or more : perfect match
        // => if l is large enough a perfect match is the only possibility
        if(i>=m_types.size())
            return 2;

        // if the command match at least as much as the required length : partial match
        if(i>=l)
            return 1;

        return 0;
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
    TextSource() : Source(L"TEXT") {
        load();
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &args, std::vector<SourceResult> &r, int def) {
        if(CString(query).Find(L'\'')==0) {
            r.push_back(SourceResult(L"", CString(query).Mid(1), CString(query).Mid(1), this, 1, 0));            
        } else if(args.size()!=0) {
            r.push_back(SourceResult(L"", query, query, this, 0, 0));
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

struct EmailVerbRule : Rule {
    EmailVerbRule() : Rule(L"TEXT", L"EMAILVERB",L"CONTACT") {}
    virtual bool execute(std::vector<SourceResult> &args) { 
        return true;
    }
};

struct WebsiteSource : Source {
    WebsiteSource() : Source(L"WEBSITE") {
        m_index[L"Google"]=SourceResult(L"Google", L"Google", L"Google", this, 0, 0);
        m_index[L"Amazon"]=SourceResult(L"Amazon", L"Amazon", L"Amazon", this, 1, 0);
        load();
    }    
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r) { 
        return Gdiplus::Bitmap::FromFile(L"..\\icons\\"+r->display+".png");
    }
};

struct SearchWithVerbSource : Source {
    SearchWithVerbSource() : Source(L"SEARCHWITHVERB") {
        m_index[L"Search With"]=SourceResult(L"Search With", L"Search With", L"Search With", this, 0, 0);
        load();
    }
};

struct ContactSource : Source {
    ContactSource() : Source(L"CONTACT") {
        m_index[L"Emmanuel Caradec"]=SourceResult(L"Emmanuel Caradec", L"Emmanuel Caradec", L"Emmanuel Caradec", this, 0, 0);
        m_index[L"Iris Bourret"]=SourceResult(L"Iris Bourret", L"Iris Bourret", L"Iris Bourret", this, 1, 0);
        load();
    }
};

struct WebSearchRule : Rule {
    WebSearchRule() : Rule(L"TEXT", L"SEARCHWITHVERB", L"WEBSITE") {}
    virtual bool execute(std::vector<SourceResult> &args) {
        if(args[2].display==L"Amazon") {
            CString q(L"http://www.amazon.fr/s/field-keywords=%q"); q.Replace(L"%q", args[0].display);            
            ShellExecute(0, 0, q, 0, 0, SW_SHOWDEFAULT);
        } else if(args[2].display==L"Google") {
            CString q(L"http://www.google.fr/search?q=%q"); q.Replace(L"%q", args[0].display);            
            ShellExecute(0, 0, q, 0, 0, SW_SHOWDEFAULT);
        }        

        return true;
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

        m_background3.Load(L"..\\background3.png");
        PremultAlpha(m_background3);

        m_focus.Load(L"..\\focus.png");
        PremultAlpha(m_focus);

        premult.Create(m_background3.GetWidth(), m_background.GetHeight(), 32, CImage::createAlphaChannel);

        Invalidate();        

        //CRect r;
        //GetWindowRect(m_hwnd, &r);
        //m_dlg.SetWindowPos(0, r.left, r.bottom, 0, 0, SWP_NOSIZE);

        RegisterHotKey(m_hwnd, 1, MOD_SHIFT, VK_SPACE);

        // sources        
        m_sources[L"FILE"].push_back(new FileSource);
        m_sources[L"FILE"].push_back(new StartMenuSource);
        m_sources[L"TEXT"].push_back(new TextSource);
        m_sources[L"CONTACT"].push_back(new ContactSource);
        
        // rules & custom sources
        m_sources[L"EMAILVERB"].push_back(new EmailVerbSource);        
        m_rules.push_back(new EmailVerbRule);

        m_sources[L"FILEVERB"].push_back(new FileVerbSource);
        m_rules.push_back(new FileVerbRule);        

        m_sources[L"WEBSITE"].push_back(new WebsiteSource);   
        m_sources[L"SEARCHWITHVERB"].push_back(new SearchWithVerbSource);        
        m_rules.push_back(new WebSearchRule);
        

        OnQueryChange(L"");
        
        // verb sources
        //m_sources[L"FILEVERB"]=new FileVerbSource;
        //m_sources[L"HISTORY"]=new HistorySource;

        //SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOSIZE);
    }
    void Invalidate() {
        Update();
    }
    void CollectItems(const CString &q, const uint pane, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int def) {
        // collect all active rules (match could have an args that tell how much to match )
        // i should probably ignore the current pane for the match or just match until pane-1 ?
        std::vector<Rule *> activerules;
        for(uint i=0;i<m_rules.size();i++)
            if(m_rules[i]->match(args, pane)>0)
               activerules.push_back(m_rules[i]); 

        // collect all active sources at this level
        std::map<CString,bool> activesources;
        for(uint i=0;i<activerules.size();i++)
            if(activerules[i]->m_types.size()>pane)
                activesources[activerules[i]->m_types[pane]]=true;

        // collect displayable items
        results.clear();        
        for(std::map<CString,bool>::iterator it=activesources.begin(); it!=activesources.end(); it++)
            for(uint i=0;i<m_sources[it->first].size();i++)
                m_sources[it->first][i]->collect(q, args, results, def);

        CString Q(q); Q.MakeUpper();
        for(uint i=0;i<results.size();i++) {
            // pourcentage de chaine correspondante ?
            results[i].rank = 100*float(Q.GetLength())/ results[i].display.GetLength() + results[i].bonus;
            results[i].source->rate(&results[i]);
        }
    }

    static int ResultSourceCmp(SourceResult &r1, SourceResult &r2) {
        return r1.rank > r2.rank;
    }
    void OnQueryChange(const CString &q) {
        m_dlg.m_resultsWnd.ResetContent();
        CollectItems(q, m_pane, m_args, m_results, 0);        
                
        std::sort(m_results.begin(), m_results.end(), ResultSourceCmp);

        for(uint i=0;i<m_results.size();i++) {
            CString s; s.Format(L"%d %s", m_results[i].rank, m_results[i].display);
            int id=m_dlg.m_resultsWnd.AddString(s);
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
            // remove all args beyond current pane
            while(m_pane+1<m_args.size())
                m_args.pop_back();

            std::vector<SourceResult> results;
            CollectItems(L"", m_pane+1, m_args, results, 1);
            std::sort(results.begin(), results.end(), ResultSourceCmp);
            if(results.size()!=0) {
                SetArg(m_pane+1,results.front()); 
            }
        }
    }
    void SetArg(uint pane, SourceResult &r) {
        if(m_args.size() < pane && m_args[pane].icon!=0) {
            delete m_args[pane].icon;
            m_args[pane].icon=0;
        }
        if(pane==m_args.size())
            m_args.push_back(r);
        else
            m_args[pane]=r;
    }
    void OnSelChange(SourceResult *r) {        
        if(m_args.size()==0)
            m_args.push_back(SourceResult());
        
        // a copy is not enough if there is deep data because the results are cleaned after the query
        SetArg(m_pane, *r);

        ShowNextArg();

        Invalidate();

        // history is nice but add too much complexity right now
        /*std::map<CString, std::vector<SourceResult> >::iterator itH=g_history.find(r->expandStr);
        if(itH!=g_history.end())
            m_args=itH->second;*/
    }
    void Update() {        
        // load icons if they aren't 
        // that means that the only element that may have an icon are in m_args
        for(uint i=0;i<m_args.size();i++) {
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
        g.SetInterpolationMode(InterpolationModeHighQuality);
        g.SetCompositingQuality(CompositingQualityHighQuality);

        g.Clear(Gdiplus::Color(0,0,0,0));

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);        

        int w;
        if(m_args.size()==3) {
            m_background3.AlphaBlend(hdc, 0, 0);
            w=m_background3.GetWidth();
        } else {
            m_background.AlphaBlend(hdc, 0, 0);
            w=m_background.GetWidth();
        }

        m_input.Draw(hdc, RectF(0,178, REAL(w), 20));
        
        // draw icon on screen        
        m_focus.AlphaBlend(hdc, 22+157*0, 22);

        m_focus.AlphaBlend(hdc, 22+157*1, 22);

        if(m_args.size()==3)
            m_focus.AlphaBlend(hdc, 22+157*2, 22);

        m_focus.AlphaBlend(hdc, 22+157*m_pane, 22);

        for(uint i=0;i<m_args.size(); i++) {
            m_args[i].source->drawItem(g, &m_args[i], RectF(22+157*REAL(i), 22, 150, 154));
        }

        POINT p1={1680/2-w/2,200};
        POINT p2={0};
        SIZE s={w, m_background.GetHeight()};
        BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, hdc, &p2, 0, &bf, ULW_ALPHA);
        
        premult.ReleaseDC();


        CRect r;
        GetWindowRect(m_hwnd, &r);
        /*m_dlg.ShowWindow(SW_SHOW);*/
        m_dlg.SetWindowPos(0, r.left+22+157*m_pane, r.bottom, 0, 0, SWP_NOSIZE);
    }
    LRESULT OnKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam) {        
        if(msg == WM_KEYDOWN && wParam == VK_RETURN)
        {
            SourceResult *r=m_dlg.GetSelectedItem();
            SetArg(m_pane, *r);

            // collect all active rules
            // might need to disambiguate here ?
            for(uint i=0;i<m_rules.size();i++)
                if(m_rules[i]->match(m_args, m_args.size())>1) {

                    for(uint a=0;a<m_args.size(); a++) {
                        m_args[a].source->validate(&m_args[a]);
                    }

                    if(m_rules[i]->execute(m_args)) {                        
                        // found one rule
                        ShowWindow(m_hwnd, SW_HIDE);
                        m_dlg.ShowWindow(SW_HIDE);

                        m_results.clear();
                        m_args.clear();
                        m_pane=0;
                        m_input.SetText(L"");
                        m_queries.clear();
                        return FALSE;
                    }
                }

            /*
            // return is the way to run commands
            // left and right are the way to navigate command
            ShowNextArg();
            m_args.push_back(SourceResult());
            m_queries.push_back(m_input.m_text);
            if(m_pane<m_args.size())
                m_pane++;

            m_input.SetText(L"");*/
            return FALSE;
        }	
        else if(msg == WM_KEYDOWN && wParam == VK_ESCAPE)
        {
            if(m_dlg.IsWindowVisible()) {
                CRect r;
                GetWindowRect(m_hwnd, &r);
                m_dlg.ShowWindow(SW_HIDE);
            }

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
            /*if(m_args.size()==0)
                return FALSE;*/

            /*std::vector<SourceResult> results;
            CollectItems(L"", m_pane+1, m_args, results, 1);
            if(results.size()!=0) {
                m_pane++;
                m_queries.push_back(m_input.m_text);
                m_input.SetText(L"");
                return FALSE;
            }*/


            uint p=m_pane;
            if(p+1<m_args.size())
                m_pane++;

            if(p<m_args.size()) {
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
            if(m_pane==0)
                return FALSE;

            if(m_pane>0)
                m_pane--;
            m_input.SetText(m_queries[m_pane]);
            m_queries.pop_back();
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_DOWN)
        {	
            CRect r;
            GetWindowRect(m_hwnd, &r);
            m_dlg.ShowWindow(SW_SHOW);
            //m_dlg.SetWindowPos(0, r.left, r.bottom, 0, 0, SWP_NOSIZE);
            //m_dlg.SetFocus();
            
            return FALSE;
        }
        else if(msg == WM_CHAR)
        {
            m_input.OnWindowMessage(msg,wParam,lParam);
            return FALSE;
        }

        return TRUE;
    }
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {   
        if(!OnKeyboardMessage(msg,wParam,lParam))
            return S_OK;

        HWND h1,h2;
        if(msg==WM_HOTKEY && wParam==1) {
            
            if(IsWindowVisible(m_hwnd)) {
                ShowWindow(m_hwnd, SW_HIDE);
                m_dlg.ShowWindow(SW_HIDE);
            } else {
                ShowWindow(m_hwnd, SW_SHOW);
                h1=SetActiveWindow(m_hwnd);
                h2=SetFocus(m_hwnd);
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
        for(uint y=0;y<img.GetHeight(); y++) 
            for(uint x=0;x<img.GetWidth(); x++) {
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
    CImage             m_background3;
    CImage             m_focus;

    CString            lastResultExpandStr;

    // new behavior
    CImage                     premult;
    uint                       m_pane;
    std::map<CString, std::vector<Source*> > m_sources;
    std::vector<Rule*>         m_rules;
    std::vector<SourceResult>  m_args;     // validated results
    std::vector<SourceResult>  m_results;  // currently displayed results
    std::vector<CString>       m_queries;

    ClauncherDlg       m_dlg;
};