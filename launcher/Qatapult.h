#pragma once

#ifdef DEBUG
//#define VLD
#endif

#ifdef VLD
#include "vld.h"
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
#include "LevhensteinDistance.h"
#include "ActiveScriptHost.h"
#include "qatapult_h.h"

extern UI *g_pUI; // very lazy way to give access to the ui to the ui window proc
extern CString settingsini;
extern pugi::xml_document settings;
extern pugi::xml_document settingsWT; // settings for the working thread

extern WNDPROC OldHotKeyEditProc;
extern int hotkeymodifiers;
extern int hotkeycode;
extern HWND g_foregroundWnd;

#define WM_UPDATEINDEX (WM_USER)
#define WM_INVALIDATEINDEX (WM_USER+1)
#define WM_INVALIDATEDISPLAY (WM_USER+2)
#define WM_PROGRESS (WM_USER+3)
#define WM_RELOAD (WM_USER+4)
#define WM_STOPWORKERTHREAD (WM_USER+5)
#define WM_RELOADSETTINGS (WM_USER+6)
#define WM_PUSHRESULT (WM_USER+7)
#define WM_PUSHRESULT2 (WM_USER+8)
#define WM_CURRENTVERSION (WM_USER+9)
#define WM_SAVESETTINGS (WM_USER+10)

struct QatapultScript;
struct PainterScript;
struct SourceOfSources;

struct Qatapult : IWindowlessGUI, UI {
    Qatapult();
    ~Qatapult();
    HANDLE m_workerthread;
    
    bool isSourceEnabled(const char *name);
    bool isSourceByDefault(const char *name);
    void Init();
    SourceResult getEmptyResult();
    void Reset();
    void LoadRules(pugi::xml_document &settings);
    void Reload();
    Source *addSource(Source *s);
    Source *addSource(const TCHAR *name,Source *s) ;
    void addRule(Type arg0,Type arg1,Type arg2,Rule *r);
    void addRule(Type arg0,Type arg1,Rule *r);
    void addRule(Type arg0,Rule *r);
    void addRule(Rule *r);
    static uint __stdcall crawlProc(Qatapult *thiz);
    HWND getHWND();
    void InvalidateIndex() ;
    void Invalidate();
    void CreateSettingsDlg();
    void CollectItems(const CString &q, const uint pane, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int def);
    static int ResultSourceCmp(SourceResult &r1, SourceResult &r2);
    void OnQueryChange(const CString &q);
    void ShowNextArg() ;
    void setArg(uint pane, SourceResult &r);
    void setRetArg(uint pane, SourceResult &r);
    void OnSelChange(SourceResult *r);

    // arg querying
    int GetCurPane();
    CString getArgString(int c,const TCHAR *name);
    int getArgsCount();
    CString getResString(INT c, const TCHAR* name);
    void setVisibleResCount(INT i);

    // collecter
    void addResults(std::vector<SourceResult> *results);

    // public drawing functions
    void drawBitmap(const TCHAR *text, INT x, INT y, INT w, INT h);
    void drawInput(INT x, INT y, INT w, INT h);
    void drawText(const TCHAR *text, INT x, INT y, INT w, INT h);
    void drawItem(INT c, INT e, INT x, INT y, INT w, INT h);
    void drawResItem(INT i, INT x, INT y, INT w, INT h);
    void drawEmphased(const TCHAR *text, const TCHAR *highlight, int flag, INT x, INT y, INT w, INT h);
    void drawResults(INT x, INT y, INT w, INT h);
    void fillRectangle(INT x, INT y, INT w, INT h, DWORD c);

    void showMenu(int xPos,int yPos);

    void Update();
    CString getQuery(int pane);
    int getFocus() { return m_pane; }
    SourceResult *GetSelectedItem();
    void ClearResults(std::vector<SourceResult> &results);
    void SetCurrentSource(int pane,Source *s,CString &q);
    void Show();
    void Hide();
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT ListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return ((Qatapult*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->WndProc(hwnd,msg,wParam,lParam);
    }

    // text input
    int                        m_editmode;
    WindowlessInput            m_input; 
    HWND                       m_hwnd;
    HWND                       m_hwndsettings;

    // sources and rules    
    SourceOfSources           *sourceofsources;
    Source                    *m_emptysource;
    Source                    *m_inputsource;
    DWORD                      m_crawlThreadId;
    DWORD                      m_mainThreadId;
    std::vector<Source*>       m_customsources;

    // runtime source and rules
    std::vector<Source*>       m_sources;
    std::vector<Rule*>         m_rules;
    std::vector<SourceResult>  m_args;     // validated results
    std::vector<SourceResult>  m_retArgs;     // validated results
    
    // ui status    
    int                        m_focusedresult;
    uint                       m_pane;
    INT                        m_crawlprogress;
    int                        m_visibleresultscount;
    int                        m_resultspos;
    int                        m_request;       // request index
    std::vector<SourceResult>  m_results;       // currently displayed results
    std::vector<SourceResult>  m_nextresults;   // currently displayed results
    std::vector<CString>       m_queries;
    std::vector<CString>       m_status;
    CString                    m_indexing;

    // painting
    CString                    m_skin;
    ActiveScriptHost           m_painter;
    QatapultScript            *m_pQatapultScript;
    PainterScript             *m_pPainterScript;

    std::map<CString,Gdiplus::Bitmap*> m_bitmaps;
    int                        m_defaultwidth;    
    CImage                     m_buffer;
    bool                       m_invalidatepending;
    int                        m_curWidth;
    int                        m_curHeight;
    float                      m_fontsize;
    DWORD                      m_textalign;
    DWORD                      m_textrenderinghint;
    DWORD                      m_stringtrimming;
    DWORD                      m_resultfocuscolor;
    DWORD                      m_resultscrollbarcolor;
    DWORD                      m_resultbgcolor;
     
    // actions
    ActiveScriptHost           m_commandsHost;
};