#include "stdafx.h"
#include "AlphaGUI.h"
#include "PainterScript.h"
#include "Collecter.h"
#include "JScriptSource.h"
#include "QatapultScript.h"

UI *g_pUI; // very lazy way to give access to the ui to the ui window proc
CString settingsini;
pugi::xml_document settings;
pugi::xml_document settingsWT; // settings for the working thread

WNDPROC OldHotKeyEditProc;
int     hotkeymodifiers=0;
int     hotkeycode=0;
HWND    g_foregroundWnd;
CString g_fontfamily;


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
    if(vk==VK_PAUSE)
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

AlphaGUI::AlphaGUI():m_input(this), m_invalidatepending(false) {
#ifdef DEBUG
    //VLDMarkAllLeaksAsReported();
#endif                
    m_hwnd=0;

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
        
    // create dialogs
    CreateSettingsDlg();

    Init();

    // repaint dialog
    Invalidate();
}
AlphaGUI::~AlphaGUI() {        
    Reset();
}
int AlphaGUI::GetCurPane() {
    return m_pane;
}
CString AlphaGUI::getArgString(int c,const TCHAR *name) {
    if(c>=m_args.size())
        return L"";
    return m_args[c].object->getString(name);
}
int AlphaGUI::getArgsCount() {
    return m_args.size();
}
bool AlphaGUI::isSourceEnabled(const char *name) {
    pugi::xml_node n=settings.select_single_node("settings/sources/"+CStringA(name)+"/enabled").node();
    if(n.empty()) return true;
    return CStringA(n.child_value())=="1";
}
bool AlphaGUI::isSourceByDefault(const char *name) {
    pugi::xml_node n=settings.select_single_node("settings/sources/"+CStringA(name)+"/default").node();
    if(n.empty()) return true;
    return CStringA(n.child_value())=="1";
}
void AlphaGUI::Init() {   
    m_crawlprogress=0;
    m_textcolor=0xFFFFFFFF;
    m_fontsize=10.0f;

    settings.load_file("settings.xml");

    g_fontfamily=GetSettingsString(L"general",L"font",L"Arial");
        
    m_skin=L"skins/"+GetSettingsString(L"general",L"skin",L"default");
  
    m_buffer.Create(640,800,32,PixelFormat32bppARGB);

    if(m_hwnd==0) {
        m_hwnd=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_TOPMOST, L"STATIC", L"", /*WS_VISIBLE|*/WS_POPUP|WS_CHILD, 0, 0, 0, 0, 0, 0, 0, 0);
        ::SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG)_WndProc);
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG)this);
    }

    
    host.Initialize(L"Qatapult",L"JScript");
    
    m_pQatapultScript=QatapultScript::Make(this);
    m_pQatapultScript->AddRef();
    
    host.AddObject(L"qatapult",(IDispatch*)m_pQatapultScript);

    host.ParseScriptText(getFileContent(m_skin+L"\\painter.js"),L"skin");

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

    std::vector<CString> collecters;
    FindFilesRecursively(L"plugins",L"*.collecter.js",collecters,99,0);
    for(std::vector<CString>::iterator it=collecters.begin(); it!=collecters.end(); it++) {
        CString str(*it);
        str=str.Mid(8);
        str=str.Left(str.Find(L"\\"));
        str=str.MakeLower();
        addSource(str, new JScriptSource(this,str,*it));
    }        

    Source *tt=addSource(new TextSource);
    tt->def=true;
    addSource(new FileVerbSource);    

    //addRule(L"CLOCK", new ClockRule);
    addRule(L"FILE", L"FILEVERB", new FileVerbRule); 
                
    TextItemSource *t;

    t=new TextItemSource(L"EMAILFILEVERB");
    addSource(t);
    t->addItem(L"Email to",L"icons\\emailto.png");
    addRule(L"FILE",t->type,L"CONTACT",new EmailFileVerbRule);

    t=new TextItemSource(L"EMAILTEXTVERB");
    addSource(t);
    t->addItem(L"Email to",L"icons\\emailto.png");        
    addRule(L"TEXT",t->type,L"CONTACT",new EmailVerbRule);        

    t=new TextItemSource(L"SEARCHWITHVERB");
    addSource(t);
    t->addItem(L"Search With",L"icons\\searchwith.png");        
    addRule(L"TEXT",t->type,L"WEBSITE",new WebSearchRule);
                
    t=new TextItemSource(L"QUITVERB");
    addSource(t);
    t->addItem(L"Quit (Q)",L"icons\\exit.png");
    t->addItem(L"Reload (Q)",L"icons\\reload.png");
    t->def=true;
    addRule(t->type,new QuitRule);

    t=new TextItemSource(L"SOURCEVERB");
    addSource(t);
    t->addItem(L"Open",L"icons\\open.png");
    addRule(L"SOURCE",t->type,new SourceRule(this));

    m_emptysource=t=new TextItemSource(L"EMPTY");
    t->addItem(L"",L"");
    addSource(m_emptysource);

    // hotkey
    hotkeycode=GetSettingsInt(L"hotKeys", L"toggleKey",VK_SPACE);
    hotkeymodifiers=GetSettingsInt(L"hotKeys", L"toggleModifier",MOD_SHIFT);      

    // define hotkey
    RegisterHotKey(g_pUI->getHWND(), 1, hotkeymodifiers, hotkeycode);

    LoadRules(settings);

    std::vector<CString> plugins;
    FindFilesRecursively(L"plugins", L"plugin.xml", plugins, 3);
    for(std::vector<CString>::iterator it=plugins.begin();it!=plugins.end();it++) {
        pugi::xml_document d;
        d.load_file(*it);
        LoadRules(d);
    }
    
    for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
        (*it)->m_pArgs=&m_args;
        (*it)->m_pUI=this;
    }
    
    for(std::vector<Rule*>::iterator it=m_rules.begin(); it!=m_rules.end(); it++)
        (*it)->m_pArgs=&m_args;    
    
    OnQueryChange(L"");
    
    // we shouldn't create a thread for each source, this is inefficient
    // crawl should be called with an empty index for each source
    //_beginthread((void (*)(void*))crawlProc, 0, this);
    //m_workerthread=CreateThread(0, 0, (LPTHREAD_START_ROUTINE)crawlProc, this, 0, &m_crawlThreadId);
    m_workerthread = (HANDLE)_beginthreadex(0, 0, (uint (__stdcall *)(void*))crawlProc, this, 0, (uint*)&m_crawlThreadId );
    
    m_mainThreadId=GetCurrentThreadId();

    BOOL b=PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);

    PostThreadMessage(m_crawlThreadId, WM_RELOADSETTINGS, 0, 0);            
    //PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);
}

SourceResult AlphaGUI::getEmptyResult() {
    SourceResult r(L"",L"",L"",m_emptysource,0,0,0);
    r.object=new Object(L"EMPTY",L"EMPTY",m_emptysource,L"");
    return r;
}
void AlphaGUI::Reset() {        
    delete m_buffer;

    bool b=!!PostThreadMessage(m_crawlThreadId,WM_STOPWORKERTHREAD,0,0);
    if(WaitForSingleObject(m_workerthread,5000)==WAIT_TIMEOUT)
        TerminateThread(m_workerthread,0);

    m_pQatapultScript=0;
    host.Reset();

    for(std::map<CString,Gdiplus::Bitmap*>::iterator it=m_bitmaps.begin(); it!=m_bitmaps.end(); it++) 
        delete it->second;
    m_bitmaps.clear();

    // bitmaps
    UnregisterHotKey(m_hwnd,1);

    ClearResults(m_results);        
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
void AlphaGUI::LoadRules(pugi::xml_document &settings) {
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
                addSource(t);
                t->def = (itarg==ns.begin()); // if first then it's a default source
                for(pugi::xpath_node_set::const_iterator itelt=elts.begin(); itelt!=elts.end(); itelt++) {
                    CString lbl=UTF8toUTF16(itelt->node().child_value("lbl"));
                    CString ico=UTF8toUTF16(itelt->node().child_value("ico"));
                    //CString regex=UTF8toUTF16(itelt->node().child_value("regex"));
                    //CString regexval=UTF8toUTF16(itelt->node().child_value("regexval"));
                    t->addItem(lbl.GetString(),ico.GetString());                        
                }
                r->m_types.push_back(Type(t->type));
            }
        }
    }
}
void AlphaGUI::Reload() {
    PostMessage(getHWND(),WM_RELOAD,0,0);
}
Source *AlphaGUI::addSource(Source *s) {
    m_sources.push_back(s);
    return s;
}
Source *AlphaGUI::addSource(const TCHAR *name,Source *s) {
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
void AlphaGUI::addRule(const CString &arg0,const CString &arg1,const CString &arg2,Rule *r) {
    r->m_types.push_back(arg0);
    r->m_types.push_back(arg1);
    r->m_types.push_back(arg2);
    m_rules.push_back(r);
}
void AlphaGUI::addRule(const CString &arg0,const CString &arg1,Rule *r) {
    r->m_types.push_back(arg0);
    r->m_types.push_back(arg1);
    m_rules.push_back(r);
}
void AlphaGUI::addRule(const CString &arg0,Rule *r) {
    r->m_types.push_back(arg0);
    m_rules.push_back(r);
}
void AlphaGUI::addRule(Rule *r) {
    m_rules.push_back(r);
}
uint __stdcall AlphaGUI::crawlProc(AlphaGUI *thiz) {        
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
HWND AlphaGUI::getHWND() {
    return m_hwnd;
}
void AlphaGUI::InvalidateIndex() {
    PostThreadMessage(m_crawlThreadId, WM_INVALIDATEINDEX, 0, 0);
}
void AlphaGUI::Invalidate() {
    if(m_invalidatepending==false) {
        PostMessage(m_hwnd, WM_INVALIDATEDISPLAY, 0, 0);        
        m_invalidatepending=true;
    }
}
extern BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void AlphaGUI::CreateSettingsDlg() {
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

    tviis.item.pszText = L"Gmail contacts";
    tviis.item.lParam=(LPARAM)hwndGmail;
    HTREEITEM htreeGmail=TreeView_InsertItem(hTreeView, &tviis);

    BOOL b=TreeView_SelectItem(hTreeView, htreeG);
}

void AlphaGUI::CollectItems(const CString &q, const uint pane, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int def) {
    // collect all active rules (match could have an args that tell how much to match )
    // i should probably ignore the current pane for the match or just match until pane-1 ?
        
    // collect displayable items
    ClearResults(results);
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
            else
                (*it)->collect(q, results, def, activesources);                
        }
        
    } else {
        std::map<CString,bool> activesources;
        m_customsources[pane]->collect(q,results,def,activesources);
    }

    uselev=0;
    CString Q(q); Q.MakeUpper();
    int qlen=max(1, Q.GetLength());
    for(uint i=0;i<results.size();i++) {
        // pourcentage de chaine correspondante ?
        CString text=results[i].object->getString(L"text");
        text.MakeUpper();
        if(uselev) {
            int len=levenshtein_distance(CStringA(Q), CStringA(text).GetString());                
            float f = 1 - float(len) / text.GetLength();
            results[i].rank = int(100*f + results[i].bonus);
        } else {
            if(results[i].source->m_prefix!=0 && text[0]==results[i].source->m_prefix)
                results[i].bonus+=100;

            results[i].rank = int(100*float(qlen) / text.GetLength() + results[i].bonus);
        }
        results[i].source->rate(&results[i]);
    }
}

int AlphaGUI::ResultSourceCmp(SourceResult &r1, SourceResult &r2) {
    return r1.rank > r2.rank;
}
void AlphaGUI::OnQueryChange(const CString &q) {
    m_focusedresult=0;

    CollectItems(q, m_pane, m_args, m_results, 0);        
                
    std::sort(m_results.begin(), m_results.end(), ResultSourceCmp);

    uint m=m_results.size();
    
    SourceResult *r=0;

    if(m_results.size()>0) {
        OnSelChange(&m_results.front());
    } else {
        OnSelChange(&getEmptyResult());
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
void AlphaGUI::ShowNextArg() {
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
void AlphaGUI::SetArg(uint pane, SourceResult &r) {
    if(pane==m_args.size()) {
        m_args.push_back(SourceResult());
        r.source->copy(r,&m_args.back());
    } else {
        m_args[pane].source->clear(m_args[pane]);
        r.source->copy(r,&m_args[pane]);
    }
}
void AlphaGUI::OnSelChange(SourceResult *r) {        
    if(m_args.size()==0)
        m_args.push_back(getEmptyResult());
        
    // a copy is not enough if there is deep data because the results are cleaned after the query
    SetArg(m_pane, *r);

    ShowNextArg();

    Invalidate();
}

static HDC   g_HDC;
void AlphaGUI::drawBitmap(const TCHAR *text, INT x, INT y, INT w, INT h){    
    if(m_bitmaps.find(text)==m_bitmaps.end()) {
        Gdiplus::Bitmap *p=Gdiplus::Bitmap::FromFile(text);
        //PremultAlpha(*p);
        
        //m_bitmaps[text]=p->Clone(0,0,p->GetWidth(),p->GetHeight(),PixelFormat32bppPARGB);

        /*Gdiplus::Bitmap *p2=new Gdiplus::Bitmap(p->GetWidth(),p->GetHeight(),PixelFormat32bppPARGB);
        Graphics g(p2);
        g.DrawImage(p,0,0,p->GetWidth(),p->GetHeight());*/
        m_bitmaps[text]=p;        
        //delete p;
    }
    
    Gdiplus::Bitmap *f=m_bitmaps[text];

    Gdiplus::Graphics g(g_HDC);
    /*g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAlias);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);        
    g.SetCompositingQuality(CompositingQualityHighQuality);*/


    g.SetInterpolationMode(InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(PixelOffsetModeHalf);
    g.SetSmoothingMode(SmoothingModeNone);

    g.DrawImage(f,x,y,w,h);
    //delete f;

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}

void AlphaGUI::drawInput(INT x, INT y, INT w, INT h){
    StringFormat sf;
    m_input.Draw(g_HDC, RectF(x,y,w,h), sf, L'',m_textcolor); /*m_args[m_pane].source->m_prefix*/

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}

void AlphaGUI::drawText(const TCHAR *text, INT x, INT y, INT w, INT h) {
    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHintAntiAlias);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    StringFormat sf;
    sf.SetAlignment(StringAlignment(m_textalign));
    sf.SetTrimming(StringTrimmingEllipsisCharacter);     
    Gdiplus::Font f(g_fontfamily, m_fontsize);
    g.DrawString(text,-1,&f,RectF(x,y,w,h),&sf,&SolidBrush(m_textcolor));
}

void AlphaGUI::drawItem(INT i, INT x, INT y, INT w, INT h){
    if(i>=m_args.size())
        return;

    Graphics g(g_HDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAlias);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);        
    g.SetCompositingQuality(CompositingQualityHighQuality);

    m_args[i].object->drawItem(g, &m_args[i], RectF(x, y, w, h));

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}

void AlphaGUI::drawEmphased(const TCHAR *text, const TCHAR *highlight, INT flag, INT x, INT y, INT w, INT h){
    Graphics g(g_HDC);    
    ::drawEmphased(g,text,highlight,RectF(x,y,w,h),flag,StringAlignment(m_textalign),10.0f,m_textcolor);
}

void AlphaGUI::drawResults(INT x, INT y, INT w, INT h){
    Graphics g(g_HDC);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    m_focusedresult=max(0,m_focusedresult);
    m_focusedresult=min(m_results.size()-1,m_focusedresult);

    int visibleresults=min(m_results.size(),int(h/40));
    if(m_focusedresult>=m_resultspos+visibleresults)
        m_resultspos=m_focusedresult-visibleresults+1;
    
    if(m_focusedresult<m_resultspos)
        m_resultspos=m_focusedresult;
    
    for(int i=m_resultspos;i<m_resultspos+visibleresults;i++) {
        int p=i-m_resultspos;
        m_results[i].object->drawListItem(g,&m_results[i],RectF(x,y+40*p,w,40),m_focusedresult==i);
    }

    m_curWidth=max(m_curWidth,x+w);
    m_curHeight=max(m_curWidth,y+h);
}
 
void AlphaGUI::Update() {
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
    PainterScript *painterscript=PainterScript::Make(this);
    painterscript->AddRef();
    ary.Add(CComVariant(CComVariant((IDispatch*)painterscript)));
    host.Run(CComBSTR(L"draw"),ary.GetSafeArrayPtr(),&ret);
        

    Gdiplus::Font arial(g_fontfamily, 50);
    Gdiplus::StringFormat sfmt(Gdiplus::StringFormat::GenericTypographic());  
    sfmt.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces|StringFormatFlagsNoWrap|StringFormatFlagsNoClip|StringFormatFlagsNoFitBlackBox);   
    sfmt.SetTrimming(StringTrimmingEllipsisCharacter);

    CRect workarea;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

    POINT p1={(workarea.left+workarea.right)/2-m_curWidth/2,200};
    POINT p2={0};
    SIZE s={m_curWidth, m_curHeight};
    BLENDFUNCTION bf={AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    BOOL b=::UpdateLayeredWindow(m_hwnd, 0, &p1, &s, g_HDC, &p2, 0, &bf, ULW_ALPHA);
      
    g.ReleaseHDC(g_HDC);
}    
CString AlphaGUI::getQuery(int p) {
    if(p==m_queries.size())
        return m_input.m_text;
    else if(p>m_queries.size())
        return L"";
    return m_queries[p];
}
SourceResult *AlphaGUI::GetSelectedItem() {
    if(m_results.size()==0)
        return 0;
    return &m_results[m_focusedresult];
}
void AlphaGUI::ClearResults(std::vector<SourceResult> &results) {
    for(uint j=0;j<results.size();j++) {            
        results[j].source->clear(results[j]);
    }
    results.clear();
}
void AlphaGUI::SetCurrentSource(int pane,Source *s,CString &q) {
    if(m_customsources.size()<=uint(pane))
        m_customsources.resize(pane+1);

    if(s==(Source*)-1)
        m_customsources[pane]=0;
    else
        m_customsources[pane]=s;
    m_input.SetText(q);
    Invalidate();
}
void AlphaGUI::Show() {
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}
void AlphaGUI::Hide() {
    ShowWindow(m_hwnd, SW_HIDE);
    KillTimer(m_hwnd, 1);
    //ShowWindow(m_listhosthwnd, SW_HIDE);
}
void AlphaGUI::showMenu(int xPos,int yPos) {
    HMENU hmenu=CreatePopupMenu();

    AppendMenu(hmenu, MF_STRING, 1, L"Options");
    AppendMenu(hmenu, MF_STRING, 0, L"Quit");
                
    POINT p={xPos,yPos};
    ClientToScreen(m_hwnd, &p);
    TrackPopupMenu(hmenu, TPM_LEFTALIGN, p.x, p.y, 0, m_hwnd, 0);
}
LRESULT AlphaGUI::OnKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    return TRUE;
}
LRESULT AlphaGUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {   
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
                Hide();

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
        if(m_pane<m_customsources.size() && m_customsources[m_pane]!=0) {
            m_customsources[m_pane]=0;
            m_input.SetText(L"");
            return FALSE;
        }

        if(m_resultsvisible) {
            m_resultsvisible=false;
            Invalidate();
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
            m_resultsvisible=false;
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
    else if(msg == WM_KEYDOWN && wParam == VK_DOWN)
    {	
        if(m_editmode==1) {
            ;
        } else {
            if(m_resultsvisible) {
                m_focusedresult++;
            } else {
                m_focusedresult=0;
            }
            m_resultsvisible=true;
            if(m_focusedresult>=m_results.size())
                m_focusedresult=m_results.size()-1;
            if(m_results.size()>0)
                OnSelChange(&m_results[m_focusedresult]);
            Invalidate();
        }
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_UP)
    {	
        if(m_editmode==1) {
            ;
        } else {
            m_focusedresult--;
            if(m_focusedresult<0)
                m_focusedresult=0;
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
            } 
            else if((wParam == L'/' || wParam == L'\\') && GetSelectedItem() && GetSelectedItem()->object && GetSelectedItem()->object->type==L"FILE")
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
            OnQueryChange(m_input.m_text);

            //Update();
            Invalidate();
        }
            
        if(wParam!=27) {
            KillTimer(m_hwnd, 1);
            SetTimer(m_hwnd, 1, 10000, 0);
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
            m_resultsvisible=false;
        }
    } else if(msg == WM_LBUTTONUP) {
        int xPos = ((int)(short)LOWORD(lParam)); 
        int yPos = ((int)(short)HIWORD(lParam)); 

        CComSafeArray<VARIANT> ary;
        CComVariant ret;
        ary.Add(CComVariant(CComVariant(xPos)));
        ary.Add(CComVariant(CComVariant(yPos)));
        host.Run(CComBSTR(L"onClick"),ary.GetSafeArrayPtr(),&ret);
            
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
        if(IsWindowVisible(m_hwnd)) {
            KillTimer(m_hwnd, 1);
            m_resultsvisible=true;
            Invalidate();
            //::ShowWindow(m_listhosthwnd, SW_SHOWNOACTIVATE);
        }
    }
    else if(msg==WM_TIMER && wParam==2)
    {
        if(IsWindowVisible(m_hwnd)) {
            Invalidate();
        }
    }
    else if(msg == WM_KILLFOCUS) {
        m_resultsvisible=false;
        Invalidate();
        //if((HWND)wParam!=m_listhosthwnd) {
        //    Hide();
        //}
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}