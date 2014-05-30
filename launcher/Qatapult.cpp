/*
Dans un premier temps utiliser map est suffisant. Inutile de créer un deuxieme type d'objet plus complexe. Une fois 
que cela sera fait, il y aurra moins besoin de creer des milliers d'objets

Est ce que toutes ces créations d'objets est toujours embetante ? Surtout maintenant que j'ai supprimé les predicats
qui rendaient le filtrage compliqué ?

Les sauvegardes de commandes seraient encore assez pratiques

Se focaliser sur des trucs simples à utiliser

Remarque : pour le javascript je dois déjà créer des objets à partir d'un object idispatch (like a map )

Tous les objets pourraient être insérés dans la même base avec 2-3 colonnes ? ID TEXT JSON ??

Trucs que je veux pouvoir faire :
- envoyer un email à quelqu'un : 
    - Email > CONTACT > TEXT
    - TEXT > Email > CONTACT
    - TEXT (emmanuel.caradec@gmail.com) > Email > TEXT ?
- tuer un processus
    - PROCESS > Kill
- manipuler les fenetres
    - WINDOW > Switch
    - WINDOW > Maximize
    - WINDOW > Minimize
- sauvegarder une commande
    - Ctrl+Enter should change anything into a command
    - Ctrl+S (Show command on the first pane, Save on the second and text on the third )
    - COMMAND > Save > TEXT => Save command in a file or a DB (DB are not for commands )
    - COMMAND > Open 
- uninstall that always works

- actions uniquements sur les images (sans predicats ???? ) => FILE > Resize > TEXT
    - Cette regle s'active uniquement si : $0.ext=='.png' || $0.ext=='.jpg' || $0.ext=='.bmp'
- actions uniquements sur les zip (sans predicats ???? )


Autorecognized types : emmanuel.caradec@gmail.com could be handled via the default text source. It already works
the issue is that I don't know how to match the rule then since the rule will ask for CONTACT it can't match TEXT.

The solution is that sources could produce somthing that they are asked : Contact source product CONTACT when contact is 
asked or when TEXT is asked and it match something interesting.


Utiliser la source donne plus de flexibilité mais moins de réutilisation à moins de passer par des objets supplémentaires

Object o(uint8 *store, std::map<Type*> &types);
o.source=this;
o.type=types["FILE"]; // we need to register types somewhere
o.addString("filename","test");
o.addString("expand","c:\\test");
results.push_back(o);

Then all the methods are on the type ?

o.type->drawItem(o);

or 

o.source->drawItem(o)


extra values could be handled inside the shell command ? like extra arguments are.

command could have a "only show when" option

putty > connect > server

would be better handled as a custom application with a keyword putty 
instead of having to look for the putty.exe file which is way too complex

*/

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
#include "CommandRule.h"
#include "Record.h"

UI       *g_pUI; // very lazy way to give access to the ui to the ui window proc
Qatapult *g_pQatapult;

#include "PluginList.h"

CString settingsini;
pugi::xml_document settings;
pugi::xml_document settingsWT; // settings for the working thread

WNDPROC OldHotKeyEditProc;
HWND    g_foregroundWnd;
CString g_fontfamily;
DWORD   g_textcolor;

struct Hotkey {
    TCHAR  *name;
    int     mod;
    int     vk;
    TCHAR  *desc;
    bool match(int mod_, int vk_) {
        return mod==mod_ && vk==vk_;
    }
};

Hotkey hotkeys[]={ 
    {L"toggle", MOD_SHIFT,          VK_SPACE,           L"Show/hide Qatapult"},         
    {L"execute", 0,                 VK_RETURN,          L"Execute the current action if possible"}, 
    {L"stack", 0,                   VK_OEM_COMMA,       L"Stack the current selection and select another object (multi selection )"}, 
    {L"cancel", 0,                  VK_ESCAPE,          L"Cancel a result, cancel a pane, close if empty"}, 
    {L"nextpane", 0,                VK_TAB,             L"Move to next pane"}, 
    {L"previouspane", MOD_SHIFT,    VK_TAB,             L"Move to previous pane"}, 
    {L"exitsource", 0,              VK_LEFT,            L"Exit a source (on a source result only )"}, 
    {L"expand", 0,                  VK_RIGHT,           L"Expand a result or enter a source"}, 
    {L"nextresult", 0,              VK_DOWN,            L"Show the result pane or focus the next result"}, 
    {L"previousresult", 0,          VK_UP,              L"Select the previous result"}, 
    {L"nextresultpage", 0,          VK_NEXT,            L"Next page of results"}, 
    {L"previousresultpage", 0,      VK_PRIOR,           L"Previous page of results"}, 
    {L"editmode", MOD_CONTROL,      'E',                L"Toggle edit mode"}, 
    {L"makecommand", MOD_CONTROL,   VK_RETURN,          L"Make a command"},
    {L"paste", MOD_CONTROL,         'V',                L"Paste (Text )"},
};

#define HK_SHOW 0
#define HK_EXECUTE 1
#define HK_STACK 2
#define HK_CANCEL 3
#define HK_NEXTPANE 4
#define HK_PREVPANE 5
#define HK_EXITSOURCE 6
#define HK_EXPANDSOURCE 7
#define HK_NEXTRESULT 8
#define HK_PREVRESULT 9 
#define HK_NEXTRESULTPAGE 10
#define HK_PREVRESULTPAGE 11
#define HK_EDITMODE 12
#define HK_MKCOMMAND 13
#define HK_PASTE 14
#define HK_LAST 15

#define WM_ICON_NOTIFY WM_APP+10


#include "SettingsDlg.h"
#include "QatapultPlugin.h"
#include "DllSource.h"

SettingsDlg* settingsdlg=0;

struct QatapultAtlModule : CAtlModule {
    HRESULT AddCommonRGSReplacements(IRegistrarBase *) {
        return S_OK;
    }
} _atlmodule;

CString getGUID() {
    CLSID  clsid;
    CoCreateGuid(&clsid);
    OLECHAR szGuid[40]={0}; int nCount = ::StringFromGUID2(clsid, szGuid, 40);
    return szGuid;
}

int DelayLoadExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS *ep) {
    return EXCEPTION_CONTINUE_EXECUTION;
}

int objects=0;
Qatapult::Qatapult():m_input(this), m_invalidatepending(false) {
#ifdef VLD
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
    
    
    CString filename;
    GetModuleFileName(0,filename.GetBufferSetLength(MAX_PATH), MAX_PATH); filename.ReleaseBuffer();
    CString cmdline="\""+filename+"\" /install=%1";
    CString icon="\""+filename+"\",128";
    CString appname=L"Qatapult";
    CString mimetype=L"application/x-qatapult";
    CString open=L"open";

    HKEY hkey;
    RegCreateKey(HKEY_CURRENT_USER, L"Software\\Classes\\.qplugin", &hkey);
    RegSetKeyValue(hkey,L"",L"Content Type", REG_SZ, mimetype, mimetype.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"",L"", REG_SZ, appname, appname.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"DefaultIcon",L"", REG_SZ, icon, icon.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"shell",L"", REG_SZ, open, open.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"shell\\open\\command",L"", REG_SZ, cmdline, cmdline.GetLength()*sizeof(WCHAR));
    RegCloseKey(hkey);

    RegCreateKey(HKEY_CURRENT_USER, L"Software\\Classes\\.qskin", &hkey);
    RegSetKeyValue(hkey,L"",L"Content Type", REG_SZ, mimetype, mimetype.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"",L"", REG_SZ, appname, appname.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"DefaultIcon",L"", REG_SZ, icon, icon.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"shell",L"", REG_SZ, open, open.GetLength()*sizeof(WCHAR));
    RegSetKeyValue(hkey,L"shell\\open\\command",L"", REG_SZ, cmdline, cmdline.GetLength()*sizeof(WCHAR));
    RegCloseKey(hkey);

    websites.upgrade();
    networkshares.upgrade();

    init();    

    // repaint dialog
    invalidate();
}
Qatapult::~Qatapult() {        
    reset();
}
int Qatapult::getCurPane() {
    return m_pane;
}
CString Qatapult::getArgString(int c,int e, const TCHAR *name) {
    if(c>=m_args.size() || m_args[c].isEmpty() || m_args[c].object(e)==0)
        return L"";

    if(CString(name)==L"json") {
        return m_args[c].object()->toJSON();
    }

    return m_args[c].object(e)->getString(name);
}
int Qatapult::getArgsCount() {
    return m_args.size();
}
bool Qatapult::isSourceEnabled(const char *name) {
    pugi::xml_node n=settings.select_single_node("settings/sources/"+CStringA(name)+"/enabled").node();
    if(n.empty()) return true;
    return CStringA(n.child_value())=="1";
}
LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg,
			   WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
    // If the user wants to close the application
    case WM_DESTROY:
        // then close it
        PostQuitMessage(WM_QUIT);
        break;
    default:
        // Process the left-over messages
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    // If something was not done, let it go
    return 0;
}

void Qatapult::init() {
    //CString tmp1=getShortcutPath(L"C:\\Users\\emmanuel\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Qatapult.exe - Raccourci.lnk");
    //CString tmp2=getShortcutPath(L"C:\\Users\\emmanuel\\Desktop\\Qatapult.exe - Raccourci.lnk");
    //CString tmp3=getShortcutPath(L"C:\\Users\\public\\Desktop\\GIMP 2.lnk");

    m_pane=0;
    m_crawlprogress=0;
    g_textcolor=0xFFFFFFFF;
    m_fontsize=10.0f;
    m_textalign=StringAlignmentCenter;
    m_textrenderinghint=TextRenderingHintAntiAliasGridFit;
    m_stringtrimming=StringTrimmingEllipsisCharacter;

    m_resultbgcolor=0xFFFFFFFF;
    m_resultfocuscolor=0xFFDDDDFF;
    m_resultscrollbarcolor=0xFF000000;

    //m_historyindex = -1; // -1 means outside the index
    //m_history.loadCommands(m_commandhistory);

    settings.load_file("settings.xml");
    //SetSettingsInt(L"sources/test", L"enabled", 0);

    //SetSettingsString(L"general",L"font",L"Arial");

    g_fontfamily=GetSettingsString(L"general",L"font",L"Arial");
        
    m_skin=GetSettingsString(L"general",L"skin",L"default");
  
    m_buffer.Create(640,800,32,PixelFormat32bppARGB);

	WNDCLASSEX WndClsEx;

	// Create the application window
	WndClsEx.cbSize        = sizeof(WNDCLASSEX);
	WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
	WndClsEx.lpfnWndProc   = WndProcedure;
	WndClsEx.cbClsExtra    = 0;
	WndClsEx.cbWndExtra    = 0;
	WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	WndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
	WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClsEx.lpszMenuName  = NULL;
	WndClsEx.lpszClassName = L"Qatapult";
	WndClsEx.hInstance     = 0;
	WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	// Register the application
	RegisterClassEx(&WndClsEx);

    HRESULT hr;
    if(m_hwnd==0) {
        m_hwnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"Qatapult", /*WS_VISIBLE|*/WS_POPUP|WS_CHILD, 0, 0, 0, 0, 0, 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);
        hr=RegisterDragDrop(m_hwnd, this);
        //Enabledrop
    }    

    m_pQatapultScript=QatapultScript::Make(this);
    m_pQatapultScript->AddRef();     

    m_painter.Initialize(L"Qatapult",L"JScript");    
    m_painter.AddObject(L"qatapult",(IDispatch*)m_pQatapultScript);
    m_painter.Require(L"skins\\"+m_skin+L"\\painter.js");

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
    
    addSource(L"Filesystem",new FileSource);
    addSource(L"IndexedFiles",new StartMenuSource(m_hwnd));
    addSource(L"Network",new NetworkSource);
    addSource(L"FileHistory",new FileHistorySource);
    addSource(L"ExplorerSelection",new CurrentSelectionSource);
    addSource(L"Windows", new WindowSource);    

    Source *tt=addSource(new TextSource);

    // should be populated from sources
    m_types.push_back(L"");
    m_types.push_back(L"FILE");
    m_types.push_back(L"TEXT");
    m_types.push_back(L"WINDOW");
    m_types.push_back(L"COMMAND");
    m_types.push_back(L"WEBSITE");

    // file rules
    addRule(Type(L"FILE",true), Keyword(L"Open",L"icons\\open.png"), new OpenFileRule);
    addRule(Type(L"FILE",true), Keyword(L"Edit",L"icons\\edit.png"), new EditFileRule);
    addRule(Type(L"FILE",true), Keyword(L"Run As",L"icons\\runas.png"), new RunAsFileRule);
    addRule(Type(L"FILE",true), Keyword(L"Properties",L"icons\\properties.png"), new PropertiesFileRule);    
    
    // qatapult rule
    addRule(Keyword(L"Qatapult (app)",L"icons\\qatapult.png"), Keyword(L"Quit",L"icons\\exit.png"), new QuitQatapultRule);
    addRule(Keyword(L"Qatapult (app)",L"icons\\qatapult.png"), Keyword(L"Reload",L"icons\\reload.png"), new ReloadQatapultRule);
    addRule(Keyword(L"Qatapult (app)",L"icons\\qatapult.png"), Keyword(L"Options",L"icons\\options.png"), new OptionsQatapultRule);    

    // empty
    TextItemSource *t;
    m_emptysource=t=new TextItemSource(L"EMPTY");
    t->addItem(L"",L"");
    addSource(m_emptysource);

    // pseudo source for object created as return
    // objects created that way should probably store which source has created them and send to the correct source for loading
    m_inputsource=new Source(L"INPUTSOURCE");
    addSource(m_inputsource);
    //m_history.m_inputsource = m_inputsource;
    

    //
    // load hotkeys
    //
    
    // special case for show
    hotkeys[HK_SHOW].vk=GetSettingsInt(L"hotKeys/"+CString(hotkeys[HK_SHOW].name), L"vk", hotkeys[HK_SHOW].vk);
    hotkeys[HK_SHOW].mod=GetSettingsInt(L"hotKeys/"+CString(hotkeys[HK_SHOW].name), L"mod", hotkeys[HK_SHOW].mod);    
    if(hotkeys[HK_SHOW].vk==-1)
        hotkeys[HK_SHOW].vk=GetSettingsInt(L"hotKeys", L"toggleKey",VK_SPACE);
    if(hotkeys[HK_SHOW].mod==-1)
        hotkeys[HK_SHOW].mod=GetSettingsInt(L"hotKeys", L"toggleModifier",MOD_SHIFT);
    RegisterHotKey(g_pUI->getHWND(), 1, hotkeys[HK_SHOW].mod, hotkeys[HK_SHOW].vk);

    // everything else
    //for(int i=1;i<HK_LAST;i++) {
    //    hotkeys[i].vk=GetSettingsInt(L"hotKeys/"+CString(hotkeys[i].name), L"vk", hotkeys[i].vk);
    //    hotkeys[i].mod=GetSettingsInt(L"hotKeys/"+CString(hotkeys[i].name), L"mod", hotkeys[i].mod);
    //}

    loadRules(settings);

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
            loadRules(d);
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

    m_args.push_back(RuleArg());
    m_args.back().m_results.push_back(SourceResult());

    m_queries.push_back(L"");
    
    onQueryChange(L"");    

    HINSTANCE hinst=GetModuleHandle(NULL);
    HICON hicon=LoadIcon(hinst,MAKEINTRESOURCE(IDR_MAINFRAME));
    m_systray.Create(hinst, 0, WM_ICON_NOTIFY, L"Qatapult", hicon, IDR_SYSTRAY);
    m_systray.SetTargetWnd(m_hwnd);

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

    //createSettingsDlg();
    //settingsdlg->CenterWindow();
    //settingsdlg->ShowWindow(SW_SHOW);
}

Type Qatapult::Keyword(const TCHAR *text, const TCHAR *icon, bool def) {
    //TextItemSource *t=new TextItemSource(getGUID());
    CString type=CString(L"keyword/")+text;
    TextItemSource *t=0;
    for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
        if((*it)->m_name == type) {
            return Type(type);
        }
    }

    t=new TextItemSource(CString(L"keyword/")+text);
    addSource(t);
    t->addItem(text,icon);

    return Type(type);
}

void clearRuleArg(RuleArg &r) {
    r.m_results.clear();
    r.m_results.push_back(SourceResult());
}

void Qatapult::reset() {
    m_systray.Destroy();
    
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

    clearResults(m_results);
//    clearResults(m_nextresults);

    for(std::vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); it++) {
        delete *it;
    }        
    m_rules.clear();
    
    for(uint i=0;i<m_args.size();i++) {        
        clearRuleArg(m_args[i]);
    }
    m_args.clear();

    int j=0;
    for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++, j++) {
        delete (*it);
    }
    m_sources.clear();

    //m_commandhistory.clear();
}

void Qatapult::loadRules(pugi::xml_document &settings) {
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
                pugi::xpath_node_set::const_iterator itelt=elts.begin();
                CHAR name[256];
                CHAR op[256];
                CStringA str(itelt->node().child_value());
                sscanf_s(str, "%[^~=]", name, sizeof(name));
                sscanf_s(str.Mid(strlen(name)), "%[~=]", op, sizeof(op));
                CString value(str.Mid(strlen(name)+strlen(op)));
                preds.push_back(Type::Predicat(name,op,value));                
                
                r->m_types.push_back(Type(type,false,preds));
            } else {
                pugi::xpath_node_set elts=itarg->node().select_nodes("item");                
                
                CString lbl=UTF8toUTF16(itarg->node().child_value("lbl"));
                CString ico=UTF8toUTF16(itarg->node().child_value("ico"));
                
                r->m_types.push_back(Keyword(lbl,ico));
            }

            r->m_types.back().m_multi=itarg->node().attribute("multi").as_bool();
        }
    }
}
void Qatapult::reload() {
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
    
    CStringA *remoteversion=new CStringA;
    HttpGet(L"http://emmanuelcaradec.com/qatapult/currentversion",remoteversion);
    PostMessage(g_pUI->getHWND(), WM_CURRENTVERSION, (WPARAM)remoteversion, 0);

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
void Qatapult::invalidateIndex() {
    PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);
}
void Qatapult::invalidate() {
    if(m_invalidatepending==false) {
        PostMessage(m_hwnd, WM_INVALIDATEDISPLAY, 0, 0);        
        m_invalidatepending=true;
    }
}

void Qatapult::destroySettingsDlg() {
    if(settingsdlg) {
        settingsdlg->DestroyWindow();
        delete settingsdlg;
        settingsdlg=0;
    }
}

extern BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Qatapult::createSettingsDlg() {    
    settingsdlg=new SettingsDlg;
    settingsdlg->m_actionsDlg.m_argsctrl[0].m_pTypes = &m_types;
    settingsdlg->m_actionsDlg.m_argsctrl[1].m_pTypes = &m_types;
    settingsdlg->m_actionsDlg.m_argsctrl[2].m_pTypes = &m_types;
    settingsdlg->Create(0);    
}

int Qatapult::getActiveRules(int pane, std::vector<RuleArg> &args, std::vector<Rule*> &activerules) {
    // get the length of the args : increment if args[i] is not an empty stack (a stacked results is considered a selection as if tabbed )
    int arglen=pane;
    if(arglen<args.size() && args[arglen].m_results.size()>1)
        arglen++;

    // find the active rules
    for(uint i=0;i<m_rules.size();i++) {
        if(m_rules[i]->match(args, arglen)>0)
            activerules.push_back(m_rules[i]);
    }

    return activerules.size();
}

bool Qatapult::allowType(const CString &type) {
    std::vector<Rule*> activerules;
    getActiveRules(m_pane,m_args,activerules);

    for(uint i=0;i<activerules.size();i++) {
        if(m_pane<activerules[i]->m_types.size() && activerules[i]->m_types[m_pane].m_type==type)
            return true;
    }
    return false;
}

#include "KVPack.h"
void Qatapult::collectItems(const CString &q, const uint pane, std::vector<RuleArg> &args, KVPack &pack, int def) {
    // collect all active rules (match could have an args that tell how much to match )
    // i should probably ignore the current pane for the match or just match until pane-1      
    uint8 *start=pack.beginBlock();

    // collect displayable items
    if(pane>=m_customsources.size() || m_customsources[pane]==0) {
        std::vector<Rule *> activerules;
        if(getActiveRules(pane,args,activerules)==0) {
            goto end;
        }

        // collect all active types           
        std::map<CString,bool> activetypes;
        for(uint i=0;i<activerules.size();i++)
            if(activerules[i]->m_types.size()>pane)
                activetypes[activerules[i]->m_types[pane].m_type]=true;

        if(activetypes.size()==0) {
            goto end;
        }

        // get results for each sources
        // - sources need to filter by themselve from the 'activesources' arguments => this allows sources to output different types at once
        for(std::vector<Source*>::iterator it=m_sources.begin();it!=m_sources.end();it++) {                
            if(pane==0 && q==L"")
                continue;            
            (*it)->collect(q, pack, def, activetypes);
            //(*it)->collect(q, results, def, activetypes);
        }
        
    } else {
        std::map<CString,bool> activetypes;
        m_customsources[pane]->collect(q,pack,def,activetypes);
        //m_customsources[pane]->collect(q,results,def,activetypes);
    }
end:
    pack.endBlock(start);

    // Everything is collected into one source.
    time_t currentTime;
    time(&currentTime);
}

int Qatapult::resultSourceCmp(SourceResult &r1, SourceResult &r2) {
    return r1.rank() > r2.rank();
}

struct Results {
    int                        request;
    int                        pane;
    CString                    query;
    KVPack                     pack;
};

void Qatapult::onQueryChange(const CString &q, bool select) {
    if(m_queries.size()<m_pane)
        m_queries.resize(m_pane+1);
    m_queries[m_pane]=q;

    m_request++;
    m_focusedresult=0;

    Results *presults=new Results;
    presults->request=m_request;
    presults->pane=m_pane;
    presults->query=q;
    
    collectItems(q, m_pane, m_args, presults->pack, 0);

    PostMessage(getHWND(), WM_PUSHRESULT,(WPARAM)presults,!select);
}
void Qatapult::showNextArg() {
    // check if there is extra args
    if(m_args.size()>0) {
        Results *presults=new Results;
        presults->request=m_request;
        presults->pane=m_pane+1;
        presults->query=L"";

        collectItems(L"", m_pane+1, m_args, presults->pack, 1);

        PostMessage(getHWND(), WM_PUSHRESULT2,(WPARAM)presults,0);
    }
}

void Qatapult::ensureArgsCount(std::vector<RuleArg> &ral, int l, int flags) {
    if(flags&EA_REMOVE_EXTRA) {
        while(ral.size()>l) {
            clearRuleArg(ral.back());
            ral.pop_back();
        }
    }
    while(ral.size()<l) {
        ral.push_back(RuleArg());
        ral.back().m_results.push_back(SourceResult());
    } 
}

void Qatapult::cancelResult() {
    // cancel args beyond the current
    ensureArgsCount(m_args,m_pane+1,EA_NO_REMOVE_EXTRA);

    // cancel one result
    if(m_args[m_pane].m_results.size()>0) {
        m_args[m_pane].m_results.pop_back();
        m_queries[m_pane]=L"";
    }

    // if there are no result left, cancel one arg
    if(m_args.back().m_results.size()==0) {
        m_args.pop_back();        
        // set current focus
        if(m_pane>0)
            m_pane--;
    }    

    // ensure there is at least one arg
    ensureArgsCount(m_args,1,EA_REMOVE_EXTRA);
    m_queries.resize(max(m_pane+1, m_args.size()));

    m_input.setText(m_queries[m_pane]);

    invalidate();
}

// setarg should not be used for appending objects
void Qatapult::setResult(uint pane, SourceResult &r) {
    ensureArgsCount(m_args,pane+1,EA_NO_REMOVE_EXTRA);
    if(m_args[pane].m_results.size()==0)
        m_args[pane].m_results.push_back(r);
    else
        m_args[pane].m_results.back()=r;
}

CString Qatapult::getResString(int i, const TCHAR *name) {
    if(i>=m_results.size())
        return L"";
    if(CString(name)==L"rank")
        return ItoS(m_results[i].rank());
    // ask object if it contains subitems ???
    return m_results[i].object()->getString(name);
}

void Qatapult::setVisibleResCount(int i) {
    m_visibleresultscount=i;
}

Object *Qatapult::getArgObject(int i, int e) {
    if(i>=m_args.size())
        return 0;
    if(e>=m_args[i].m_results.size())
        return 0;
    return m_args[i].object(e).get();
}

Object *Qatapult::getResObject(int i) {
    if(i>=m_results.size())
        return 0;

    return m_results[i].object().get();
}

void Qatapult::onSelChange(SourceResult *r) {
    setResult(m_pane,*r);
    showNextArg();
    invalidate();
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
    m_input.draw(g, RectF(float(x),float(y),float(w),float(h)), sf, L'',m_fontsize,g_textcolor,m_textrenderinghint); /*m_args[m_pane].source->m_prefix*/

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

void Qatapult::drawItem(INT i, INT e, INT x, INT y, INT w, INT h){
    Object *o=getArgObject(i,e);
    if(!o)
        return;

    Graphics g(g_HDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);        
    g.SetCompositingQuality(CompositingQualityHighQuality);
    
    o->drawIcon(g, RectF(float(x), float(y), float(w), float(h)));

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}

void Qatapult::drawResItem(INT i, INT x, INT y, INT w, INT h){
    Object *o=getResObject(i);
    if(!o)
        return;

    Graphics g(g_HDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);        
    g.SetCompositingQuality(CompositingQualityHighQuality);

    o->drawIcon(g, RectF(float(x), float(y), float(w), float(h)));

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}


void Qatapult::drawEmphased(const TCHAR *text, const TCHAR *highlight, INT flag, INT from, INT x, INT y, INT w, INT h){
    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHint(m_textrenderinghint));
    ::drawEmphased(g,text,highlight,RectF(float(x),float(y),float(w),float(h)),flag,from,StringAlignment(m_textalign),m_fontsize,g_textcolor);
}

void Qatapult::drawResults(INT x, INT y, INT w, INT h){
    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHint(m_textrenderinghint));
    //g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit );

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
        if(getResObject(i))
            getResObject(i)->drawListItem(g,RectF(float(x),float(y+40*p),float(rw),float(40)),m_fontsize,m_focusedresult==i,g_textcolor,m_resultbgcolor,m_resultfocuscolor);
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

void Qatapult::update() {
    CString str;
    m_invalidatepending=false;        

    // load icons if they aren't 
    // that means that the only element that may have an icon are in m_args

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
    if(p==m_pane)
        return m_input.m_text;
    else if(p>m_queries.size())
        return L"";
    return m_queries[p];
}
SourceResult *Qatapult::getSelectedItem() {
    if(m_results.size()==0)
        return 0;
    return &m_results[m_focusedresult];
}
void Qatapult::clearResults(std::vector<SourceResult> &results) {
    results.clear();
}
void Qatapult::setCurrentSource(int pane,Source *s,CString &q) {
    if(m_customsources.size()<=uint(pane))
        m_customsources.resize(pane+1);

    if(s==(Source*)-1)
        m_customsources[pane]=0;
    else
        m_customsources[pane]=s;
    m_input.setText(q);
    invalidate();
}
void Qatapult::show() {
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}
void Qatapult::hide() {
    ShowWindow(m_hwnd, SW_HIDE);
    KillTimer(m_hwnd, 1);
    //ShowWindow(m_listhosthwnd, SW_HIDE);
}
void Qatapult::showMenu(int xPos,int yPos) {
    HMENU hmenu=CreatePopupMenu();

    AppendMenu(hmenu, MF_STRING, ID_SYSTRAY_OPTIONS, L"Show Options");
    AppendMenu(hmenu, MF_STRING, ID_SYSTRAY_QUIT, L"Quit");
                
    POINT p={xPos,yPos};
    ClientToScreen(m_hwnd, &p);
    TrackPopupMenu(hmenu, TPM_LEFTALIGN, p.x, p.y, 0, m_hwnd, 0);
}
void Qatapult::clearPanes() {
    m_pane=0;
    m_args.clear();
    m_queries.clear();
    m_queries.push_back(L"");
    m_input.setText(L"");
    //m_customsources.clear();

    clearResults(m_results);
//    clearResults(m_nextresults);
    invalidate();
}

void Qatapult::exec() {
    // collect all active rules
    // might need to disambiguate here ?
    for(uint i=0;i<m_rules.size();i++) {
        if(m_rules[i]->match(m_args, m_args.size())>1) {
            // found one rule
            hide();

            if(m_rules[i]->execute(m_args)) {

                for(uint a=0;a<m_args.size(); a++) {
                    for(uint r=0;r<m_args[a].m_results.size(); r++) {
                        if(m_args[a].source())
                            m_args[a].source()->validate(&m_args[a].item(r));
                    }
                }
                    
                clearPanes();
                return;
            }
        }
    }
}

int clamp(int v, int _min, int _max) {
    if(v<_min)
        v=_min;
    if(v>_max)
        v=_max;
    return v;
}

/*void Qatapult::selectHistory(int historyindex) {
    clearPanes();

    m_historyindex = clamp(historyindex, -1, m_commandhistory.size()-1);
    if(m_historyindex==-1)
        return;    
    
    m_args = m_commandhistory[m_historyindex]->m_args;
    m_pane = m_commandhistory[m_historyindex]->m_args.size()-1;
    m_queries.resize(m_pane+1);
    invalidate();

}
void Qatapult::saveCommand(CommandObject *c) {
    std::shared_ptr<CommandObject> cmd(new CommandObject(m_args, m_inputsource));
    m_commandhistory.push_back(cmd);
    SourceResult r;
    r.object() = m_commandhistory.back();

    m_history.saveCommand(cmd.get());
}*/

BOOL Qatapult::isAccelerator(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(hwnd!=m_hwnd)
        return TRUE;

    bool bShift=(GetKeyState(VK_SHIFT)&0xa000)!=0;
    bool bCtrl=(GetKeyState(VK_CONTROL)&0xa000)!=0;
    bool bAlt=(GetKeyState(VK_MENU)&0xa000)!=0;
    
    int mod = 0;
    if( bShift )
        mod |= MOD_SHIFT;
    if( bCtrl )
        mod |= MOD_CONTROL;
    if( bAlt )
        mod |= MOD_ALT;
    
    if(msg == WM_KEYDOWN) {        
        CComSafeArray<VARIANT> ary;
        CComVariant ret;
        ary.Add(CComVariant(CComVariant(wParam)));
        m_painter.Run(L"onKeyDown",ary.GetSafeArrayPtr(),&ret);
        if(ret.vt==VT_BOOL)
            return ret.boolVal;
    }

    if(msg == WM_KEYDOWN && hotkeys[HK_STACK].match(mod,wParam) )
    {
        //
        // stack an object on the current rule argument, if there is at least one multi rule matching this type
        //

        // find if we have a multi rule that match arg, exit if there isn't
        bool allowmulti=false;        
        for(uint i=0;i<m_rules.size();i++)
            if(m_rules[i]->match(m_args, m_pane+1)>=1) {
                if(m_rules[i]->m_types[m_pane].m_multi) {
                    allowmulti=true;
                    break;
                }
            }
        if(!allowmulti)
            return FALSE;
            
        // when we press return the object is already focused, we don't need to assign it to m_args again
        // just add a free slot to add something else
        m_args[m_pane].m_results.push_back(SourceResult());

        m_input.setText(L"");
        clearResults(m_results);
//        clearResults(m_nextresults);
        invalidate();            
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && hotkeys[HK_MKCOMMAND].match(mod,wParam) )
    {
        // FIXME : command object : must copy 3 data objects ?
        // data format must be improved to allow arrays, etc...
        // this would allow great flexibility in data usage as it will be possible to get data with
        // $0.path or $0[0].path, etc...

        //CommandObject *c=new CommandObject(m_args, m_inputsource);
        //clearPanes();
        //onSelChange(&SourceResult(c));
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && hotkeys[HK_EXECUTE].match(mod,wParam))
    {
        if(m_editmode==1 && bShift) {
            m_input.appendAtCaret(L"\n");
            invalidate();
            return FALSE;
        }
        
        exec();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && hotkeys[HK_CANCEL].match(mod,wParam))
    {
        if(m_pane<m_customsources.size() && m_customsources[m_pane]!=0) {
            m_customsources[m_pane]=0;
            m_input.setText(L"");
            invalidate();
            return FALSE;
        }
        
        if(m_editmode==1) {
            m_editmode=0;
            if(m_input.m_text[0]==L'.')
                m_input.m_text=m_input.m_text.Mid(1);
            invalidate();
            return FALSE;
        }
        
        cancelResult();
        
        if(m_pane==0 && m_args.size()==1 && m_args[0].m_results.size()==1 && m_args[0].m_results.back().source()==0) {
            ShowWindow(m_hwnd, SW_HIDE);
            m_input.setText(L"");
            invalidate();
            return FALSE;
        }

        showNextArg();
        invalidate();

        return FALSE;
    }
    else if(msg == WM_KEYDOWN && hotkeys[HK_NEXTPANE].match(mod,wParam))
    {
        // clear the idle
        KillTimer(m_hwnd, 1);

        m_editmode=0;
        uint p=m_pane;
        if(p+1<m_args.size()) {
            m_pane++;

            if(m_pane>=m_queries.size())
                m_queries.resize(m_pane+1);

            m_input.setText(m_queries[m_pane]);
        }
        onQueryChange(m_queries[m_pane],true);

        return FALSE;
    }
    else if(msg == WM_KEYDOWN && hotkeys[HK_PREVPANE].match(mod,wParam) )
    {
        // clear the idle
        KillTimer(m_hwnd, 1);

        m_editmode=0;

        if(m_pane==0)
            return FALSE;

        if(m_pane>0)
            m_pane--;
            
        m_input.setText(m_queries[m_pane]);            
        
        onQueryChange(m_queries[m_pane],false);

        return FALSE;
    }
    else if(msg==WM_KEYDOWN &&  hotkeys[HK_EDITMODE].match(mod,wParam) )
    {
        if(m_editmode==0)
            m_editmode=1;
        else
            m_editmode=0;
        if(m_editmode==0)
            m_input.m_caretpos=m_input.m_text.GetLength();
        invalidate();
        return FALSE;
    }
    // source exploration
    else if(msg == WM_KEYDOWN && hotkeys[HK_EXITSOURCE].match(mod,wParam) && m_pane<m_customsources.size())
    {
        m_customsources[m_pane]=0;
        m_input.setText(L"");
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && hotkeys[HK_EXPANDSOURCE].match(mod,wParam))
    {
        SourceResult *r=getSelectedItem();
        if(!r)
            return FALSE;
        CString q;
        Source *s=r->source()->getSubSource(*r,q);
        if(s!=0) {
            setCurrentSource(m_pane,s,q);
            m_input.setText(L"");
        } else {
            m_input.setText(r->object()->getString(L"expand"));
        }
        onQueryChange(m_input.m_text);
        invalidate();
        return FALSE;
    }

    /*else if(msg == WM_KEYDOWN && wParam == 'P' && mod == MOD_CONTROL)
    {
        selectHistory(m_historyindex+1);
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == 'N' && mod == MOD_CONTROL)
    {
        selectHistory(m_historyindex-1);
        return FALSE;
    }*/

    else if(msg == WM_KEYDOWN && m_editmode!=1 && hotkeys[HK_NEXTRESULT].match(mod,wParam) || hotkeys[HK_NEXTRESULTPAGE].match(mod,wParam) )
    {	
        if(hotkeys[HK_NEXTRESULT].match(mod,wParam)) {
            m_focusedresult++;
        } else if(hotkeys[HK_NEXTRESULTPAGE].match(mod,wParam)) {
            m_focusedresult+=m_visibleresultscount;
        }

        if(m_focusedresult>=m_results.size())
            m_focusedresult=m_results.size()-1;

        if(m_focusedresult>=m_resultspos+m_visibleresultscount)
            m_resultspos=m_focusedresult-m_visibleresultscount+1;
    
        if(m_results.size()>0)
            onSelChange(&SourceResult(m_results[m_focusedresult].m_object->clone()));
        invalidate();

        return FALSE;
    }
    else if(msg == WM_KEYDOWN && m_editmode!=1 && hotkeys[HK_PREVRESULT].match(mod,wParam) || hotkeys[HK_PREVRESULTPAGE].match(mod,wParam) )
    {	
        if(hotkeys[HK_PREVRESULT].match(mod,wParam))
            m_focusedresult--;
        else if(hotkeys[HK_PREVRESULTPAGE].match(mod,wParam))
            m_focusedresult-=m_visibleresultscount;

        if(m_focusedresult<0)
            m_focusedresult=0;

        if(m_focusedresult<m_resultspos)
            m_resultspos=m_focusedresult;

        if(m_results.size()>m_focusedresult)
            onSelChange(&SourceResult(m_results[m_focusedresult].m_object->clone()));
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && hotkeys[HK_PASTE].match(mod,wParam) ) { // ctrl+v
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
        onQueryChange(m_input.m_text);
        invalidate();
        return FALSE;
    }
    // input & edit mode
    else if(msg == WM_KEYDOWN && m_editmode==1 && wParam == VK_LEFT)
    {
        m_input.moveCaretLeft(bCtrl);
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && m_editmode==1 && wParam == VK_RIGHT)
    {
        m_input.moveCaretRight(bCtrl);
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_BACK)
    {
        m_input.back(bCtrl);
        onQueryChange(m_input.m_text);
        invalidate();
        return FALSE;
    }        
    else if(msg == WM_KEYDOWN && wParam == VK_DELETE && m_editmode==1)
    {
        bool bCtrl=!!(GetKeyState(VK_CONTROL)&0x8000);
        m_input.del(bCtrl);
        onQueryChange(m_input.m_text);
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_HOME && m_editmode==1)
    {
        m_input.home();
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_END && m_editmode==1)
    { 
        m_input.end();
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_INSERT)
    {
        m_editmode=m_editmode?0:1;
        if(m_editmode==0)
            m_input.m_caretpos=m_input.m_text.GetLength();
        invalidate();
        return FALSE;
    }
    

    return TRUE;
}

#include "unzip.h"

LRESULT Qatapult::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {   
    if(msg == WM_COPYDATA)
    {
        COPYDATASTRUCT *cpd=(COPYDATASTRUCT*)lParam;
        CString str;
        str.SetString((WCHAR*)cpd->lpData, cpd->cbData/sizeof(WCHAR));

        if(str.Left(4)==L"/run=") {
            CComVariant ret;
            m_commandsHost.Eval(str, &ret);
        } else if(str.Left(9)==L"/install=") {            
            CString filename=str.Mid(9);

            CString outputdir;
            if(filename.Right(8)==L".qplugin")
                outputdir=L"plugins";
            else if(filename.Right(6)==L".qskin")
                outputdir=L"skins";
            else
                return S_FALSE;

            HZIP hz = OpenZip(filename,0);
            if(hz) {
                SetUnzipBaseDir(hz, outputdir);
                
                ZIPENTRY ze; GetZipItem(hz,-1,&ze); int numitems=ze.index;                
                // -1 gives overall information about the zipfile
                for (int zi=0; zi<numitems; zi++)
                {
                    ZIPENTRY ze; GetZipItem(hz,zi,&ze); // fetch individual details
                    UnzipItem(hz, zi, ze.name);         // e.g. the item's name.
                }

                PostMessage(getHWND(),WM_RELOAD,1,0); // reload without showing qatapult

                MessageBox(MB_OK,L"Your Qatapult plugin has been installed successfully.",L"Qatapult",MB_OK);                
            } else {
                MessageBox(MB_OK,L"This Qatapult plugin seems corrupted. Installation has aborted.",L"Qatapult",MB_OK);
            }
            CloseZip(hz);
        }
        return S_OK;
    }
    else if(msg == WM_CURRENTVERSION)
    {
        CStringA *currentversionstr=(CStringA*)wParam;
        __int64 remoteversion=_ttoi64(CString(*currentversionstr));
        __int64 currentversion=_ttoi64(getFileContent(L"currentversion"));
        if(CString(*currentversionstr)!=L"" && currentversion<remoteversion ) {
            if( MessageBox(0, L"A new version of Qatapult is available. Do you want to get it ?", L"Qatapult", MB_YESNO) == IDYES)
                ShellExecute(0, 0, L"http://emmanuelcaradec.com/qatapult",0,0,SW_SHOW);
        }
        delete currentversionstr;
    }
    else if(msg == WM_PUSHRESULT)
    {
        Results *p=(Results*)wParam;
        if(m_request==p->request) {
            m_resultsPack.clear();
            m_resultsPack=p->pack;

            m_results.clear();
            for(uint8 *pobj=p->pack.begin(); pobj<p->pack.end(); pobj+=*(uint32*)pobj) {
                m_results.push_back(new Object(pobj));
                m_results.back().source()->rate(p->query,&m_results.back());
            }

            std::sort(m_results.begin(), m_results.end(), resultSourceCmp);

            // select the first result if any
            // it must probably be copied and extracted ? Malloc and memcpy are probably enough
            // but in that case the object must delete the pointer to data as it own it
            // unlike in the large block case
            if(lParam==0)
                onSelChange(m_results.size()>0?&SourceResult(m_results.front().object()->clone()):&SourceResult());

            invalidate();
        }
        delete p;
        return TRUE;
    }
    else if(msg == WM_PUSHRESULT2)
    {
        Results *p=(Results*)wParam;
        if(m_request==p->request) {
            // remove all args beyond next pane
            ensureArgsCount(m_args,m_pane+1);
            //m_nextResultsPack.clear();
            //m_nextResultsPack=p->pack;

            std::vector<SourceResult>  nextresults;

            nextresults.clear();
            for(uint8 *pobj=p->pack.begin(); pobj<p->pack.end(); pobj+=*(uint32*)pobj) {
                nextresults.push_back(new Object(pobj));
                nextresults.back().source()->rate(p->query,&nextresults.back());
            }

            std::sort(nextresults.begin(), nextresults.end(), resultSourceCmp);

            // initialize the second result, let empty if nothing match to prevent tabbing to it
            if(nextresults.size()>0)
                setResult(m_pane+1,SourceResult(nextresults.front().object()->clone()));

            p->pack.clear();

            invalidate();
        }
        delete p;
        return TRUE;
    }    
    else if(msg == WM_CHAR)
    {
        if((wParam == L'/' || wParam == L'\\') && getSelectedItem() && getSelectedItem()->object() && getSelectedItem()->object()->type==L"FILE")
        {
            SourceResult *r=getSelectedItem();
            CString path=r->object()->getString(L"expand");
            m_input.setText(path);
        }
        else if((wParam == L'?') && getSelectedItem() && getSelectedItem()->object() && getSelectedItem()->object()->type==L"FILE")
        {
            SourceResult *r=getSelectedItem();
            CString t(r->object()->getString(L"expand"));
            t.TrimRight(L'\\');
            CString d=t.Left(t.ReverseFind(L'\\'));
            if(d==L"")
                m_input.setText(L"");
            else
                m_input.setText(d+L"\\");
        }
        else //if(wParam>'0')
        {
            m_input.appendAtCaret((TCHAR)wParam);
        }
        onQueryChange(m_input.m_text);

        invalidate(); 
    }
    else if(msg==WM_CLOSE)
    {
        DestroyWindow(hwnd);
        return S_OK;
    }
    else if(msg==WM_INVALIDATEDISPLAY)
    {
        update();
    }
    else if(msg==WM_RELOAD)
    {
        reset();
        init();
        if(wParam==0)
            ShowWindow(m_hwnd, SW_SHOW);
    }
    else if(msg==WM_COMMAND)
    {
        if(wParam==ID_SYSTRAY_SHOW) {
            show();
        } else if(wParam==ID_SYSTRAY_QUIT) {
            PostQuitMessage(0);
        } else if(wParam==ID_SYSTRAY_OPTIONS) {                                        
            // create dialogs
            if(!settingsdlg)
                createSettingsDlg();

            settingsdlg->CenterWindow();
            settingsdlg->ShowWindow(SW_SHOW);
            ShowWindow(m_hwnd, SW_HIDE);
        }
    }
    else if(msg == WM_LBUTTONUP)
    {
        int xPos = ((int)(short)LOWORD(lParam)); 
        int yPos = ((int)(short)HIWORD(lParam)); 

        CComSafeArray<VARIANT> ary;
        CComVariant ret;
        ary.Add(CComVariant(CComVariant(xPos)));
        ary.Add(CComVariant(CComVariant(yPos)));
        m_painter.Run(CComBSTR(L"onClick"),ary.GetSafeArrayPtr(),&ret);
    }
    else if(msg==WM_HOTKEY && wParam==1)
    {
        g_foregroundWnd=GetForegroundWindow();            
        if(IsWindowVisible(m_hwnd)) {            
            clearPanes();            
            hide();
        } else {
            show();
        }
    }
    else if(msg==WM_UPDATEINDEX)
    {
        //((std::map<CString,SourceResult> *)wParam)->begin()->second.source->updateIndex(((std::map<CString,SourceResult> *)wParam));
        //((std::map<CString,SourceResult> *)wParam)->begin()->second.source->save();
        OutputDebugString(L"crawling complete\n");
    }
    else if(msg==WM_PROGRESS)
    { // progress
        m_crawlprogress=wParam;
        if(wParam!=100)                
            m_indexing=L"Updating index : "+ItoS(wParam)+L"%";
        else
            m_indexing=L"";

        invalidate();
    }
    else if(msg == WM_NCHITTEST)
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
            invalidate();
        }
    }
    else if(msg == WM_KILLFOCUS)
    {
        // TODO : call the painter to signal focus lost
        invalidate();
    }
    else if(msg == WM_RUNRULE || msg == WM_SHOWRULE)
    {
        std::vector<RuleArg> *ruleargs=(std::vector<RuleArg>*)wParam;
        clearPanes();
        m_args=*ruleargs;
        delete ruleargs;
        
        if(m_args.size()==0) {
            if(msg == WM_SHOWRULE)
                show();
            return S_OK;
        }

        m_pane=m_args.size()-1;
        m_queries.resize(m_args.size());        

        if(msg == WM_RUNRULE)
            exec();
        else {
            show();
            onSelChange(&m_args[m_pane].m_results.front());
        }
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}


HRESULT Qatapult::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
    HRESULT hres = E_FAIL;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    if (pDataObj && SUCCEEDED (pDataObj->GetData(&fmte, &medium)))
    {
        TCHAR szFileDropped [MAX_PATH];
        int filecount=DragQueryFile((HDROP)medium.hGlobal,-1, 0,0);
        ensureArgsCount(m_args, m_pane+1);
        for(int i=0;i<filecount;i++) {
            DragQueryFile((HDROP)medium.hGlobal, i, szFileDropped, sizeof (szFileDropped));

            CString path=szFileDropped;

            if(m_args[m_pane].m_results.back().source()==0)
                m_args[m_pane].m_results.back()=getResultFromFilePath(path,m_inputsource);
            else
                m_args[m_pane].m_results.push_back(getResultFromFilePath(path,m_inputsource));
        }

        if (medium.pUnkForRelease)
            medium.pUnkForRelease->Release ();
        else
            GlobalFree(medium.hGlobal);
    }        
        
    //onSelChange(&m_args[m_pane].m_results.back());
    ensureArgsCount(m_args,m_pane+1,EA_NO_REMOVE_EXTRA);
    showNextArg();
    invalidate();
    return S_OK;
}