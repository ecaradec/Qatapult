#include "SourceResult.h"
#include "Source.h"
#include "FileSource.h"
#include "StartMenuSource.h"
#include "FileVerbSource.h"
#include "EmailVerbSource.h"
#include "IWindowlessGUI.h"
#include "WindowlessInput.h"
#include "Rule.h"

// TODO
// [_] filtering outside of commands (not sure this is a good thing, doesn't work nicely for files
// [_] shortcuts are considered files (this is annoying with the tab completion because it expand to whole path )
// [_] add an index for the file source with a set of default folders
// [_] add an index for the startmenu source
// [_] add usage info in the index for ranking purpose
// [_] SourceResult should contains an map to store it's data probably, would solve most problems

struct FileVerbRule : Rule {
    FileVerbRule() : Rule(L"FILE",L"FILEVERB") {}
    bool execute(std::vector<SourceResult> &args) {
        CString path;
        (*m_pArgs)[0].source->getData(args[0].key, L"PATH", (char*)path.GetBufferSetLength(MAX_PATH), MAX_PATH); path.ReleaseBuffer();

        CString fp(path); fp.TrimRight(L"\\");
        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString f=fp.Mid(fp.ReverseFind(L'\\')+1);

        CComPtr<IContextMenu> pCM;
        getContextMenu(d, f, &pCM);

        HMENU hmenu=CreatePopupMenu();
        pCM->QueryContextMenu(hmenu, 0, 0, 0xFFFF, CMF_DEFAULTONLY);

        ProcessCMCommand(pCM, args[1].id);        
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
        m_ignoreemptyquery=true;
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &r, int def) {
        if(CString(query).Find(L'\'')==0) {
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

struct EmailVerbRule : Rule {
    EmailVerbRule() : Rule(L"TEXT", L"EMAILVERB",L"CONTACT") {}
    virtual bool execute(std::vector<SourceResult> &args) { 
        return true;
    }
};

// those kind of sources could have a simplified load and save ?
struct WebsiteSource : Source {
    WebsiteSource() : Source(L"WEBSITE") {
        // fixing something here will be overwritten by the load from the index below
        load();
        m_index[L"Google"]=SourceResult(L"Google", L"Google", L"Google", this, 0, 0, m_index[L"Google"].bonus);
        m_index[L"Amazon"]=SourceResult(L"Amazon", L"Amazon", L"Amazon", this, 1, 0, m_index[L"Amazon"].bonus);
    }    
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r) { 
        return Gdiplus::Bitmap::FromFile(L"..\\icons\\"+r->display+".png");
    }
};

struct SearchWithVerbSource : Source {
    SearchWithVerbSource() : Source(L"SEARCHWITHVERB") {        
        load();
        m_index[L"Search With"]=SourceResult(L"Search With", L"Search With", L"Search With", this, 0, 0, m_index[L"Search With"].bonus);
    }
};

struct ContactSource : Source {
    ContactSource() : Source(L"CONTACT") {        
        //m_index[L"Iris Bourret"]=SourceResult(L"Iris Bourret", L"Iris Bourret", L"Iris Bourret", this, 1, 0);
        load();
        m_index[L"Emmanuel Caradec"]=SourceResult(L"Emmanuel Caradec", L"Emmanuel Caradec", L"Emmanuel Caradec", this, 0, 0, m_index[L"Emmanuel Caradec"].bonus);
    }
};

struct QuitVerbSource : Source {
    QuitVerbSource() : Source(L"QUITVERB") {        
        load();
        m_index[L"Quit (QSLL )"]=SourceResult(L"Quit (QSLL )", L"Quit (QSLL )", L"Quit (QSLL )", this, 0, 0, m_index[L"Quit (QSLL )"].bonus);
        m_ignoreemptyquery=true;
    }
};

struct QuitRule : Rule {
    QuitRule() : Rule(L"QUITVERB") {}
    virtual bool execute(std::vector<SourceResult> &args) {
        PostQuitMessage(0);
        return true;
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
struct AlphaGUI : IWindowlessGUI {
    AlphaGUI():m_input(this) {
        //WNDCLASS wnd={0};
        //wnd.style=0;
        //wnd.lpfnWndProc=::DefWindowProc;
        //wnd.lpfnWndProc=(WNDPROC)AlphaGUI::_WndProc;
        //wnd.lpszClassName=L"GUI";            
        //ATOM clss=RegisterClass(&wnd);
        m_pane=0;    

        m_hwnd=CreateWindowEx(WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"", WS_VISIBLE|WS_POPUP|WS_CHILD, 0, 0, 0, 0, 0, 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);        
        
        m_listhosthwnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST, L"STATIC", L"", WS_POPUP|WS_THICKFRAME, 0, 0, 250, 400, 0, 0, 0, 0); // fix parent
        CRect rc;
        GetClientRect(m_listhosthwnd,&rc);
        m_listhwnd=CreateWindow(L"ListBox", L"", WS_VISIBLE|WS_CHILD|LBS_NOTIFY|LBS_HASSTRINGS|WS_VSCROLL, 0, 0, rc.Width(), rc.Height(), m_listhosthwnd, 0, 0, 0); // fix parent
        ::SetWindowLongPtr(m_listhosthwnd, GWLP_WNDPROC, (LONG)_ListBoxWndProc);
        ::SetWindowLongPtr(m_listhosthwnd, GWLP_USERDATA, (LONG)this);

        // this is not the correct way to get gui font : 
        // http://fox-toolkit.2306850.n4.nabble.com/getSystemFont-call-to-SystemParametersInfo-fails-when-WINVER-gt-0x0600-td4011173.html
        // http://blogs.msdn.com/b/oldnewthing/archive/2005/07/07/436435.aspx
        // but this is so easy .... 
        WPARAM w=(WPARAM)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(m_listhwnd, WM_SETFONT, w, TRUE);

        m_background.Load(L"..\\background.png");
        PremultAlpha(m_background);
        
        m_background3.Load(L"..\\background3.png");
        PremultAlpha(m_background3);

        m_focus.Load(L"..\\focus.png");
        PremultAlpha(m_focus);

        premult.Create(m_background3.GetWidth(), m_background.GetHeight(), 32, CImage::createAlphaChannel);

        Invalidate();        

        RegisterHotKey(m_hwnd, 1, MOD_SHIFT, VK_SPACE);

        // sources        
        m_sources[L"FILE"].push_back(new FileSource);
        m_sources[L"FILE"].push_back(new StartMenuSource(m_hwnd));
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
        
        m_sources[L"QUITVERB"].push_back(new QuitVerbSource);        
        m_rules.push_back(new QuitRule);

        for(std::map<CString, std::vector<Source*> >::iterator it=m_sources.begin(); it!=m_sources.end(); it++)
            for(int i=0;i<it->second.size();i++)
                it->second[i]->m_pArgs=&m_args;

        for(std::vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); it++)
            (*it)->m_pArgs=&m_args;

        OnQueryChange(L"");
        


        // we shouldn't create a thread for each source, this is inefficient
        // crawl should be called with an empty index for each source
        HANDLE h=CreateThread(0, 0, (LPTHREAD_START_ROUTINE)crawlProc, this, 0, 0);

        // verb sources
        //m_sources[L"FILEVERB"]=new FileVerbSource;
        //m_sources[L"HISTORY"]=new HistorySource;

        //SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOSIZE);
    }
    static DWORD __stdcall crawlProc(AlphaGUI *thiz) {
        //Sleep(10*1000);
        std::map<CString,SourceResult> *index=new std::map<CString,SourceResult>;
        thiz->m_sources[L"FILE"][1]->crawl(index);
        PostMessage(thiz->m_hwnd, WM_USER, (WPARAM)index, 0);
        return TRUE;
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
                if(m_sources[it->first][i]->m_ignoreemptyquery == true && q==L"")
                    ;
                else
                    m_sources[it->first][i]->collect(q, results, def);

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
        ::SendMessage(m_listhwnd, LB_RESETCONTENT, 0, 0); // TOFIX
        CollectItems(q, m_pane, m_args, m_results, 0);        
                
        std::sort(m_results.begin(), m_results.end(), ResultSourceCmp);

        for(uint i=0;i<m_results.size();i++) {
            CString s; s.Format(L"%d %s", m_results[i].rank, m_results[i].display);
            int id=::SendMessage(m_listhwnd, LB_ADDSTRING, 0, (LPARAM)s.GetString()); // TOFIX
            ::SendMessage(m_listhwnd, LB_SETITEMDATA, id, (LPARAM)&m_results[i]);
        }

        SourceResult *r=0;
        //if(m_results.size()) {
        //    int id=::SendMessage(m_listhwnd, LB_ADDSTRING, 0, (LPARAM)L"TEST"); // TOFIX
        //    ::SendMessage(m_listhwnd, LB_SETITEMDATA, id, 1);
        //    int j=SendMessage(m_listhwnd, LB_GETITEMDATA, id, 0);
        //}


        if(m_results.size()>0) {
            OnSelChange(&m_results.front());
        } else {
            while(m_pane<m_args.size())
                m_args.pop_back();
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
        
        CRect workarea;
        ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

        POINT p1={(workarea.left+workarea.right)/2-w/2,200};
        POINT p2={0};
        SIZE s={w, m_background.GetHeight()};
        BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, hdc, &p2, 0, &bf, ULW_ALPHA);
        
        premult.ReleaseDC(); 

        CRect r;
        GetWindowRect(m_hwnd, &r);
        SetWindowPos(m_listhosthwnd, 0, r.left+22+157*m_pane, r.bottom, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE);
    }
    SourceResult *GetSelectedItem() {
        int sel=::SendMessage(m_listhwnd, LB_GETCARETINDEX, 0, 0);
        //::SendMessage(m_listhwnd, LB_GETSELITEMS, 1, (LPARAM)&sel);
        return (SourceResult*)::SendMessage(m_listhwnd, LB_GETITEMDATA, sel, 0);
    }
    LRESULT OnKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if(msg == WM_KEYDOWN && wParam == VK_RETURN)
        {
            // return is the way to run commands
            // left and right are the way to navigate command

            SourceResult *r=GetSelectedItem();
            SetArg(m_pane, *r);

            // collect all active rules
            // might need to disambiguate here ?
            for(uint i=0;i<m_rules.size();i++)
                if(m_rules[i]->match(m_args, m_args.size())>1) {

                    if(m_rules[i]->execute(m_args)) {                        
                        // found one rule
                        ShowWindow(m_hwnd, SW_HIDE);
                        ShowWindow(m_listhosthwnd, SW_HIDE);

                        m_results.clear();
                        m_args.clear();
                        m_pane=0;
                        m_input.SetText(L"");
                        m_queries.clear();

                        for(uint a=0;a<m_args.size(); a++) {
                            m_args[a].source->validate(&m_args[a]);
                        }
                        return FALSE;
                    }
                }
            return FALSE;
        }	
        else if(msg == WM_KEYDOWN && wParam == VK_ESCAPE)
        {
            if(IsWindowVisible(m_listhosthwnd)) {
                CRect r;
                GetWindowRect(m_hwnd, &r);
                ShowWindow(m_listhosthwnd, SW_HIDE);
                return FALSE;
            }

            if(m_pane>0) {                                
                m_input.SetText(m_queries.size()==0?L"":m_queries.back());
                m_pane--;
                m_args.pop_back();       
                m_queries.pop_back();

                ShowNextArg();
                Invalidate();
            }
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_TAB)
        {        
            SourceResult *r=GetSelectedItem();
            m_input.SetText(r->expand);            
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_RIGHT)
        {
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
            ::ShowWindow(m_listhosthwnd, SW_SHOW);
            ::SetActiveWindow(m_listhosthwnd);
            ::SetFocus(m_listhwnd);
            
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
                ShowWindow(m_listhosthwnd, SW_HIDE);
            } else {
                ShowWindow(m_hwnd, SW_SHOW);
                h1=SetActiveWindow(m_hwnd);
                h2=SetFocus(m_hwnd);
            }
        } else if(msg==WM_USER) {
            ((std::map<CString,SourceResult> *)wParam)->begin()->second.source->updateIndex(((std::map<CString,SourceResult> *)wParam));
            ((std::map<CString,SourceResult> *)wParam)->begin()->second.source->save();
            OutputDebugString(L"crawling complete\n");
        }
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
    LRESULT ListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {           
        if(msg==WM_COMMAND && HIWORD(wParam)==LBN_SELCHANGE)
            OnSelChange(GetSelectedItem());

        if(msg == WM_CHAR)
            m_input.OnWindowMessage(msg,wParam,lParam);

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    static LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return ((AlphaGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->WndProc(hwnd,msg,wParam,lParam);
    }
    static LRESULT _ListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
       
        return ((AlphaGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->ListBoxWndProc(hwnd,msg,wParam,lParam);
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
            }
    }
    // old behavior
    WindowlessInput    m_input;
    HWND               m_hwnd;
    CImage             m_background;
    CImage             m_background3;
    CImage             m_focus;

    CString            lastResultExpand;

    // new behavior    

    CImage                     premult;
    uint                       m_pane;
    std::map<CString, std::vector<Source*> > m_sources;
    std::vector<Rule*>         m_rules;
    std::vector<SourceResult>  m_args;     // validated results
    std::vector<SourceResult>  m_results;  // currently displayed results
    std::vector<CString>       m_queries;

    HWND                       m_listhwnd;
    HWND                       m_listhosthwnd;
};