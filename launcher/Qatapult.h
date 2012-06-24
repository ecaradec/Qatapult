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
#include "SystemTraySDK.h"

#include "ContactSource.h"
#include "SendEmail.h"
#include "EmailVerbRule.h"
#include "EmailFileVerbRule.h"
#include "WebsitePlugin.h"
#include "LevhensteinDistance.h"
#include "ActiveScriptHost.h"
#include "qatapult_h.h"
#include "CommandObject.h"
#include "History.h"

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

#define EA_NO_REMOVE_EXTRA 0
#define EA_REMOVE_EXTRA 1

void copySourceResult(RuleArg &ra, SourceResult &r);

struct Qatapult : IWindowlessGUI, UI, IDropTarget {
    Qatapult();
    ~Qatapult();
    HANDLE m_workerthread;
    
    bool isSourceEnabled(const char *name);
    bool isSourceByDefault(const char *name);
    void init();
    void reset();
    void loadRules(pugi::xml_document &settings);
    void reload();
    Source *addSource(Source *s);
    Source *addSource(const TCHAR *name,Source *s) ;
    void addRule(Type arg0,Type arg1,Type arg2,Rule *r);
    void addRule(Type arg0,Type arg1,Rule *r);
    void addRule(Type arg0,Rule *r);
    void addRule(Rule *r);
    static uint __stdcall crawlProc(Qatapult *thiz);
    HWND getHWND();
    void invalidateIndex() ;
    void invalidate();
    void createSettingsDlg();
    int  getActiveRules(int pane, std::vector<RuleArg> &args, std::vector<Rule*> &activerules);
    bool allowType(const CString &type);
    void collectItems(const CString &q, const uint pane, std::vector<RuleArg> &args, std::vector<SourceResult> &results, int def);
    static int resultSourceCmp(SourceResult &r1, SourceResult &r2);
    void onQueryChange(const CString &q, bool select=true);
    void showNextArg() ;
    void clearPanes();
    void exec();
    //void saveCommand(CommandObject *c);
    //void selectHistory(int historyindex);
    
    void onSelChange(SourceResult *r);
    void setRetArg(uint pane, SourceResult &r);    

    // stack modification function
    void setResult(uint pane, SourceResult &r);
    void addEmptyResult(uint pane);
    void cancelResult();
    
    // helpers
    Object *getArgObject(int i, int e);
    Object *getResObject(int i);
    void ensureArgsCount(std::vector<RuleArg> &ral,int l, int flags=EA_REMOVE_EXTRA);    

    // arg querying
    int getCurPane();
    CString getArgString(int c,int e,const TCHAR *name);
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

    void update();
    CString getQuery(int pane);
    int getFocus() { return m_pane; }
    SourceResult *getSelectedItem();
    void clearResults(std::vector<SourceResult> &results);
    void setCurrentSource(int pane,Source *s,CString &q);
    void show();
    void hide();
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT ListBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return ((Qatapult*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->WndProc(hwnd,msg,wParam,lParam);
    }

    // drag&drop
	STDMETHOD_(ULONG, AddRef)() {
		return 0;
	}
	STDMETHOD_(ULONG, Release)() {
		return 0;
	}
	STDMETHOD(QueryInterface)(REFIID, void**) {
		return E_NOINTERFACE;
	}
    STDMETHOD(DragEnter)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
        return S_OK;
    }        
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)  {
        return allowType(L"FILE") ? S_OK : S_FALSE;
    }        
    STDMETHOD(DragLeave)() {
        return S_OK;
    }        
    STDMETHOD(Drop)(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

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
    std::vector<RuleArg>       m_args;     // validated results
    std::vector<RuleArg>       m_retArgs;  // validated results

    //History                    m_history;
    //std::vector<std::shared_ptr<CommandObject> > m_commandhistory;
    //int                        m_historyindex;
    
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


    // gui stuff
    CSystemTray                m_systray;
};