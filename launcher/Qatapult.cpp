#include "stdafx.h"
#include "Qatapult.h"
#include "PainterScript.h"
#include "Collecter.h"
#include "JScriptSource.h"
#include "QatapultScript.h"
#include "TextItemSource.h"
#include "WindowSource.h"
#include "ShellExecuteRule.h"
#include "ScriptRule.h"
#include "SourceOfSources.h"
#include "SourceRule.h"

UI *g_pUI; // very lazy way to give access to the ui to the ui window proc
Qatapult *g_pQatapult;
#include "QatapultPlugin.h"

CString settingsini;
pugi::xml_document settings;
pugi::xml_document settingsWT; // settings for the working thread

WNDPROC OldHotKeyEditProc;
int     hotkeymodifiers=0;
int     hotkeycode=0;
HWND    g_foregroundWnd;
CString g_fontfamily;
DWORD   g_textcolor;

struct QatapultAtlModule : CAtlModule {
    HRESULT AddCommonRGSReplacements(IRegistrarBase *) {
        return S_OK;
    }
} _atlmodule;


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
    if(vk==VK_SPACE)
        c=L"SPACE";
    else if(vk==VK_PAUSE)
        c=L"PAUSE";
    else if(vk==VK_ESCAPE)
        c=L"ESCAPE";            
    else
        c=CString((TCHAR)vk);
            
    return mod+c;
}

CString getGUID() {
    CLSID  clsid;
    CoCreateGuid(&clsid);
    OLECHAR szGuid[40]={0}; int nCount = ::StringFromGUID2(clsid, szGuid, 40);
    return szGuid;
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
            OldHotKeyEditProc=SubclassWindowX(GetDlgItem(hWnd, IDC_HOTKEY1), HotKeyEditProc);
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
                // Get the first child window. Use it.
                HWND hwndChild = ::GetWindow(hWnd, GW_CHILD | GW_HWNDFIRST );
                while( hwndChild )
                {
                    ::SendMessage(hwndChild, WM_SAVESETTINGS, 0, 0);
                    // Get the next window. Use it.
                    hwndChild = ::GetWindow( hwndChild, GW_HWNDNEXT );
                }
                g_pQatapult->Reload();
                //g_pUI->InvalidateIndex();
                ::EndDialog(hWnd, 0);
            } else if(wParam==IDCANCEL) {
                ::EndDialog(hWnd, 0);
                g_pQatapult->Show();
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

struct PluginDesc {
    PluginDesc(){}
    PluginDesc(const TCHAR *n): name(n), key(n) {
    }
    CString key;
    CString name;
    CString desc;
};

void GetSubFolderList(const TCHAR *path, std::vector<CString> &subfolders) {
    TCHAR szFullPattern[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    // first we are going to process any subdirectories
    PathCombine(szFullPattern, path, _T("*"));
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && CString(FindFileData.cFileName)!=L"." && CString(FindFileData.cFileName)!=L"..")
                subfolders.push_back(FindFileData.cFileName);
        } while(FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
}

void getPluginList(std::vector<PluginDesc> &plugins) {
    plugins.push_back(L"Filesystem");
    plugins.push_back(L"IndexedFiles");
    plugins.push_back(L"Network");
    plugins.push_back(L"Contacts");
    plugins.push_back(L"Websites");
    plugins.push_back(L"FileHistory");
    plugins.push_back(L"ExplorerSelection");
    plugins.push_back(L"Windows");

    plugins.push_back(L"EmailFile");
    plugins.push_back(L"EmailText");
    plugins.push_back(L"WebsiteSearch");

    std::vector<CString> pluginsfolders;
    GetSubFolderList(L"plugins",pluginsfolders);
    for(std::vector<CString>::iterator it=pluginsfolders.begin();it!=pluginsfolders.end();it++) {
        CString key(*it);
        CString pluginxml=L"plugins\\"+*it+"\\plugin.xml";

        pugi::xml_document d;
        d.load_file(pluginxml);
        CStringA name=d.select_single_node("settings").node().child_value("name");
        CStringA desc=d.select_single_node("settings").node().child_value("description");        

        PluginDesc pd;
        pd.key=key;
        if(name==L"") {            
            pd.name=key;
            pd.desc=UTF8toUTF16(desc);
        } else {
            pd.name=UTF8toUTF16(name);
            pd.desc=UTF8toUTF16(desc);            
        }
        plugins.push_back(pd);
    }
}



class PluginsDlg : public CDialogImpl<PluginsDlg>
{
public:
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(PluginsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SAVESETTINGS, OnSaveSettings)        
    END_MSG_MAP()

    std::vector<PluginDesc> m_plugins;
    HWND                    m_hListView;

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {         
        INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        RECT rcClient;                       // The parent window's client area.

        GetClientRect(&rcClient); 

        // Create the list-view window in report view with label editing enabled.
        m_hListView = CreateWindow(WC_LISTVIEW, 
                                      L"",
                                      WS_CHILD | LVS_REPORT | WS_VISIBLE | WS_BORDER,
                                      0, 0,
                                      rcClient.right - rcClient.left,
                                      rcClient.bottom - rcClient.top,
                                      m_hWnd,
                                      0,
                                      0,
                                      NULL); 

        ListView_SetExtendedListViewStyle(m_hListView, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

        LVCOLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText=L"Active";
        lvc.fmt = LVCFMT_CENTER;
        lvc.cx=20;
        ListView_InsertColumn(m_hListView, 0, &lvc);

        lvc.pszText=L"Name";
        lvc.cx=120;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(m_hListView, 1, &lvc);

        lvc.pszText=L"Description";
        lvc.cx=rcClient.right - rcClient.left - 160;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(m_hListView, 2, &lvc);

        int z=0;
        Qatapult *pQ=(Qatapult*)lParam;
        int i=0;
        getPluginList(m_plugins);
        for(int i=0;i<m_plugins.size();i++) {
            LVITEM lvi;
            memset(&lvi, 0, sizeof(lvi));
            lvi.pszText=(LPWSTR)L"";
            lvi.mask=LVFIF_TEXT;
            lvi.iItem=i;
            int iitem=ListView_InsertItem(m_hListView, &lvi);
            ListView_SetItemText(m_hListView, iitem, 1, (LPWSTR)m_plugins[i].name.GetString());

            ListView_SetItemText(m_hListView, iitem, 2, (LPWSTR)m_plugins[i].desc.GetString());

            bool bChecked=pQ->isSourceEnabled(UTF16toUTF8(m_plugins[i].key));
            ListView_SetCheckState(m_hListView, iitem, bChecked);
        }

        //hcbox=CreateWindow(L"BUTTON", (*it)->m_name, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 0, i, 300, 25, hWnd, 0, 0, 0);
        //HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
		//SendMessage(hcbox, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));*/
        return TRUE;    // let the system set the focus
    }
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        m_plugins.clear();
        m_hListView=0;
        return TRUE;
    }
    LRESULT OnSaveSettings(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        for(int i=0;i<m_plugins.size();i++) {
            SetSettingsInt(L"sources/"+m_plugins[i].key, L"enabled", !!ListView_GetCheckState(m_hListView,i));
        }
        return TRUE;
    }
};

int objects=0;
Qatapult::Qatapult():m_input(this), m_invalidatepending(false) {
#ifdef DEBUG
    VLDMarkAllLeaksAsReported();
#endif          
    g_pQatapult=this;

    m_hwnd=0;
    m_request=0;
    m_pane=0;
    m_editmode=0;

    WCHAR curDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, curDir);

    g_pUI=this;

    if(GetFileAttributes(L"settings.xml") == INVALID_FILE_ATTRIBUTES) {
        SHFILEOPSTRUCT sffo={0};
        sffo.wFunc=FO_COPY;
        sffo.pFrom=L"settings.default.xml\0";
        sffo.pTo=L"settings.xml\0";
        SHFileOperation(&sffo);
    }
        
    CreateDirectory(L"databases", 0);

    CreateDirectory(L"skins", 0);        

    Init();

    // repaint dialog
    Invalidate();
}
Qatapult::~Qatapult() {        
    Reset();
}
int Qatapult::GetCurPane() {
    return m_pane;
}
CString Qatapult::getArgString(int c,const TCHAR *name) {
    if(c>=m_args.size())
        return L"";
    return m_args[c].object->getString(name);
}
int Qatapult::getArgsCount() {
    return m_args.size();
}
bool Qatapult::isSourceEnabled(const char *name) {
    pugi::xml_node n=settings.select_single_node("settings/sources/"+CStringA(name)+"/enabled").node();
    if(n.empty()) return true;
    return CStringA(n.child_value())=="1";
}
bool Qatapult::isSourceByDefault(const char *name) {
    pugi::xml_node n=settings.select_single_node("settings/sources/"+CStringA(name)+"/default").node();
    if(n.empty()) return true;
    return CStringA(n.child_value())=="1";
}

template<typename T>
std::vector<T> Array(T &t0) {
    std::vector<T> v;
    v.push_back(t0);
    return v;
}

template<typename T>
std::vector<T> Array(T &t0,T &t1) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    return v;
}

template<typename T>
std::vector<T> Array(T &t0,T &t1, T &t2) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    v.push_back(t1);
    return v;
}

#include <atlrx.h>

/*
class CSearchFoldersDlg : public CDialogImpl<CSearchFoldersDlg>
{
    public:
    enum { IDD = IDD_GMAILCONTACTS };
 
    BEGIN_MSG_MAP(CSearchFoldersDlg)
    END_MSG_MAP()
};

*/
/*
CAboutDlg about;    
CSearchFoldersDlg searchfolder;
about.Create(0);    
about.ShowWindow(SW_SHOW);
searchfolder.Create(about.m_hWnd);
*/

void Qatapult::Init() {
    m_crawlprogress=0;
    g_textcolor=0xFFFFFFFF;
    m_fontsize=10.0f;
    m_textalign=StringAlignmentCenter;
    m_textrenderinghint=TextRenderingHintAntiAliasGridFit;
    m_stringtrimming=StringTrimmingEllipsisCharacter;

    m_resultbgcolor=0xFFFFFFFF;
    m_resultfocuscolor=0xFFDDDDFF;
    m_resultscrollbarcolor=0xFF000000;

    settings.load_file("settings.xml");
    //SetSettingsInt(L"sources/test", L"enabled", 0);

    //SetSettingsString(L"general",L"font",L"Arial");

    g_fontfamily=GetSettingsString(L"general",L"font",L"Arial");
        
    m_skin=L"skins/"+GetSettingsString(L"general",L"skin",L"default");
  
    m_buffer.Create(640,800,32,PixelFormat32bppARGB);

    if(m_hwnd==0) {
        m_hwnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"", /*WS_VISIBLE|*/WS_POPUP|WS_CHILD, 0, 0, 0, 0, 0, 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);
    }    

    m_pQatapultScript=QatapultScript::Make(this);
    m_pQatapultScript->AddRef();    

    m_painter.Initialize(L"Qatapult",L"JScript");    
    m_painter.AddObject(L"qatapult",(IDispatch*)m_pQatapultScript);
    m_painter.Require(m_skin+L"\\painter.js");

    m_pPainterScript=PainterScript::Make(this);
    m_pPainterScript->AddRef();


    m_commandsHost.Initialize(L"Qatapult",L"JScript");
    m_commandsHost.AddObject(L"qatapult",(IDispatch*)m_pQatapultScript);
    

    m_focusedresult=0;
    m_resultspos=0;
    m_textalign=StringAlignmentCenter;

    // the sources can't be unloaded and reloaded easily as they use threads
    // sources
    //settings.select_single_node("settings/sources/")

    sourceofsources=new SourceOfSources;
    addSource(sourceofsources);
    sourceofsources->def=true;

    addSource(L"Filesystem",new FileSource);
    addSource(L"IndexedFiles",new StartMenuSource(m_hwnd));
    addSource(L"Network",new NetworkSource);
    addSource(L"Contacts",new ContactSource);
    addSource(L"Websites",new WebsiteSource);
    addSource(L"FileHistory",new FileHistorySource);
    addSource(L"ExplorerSelection",new CurrentSelectionSource);
    addSource(L"Windows", new WindowSource);

    Source *tt=addSource(new TextSource);
    tt->def=true;
    addSource(new FileVerbSource);    

    //addRule(L"CLOCK", new ClockRule);
    addRule(Type(L"FILE"), Type(L"FILEVERB"), new FileVerbRule); 
                
    TextItemSource *t;

    t=new TextItemSource(L"QATAPULTVERB");
    addSource(t);
    t->addItem(L"Quit",L"icons\\exit.png");
    t->addItem(L"Reload",L"icons\\reload.png");
    t->addItem(L"Options",L"icons\\options.png");
    //t->def=true;
    addRule(Type(L"FILE", Array(Type::Predicat(L"rfilename", L"QATAPULT.EXE"))), Type(L"QATAPULTVERB"), new QatapultRule);

    if(isSourceEnabled("EmailFile")) {
        t=new TextItemSource(L"EMAILFILEVERB");
        addSource(t);
        t->addItem(L"Email to",L"icons\\emailto.png");
        addRule(Type(L"FILE"),Type(t->type),Type(L"CONTACT"),new EmailFileVerbRule);
    }

    if(isSourceEnabled("EmailText")) {
        t=new TextItemSource(L"EMAILTEXTVERB");
        addSource(t);
        t->addItem(L"Email to",L"icons\\emailto.png");        
        addRule(Type(L"TEXT"),Type(t->type),Type(L"CONTACT"),new EmailVerbRule);        
    }

    if(isSourceEnabled("WebsiteSearch")) {
        t=new TextItemSource(L"SEARCHWITHVERB");
        addSource(t);
        t->addItem(L"Search With",L"icons\\searchwith.png");        
        addRule(Type(L"TEXT"),Type(t->type),Type(L"WEBSITE"),new WebSearchRule);
    }

    t=new TextItemSource(L"SOURCEVERB");
    addSource(t);
    t->addItem(L"Open",L"icons\\open.png");
    addRule(Type(L"SOURCE"),Type(t->type),new SourceRule(this));

    m_emptysource=t=new TextItemSource(L"EMPTY");
    t->addItem(L"",L"");
    addSource(m_emptysource);

    // pseudo source for object created as return
    m_inputsource=new Source(L"INPUTSOURCE");
    addSource(m_inputsource);

    // hotkey
    hotkeycode=GetSettingsInt(L"hotKeys", L"toggleKey",VK_SPACE);
    hotkeymodifiers=GetSettingsInt(L"hotKeys", L"toggleModifier",MOD_SHIFT);      

    // define hotkey
    RegisterHotKey(g_pUI->getHWND(), 1, hotkeymodifiers, hotkeycode);

    LoadRules(settings);

    // load plugins rules & sources
    std::vector<CString> pluginsfolders;
    GetSubFolderList(L"plugins", pluginsfolders);
    for(std::vector<CString>::iterator it=pluginsfolders.begin(); it!=pluginsfolders.end(); it++) {                
        CString folder=L"plugins\\"+*it;
        CString pluginname=*it;
        if(!isSourceEnabled(CStringA(pluginname)))
            continue;
        
        if(FileExists(folder+L"\\plugin.xml")) {
            pugi::xml_document d;
            d.load_file(folder+L"\\plugin.xml");
            LoadRules(d);
        }

        std::vector<CString> collectors;
        FindFilesRecursively(folder,L"*.collecter.js",collectors,1,0);
        FindFilesRecursively(folder,L"*.collector.js",collectors,1,0);
        for(std::vector<CString>::iterator it=collectors.begin(); it!=collectors.end(); it++) {
            CString str(*it);
            str=str.Mid(8);
            str=str.Left(str.Find(L"\\"));
            str=str.MakeLower();
            Source *s=new JScriptSource(this,str,*it);
            s->def=true;
            addSource(s);
        }

        std::vector<CString> commands;
        FindFilesRecursively(folder,L"*.command.js",commands,1,0);
        for(std::vector<CString>::iterator it=commands.begin(); it!=commands.end(); it++) {
            m_commandsHost.Require(*it);
        }
    }
    
    for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
        (*it)->m_pUI=this;
    }
    
    for(std::vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); it++) {
        (*it)->m_pArgs=&m_args;
    }
    
    OnQueryChange(L"");

    
    // create dialogs
    CreateSettingsDlg();
    

    //CStringA *remoteversion=new CStringA;
    //HttpGet(L"http://emmanuelcaradec.com/qatapult/currentversion.txt",remoteversion);
    //PostMessage(g_pUI->getHWND(), WM_CURRENTVERSION, (WPARAM)remoteversion, 0);


    // we shouldn't create a thread for each source, this is inefficient
    // crawl should be called with an empty index for each source
    //_beginthread((void (*)(void*))crawlProc, 0, this);
    //m_workerthread=CreateThread(0, 0, (LPTHREAD_START_ROUTINE)crawlProc, this, 0, &m_crawlThreadId);
    m_workerthread = (HANDLE)_beginthreadex(0, 0, (uint (__stdcall *)(void*))crawlProc, this, 0, (uint*)&m_crawlThreadId );
    
    m_mainThreadId=GetCurrentThreadId();

    BOOL b=PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);

    PostThreadMessage(m_crawlThreadId, WM_RELOADSETTINGS, 0, 0);            
    //PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);
    //PostQuitMessage(0);
}

SourceResult Qatapult::getEmptyResult() {
    SourceResult r(L"",L"",L"",m_emptysource,0,0,0);
    r.object=new Object(L"EMPTY",L"EMPTY",m_emptysource,L"");
    return r;
}

PluginsDlg pluginsdlg;
SearchFoldersDlg searchfolderdlg;

void Qatapult::Reset() {
    if(pluginsdlg.IsWindow())
        pluginsdlg.DestroyWindow();

    if(searchfolderdlg.IsWindow())
        searchfolderdlg.DestroyWindow();

    m_buffer.Destroy();

    bool b=!!PostThreadMessage(m_crawlThreadId,WM_STOPWORKERTHREAD,0,0);
    if(WaitForSingleObject(m_workerthread,5000)==WAIT_TIMEOUT)
        TerminateThread(m_workerthread,0);

    if(m_pPainterScript)
        m_pPainterScript->Release();
    m_pPainterScript=0;
    
    if(m_pQatapultScript)
        m_pQatapultScript->Release();
    m_pQatapultScript=0;

    m_painter.Reset();

    m_commandsHost.Reset();

    for(std::map<CString,Gdiplus::Bitmap*>::iterator it=m_bitmaps.begin(); it!=m_bitmaps.end(); it++) 
        delete it->second;
    m_bitmaps.clear();

    // bitmaps
    UnregisterHotKey(m_hwnd,1);

    ClearResults(m_results);
    ClearResults(m_nextresults);

    for(std::vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); it++) {
        delete *it;
    }        
    m_rules.clear();

    for(uint i=0;i<m_args.size();i++) {
        if(m_args[i].source)
            m_args[i].source->clear(m_args[i]);
    }
    m_args.clear();

    int j=0;
    for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++, j++) {
        delete (*it);
    }
    m_sources.clear();
}

void Qatapult::LoadRules(pugi::xml_document &settings) {
    pugi::xpath_node_set ns=settings.select_nodes("/settings/rules/rule");
    for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++) {
        Rule *r=0;
        CString script=it->node().child_value("script");
        if(script!=L"") {
            r=new ScriptRule(&m_commandsHost,UTF8toUTF16(it->node().child_value("script")));
        } else {
            r=new ShellExecuteRule(UTF8toUTF16(it->node().child_value("cmd")),
                                   UTF8toUTF16(it->node().child_value("args")),
                                   UTF8toUTF16(it->node().child_value("workdir")));
        }
        m_rules.push_back(r);
        
        pugi::xpath_node_set ns=it->node().select_nodes("arg");
        for(pugi::xpath_node_set::const_iterator itarg=ns.begin(); itarg!=ns.end(); itarg++) {
            CString arg=itarg->node().first_child().value();                
            if(arg.GetLength()!=0) {
                r->m_types.push_back(Type(arg));
            } else if(CString(itarg->node().child_value("type"))!=L"") {
                CString type(itarg->node().child_value("type"));

                std::vector<Type::Predicat> preds;
                pugi::xpath_node_set elts=itarg->node().select_nodes("pred");
                for(pugi::xpath_node_set::const_iterator itelt=elts.begin(); itelt!=elts.end(); itelt++) {
                    CHAR name[256];
                    CHAR op[256];
                    CStringA str(itelt->node().child_value());
                    sscanf_s(str, "%[^~=]", name, sizeof(name));
                    sscanf_s(str.Mid(strlen(name)), "%[~=]", op, sizeof(op));
                    CString value(str.Mid(strlen(name)+strlen(op)));
                    preds.push_back(Type::Predicat(name,value,op));
                }
                
                r->m_types.push_back(Type(type,preds));
            } else {
                pugi::xpath_node_set elts=itarg->node().select_nodes("item");
                TextItemSource *t=new TextItemSource(getGUID());
                addSource(t);
                t->def = (itarg==ns.begin()); // if first then it's a default source
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
void Qatapult::Reload() {
    PostMessage(getHWND(),WM_RELOAD,0,0);
}
Source *Qatapult::addSource(Source *s) {
    m_sources.push_back(s);
    return s;
}
Source *Qatapult::addSource(const TCHAR *name,Source *s) {
    CStringA n(UTF16toUTF8(name));
    if(isSourceEnabled(n)) {
        sourceofsources->m_sources.push_back(s);
        addSource(s);
        s->def=isSourceByDefault(n);        
    } else {
        delete s;
    }
    return s;
}
void Qatapult::addRule(Type arg0,Type arg1,Type arg2,Rule *r) {
    r->m_types.push_back(arg0);
    r->m_types.push_back(arg1);
    r->m_types.push_back(arg2);
    m_rules.push_back(r);
}
void Qatapult::addRule(Type arg0,Type arg1,Rule *r) {
    r->m_types.push_back(arg0);
    r->m_types.push_back(arg1);
    m_rules.push_back(r);
}
void Qatapult::addRule(Type arg0,Rule *r) {
    r->m_types.push_back(arg0);
    m_rules.push_back(r);
}
void Qatapult::addRule(Rule *r) {
    m_rules.push_back(r);
}
uint __stdcall Qatapult::crawlProc(Qatapult *thiz) {    
    // personal copy of the settings file for the thread
    settingsWT.load_file("settings.xml");
    MSG msg;
    msg.message=WM_INVALIDATEINDEX; // create a fake message on first pass
    do {            
        switch(msg.message)  {
            case WM_INVALIDATEINDEX:
                {                        
                    BOOL b=PostMessage(thiz->m_hwnd, WM_PROGRESS, 0, 0);
                    float nbsources=float(thiz->m_sources.size());
                    float isources=0;
                    for(std::vector<Source*>::iterator it=thiz->m_sources.begin(); it!=thiz->m_sources.end();it++) {
                        // if a stop thread message is available stop everything
                        if(PeekMessage(&msg,0,WM_STOPWORKERTHREAD,WM_STOPWORKERTHREAD,PM_NOREMOVE)) {
                            b=PostMessage(thiz->m_hwnd, WM_PROGRESS, 100, 0);
                            goto stop;
                        }
                        (*it)->crawl();

                        isources++;
                        b=PostMessage(thiz->m_hwnd, WM_PROGRESS, WPARAM(100.0f*isources/nbsources), 0);
                    }
                }
                break;
            case WM_STOPWORKERTHREAD:
                goto stop;
                break;
            case WM_RELOADSETTINGS:
                settingsWT.load_file("settings.xml");
                break;
        }
        
        //TranslateMessage(&msg);
        //DispatchMessage(&msg);
    } while(GetMessage(&msg, 0, 0, 0)!=0);

    stop:
    _endthread();
    return TRUE;
}
HWND Qatapult::getHWND() {
    return m_hwnd;
}
void Qatapult::InvalidateIndex() {
    PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);
}
void Qatapult::Invalidate() {
    if(m_invalidatepending==false) {
        PostMessage(m_hwnd, WM_INVALIDATEDISPLAY, 0, 0);        
        m_invalidatepending=true;
    }
}

extern BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Qatapult::CreateSettingsDlg() {
    m_hwndsettings=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), 0, (DLGPROC)SettingsDlgProc);  //CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GET_GMAILAUTHCODE), 0, DlgProc);
    HWND hTreeView=GetDlgItem(m_hwndsettings, IDC_TREE);    

    pluginsdlg.Create(m_hwndsettings);
    pluginsdlg.SetWindowPos(0, 160, 11, 0, 0, SWP_NOSIZE);

    HWND hwndGmail=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GMAILCONTACTS), m_hwndsettings, (DLGPROC)DlgProc);
    SetWindowPos(hwndGmail, 0, 160, 0, 0, 0, SWP_NOSIZE);

    //HWND hwndEmail=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EMAIL), m_hwndsettings, (DLGPROC)DlgProc);
    //SetWindowPos(hwndEmail, 0, 160, 0, 0, 0, SWP_NOSIZE);

    searchfolderdlg.Create(m_hwndsettings);
    searchfolderdlg.SetWindowPos(0, 160, 11, 0, 0, SWP_NOSIZE);

    HWND hwndG=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GENERAL), m_hwndsettings, (DLGPROC)GeneralDlgProc);
    SetWindowPos(hwndG, 0, 160, 0, 0, 0, SWP_NOSIZE);


    TV_INSERTSTRUCT tviis;
    ZeroMemory(&(tviis.item), sizeof(TV_ITEM));    
    tviis.itemex.state=TVIS_BOLD;    
    tviis.itemex.stateMask=TVIS_BOLD;
    tviis.hParent = TVI_ROOT;

    tviis.item.pszText = L"Preferences";
    tviis.item.mask = TVIF_TEXT|TVIF_STATE;
    HTREEITEM htreeP=TreeView_InsertItem(hTreeView, &tviis);

    tviis.item.mask = TVIF_TEXT|TVIF_PARAM;

    tviis.item.pszText = L"General";
    tviis.item.lParam=(LPARAM)hwndG;
    HTREEITEM htreeG=TreeView_InsertItem(hTreeView, &tviis);

    tviis.item.pszText = L"Plugins";
    tviis.item.lParam=(LPARAM)pluginsdlg.m_hWnd;
    HTREEITEM htreePl=TreeView_InsertItem(hTreeView, &tviis);

    tviis.item.pszText = L"Plugins preferences";
    tviis.item.mask = TVIF_TEXT|TVIF_STATE;
    HTREEITEM htreePg=TreeView_InsertItem(hTreeView, &tviis);

    tviis.item.mask = TVIF_TEXT|TVIF_PARAM;

    tviis.item.pszText = L"Search folders";
    tviis.item.lParam=(LPARAM)searchfolderdlg.m_hWnd;
    HTREEITEM htreeSF=TreeView_InsertItem(hTreeView, &tviis);

    tviis.item.pszText = L"Gmail contacts";
    tviis.item.lParam=(LPARAM)hwndGmail;
    HTREEITEM htreeGmail=TreeView_InsertItem(hTreeView, &tviis);

    BOOL b=TreeView_SelectItem(hTreeView, htreeG);
}

void Qatapult::CollectItems(const CString &q, const uint pane, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int def) {
    ClearResults(results);

    // collect all active rules (match could have an args that tell how much to match )
    // i should probably ignore the current pane for the match or just match until pane-1  

    // collect displayable items
    if(pane>=m_customsources.size() || m_customsources[pane]==0) {

        std::vector<Rule *> activerules;
        for(uint i=0;i<m_rules.size();i++)
            if(m_rules[i]->match(args, pane)>0)
                activerules.push_back(m_rules[i]); 

        if(activerules.size()==0) {
            return;
        }

        // collect all active sources at this level            
        std::map<CString,bool> activesources;
        for(uint i=0;i<activerules.size();i++)
            if(activerules[i]->m_types.size()>pane)
                activesources[activerules[i]->m_types[pane].m_type]=true;

        if(activesources.size()==0) {
            return;
        }

        for(std::vector<Source*>::iterator it=m_sources.begin();it!=m_sources.end();it++) {                
            if(pane==0 && q==L"")
                ;
            else if(pane==0 && !(*it)->def)
                ;
            else {
                //LONGLONG t0;
	            //QueryPerformanceCounter((LARGE_INTEGER *) &t0);

                (*it)->collect(q, results, def, activesources);

                //LONGLONG t1;
	            //QueryPerformanceCounter((LARGE_INTEGER *) &t1);
                //OutputDebugString((*it)->m_name+" : "+ItoS(t1-t0)+"\n");
            }
        }
        
    } else {
        std::map<CString,bool> activesources;
        m_customsources[pane]->collect(q,results,def,activesources);
    }

    time_t currentTime;
    time(&currentTime);

    int matchingBonus;
    int usageBonus;
    int lastUseBonus;
    int tdiff;

    uselev=0;
    CString Q(q); Q.MakeUpper();
    int qlen=max(1, Q.GetLength());
    for(uint i=0;i<results.size();i++) {
        // pourcentage de chaine correspondante ?
        results[i].source->rate(q,&results[i]);
    }
}

int Qatapult::ResultSourceCmp(SourceResult &r1, SourceResult &r2) {
    return r1.rank > r2.rank;
}

struct Results {
    int                        request;
    int                        pane;
    std::vector<SourceResult>  results;
};

void Qatapult::OnQueryChange(const CString &q) {
    m_request++;
    m_focusedresult=0;

    Results *presults=new Results;
    presults->request=m_request;
    presults->pane=m_pane;

    CollectItems(q, m_pane, m_args, presults->results, 0);        

    PostMessage(getHWND(), WM_PUSHRESULT,(WPARAM)presults,0);
}
void Qatapult::ShowNextArg() {
    // check if there is extra args     
    if(m_args.size()>0) {
        Results *presults=new Results;
        presults->request=m_request;
        presults->pane=m_pane+1;

        CollectItems(L"", m_pane+1, m_args, presults->results, 1);

        PostMessage(getHWND(), WM_PUSHRESULT2,(WPARAM)presults,0);
    }
}
void Qatapult::setArg(uint pane, SourceResult &r) {
    if(pane==m_args.size()) {
        m_args.push_back(SourceResult());
        r.source->copy(r,&m_args.back());
    } else {
        m_args[pane].source->clear(m_args[pane]);
        r.source->copy(r,&m_args[pane]);        
    }
}

void Qatapult::setRetArg(uint pane, SourceResult &r) {
    while(m_retArgs.size()<=pane) m_retArgs.push_back(SourceResult());
    m_retArgs[pane]=r;      
}

void Qatapult::OnSelChange(SourceResult *r) {        
    if(m_args.size()==0)
        m_args.push_back(getEmptyResult());
        
    // a copy is not enough if there is deep data because the results are cleaned after the query
    setArg(m_pane, *r);
    
    ShowNextArg();

    Invalidate();
}

CString Qatapult::getResString(int i, const TCHAR *name) {
    if(i>=m_results.size())
        return L"";
    if(CString(name)==L"rank")
        return ItoS(m_results[i].rank);
    return m_results[i].object->getString(name);
}

void Qatapult::setVisibleResCount(int i) {
    m_visibleresultscount=i;
}

static HDC   g_HDC;
void Qatapult::drawBitmap(const TCHAR *text, INT x, INT y, INT w, INT h){    
    if(m_bitmaps.find(text)==m_bitmaps.end()) {
        Gdiplus::Bitmap *p=Gdiplus::Bitmap::FromFile(text);
        m_bitmaps[text]=p;        
    }
    
    Gdiplus::Bitmap *f=m_bitmaps[text];

    Gdiplus::Graphics g(g_HDC);

    g.SetInterpolationMode(InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(PixelOffsetModeHalf);
    g.SetSmoothingMode(SmoothingModeNone);

    g.DrawImage(f,x,y,w,h);
    //delete f;

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}

void Qatapult::drawInput(INT x, INT y, INT w, INT h){
    StringFormat sf;

    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHint(m_textrenderinghint));
    m_input.Draw(g, RectF(float(x),float(y),float(w),float(h)), sf, L'',m_fontsize,g_textcolor); /*m_args[m_pane].source->m_prefix*/

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}

void Qatapult::drawText(const TCHAR *text, INT x, INT y, INT w, INT h) {
    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHint(m_textrenderinghint));
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    StringFormat sf;
    sf.SetAlignment(StringAlignment(m_textalign));
    sf.SetTrimming(StringTrimming(m_stringtrimming));
    Gdiplus::Font f(g_fontfamily, m_fontsize);
    g.DrawString(text,-1,&f,RectF(float(x),float(y),float(w),float(h)),&sf,&SolidBrush(g_textcolor));
}

void Qatapult::drawItem(INT i, INT x, INT y, INT w, INT h){
    if(i>=m_args.size())
        return;

    Graphics g(g_HDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);        
    g.SetCompositingQuality(CompositingQualityHighQuality);

    m_args[i].object->drawItem(g, &m_args[i], RectF(float(x), float(y), float(w), float(h)));

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}

void Qatapult::drawResItem(INT i, INT x, INT y, INT w, INT h){
    if(i>=m_results.size())
        return;

    Graphics g(g_HDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);        
    g.SetCompositingQuality(CompositingQualityHighQuality);

    m_results[i].object->drawItem(g, &m_results[i], RectF(float(x), float(y), float(w), float(h)));

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}


void Qatapult::drawEmphased(const TCHAR *text, const TCHAR *highlight, INT flag, INT x, INT y, INT w, INT h){
    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHint(m_textrenderinghint));
    ::drawEmphased(g,text,highlight,RectF(float(x),float(y),float(w),float(h)),flag,StringAlignment(m_textalign),m_fontsize,g_textcolor);
}

void Qatapult::drawResults(INT x, INT y, INT w, INT h){
    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHint(m_textrenderinghint));

    m_focusedresult=max(0,m_focusedresult);
    m_focusedresult=min(m_results.size()-1,m_focusedresult);

    m_visibleresultscount=min(m_results.size(),int(h/40));
    if(m_focusedresult>=m_resultspos+m_visibleresultscount)
        m_resultspos=m_focusedresult-m_visibleresultscount+1;
    
    if(m_focusedresult<m_resultspos)
        m_resultspos=m_focusedresult;
    
    int rw;
    if(m_visibleresultscount<m_results.size())
        rw=w-9;
    else
        rw=w;

    for(int i=m_resultspos;i<m_resultspos+m_visibleresultscount;i++) {
        int p=i-m_resultspos;
        m_results[i].object->drawListItem(g,&m_results[i],RectF(float(x),float(y+40*p),float(rw),float(40)),m_fontsize,m_focusedresult==i,g_textcolor,m_resultbgcolor,m_resultfocuscolor);
    }
    
    if(m_visibleresultscount<m_results.size()) {        
        g.FillRectangle(&SolidBrush(Color(m_resultbgcolor)),x+w-9,y,9,h);
        g.FillRectangle(&SolidBrush(Color(m_resultscrollbarcolor)),x+w-7,y+2+(h-4)*m_resultspos/m_results.size(),5,(h-4)*m_visibleresultscount/m_results.size());
    }

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}
 
void Qatapult::fillRectangle(INT x, INT y, INT w, INT h, DWORD c){
    Graphics g(g_HDC);
    g.FillRectangle(&SolidBrush(Color(c)),x,y,w,h);
}

void Qatapult::Update() {
    CString str;
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
    g_HDC=m_buffer.GetDC();
    Graphics g(g_HDC);

    g.Clear(Gdiplus::Color(0,0,0,0));
    //g.Clear(Gdiplus::Color(0xFF,0xFF,0xFF,0xFF));
    //g.FillRectangle(&SolidBrush(Color(0xFF,0,0,0)),RectF(0,0,400,400));

    m_status.resize(m_args.size());

    m_curWidth=0;
    m_curHeight=0;
    
    CComVariant ret;
    CComSafeArray<VARIANT> ary;
    CComVariant v;        
    ary.Add(CComVariant(CComVariant((IDispatch*)m_pPainterScript)));
    m_painter.Run(CComBSTR(L"draw"),ary.GetSafeArrayPtr(),&ret);
        
    CRect workarea;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

    POINT p1={(workarea.left+workarea.right)/2-m_curWidth/2,200};
    POINT p2={0};
    SIZE s={m_curWidth, m_curHeight};
    BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, g_HDC, &p2, 0, &bf, ULW_ALPHA);
      
    m_buffer.ReleaseDC();
}    
CString Qatapult::getQuery(int p) {
    if(p==m_queries.size())
        return m_input.m_text;
    else if(p>m_queries.size())
        return L"";
    return m_queries[p];
}
SourceResult *Qatapult::GetSelectedItem() {
    if(m_results.size()==0)
        return 0;
    return &m_results[m_focusedresult];
}
void Qatapult::ClearResults(std::vector<SourceResult> &results) {
    for(uint j=0;j<results.size();j++) {            
        results[j].source->clear(results[j]);
    }
    results.clear();
}
void Qatapult::SetCurrentSource(int pane,Source *s,CString &q) {
    if(m_customsources.size()<=uint(pane))
        m_customsources.resize(pane+1);

    if(s==(Source*)-1)
        m_customsources[pane]=0;
    else
        m_customsources[pane]=s;
    m_input.SetText(q);
    Invalidate();
}
void Qatapult::Show() {
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}
void Qatapult::Hide() {
    ShowWindow(m_hwnd, SW_HIDE);
    KillTimer(m_hwnd, 1);
    //ShowWindow(m_listhosthwnd, SW_HIDE);
}
void Qatapult::showMenu(int xPos,int yPos) {
    HMENU hmenu=CreatePopupMenu();

    AppendMenu(hmenu, MF_STRING, 1, L"Options");
    AppendMenu(hmenu, MF_STRING, 0, L"Quit");
                
    POINT p={xPos,yPos};
    ClientToScreen(m_hwnd, &p);
    TrackPopupMenu(hmenu, TPM_LEFTALIGN, p.x, p.y, 0, m_hwnd, 0);
}
LRESULT Qatapult::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {   
    if(msg == WM_KEYDOWN) {        
        CComSafeArray<VARIANT> ary;
        CComVariant ret;
        ary.Add(CComVariant(CComVariant(wParam)));
        m_painter.Run(L"onKeyDown",ary.GetSafeArrayPtr(),&ret);
        if(ret.vt==VT_BOOL)
            return ret.boolVal;
    }    
    
    if(msg == WM_CURRENTVERSION)
    {
        CStringA *currentversion=(CStringA*)wParam;
        if(CString(*currentversion)!=L"" && CString(*currentversion)!=getFileContent(L"currentversion.txt")) {
            if( MessageBox(0, L"A new version of Qatapult is available. Do you want to get it ?", L"Qatapult", MB_YESNO) == IDYES)
                ShellExecute(0, 0, L"http://emmanuelcaradec.com/qatapult",0,0,SW_SHOW);
        }
        delete currentversion;
    }
    else if(msg == WM_PUSHRESULT)
    {
        Results *p=(Results*)wParam;
        if(m_request==p->request) {
            // because there is only one set of results for the moment this is enough
            // if we post from things with delay like timer or httprequest we should only
            // clear on the first message
            ClearResults(m_results);            
            
            for(int i=0;i<p->results.size();i++) {
                m_results.push_back(p->results[i]);
            }
            std::sort(m_results.begin(), m_results.end(), ResultSourceCmp);

            if(m_results.size()>0) {
                OnSelChange(&m_results.front());
            } else {
                SourceResult &r=getEmptyResult();
                OnSelChange(&r);
                r.source->clear(r);
            }
            delete p;
        }
        return TRUE;
    }
    else if(msg == WM_PUSHRESULT2)
    {
        Results *p=(Results*)wParam;
        if(m_request==p->request) {
            // remove all args beyond current pane
            while(m_pane+1<m_args.size()) {
                m_args.back().source->clear(m_args.back());
                m_args.pop_back();
            }

            ClearResults(m_nextresults);

            for(int i=0;i<p->results.size();i++) {
                m_nextresults.push_back(p->results[i]);
            }
            std::sort(m_nextresults.begin(), m_nextresults.end(), ResultSourceCmp);

            if(m_nextresults.size()!=0) {
                setArg(m_pane+1,m_nextresults.front());
            }

            delete p;
        }
        return TRUE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_RETURN)
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
        setArg(m_pane, *r);

        // collect all active rules
        // might need to disambiguate here ?
        for(uint i=0;i<m_rules.size();i++)
            if(m_rules[i]->match(m_args, m_args.size())>1) {
                // found one rule
                Hide();

                m_retArgs.clear();
                if(m_rules[i]->execute(m_args)) {                                                
                    for(uint a=0;a<m_args.size(); a++) {
                        m_args[a].source->validate(&m_args[a]);
                    }

                    ClearResults(m_results);
                    ClearResults(m_nextresults);

                    // don't clear args if 
                    if(m_retArgs.size() == 0) {
                        m_args.clear();
                        m_pane=0;
                        m_queries.clear();                        
                    } else {
                        m_args=m_retArgs;
                        m_pane=m_args.size();
                        m_queries.clear();
                        m_queries.resize(m_args.size());
                        Invalidate();
                        Show();
                    }

                    m_input.SetText(L"");                    

                    return FALSE;
                }
            }
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_ESCAPE)
    {
        if(m_pane<m_customsources.size() && m_customsources[m_pane]!=0) {
            m_customsources[m_pane]=0;
            m_input.SetText(L"");
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
        // clear the idle
        KillTimer(m_hwnd, 1);

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
            bool bCtrl=!!(GetKeyState(VK_CONTROL)&0x8000);
            m_input.moveCaretLeft(bCtrl);
        } else if(m_pane<m_customsources.size()) {
            m_customsources[m_pane]=0;
            m_input.SetText(L"");
            Invalidate();
        }
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_RIGHT)
    {
        if(m_editmode==1) {
            bool bCtrl=!!(GetKeyState(VK_CONTROL)&0x8000);
            m_input.moveCaretRight(bCtrl);
        } else {
            SourceResult *r=GetSelectedItem();
            if(!r)
                return FALSE;
            CString q;
            Source *s=r->source->getSource(*r,q);
            if(s!=0) {
                SetCurrentSource(m_pane,s,q);
            } else {
                m_input.SetText(r->object->getString(L"expand"));
            }
        }
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_BACK)
    {
        bool bCtrl=!!(GetKeyState(VK_CONTROL)&0x8000);
        m_input.back(bCtrl);            
        return FALSE;
    }        
    else if(msg == WM_KEYDOWN && wParam == VK_DELETE)
    {
        bool bCtrl=!!(GetKeyState(VK_CONTROL)&0x8000);
        m_input.del(bCtrl);
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && (wParam == VK_DOWN || wParam==VK_NEXT))
    {	
        if(m_editmode==1) {
            ;
        } else {
            if(wParam == VK_DOWN) {
                m_focusedresult++;
            } else if(wParam == VK_NEXT) {
                m_focusedresult+=m_visibleresultscount;
            }

            if(m_focusedresult>=m_results.size())
                m_focusedresult=m_results.size()-1;

            if(m_focusedresult>=m_resultspos+m_visibleresultscount)
                m_resultspos=m_focusedresult-m_visibleresultscount+1;
    
            if(m_results.size()>0)
                OnSelChange(&m_results[m_focusedresult]);
            Invalidate();
        }
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && (wParam == VK_UP || wParam == VK_PRIOR))
    {	
        if(m_editmode==1) {
            ;
        } else {
            if(wParam == VK_UP)
                m_focusedresult--;
            else if(wParam == VK_PRIOR)
                m_focusedresult-=m_visibleresultscount;

            if(m_focusedresult<0)
                m_focusedresult=0;

            if(m_focusedresult<m_resultspos)
                m_resultspos=m_focusedresult;

            if(m_results.size()>m_focusedresult)
                OnSelChange(&m_results[m_focusedresult]);
            Invalidate();
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
    else if(msg==WM_KEYDOWN && wParam=='E' && !!(GetKeyState(VK_CONTROL)&0x8000))
    {
        if(m_editmode==0)
            m_editmode=1;
        else
            m_editmode=0;
        if(m_editmode==0)
            m_input.m_caretpos=m_input.m_text.GetLength();
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
            } else if((wParam == L'/' || wParam == L'\\') && GetSelectedItem() && GetSelectedItem()->object && GetSelectedItem()->object->type==L"FILE")
            {
                SourceResult *r=GetSelectedItem();
                CString path=r->object->getString(L"expand");
                m_input.SetText(path);
            }
            else if((wParam == L'?') && GetSelectedItem() && GetSelectedItem()->object && GetSelectedItem()->object->type==L"FILE")
            {
                SourceResult *r=GetSelectedItem();
                CString t(r->object->getString(L"expand"));
                t.TrimRight(L'\\');
                CString d=t.Left(t.ReverseFind(L'\\'));
                if(d==L"")
                    m_input.SetText(L"");
                else
                    m_input.SetText(d+L"\\");
            } else if(wParam<VK_SPACE) {
                ;
            } else {
                m_input.appendAtCaret((TCHAR)wParam);
            }

            // any query change cause new search
            if(wParam!=VK_RETURN)
                OnQueryChange(m_input.m_text);

            //Update();
            Invalidate();
        }
            
        if(wParam!=VK_ESCAPE && wParam!=VK_TAB) {
            KillTimer(m_hwnd, 1);
            SetTimer(m_hwnd, 1, 5000, 0);
        }
    } else if(msg==WM_CLOSE) {
        DestroyWindow(hwnd);
        return S_OK;
    } else if(msg==WM_INVALIDATEDISPLAY) {
        Update();
    } else if(msg==WM_RELOAD) {
        Reset();
        Init();
        ShowWindow(m_hwnd, SW_SHOW);
    } else if(msg==WM_COMMAND) {
        if(wParam==0)
            PostQuitMessage(0);
        else if(wParam==1) {    
            CenterWindow(m_hwndsettings);
            ShowWindow(m_hwndsettings,SW_SHOW);
            ShowWindow(m_hwnd, SW_HIDE);
        }
    } else if(msg == WM_LBUTTONUP) {
        int xPos = ((int)(short)LOWORD(lParam)); 
        int yPos = ((int)(short)HIWORD(lParam)); 

        CComSafeArray<VARIANT> ary;
        CComVariant ret;
        ary.Add(CComVariant(CComVariant(xPos)));
        ary.Add(CComVariant(CComVariant(yPos)));
        m_painter.Run(CComBSTR(L"onClick"),ary.GetSafeArrayPtr(),&ret);
            
        /*if(CRect(CPoint(m_curWidth-20,5), CSize(15,15)).PtInRect(CPoint(xPos, yPos))) {
            HMENU hmenu=CreatePopupMenu();

            AppendMenu(hmenu, MF_STRING, 1, L"Options");
            AppendMenu(hmenu, MF_STRING, 0, L"Quit");
                
            POINT p={m_curWidth-20+5,5+5};
            ClientToScreen(m_hwnd, &p);
            TrackPopupMenu(hmenu, TPM_LEFTALIGN, p.x, p.y, 0, m_hwnd, 0);
            //OutputDebugStringA("click");
        }*/
    } else if(msg==WM_HOTKEY && wParam==1) {
        g_foregroundWnd=GetForegroundWindow();            
        if(IsWindowVisible(m_hwnd)) {
            m_pane=0;
            m_args.clear();
            m_queries.clear();
            m_input.SetText(L"");
            m_customsources.clear();

            Hide();
        } else {
            Show();
        }
    } else if(msg==WM_UPDATEINDEX) {
        //((std::map<CString,SourceResult> *)wParam)->begin()->second.source->updateIndex(((std::map<CString,SourceResult> *)wParam));
        //((std::map<CString,SourceResult> *)wParam)->begin()->second.source->save();
        OutputDebugString(L"crawling complete\n");
    } else if(msg==WM_PROGRESS) { // progress
        m_crawlprogress=wParam;
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
        KillTimer(m_hwnd, 1);
        if(IsWindowVisible(m_hwnd)) {
            CComSafeArray<VARIANT> ary;
            CComVariant ret;
            ary.Add(CComVariant(CComVariant(wParam)));
            m_painter.Run(L"onIdle",ary.GetSafeArrayPtr(),&ret);
        }
    }
    else if(msg==WM_TIMER && wParam==2)
    {
        if(IsWindowVisible(m_hwnd)) {
            Invalidate();
        }
    }
    else if(msg == WM_KILLFOCUS) {
        // TODO : call the painter to signal focus lost
        Invalidate();
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}