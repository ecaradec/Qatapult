#pragma once
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
//#include "vld.h"
#endif

#include "resource.h"
//#include <atlrx.h>
#include "premultAlpha.h"
#include "md5.h"
#include "utf8.h"
#include "CriticalSection.h"
#include "pugixml.hpp"
#include "simpleini.h"
#include "geticon.h"
#include "ShellLink.h"
#include "Utility.h"
#include "SourceResult.h"
#include "FileObject.h"

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
#include "ActiveScriptHost.h"
#include "qatapult_h.h"
#include <dispex.h>

extern UI *g_pUI; // very lazy way to give access to the ui to the ui window proc
extern CString settingsini;
extern pugi::xml_document settings;
extern pugi::xml_document settingsWT; // settings for the working thread

extern WNDPROC OldHotKeyEditProc;
extern int hotkeymodifiers;
extern int hotkeycode;
extern HWND g_foregroundWnd;

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
    bool execute(std::vector<SourceResult> &args) {
        return true;
    }
};
*/

#define WM_UPDATEINDEX (WM_USER)
#define WM_INVALIDATEINDEX (WM_USER+1)
#define WM_INVALIDATEDISPLAY (WM_USER+2)
#define WM_PROGRESS (WM_USER+3)
#define WM_RELOAD (WM_USER+4)
#define WM_STOPWORKERTHREAD (WM_USER+5)
#define WM_RELOADSETTINGS (WM_USER+6)

struct TextItemSource : Source {
    TextItemSource(const TCHAR *name) : Source(name) {
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        if(!r->object)
            return 0;
        return Gdiplus::Bitmap::FromFile(m_index[r->object->key].iconname);
    }
    void addItem(const TCHAR *str,const TCHAR *iconname) {
        m_index[str]=SourceResult(str,str,str, this, 0, 0, m_index[str].bonus);
        m_index[str].iconname=iconname;
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            // (*m_pArgs)[g_pUI->GetCurPane()-1].display
            //CString path=(*m_pArgs)[0].source->getString((*m_pArgs)[0],L"lpath");
            /*CAtlRegExp<> re;            
            re.Parse(L"{[0-9]?[0-9]}:{[0-9][0-9]}");
            if(path.Right(4)==L".lnk")*/
            if(FuzzyMatch(it->second.display,q)) {
                results.push_back(it->second);
                Object *o=new Object(it->second.expand, type, this, it->second.expand);
                o->values[L"icon"]=it->second.iconname;
                results.back().object=o;
            }
        }
    }
    std::map<CString, SourceResult> m_index;
};

struct WindowSource : Source {
    WindowSource() : Source(L"WINDOW",L"Windows (Catalog )") {
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\window.png");
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        std::vector<HWND> windowList;
        EnumWindows(CollectWindows,(LPARAM)&windowList);
        for(std::vector<HWND>::iterator it=windowList.begin(); it!=windowList.end(); it++) {
            if(!::IsWindowVisible(*it))
                continue;
            TCHAR title[MAX_PATH]={0};
            GetWindowText(*it,title,sizeof(title));
            if(FuzzyMatch(title,q)) {
                results.push_back(SourceResult(title,title,title,this,0,*it,0));
                Object *o=new Object(ItoS((int)*it),type,this,title);
                o->values[L"title"]=title;
                o->values[L"hwnd"]=ItoS((int)*it);
                results.back().object=o;
            }
        }
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

struct SourceOfSources : Source {
    SourceOfSources() : Source(L"SOURCE",L"Source of sources") {
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\source.png");
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
            if(FuzzyMatch((*it)->m_name,q)) {
                SourceResult r;
                r.expand=r.display=(*it)->m_name;
                r.source=this;
                r.data=*it;
                results.push_back(r);
                results.back().object=new Object((*it)->m_name,type,this,(*it)->m_name);
            }        
        }
    }
    virtual Source *getSource(SourceResult &sr, CString &q) {
        q=L"";
        return (Source*)sr.data;
    }
    std::vector<Source*> m_sources;
};

struct SourceRule : Rule {
    SourceRule(UI *pUI):m_pUI(pUI) {
    }
    bool execute(std::vector<SourceResult> &args) {
        CString q;
        Source *s=(*m_pArgs)[0].source->getSource((*m_pArgs)[0],q);
        m_pUI->SetCurrentSource(0,s,q);
        m_pUI->Show();
        return true;
    }
    UI *m_pUI;
};

struct QatapultScript;

struct AlphaGUI : IWindowlessGUI, UI {
    AlphaGUI();
    ~AlphaGUI();
    HANDLE m_workerthread;
    int GetCurPane();
    CString getArgString(int c,const TCHAR *name);
    int getArgsCount();
    bool isSourceEnabled(const char *name);
    bool isSourceByDefault(const char *name);
    void Init();
    SourceResult getEmptyResult();
    void Reset();
    void LoadRules(pugi::xml_document &settings);
    void Reload();
    Source *addSource(Source *s);
    Source *addSource(const TCHAR *name,Source *s) ;
    void addRule(const CString &arg0,const CString &arg1,const CString &arg2,Rule *r);
    void addRule(const CString &arg0,const CString &arg1,Rule *r);
    void addRule(const CString &arg0,Rule *r);
    void addRule(Rule *r);
    static uint __stdcall crawlProc(AlphaGUI *thiz);
    HWND getHWND();
    void InvalidateIndex() ;
    void Invalidate();
    void CreateSettingsDlg();
    void CollectItems(const CString &q, const uint pane, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int def);
    static int ResultSourceCmp(SourceResult &r1, SourceResult &r2);
    void OnQueryChange(const CString &q);
    void ShowNextArg() ;
    void SetArg(uint pane, SourceResult &r);
    void OnSelChange(SourceResult *r);


    // public drawing functions
    void drawBitmap(const TCHAR *text, INT x, INT y, INT w, INT h);
    void drawInput(INT x, INT y, INT w, INT h);
    void drawText(const TCHAR *text, INT x, INT y, INT w, INT h);
    void drawItem(INT c, INT x, INT y, INT w, INT h);
    void drawEmphased(const TCHAR *text, const TCHAR *highlight, int flag, INT x, INT y, INT w, INT h);
    void drawResults(INT x, INT y, INT w, INT h);

    void showMenu(int xPos,int yPos);

    void Update();
    CString getQuery(int pane);
    int getFocus() { return m_pane; }
    SourceResult *GetSelectedItem();
    void ClearResults(std::vector<SourceResult> &results);
    void SetCurrentSource(int pane,Source *s,CString &q);
    void Show();
    void Hide();
    LRESULT OnKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT ListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return ((AlphaGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->WndProc(hwnd,msg,wParam,lParam);
    }

    // text input
    int                        m_editmode;
    WindowlessInput            m_input; 
    HWND                       m_hwnd;
    HWND                       m_hwndsettings;

    // sources and rules    
    SourceOfSources           *sourceofsources;
    Source                    *m_emptysource;
    DWORD                      m_crawlThreadId;
    DWORD                      m_mainThreadId;
    std::vector<Source*>       m_customsources;

    // runtime source and rules
    std::vector<Source*>       m_sources;
    std::vector<Rule*>         m_rules;
    std::vector<SourceResult>  m_args;     // validated results
    
    // ui status
    int                        m_focusedresult;
    uint                       m_pane;
    INT                        m_crawlprogress;
    bool                       m_resultsvisible;
    int                        m_resultspos;
    std::vector<SourceResult>  m_results;  // currently displayed results
    std::vector<CString>       m_queries;
    std::vector<CString>       m_status;        
    CString                    m_indexing;

    // painting
    CString                    m_skin;
    std::map<CString,Gdiplus::Bitmap*> m_bitmaps;
    ActiveScriptHost           host;
    QatapultScript            *m_pQatapultScript;    
    int                        m_defaultwidth;    
    CImage                     m_buffer;
    bool                       m_invalidatepending;
    int                        m_curWidth;
    int                        m_curHeight;        
    INT                        m_textcolor;    
    float                      m_fontsize;
    DWORD                      m_textalign;
    DWORD                      m_textrenderinghint;
    DWORD                      m_stringtrimming;
};