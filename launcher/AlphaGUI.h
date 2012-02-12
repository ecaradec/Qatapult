// TODO

// [_] rendre la mise a jour de l'index visible
// [_] reecrire les CString en wstring
// [_] corriger le probleme de troisieme pan qui ne s'ouvre pas si il n'y a pas de resultats
// [_] supprimer la boite de dialogue qui apparait quand on change les contacts gmail et la remplacer par une mise a jour de l'index
// [_] permettre de choisir le raccourci clavier
// [_] definir des actions pour l'horloge
// [_] definir des backgrounds globals à une action
// [_] faire un setup
// [X] deplacer mon blog sur emmanuelcaradec

#ifdef DEBUG
#include "vld.h"
#endif

#include "resource.h"
#include "premultAlpha.h"
#include "sqlite3/sqlite3.h"
#include "md5.h"
#include "utf8.h"
#include "CriticalSection.h"
#include "pugixml.hpp"

#include "simpleini.h"
#include "geticon.h"
#include "SourceResult.h"
#include "Source.h"
#include "DBSource.h"
#include "Rule.h"
#include "FileSource.h"
#include "NetworkSource.h"
#include "StartMenuSource.h"
#include "CurrentSelectionSource.h"
#include "FileVerbSource.h"
#include "FileVerbRule.h"
#include "EmailVerbSource.h"
#include "IWindowlessGUI.h"
#include "WindowlessInput.h"
#include "TextSource.h"
#include "ContactSource.h"
#include "SendEmail.h"
#include "EmailVerbRule.h"
#include "EmailFileVerbRule.h"
#include "WebsitePlugin.h"
#include "QuitPlugin.h"
#include "LevhensteinDistance.h"

WNDPROC OldHotKeyEditProc;
int hotkeymodifiers=0;
int hotkeycode=0;
HWND g_foregroundWnd;

CString HotKeyToString(int modifier, int vk) {    
    CString mod;
    if(modifier&MOD_SHIFT)
        mod+=L"SHIFT+";
    if(modifier&MOD_CONTROL)
        mod+=L"CONTROL+";
    if(modifier&MOD_ALT)
        mod+=L"ALT+";
    if(modifier&MOD_WIN)
        mod+=L"WIN+";

    CString c;
    if(vk==VK_PAUSE)
        c=L"PAUSE";
    else if(vk==VK_ESCAPE)
        c=L"ESCAPE";            
    else
        c=CString((TCHAR)vk);
            
    return mod+c;
}

BOOL CALLBACK HotKeyEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg==WM_KEYDOWN) {
        // keys to ignore
        if(wParam==VK_CAPITAL)
            return TRUE;

        if(wParam>VK_MENU) {
            hotkeymodifiers=0;
            CString mod;
            BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
            if(bShift) {
                hotkeymodifiers|=MOD_SHIFT;
            }
            BOOL bCtl = GetKeyState(VK_CONTROL) & 0x8000;
            if(bCtl) {
                hotkeymodifiers|=MOD_CONTROL;
            }
            BOOL bAlt = GetKeyState(VK_MENU) & 0x8000;
            if(bAlt) {
                hotkeymodifiers|=MOD_ALT;
            }
            BOOL blWin= GetKeyState(VK_LWIN) & 0x8000;
            BOOL brWin= GetKeyState(VK_RWIN) & 0x8000;
            if(blWin ||brWin) {
                hotkeymodifiers|=MOD_WIN;
            }

            hotkeycode=wParam;
            
            SetWindowText(hWnd,HotKeyToString(hotkeymodifiers, hotkeycode));            
        }
        return TRUE;
    } else if(msg==WM_KEYUP) {
        return TRUE;
    } else if(msg==WM_CHAR) {
        return TRUE;
    }
    return CallWindowProc(OldHotKeyEditProc,hWnd, msg, wParam, lParam);
}

BOOL CALLBACK GeneralDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            OldHotKeyEditProc=SubclassWindow(GetDlgItem(hWnd, IDC_HOTKEY1), HotKeyEditProc);
            CString txt=HotKeyToString(hotkeymodifiers,hotkeycode);
            SetDlgItemText(hWnd,IDC_HOTKEY1, txt);
            return TRUE;
        }
        case WM_COMMAND:
        {
            if(wParam==IDC_SAVEHOTKEY) {
                SetSettingsInt(L"hotKeys", L"toggleKey", hotkeycode);
                SetSettingsInt(L"hotKeys", L"toggleModifier", hotkeymodifiers);
                UnregisterHotKey(g_pUI->getHWND(), 1);
                RegisterHotKey(g_pUI->getHWND(), 1, hotkeymodifiers, hotkeycode);                
            }            
        }        
    }
    return FALSE;
}

int DelayLoadExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS *ep) {
    return EXCEPTION_CONTINUE_EXECUTION;
}
struct ClockSource : Source {
    ClockSource() : Source(L"CLOCK") {
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
        if(sr->icon)
            g.DrawImage(sr->icon, RectF(r.X+10, r.Y+10, 128, 128));

        SYSTEMTIME st;
        GetSystemTime(&st);
        
        Gdiplus::Matrix m;
        
        if(m_hasGdipDrawImageFX) {
            m.Reset();
            m.Translate(r.X+10+128/2,r.Y+10+128/2);
            m.Rotate(((float)st.wHour+(float)st.wMinute/60)/24*360+180);
            m.Translate(-4,-4);
            g.DrawImage(hours, &RectF(0, 0, 8, 40), &m, 0, 0, Gdiplus::UnitPixel);

            m.Reset();
            m.Translate(r.X+10+128/2,r.Y+10+128/2);
            m.Rotate(((float)st.wMinute+(float)st.wSecond/60)/60*360+180);
            m.Translate(-4,-4);
            g.DrawImage(minutes, &RectF(0, 0, 8, 40), &m, 0, 0, Gdiplus::UnitPixel);

            m.Reset();
            m.Translate(r.X+10+128/2,r.Y+10+128/2);
            m.Rotate((float)st.wSecond/60*360+180);
            m.Translate(-2,-4);
            g.DrawImage(seconds, &RectF(0, 0, 8, 40), &m, 0, 0, Gdiplus::UnitPixel);
        }

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(GetSettingsString(L"general",L"font",L"Arial"), 8.0f);
        //g.DrawString(sr->display, sr->display.GetLength(), &f, RectF(r.X, r.Y+r.Height-15, r.Width, 20), &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
        drawEmphased(g, sr->display, m_pUI->getQuery(), RectF(r.X, r.Y+r.Height-20, r.Width, 20));
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\clock.png");
    }
    CString getString(SourceResult &sr,const TCHAR *val) {
        return L"";
    }

    bool    m_hasGdipDrawImageFX;
    int     angle;
    Bitmap *hours;
    Bitmap *minutes;
    Bitmap *seconds;
};

struct ClockRule : Rule {
    ClockRule() {}
    bool execute(std::vector<SourceResult> &args) {
        return true;
    }
};


#define WM_UPDATEINDEX (WM_USER)
#define WM_INVALIDATEINDEX (WM_USER+1)
#define WM_INVALIDATEDISPLAY (WM_USER+2)
#define WM_PROGRESS (WM_USER+3)
#define WM_RELOAD (WM_USER+4)
#define WM_STOPWORKERTHREAD (WM_USER+5)

void CenterWindow(HWND hwnd) {
    CRect workarea;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

    CRect r;
    GetWindowRect(hwnd, r);
    
    SetWindowPos(hwnd, 0, (workarea.left+workarea.right)/2-r.Width()/2, max(workarea.top, (workarea.top+workarea.bottom)/2 - r.Height()/2), 0, 0, SWP_NOSIZE);
}

BOOL CALLBACK ToggleSettingsEWProc(HWND hwnd, LPARAM lParam) {
    TCHAR className[256];
    GetClassName(hwnd, className, sizeof(className));
    if(_tcscmp(L"#32770", className)==0)
        ShowWindow(hwnd, (hwnd==(HWND)lParam)?SW_SHOW:SW_HIDE);
    return TRUE;
}

BOOL CALLBACK SettingsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND htreeview;
    switch(msg)
    {
        case WM_INITDIALOG:
            // doesn't seem to work
            htreeview=GetDlgItem(hWnd, IDC_TREE); 
            TreeView_SetExtendedStyle(htreeview, TVS_FULLROWSELECT, TVS_FULLROWSELECT);            
        return TRUE;
        case WM_COMMAND:
            if(wParam==IDOK) {
                g_pUI->InvalidateIndex();
                ::EndDialog(hWnd, 0);
            } else if(wParam==IDCANCEL) {
                ::EndDialog(hWnd, 0);
            }
        return TRUE;
        case WM_NOTIFY:
            if(((NMHDR*)lParam)->code==TVN_SELCHANGED) {
                NMTREEVIEW *nmtv=(NMTREEVIEW*)lParam;
                EnumChildWindows(hWnd, ToggleSettingsEWProc, nmtv->itemNew.lParam);
            }
        return TRUE;
    }
    return FALSE;
}

BOOL CALLBACK EmailDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        return TRUE;
        case WM_NOTIFY:
        return TRUE;
    }
    return FALSE;
}

CString getGUID() {
    CLSID  clsid;
    CoCreateGuid(&clsid);
    OLECHAR szGuid[40]={0}; int nCount = ::StringFromGUID2(clsid, szGuid, 40);
    return szGuid;
}

struct TextItemSource : Source {
    TextItemSource(const TCHAR *name) : Source(name) {
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(m_index[r->key].iconname);
    }
    void addItem(const TCHAR *str,const TCHAR *iconname) {
        m_index[str]=SourceResult(str,str,str, this, 0, 0, m_index[str].bonus);
        m_index[str].iconname=iconname;
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(CString(it->second.display).MakeUpper().Find(q)!=-1) {
                results.push_back(it->second);
            }
        }
    } 
    CString getString(SourceResult &sr,const TCHAR *val_) {
        if(CString(val_)==L"text") {
            return sr.display;
        }
        return L"";
    }
};

struct WindowSource : Source {
    WindowSource() : Source(L"WINDOW") {
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\window.png");
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString q(query); q.MakeUpper();
        std::vector<HWND> windowList;
        EnumWindows(CollectWindows,(LPARAM)&windowList);
        for(std::vector<HWND>::iterator it=windowList.begin(); it!=windowList.end(); it++) {
            TCHAR title[MAX_PATH];
            GetWindowText(*it,title,sizeof(title));
            if(CString(title).Find(q)!=-1) {
                results.push_back(SourceResult(title,title,title,this,0,*it,0));
            }
        }
    } 
    CString getString(SourceResult &sr,const TCHAR *val_) {
        if(CString(val_)==L"title") {
            return sr.display;
        } else if(CString(val_)==L"hwnd") {
            return ItoS((int)sr.data);
        }
        return L"";
    }
    static BOOL __stdcall CollectWindows(HWND hwnd, LPARAM lparam) {
        std::vector<HWND> *hwndlist=(std::vector<HWND>*)lparam;
        hwndlist->push_back(hwnd);
        return TRUE;
    }
};

// types de command
// file,copyto,file => copy $p0.path $p2.path
// text,clip => clip $p0.text
// parseur : chercher les *, renvoyer la chaine, 
struct CommandRule : Rule {
    CommandRule(const CString &cmd,const CString &args, const CString &workdir):m_command(cmd),m_args(args),m_workdir(workdir) {
        m_command.Trim();
    } // the type must be precised later
    bool execute(std::vector<SourceResult> &args) {
        CString cmd=expand(m_command);
        CString arg=expand(m_args);
        CString workdir=expand(m_workdir);

        OutputDebugString(Format(L"shellexecute '%s' '%s' '%s'\n",cmd,arg,workdir));
        ShellExecute(0, 0, cmd, arg, workdir, SW_SHOWDEFAULT);
        return true;
    }
    CString expand(const CString &str) {
        CString tmp;
        for(int i=0;i<str.GetLength();i++) {
            TCHAR c=str[i];
            if(c=='$') {
                i++;
                CString id;
                CString arg;
                while(str[i]>='0' && str[i]<='9') {
                    id+=str[i];
                    i++;
                }
                i++;
                while(str[i]>='a' && str[i]<='z') {
                    arg+=str[i];
                    i++;
                }
                                
                CString val=(*m_pArgs)[_ttoi(id)].source->getString((*m_pArgs)[_ttoi(id)],arg);
                tmp+=val;
                tmp+=str[i];
            } else {
                tmp+=c;
            }
        }
        return tmp;
    }
    CString m_command;
    CString m_args;
    CString m_workdir;
};

// SHGetImageList
struct AlphaGUI : IWindowlessGUI, UI {
    AlphaGUI():m_input(this), m_invalidatepending(false) {
#ifdef DEBUG
        VLDMarkAllLeaksAsReported();
#endif

        m_pane=0;
        m_editmode=0;

        WCHAR curDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, curDir);
        settingsini=CString(curDir)+L"\\settings.ini";

        g_pUI=this;

        if(GetFileAttributes(L"settings.xml") == INVALID_FILE_ATTRIBUTES) {
            SHFILEOPSTRUCT sffo={0};
            sffo.wFunc=FO_COPY;
            sffo.pFrom=L"settings.default.xml\0";
            sffo.pTo=L"settings.xml\0";
            SHFileOperation(&sffo);
        }
        
        CreateDirectory(L"databases", 0);
        
        pugi::xml_parse_result result = settings.load_file("settings.xml");

        // hotkey
        hotkeycode=GetSettingsInt(L"hotKeys", L"toggleKey",VK_SPACE);
        hotkeymodifiers=GetSettingsInt(L"hotKeys", L"toggleModifier",MOD_SHIFT);        

        // bitmaps
        m_textbackground.Load(L"textbackground.png");
        PremultAlpha(m_textbackground);

        m_background.Load(L"background.png");
        PremultAlpha(m_background);
        
        m_background3.Load(L"background3.png");
        PremultAlpha(m_background3);

        m_focus.Load(L"focus.png");
        PremultAlpha(m_focus);

        m_knob.Load(L"ui-radio-button.png");
        PremultAlpha(m_knob);

        premult.Create(m_background3.GetWidth(), m_background.GetHeight(), 32, CImage::createAlphaChannel);        

        // create dialogs
        CreateSettingsDlg();

        // repaint dialog
        Invalidate(); 

        // define hotkey
        RegisterHotKey(g_pUI->getHWND(), 1, hotkeymodifiers, hotkeycode);


        Init();

        m_mainThreadId=GetCurrentThreadId();
    }
    ~AlphaGUI() {
        Reset();
    }
    HANDLE m_workerthread;
    void Init() {        
        // sources 
        addSource(new FileSource);
        addSource(new StartMenuSource(m_hwnd));
        addSource(new CurrentSelectionSource);
        addSource(new NetworkSource);       
        addSource(new TextSource);        
        addSource(new ContactSource);
        addSource(new ClockSource);
        addSource(new FileVerbSource);
        addSource(new WebsiteSource);
        addSource(new WindowSource);        
        
        addRule(L"CLOCK", new ClockRule);
        addRule(L"FILE", L"FILEVERB", new FileVerbRule);     
        
        TextItemSource *t;

        t=new TextItemSource(L"EMAILFILEVERB");
        m_sources[t->type].push_back(t);
        t->addItem(L"Email to",L"icons\\emailto.png");
        addRule(L"FILE",t->type,L"CONTACT",new EmailFileVerbRule);

        t=new TextItemSource(L"EMAILTEXTVERB");
        m_sources[t->type].push_back(t);
        t->addItem(L"Email to",L"icons\\emailto.png");        
        addRule(L"TEXT",t->type,L"CONTACT",new EmailVerbRule);        

        t=new TextItemSource(L"SEARCHWITHVERB");
        m_sources[t->type].push_back(t);
        t->addItem(L"Search With",L"icons\\searchwith.png");        
        addRule(L"TEXT",t->type,L"WEBSITE",new WebSearchRule);
                
        t=new TextItemSource(L"QUITVERB");
        m_sources[t->type].push_back(t);
        t->addItem(L"Quit (Q)",L"icons\\exit.png");
        t->addItem(L"Reload (Q)",L"icons\\reload.png");
        addRule(t->type,new QuitRule);

        t=new TextItemSource(L"EMPTY");
        m_sources[t->type].push_back(t);
        m_nullresult.source=t;
        

        LoadRules(settings);

        std::vector<CString> plugins;
        FindFilesRecursively(L"plugins", L"plugin.xml", plugins, 3);
        for(std::vector<CString>::iterator it=plugins.begin();it!=plugins.end();it++) {
            pugi::xml_document d;
            d.load_file(*it);
            LoadRules(d);
        }

        for(std::map<CString, std::vector<Source*> >::iterator it=m_sources.begin(); it!=m_sources.end(); it++)
            for(uint i=0;i<it->second.size();i++) {
                it->second[i]->m_pArgs=&m_args;
                it->second[i]->m_pUI=this;
            }

        for(std::vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); it++)
            (*it)->m_pArgs=&m_args;

        OnQueryChange(L"");

        // we shouldn't create a thread for each source, this is inefficient
        // crawl should be called with an empty index for each source
        //_beginthread((void (*)(void*))crawlProc, 0, this);
        //m_workerthread=CreateThread(0, 0, (LPTHREAD_START_ROUTINE)crawlProc, this, 0, &m_crawlThreadId);

        m_workerthread = (HANDLE)_beginthreadex(0, 0, (uint (__stdcall *)(void*))crawlProc, this, 0, (uint*)&m_crawlThreadId );
    }
    void LoadRules(pugi::xml_document &settings) {
        pugi::xpath_node_set ns=settings.select_nodes("/settings/rules/rule");
        for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++) {
            Rule *r=new CommandRule(UTF8toUTF16(it->node().child_value("cmd")),
                                    UTF8toUTF16(it->node().child_value("args")),
                                    UTF8toUTF16(it->node().child_value("workdir")));
            m_rules.push_back(r);
        
            pugi::xpath_node_set ns=it->node().select_nodes("arg");
            for(pugi::xpath_node_set::const_iterator itarg=ns.begin(); itarg!=ns.end(); itarg++) {
                CString arg=itarg->node().first_child().value();
                if(arg.GetLength()!=0) {
                    r->m_types.push_back(Type(arg));
                } else {
                    pugi::xpath_node_set elts=itarg->node().select_nodes("item");                    
                    TextItemSource *t=new TextItemSource(getGUID());
                    m_sources[t->type].push_back(t);
                    for(pugi::xpath_node_set::const_iterator itelt=elts.begin(); itelt!=elts.end(); itelt++) {
                        CString lbl=UTF8toUTF16(itelt->node().child_value("lbl"));
                        CString ico=UTF8toUTF16(itelt->node().child_value("ico"));
                        t->addItem(lbl.GetString(),ico.GetString());                        
                    }
                    r->m_types.push_back(Type(t->type));
                }
            }
        }
    }
    void Reset() {
        PostThreadMessage(m_crawlThreadId, WM_STOPWORKERTHREAD, 0, 0);
        WaitForSingleObject(m_workerthread,INFINITE);

        ClearResults(m_results);
        for(int i=0;i<m_args.size();i++) {
            if(m_args[i].source)
                m_args[i].source->clear(m_args[i]);
        }
        m_args.clear();

        for(std::map<CString, std::vector<Source*> >::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
            for(uint i=0;i<it->second.size();i++)
                delete it->second[i];
        }
        for(std::vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); it++) {
            delete *it;
        }
        m_sources.clear();
        m_rules.clear();
    }
    void Reload() {
        PostMessage(getHWND(),WM_RELOAD,0,0);
    }
    void addSource(Source *s) {
        m_sources[s->type].push_back(s);
    }
    void addRule(const CString &arg0,const CString &arg1,const CString &arg2,Rule *r) {
        r->m_types.push_back(arg0);
        r->m_types.push_back(arg1);
        r->m_types.push_back(arg2);
        m_rules.push_back(r);
    }
    void addRule(const CString &arg0,const CString &arg1,Rule *r) {
        r->m_types.push_back(arg0);
        r->m_types.push_back(arg1);
        m_rules.push_back(r);
    }
    void addRule(const CString &arg0,Rule *r) {
        r->m_types.push_back(arg0);
        m_rules.push_back(r);
    }
    void addRule(Rule *r) {
        m_rules.push_back(r);
    }

    DWORD m_crawlThreadId;
    DWORD m_mainThreadId;
    static uint __stdcall crawlProc(AlphaGUI *thiz) {        
        MSG msg;
        msg.message=WM_INVALIDATEINDEX; // create a fake message on first pass
        do {        
            switch(msg.message)  {
                case WM_INVALIDATEINDEX:
                    {
                        BOOL b=PostMessage(thiz->m_hwnd, WM_PROGRESS, 0, 0);
                        float nbsources=float(thiz->m_sources.size());
                        float isources=0;
                        for(std::map<CString, std::vector<Source*> >::iterator it=thiz->m_sources.begin(); it!=thiz->m_sources.end();it++) {                        
                            for(uint i=0;i<it->second.size();i++)
                                it->second[i]->crawl();

                            isources++;
                            b=PostMessage(thiz->m_hwnd, WM_PROGRESS, WPARAM(100.0f*isources/nbsources), 0);
                        }
                    }
                case WM_STOPWORKERTHREAD:
                    goto stop;
                    break;
            }
            
            //TranslateMessage(&msg);
            //DispatchMessage(&msg);
        } while(GetMessage(&msg, 0, WM_INVALIDATEINDEX, WM_INVALIDATEINDEX+1)!=0);

        stop:
        _endthread();
        return TRUE;
    }
    HWND getHWND() {
        return m_hwnd;
    }
    void InvalidateIndex() {
        PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);
    }
    void Invalidate() {
        if(m_invalidatepending==false) {
            PostMessage(m_hwnd, WM_INVALIDATEDISPLAY, 0, 0);        
            m_invalidatepending=true;
        }
    }
    void CreateSettingsDlg() {
        m_hwnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"", WS_VISIBLE|WS_POPUP|WS_CHILD, 0, 0, 0, 0, 0, 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);        
            
        m_listhosthwnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST, L"STATIC", L"", WS_POPUP|WS_THICKFRAME, 0, 0, m_background.GetWidth(), 10*40+GetSystemMetrics(SM_CYFRAME)*2, 0, 0, 0, 0); // fix parent
        CRect rc;
        GetClientRect(m_listhosthwnd,&rc);
        m_listhwnd=CreateWindow(L"ListBox", L"", WS_VISIBLE|WS_CHILD|LBS_NOTIFY|LBS_HASSTRINGS|WS_VSCROLL|LBS_OWNERDRAWFIXED, 0, 0, rc.Width(), rc.Height(), m_listhosthwnd, 0, 0, 0); // fix parent
        ::SetWindowLongPtr(m_listhosthwnd, GWLP_WNDPROC, (LONG)_ListBoxWndProc);
        ::SetWindowLongPtr(m_listhosthwnd, GWLP_USERDATA, (LONG)this);
        ::SendMessage(m_listhwnd, LB_SETITEMHEIGHT, 0, 40);

        m_hwndsettings=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), 0, (DLGPROC)SettingsDlgProc);  //CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GET_GMAILAUTHCODE), 0, DlgProc);
        HWND hTreeView=GetDlgItem(m_hwndsettings, IDC_TREE);

        HWND hwndGmail=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GMAILCONTACTS), m_hwndsettings, (DLGPROC)DlgProc);
        SetWindowPos(hwndGmail, 0, 160, 0, 0, 0, SWP_NOSIZE);

        //HWND hwndEmail=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EMAIL), m_hwndsettings, (DLGPROC)DlgProc);
        //SetWindowPos(hwndEmail, 0, 160, 0, 0, 0, SWP_NOSIZE);

        HWND hwndSF=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SEARCHFOLDERS), m_hwndsettings, (DLGPROC)SearchFolderDlgProc);
        SetWindowPos(hwndSF, 0, 160, 0, 0, 0, SWP_NOSIZE);


        HWND hwndG=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GENERAL), m_hwndsettings, (DLGPROC)GeneralDlgProc);
        SetWindowPos(hwndG, 0, 160, 0, 0, 0, SWP_NOSIZE);


        TV_INSERTSTRUCT tviis;
        ZeroMemory(&(tviis.item), sizeof(TV_ITEM));
        tviis.item.mask = TVIF_TEXT|TVIF_PARAM;
        tviis.hParent = TVI_ROOT;

        tviis.item.pszText = L"General";
        tviis.item.lParam=(LPARAM)hwndG;
        HTREEITEM htreeG=TreeView_InsertItem(hTreeView, &tviis);

        tviis.item.pszText = L"Search folders";
        tviis.item.lParam=(LPARAM)hwndSF;
        HTREEITEM htreeSF=TreeView_InsertItem(hTreeView, &tviis);

        //tviis.item.pszText = L"Email";
        //tviis.item.lParam=(LPARAM)hwndEmail;
        //HTREEITEM htreeEmail=TreeView_InsertItem(hTreeView, &tviis);

        tviis.item.pszText = L"Gmail contacts";
        tviis.item.lParam=(LPARAM)hwndGmail;
        HTREEITEM htreeGmail=TreeView_InsertItem(hTreeView, &tviis);

        BOOL b=TreeView_SelectItem(hTreeView, htreeG);
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
                activesources[activerules[i]->m_types[pane].m_type]=true;

        // collect displayable items
        ClearResults(results);
        //results.clear();        
        for(std::map<CString,bool>::iterator it=activesources.begin(); it!=activesources.end(); it++) {
            for(uint i=0;i<m_sources[it->first].size();i++) {
                if(pane==0 && q==L""/*m_sources[it->first][i]->m_ignoreemptyquery == true && q==L""*/)
                    ;
                else
                    m_sources[it->first][i]->collect(q, results, def);
            }
        }

        uselev=0;
        CString Q(q); Q.MakeUpper();
        int qlen=max(1, Q.GetLength());
        for(uint i=0;i<results.size();i++) {
            // pourcentage de chaine correspondante ?            
            CStringA item=results[i].display;
            item.MakeUpper();
            if(uselev) {
                int len=levenshtein_distance(CStringA(Q), item.GetString());                
                float f = 1 - float(len) / results[i].display.GetLength();
                results[i].rank = int(100*f + results[i].bonus);
            } else {
                if(results[i].source->m_prefix!=0 && results[i].display[0]==results[i].source->m_prefix)
                    results[i].bonus+=100;

                results[i].rank = int(100*float(qlen) / results[i].display.GetLength() + results[i].bonus);
            }
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

        //int m=m=min(50,m_results.size());
        uint m=m_results.size();
        //if(uselev)
        //    m=min(50,m_results.size());
        //else
        //    m=m_results.size();

        //for(uint i=0;i<min(50,m_results.size());i++) {
        for(uint i=0;i<m;i++) {
            CString s; s.Format(L"%d %s", m_results[i].rank, m_results[i].display);
            int id=::SendMessage(m_listhwnd, LB_ADDSTRING, 0, (LPARAM)s.GetString()); // TOFIX
            ::SendMessage(m_listhwnd, LB_SETITEMDATA, id, (LPARAM)&m_results[i]);
        }

        ::SendMessage(m_listhwnd,LB_SETCURSEL,0,0);

        SourceResult *r=0;

        if(m_results.size()>0) {
            OnSelChange(&m_results.front());
        } else {
            OnSelChange(&m_nullresult);
        }        

        ShowNextArg();

        // animation timer
        int refresh=0;
        for(uint i=0; i<m_args.size(); i++) {
            if(m_args[i].source)
                refresh=max(m_args[i].source->m_refreshPeriod, refresh);
        }

        if(refresh!=0)
            SetTimer(m_hwnd, 2, refresh, 0);
        else
            KillTimer(m_hwnd,2);
    }
    void ShowNextArg() {
        // check if there is extra args     
        if(m_args.size()>0) {
            // remove all args beyond current pane
            while(m_pane+1<m_args.size()) {
                m_args.back().source->clear(m_args.back());
                m_args.pop_back();
            }

            std::vector<SourceResult> results;
            CollectItems(L"", m_pane+1, m_args, results, 1);
            std::sort(results.begin(), results.end(), ResultSourceCmp);
            if(results.size()!=0) {
                SetArg(m_pane+1,results.front()); 
            }
        }
    }
    void SetArg(uint pane, SourceResult &r) {
        if(pane==m_args.size()) {
            m_args.push_back(SourceResult());
            r.source->copy(r,&m_args.back());
        } else {
            m_args[pane].source->clear(m_args[pane]);
            r.source->copy(r,&m_args[pane]);
        }
    }
    void OnSelChange(SourceResult *r) {        
        if(m_args.size()==0)
            m_args.push_back(m_nullresult);
        
        // a copy is not enough if there is deep data because the results are cleaned after the query
        SetArg(m_pane, *r);

        ShowNextArg();

        Invalidate();
    }
    void Update() {        
        m_invalidatepending=false;        

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
        
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintAntiAlias);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);        
        g.SetCompositingQuality(CompositingQualityHighQuality);
        g.Clear(Gdiplus::Color(0,0,0,0));

        // if there is a text and it begin by a dot : switch to large text mode
        if(m_editmode==1) {
            m_textbackground.AlphaBlend(hdc, 0, 0);
            m_curWidth=m_textbackground.GetWidth();

            StringFormat sf;
            m_input.Draw(hdc, RectF(27.0f,27.0f, REAL(m_curWidth-54.0f), m_textbackground.GetHeight()-54.0f), sf, m_args[m_pane].source->m_prefix );
        } else {
            StringFormat sfcenter;
            sfcenter.SetAlignment(StringAlignmentCenter);    
            sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);
        
            if(m_args.size()==3) {
                m_background3.AlphaBlend(hdc, 0, 0);
                m_curWidth=m_background3.GetWidth();
            } else {
                m_background.AlphaBlend(hdc, 0, 0);
                m_curWidth=m_background.GetWidth();
            }

            // draw icon on screen        
            m_focus.AlphaBlend(hdc, 22+157*0, 22);

            m_focus.AlphaBlend(hdc, 22+157*1, 22);

            m_knob.AlphaBlend(hdc, m_curWidth-20, 5);

            if(m_args.size()==3)
                m_focus.AlphaBlend(hdc, 22+157*2, 22);

            m_focus.AlphaBlend(hdc, 22+157*m_pane, 22);

            m_status.resize(m_args.size());

            for(uint i=0;i<m_args.size(); i++) {
                m_status[i]="";
                m_displayPane=i;
                m_args[i].source->drawItem(g, &m_args[i], RectF(22+157*REAL(i), 22, 150, 154));            
            }        

            Gdiplus::Font f(GetSettingsString(L"general",L"font",L"Arial"), 8);
            if(m_indexing!=L"") {
                g.DrawString(m_indexing, -1, &f, RectF(5.0f, 5.0f, float(m_curWidth), 20.0f), &sfcenter, &SolidBrush(Color(0x88FFFFFF)));
            }

            //CString text(L"status");
            if(m_pane<m_status.size()) {
                drawEmphased(g,m_status[m_pane],m_input.m_text,RectF(15,180, REAL(m_curWidth-30), 15),DE_COLOR);
                //g.DrawString(m_status[m_pane], -1, &f, RectF(15,180, REAL(m_curWidth-30), 15), &sfcenter, &SolidBrush(Color(0x88FFFFFF)));
            }
        }

        Gdiplus::Font arial(GetSettingsString(L"general",L"font",L"Arial"), 50);
        Gdiplus::StringFormat sfmt(Gdiplus::StringFormat::GenericTypographic());  
        sfmt.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces|StringFormatFlagsNoWrap|StringFormatFlagsNoClip|StringFormatFlagsNoFitBlackBox);   
        sfmt.SetTrimming(StringTrimmingEllipsisCharacter);        

        // if the space is less than 2 hyphen then there is no hyphen
        /*RectF hyphenbbox;
        g.MeasureString(L"…", -1, &arial, PointF(0,0), &sfmt, &hyphenbbox);
        float w=hyphenbbox.Width*2; // 22 is the minimum that trigger an hyphen

        g.FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(255,0,0,0)),RectF(20,0,w,100));
        g.DrawString(L"visualvisualvisualvisual", -1, &arial, RectF(20,0,w,100), &sfmt, &Gdiplus::SolidBrush(Gdiplus::Color(255,255,255,255)));*/

        CRect workarea;
        ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

        POINT p1={(workarea.left+workarea.right)/2-m_curWidth/2,200};
        POINT p2={0};
        SIZE s={m_curWidth, m_background.GetHeight()};
        BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, hdc, &p2, 0, &bf, ULW_ALPHA);
        
        premult.ReleaseDC(); 
        

        CRect r;
        GetWindowRect(m_hwnd, &r);
        SetWindowPos(m_listhosthwnd, 0, r.left+157*m_pane, r.bottom, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE);   
    }    
    CString getQuery() {
        if(m_displayPane==m_queries.size())
            return m_input.m_text;
        else if(m_displayPane>m_queries.size())
            return L"";
        return m_queries[m_displayPane];
    }
    void setStatus(const CString &s) {
        m_status[m_displayPane]=s;
    }
    SourceResult *GetSelectedItem() {
        int sel=::SendMessage(m_listhwnd, LB_GETCARETINDEX, 0, 0);
        //::SendMessage(m_listhwnd, LB_GETSELITEMS, 1, (LPARAM)&sel);
        SourceResult *sr=(SourceResult*)::SendMessage(m_listhwnd, LB_GETITEMDATA, sel, 0);
        if(sr!=(SourceResult*)-1)
            return sr;
        return 0;
    }
    void ClearResults(std::vector<SourceResult> &results) {
        for(uint j=0;j<results.size();j++) {            
            results[j].source->clear(results[j]);
        }
        results.clear();
    }
    LRESULT OnKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        if(msg == WM_KEYDOWN && wParam == VK_RETURN)
        {
            if((GetKeyState(VK_SHIFT)&0xa000)!=0) {
                m_input.appendAtCaret(L"\n");
                return FALSE;
            }
            // return is the way to run commands
            // left and right are the way to navigate command

            SourceResult *r=GetSelectedItem();
            // handle null result
            if(r==0)
                return FALSE;
            SetArg(m_pane, *r);

            // collect all active rules
            // might need to disambiguate here ?
            for(uint i=0;i<m_rules.size();i++)
                if(m_rules[i]->match(m_args, m_args.size())>1) {
                    // found one rule
                    ShowWindow(m_hwnd, SW_HIDE);
                    ShowWindow(m_listhosthwnd, SW_HIDE);                       

                    if(m_rules[i]->execute(m_args)) {                                                
                        for(uint a=0;a<m_args.size(); a++) {
                            m_args[a].source->validate(&m_args[a]);
                        }

                        ClearResults(m_results);
                        //m_results.clear();
                        m_args.clear();
                        m_pane=0;
                        m_input.SetText(L"");
                        m_queries.clear();

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

            if(m_editmode==1) {
                m_editmode=0;
                if(m_input.m_text[0]==L'.')
                    m_input.m_text=m_input.m_text.Mid(1);
                Invalidate();
                return FALSE;
            }

            if(m_pane==0) {
                ShowWindow(m_hwnd, SW_HIDE);
                ShowWindow(m_listhosthwnd, SW_HIDE);

                m_input.SetText(L"");
                return FALSE;
            }

            if(m_pane>0) {                                
                CString query=m_queries.size()==0?L"":m_queries.back();                

                m_pane--;
                m_args.pop_back();       
                m_queries.pop_back();
                
                m_input.SetText(query);
                ShowNextArg();
                Invalidate();
            }            
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_TAB)
        {
            m_editmode=0;
            if((GetKeyState(VK_SHIFT)&0xa000)==0) {
                uint p=m_pane;
                if(p+1<m_args.size()) {
                    m_pane++;

                    if(p<m_args.size()) {
                        m_queries.push_back(m_input.m_text);
                        m_input.SetText(L"");
                    } else {
                        m_queries.push_back(L"");
                        m_input.SetText(L"");
                    }   
                }
                
                // alternate cyclic move with tab (ewemoa proposition )
                //m_pane=(m_pane+1)%m_args.size();
                //uint p=m_pane;

            } else {
                if(m_pane==0)
                    return FALSE;

                if(m_pane>0)
                    m_pane--;
                // alternate cyclic move with tab (ewemoa proposition )
                //m_pane=(m_pane-1)%m_args.size();

                m_input.SetText(m_queries[m_pane]);
                m_queries.pop_back();
            
            }
            
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_LEFT)
        {
            if(m_editmode==1) {
                bool bCtrl=GetKeyState(VK_CONTROL)&0x8000;
                m_input.moveCaretLeft(bCtrl);
            }
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_RIGHT)
        {
            if(m_editmode==1) {
                bool bCtrl=GetKeyState(VK_CONTROL)&0x8000;
                m_input.moveCaretRight(bCtrl);
            } else {
                SourceResult *r=GetSelectedItem();
                m_input.SetText(r->expand);       
            }
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_BACK)
        {
            bool bCtrl=GetKeyState(VK_CONTROL)&0x8000;
            m_input.back(bCtrl);            
            return FALSE;
        }        
        else if(msg == WM_KEYDOWN && wParam == VK_DELETE)
        {
            bool bCtrl=GetKeyState(VK_CONTROL)&0x8000;
            m_input.del(bCtrl);
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_DOWN)
        {	
            if(m_editmode==1) {
                ;
            } else {
                if(!::IsWindowVisible(m_listhosthwnd)) {
                    CRect r;
                    GetWindowRect(m_hwnd, &r);
                    ::ShowWindow(m_listhosthwnd, SW_SHOW);
                    ::SetActiveWindow(m_listhosthwnd);                
                    ::SetFocus(m_listhwnd);
                    ::SendMessage(m_listhwnd,LB_SETCURSEL,0,0);
                }
            }
            return FALSE;
        }
        else if(msg == WM_KEYDOWN && wParam == VK_HOME)
        {
            if(m_editmode==1) {
                m_input.home();
            }
        }
        else if(msg == WM_KEYDOWN && wParam == VK_END)
        { 
            if(m_editmode==1) {
                m_input.end();
            }
        }
        else if(msg == WM_KEYDOWN && wParam == VK_INSERT)
        {
            m_editmode=m_editmode?0:1;
            if(m_editmode==0)
                m_input.m_caretpos=m_input.m_text.GetLength();
            Invalidate();
        }
        else if(msg == WM_CHAR)
        {                        
            if(msg==WM_CHAR) {
                if(wParam>VK_F12 && wParam<=VK_LAUNCH_APP2) {
                    ;
                } else if(wParam==22) { // ctrl+v
                    OpenClipboard(0);
                    HANDLE htext=GetClipboardData(CF_UNICODETEXT);
                    if(htext) {
                        LPVOID lpvoid=GlobalLock(htext);
                        CString clip((WCHAR*)lpvoid);
                        GlobalUnlock(lpvoid);                    
                        CloseClipboard();

                        m_input.appendAtCaret(clip);
                    } else {
                        HANDLE htext=GetClipboardData(CF_TEXT);
                        LPVOID lpvoid=GlobalLock(htext);
                        CString clip((CHAR*)lpvoid);
                        GlobalUnlock(lpvoid);                    
                        CloseClipboard();

                        m_input.appendAtCaret(clip);
                    }
                } else if(wParam=='.') {
                    if(m_editmode==0)
                        m_editmode=1;
                    else
                        m_input.appendAtCaret((TCHAR)wParam);                    
                    if(m_editmode==0)
                        m_input.m_caretpos=m_input.m_text.GetLength();
                } else if(wParam<VK_SPACE) {
                    ;
                } else {
                    m_input.appendAtCaret((TCHAR)wParam);
                }                

                // any query change cause new search
                OnQueryChange(m_input.m_text);

                Invalidate();
            }
            
            if(wParam!=27) {
                KillTimer(m_hwnd, 1);
                SetTimer(m_hwnd, 1, 10000, 0);
            }

            return FALSE;
        }

        return TRUE;
    }
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {   
        if(!OnKeyboardMessage(msg,wParam,lParam))
            return S_OK;

        if(msg==WM_INVALIDATEDISPLAY) {
            Update();
        } else if(msg==WM_RELOAD) {
            Reset();
            Init();            
        } else if(msg==WM_COMMAND) {
            if(wParam==0)
                PostQuitMessage(0);
            else if(wParam==1) {    
                CenterWindow(m_hwndsettings);
                ShowWindow(m_hwndsettings,SW_SHOW);
                ShowWindow(m_hwnd, SW_HIDE);
                ShowWindow(m_listhosthwnd, SW_HIDE);                        
            }
        } else if(msg == WM_LBUTTONUP) {
            int xPos = ((int)(short)LOWORD(lParam)); 
            int yPos = ((int)(short)HIWORD(lParam)); 
            
            if(CRect(CPoint(m_curWidth-20,5), CSize(15,15)).PtInRect(CPoint(xPos, yPos))) {
                HMENU hmenu=CreatePopupMenu();

                AppendMenu(hmenu, MF_STRING, 1, L"Options");
                AppendMenu(hmenu, MF_STRING, 0, L"Quit");
                
                POINT p={m_curWidth-20+5,5+5};
                ClientToScreen(m_hwnd, &p);
                TrackPopupMenu(hmenu, TPM_LEFTALIGN, p.x, p.y, 0, m_hwnd, 0);
                //OutputDebugStringA("click");
            }
        } else if(msg==WM_HOTKEY && wParam==1) {
            g_foregroundWnd=GetForegroundWindow();
            if(IsWindowVisible(m_hwnd)) {
                m_pane=0;
                m_args.clear();
                m_queries.clear();
                m_input.SetText(L"");

                ShowWindow(m_hwnd, SW_HIDE);
                ShowWindow(m_listhosthwnd, SW_HIDE);
            } else {
                ShowWindow(m_hwnd, SW_SHOW);
                SetForegroundWindow(m_hwnd);
                //h1=SetActiveWindow(m_hwnd);
                //h2=SetFocus(m_hwnd);
            }
        } else if(msg==WM_UPDATEINDEX) {
            //((std::map<CString,SourceResult> *)wParam)->begin()->second.source->updateIndex(((std::map<CString,SourceResult> *)wParam));
            //((std::map<CString,SourceResult> *)wParam)->begin()->second.source->save();
            OutputDebugString(L"crawling complete\n");
        } else if(msg==WM_PROGRESS) { // progress
            if(wParam!=100)                
                m_indexing=L"Updating index : "+ItoS(wParam)+L"%";
            else
                m_indexing=L"";
            Invalidate();
        } else if(msg == WM_NCHITTEST)
        {
            /*int xPos = ((int)(short)LOWORD(lParam)); 
            int yPos = ((int)(short)HIWORD(lParam)); 
            if(CRect(CPoint(m_curWidth-20,5), CSize(10,10)).PtInRect(CPoint(xPos, yPos))) {
                return HTCLIENT;
            }
            return HTCAPTION;*/
        }
        else if(msg==WM_TIMER && wParam==1)
        {
            if(IsWindowVisible(m_hwnd)) {
                KillTimer(m_hwnd, 1);
                ::ShowWindow(m_listhosthwnd, SW_SHOWNOACTIVATE);
            }
        }
        else if(msg==WM_TIMER && wParam==2)
        {
            if(IsWindowVisible(m_hwnd)) {
                Invalidate();
            }
        }
        else if(msg == WM_KILLFOCUS) {
            if((HWND)wParam!=m_listhosthwnd) {
                ShowWindow(m_hwnd, SW_HIDE);
                ShowWindow(m_listhosthwnd, SW_HIDE);
            }
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
    LRESULT ListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {           
        if(msg==WM_COMMAND && HIWORD(wParam)==LBN_SELCHANGE)
            OnSelChange(GetSelectedItem());
        else if(msg==WM_DRAWITEM) {
            DRAWITEMSTRUCT *dis=(DRAWITEMSTRUCT*)lParam;
            Graphics g(dis->hDC);
            g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

            Gdiplus::RectF r; 
            r.X=float(dis->rcItem.left);
            r.Y=float(dis->rcItem.top);
            r.Width=float(dis->rcItem.right-dis->rcItem.left);
            r.Height=float(dis->rcItem.bottom-dis->rcItem.top);

            if(dis->itemData && ((SourceResult*)dis->itemData)->source)
                ((SourceResult*)dis->itemData)->source->drawListItem(g,dis,r);
            //(SourceResult*)::SendMessage(m_listhwnd, LB_GETITEMDATA, sel, 0)
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    static LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return ((AlphaGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->WndProc(hwnd,msg,wParam,lParam);
    }
    static LRESULT _ListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
       
        return ((AlphaGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->ListBoxWndProc(hwnd,msg,wParam,lParam);
    }

    // old behavior
    int                m_editmode;

    WindowlessInput    m_input;
    HWND               m_hwnd;    
    CImage             m_background;
    CImage             m_background3;
    CImage             m_focus;
    CImage             m_knob;

    // text background
    CImage             m_textbackground;

    CString            lastResultExpand;

    // new behavior    
    uint                       m_displayPane;
    bool                       m_invalidatepending;
    int                        m_curWidth; // current width
    CImage                     premult;
    uint                       m_pane;
    std::map<CString, std::vector<Source*> > m_sources;
    SourceResult               m_nullresult;
    std::vector<Rule*>         m_rules;
    std::vector<SourceResult>  m_args;     // validated results
    std::vector<SourceResult>  m_results;  // currently displayed results
    std::vector<CString>       m_queries;
    std::vector<CString>       m_status;

    HWND                       m_listhwnd;
    HWND                       m_listhosthwnd;
    HWND                       m_hwndsettings;
    CString                    m_indexing;
};