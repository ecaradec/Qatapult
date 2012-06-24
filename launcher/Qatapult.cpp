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

#define WM_ICON_NOTIFY WM_APP+10

struct QatapultAtlModule : CAtlModule {
    HRESULT AddCommonRGSReplacements(IRegistrarBase *) {
        return S_OK;
    }
} _atlmodule;
   
CString getNameFromVK(int vk) {
    CString v;
    switch(vk) {
    case VK_LBUTTON                 : v=L"LBUTTON"; break;
    case VK_RBUTTON                 : v=L"RBUTTON"; break;
    case VK_CANCEL                  : v=L"CANCEL"; break;
    case VK_MBUTTON                 : v=L"MBUTTON"; break;
    case VK_XBUTTON1                : v=L"XBUTTON1"; break;
    case VK_XBUTTON2                : v=L"XBUTTON2"; break;
    case VK_BACK                    : v=L"BACK"; break;
    case VK_TAB                     : v=L"TAB"; break;
    case VK_CLEAR                   : v=L"CLEAR"; break;
    case VK_RETURN                  : v=L"RETURN"; break;
    //case VK_SHIFT                   : v=L"SHIFT"; break;
    //case VK_CONTROL                 : v=L"CONTROL"; break;
    //case VK_MENU                    : v=L"MENU"; break;
    case VK_PAUSE                   : v=L"PAUSE"; break;
    case VK_CAPITAL                 : v=L"CAPITAL"; break;
    case VK_KANA                    : v=L"KANA"; break;
    case VK_JUNJA                   : v=L"JUNJA"; break;
    case VK_FINAL                   : v=L"FINAL"; break;
    case VK_HANJA                   : v=L"HANJA"; break;
    case VK_ESCAPE                  : v=L"ESCAPE"; break;
    case VK_CONVERT                 : v=L"CONVERT"; break;
    case VK_NONCONVERT              : v=L"NONCONVERT"; break;
    case VK_ACCEPT                  : v=L"ACCEPT"; break;
    case VK_MODECHANGE              : v=L"MODECHANGE"; break;
    case VK_SPACE                   : v=L"SPACE"; break;
    case VK_PRIOR                   : v=L"PRIOR"; break;
    case VK_NEXT                    : v=L"NEXT"; break;
    case VK_END                     : v=L"END"; break;
    case VK_HOME                    : v=L"HOME"; break;
    case VK_LEFT                    : v=L"LEFT"; break;
    case VK_UP                      : v=L"UP"; break;
    case VK_RIGHT                   : v=L"RIGHT"; break;
    case VK_DOWN                    : v=L"DOWN"; break;
    case VK_SELECT                  : v=L"SELECT"; break;
    case VK_PRINT                   : v=L"PRINT"; break;
    case VK_EXECUTE                 : v=L"EXECUTE"; break;
    case VK_SNAPSHOT                : v=L"SNAPSHOT"; break;
    case VK_INSERT                  : v=L"INSERT"; break;
    case VK_DELETE                  : v=L"DELETE"; break;
    case VK_HELP                    : v=L"HELP"; break;
    case VK_LWIN                    : v=L"LWIN"; break;
    case VK_RWIN                    : v=L"RWIN"; break;
    case VK_APPS                    : v=L"APPS"; break;
    case VK_SLEEP                   : v=L"SLEEP"; break;
    case VK_NUMPAD0                 : v=L"NUMPAD0"; break;
    case VK_NUMPAD1                 : v=L"NUMPAD1"; break;
    case VK_NUMPAD2                 : v=L"NUMPAD2"; break;
    case VK_NUMPAD3                 : v=L"NUMPAD3"; break;
    case VK_NUMPAD4                 : v=L"NUMPAD4"; break;
    case VK_NUMPAD5                 : v=L"NUMPAD5"; break;
    case VK_NUMPAD6                 : v=L"NUMPAD6"; break;
    case VK_NUMPAD7                 : v=L"NUMPAD7"; break;
    case VK_NUMPAD8                 : v=L"NUMPAD8"; break;
    case VK_NUMPAD9                 : v=L"NUMPAD9"; break;
    case VK_MULTIPLY                : v=L"MULTIPLY"; break;
    case VK_ADD                     : v=L"ADD"; break;
    case VK_SEPARATOR               : v=L"SEPARATOR"; break;
    case VK_SUBTRACT                : v=L"SUBTRACT"; break;
    case VK_DECIMAL                 : v=L"DECIMAL"; break;
    case VK_DIVIDE                  : v=L"DIVIDE"; break;
    case VK_F1                      : v=L"F1"; break;
    case VK_F2                      : v=L"F2"; break;
    case VK_F3                      : v=L"F3"; break;
    case VK_F4                      : v=L"F4"; break;
    case VK_F5                      : v=L"F5"; break;
    case VK_F6                      : v=L"F6"; break;
    case VK_F7                      : v=L"F7"; break;
    case VK_F8                      : v=L"F8"; break;
    case VK_F9                      : v=L"F9"; break;
    case VK_F10                     : v=L"F10"; break;
    case VK_F11                     : v=L"F11"; break;
    case VK_F12                     : v=L"F12"; break;
    case VK_F13                     : v=L"F13"; break;
    case VK_F14                     : v=L"F14"; break;
    case VK_F15                     : v=L"F15"; break;
    case VK_F16                     : v=L"F16"; break;
    case VK_F17                     : v=L"F17"; break;
    case VK_F18                     : v=L"F18"; break;
    case VK_F19                     : v=L"F19"; break;
    case VK_F20                     : v=L"F20"; break;
    case VK_F21                     : v=L"F21"; break;
    case VK_F22                     : v=L"F22"; break;
    case VK_F23                     : v=L"F23"; break;
    case VK_F24                     : v=L"F24"; break;
    case VK_NUMLOCK                 : v=L"NUMLOCK"; break;
    case VK_SCROLL                  : v=L"SCROLL"; break;
    case VK_OEM_NEC_EQUAL           : v=L"OEM_NEC_EQUAL"; break;
    case VK_OEM_FJ_MASSHOU          : v=L"OEM_FJ_MASSHOU"; break;
    case VK_OEM_FJ_TOUROKU          : v=L"OEM_FJ_TOUROKU"; break;
    case VK_OEM_FJ_LOYA             : v=L"OEM_FJ_LOYA"; break;
    case VK_OEM_FJ_ROYA             : v=L"OEM_FJ_ROYA"; break;
    case VK_LSHIFT                  : v=L"LSHIFT"; break;
    case VK_RSHIFT                  : v=L"RSHIFT"; break;
    case VK_LCONTROL                : v=L"LCONTROL"; break;
    case VK_RCONTROL                : v=L"RCONTROL"; break;
    case VK_LMENU                   : v=L"LMENU"; break;
    case VK_RMENU                   : v=L"RMENU"; break;
    case VK_BROWSER_BACK            : v=L"BROWSER_BACK"; break;
    case VK_BROWSER_FORWARD         : v=L"BROWSER_FORWARD"; break;
    case VK_BROWSER_REFRESH         : v=L"BROWSER_REFRESH"; break;
    case VK_BROWSER_STOP            : v=L"BROWSER_STOP"; break;
    case VK_BROWSER_SEARCH          : v=L"BROWSER_SEARCH"; break;
    case VK_BROWSER_FAVORITES       : v=L"BROWSER_FAVORITES"; break;
    case VK_BROWSER_HOME            : v=L"BROWSER_HOME"; break;
    case VK_VOLUME_MUTE             : v=L"VOLUME_MUTE"; break;
    case VK_VOLUME_DOWN             : v=L"VOLUME_DOWN"; break;
    case VK_VOLUME_UP               : v=L"VOLUME_UP"; break;
    case VK_MEDIA_NEXT_TRACK        : v=L"MEDIA_NEXT_TRACK"; break;
    case VK_MEDIA_PREV_TRACK        : v=L"MEDIA_PREV_TRACK"; break;
    case VK_MEDIA_STOP              : v=L"MEDIA_STOP"; break;
    case VK_MEDIA_PLAY_PAUSE        : v=L"MEDIA_PLAY_PAUSE"; break;
    case VK_LAUNCH_MAIL             : v=L"LAUNCH_MAIL"; break;
    case VK_LAUNCH_MEDIA_SELECT     : v=L"LAUNCH_MEDIA_SELECT"; break;
    case VK_LAUNCH_APP1             : v=L"LAUNCH_APP1"; break;
    case VK_LAUNCH_APP2             : v=L"LAUNCH_APP2"; break;
    case VK_OEM_1                   : v=L";";break;
    case VK_OEM_PLUS                : v=L"+";break;
    case VK_OEM_COMMA               : v=L",";break;
    case VK_OEM_MINUS               : v=L"-";break;
    case VK_OEM_PERIOD              : v=L".";break;
    case VK_OEM_2                   : v=L"/";break;
    case VK_OEM_3                   : v=L"`";break;
    case VK_OEM_4                   : v=L"[";break;
    case VK_OEM_5                   : v=L"\\";break;
    case VK_OEM_6                   : v=L"]";break;
    case VK_OEM_7                   : v=L"'";break;
    case VK_OEM_8                   : v=L"OEM_8";break;
    case VK_OEM_AX                  : v=L"OEM_AX";break;
    case VK_OEM_102                 : v=L"<";break;
    case VK_ICO_HELP                : v=L"ICO_HELP";break;
    case VK_ICO_00                  : v=L"00";break;

    case VK_PROCESSKEY              : v=L"PROCESSKEY"; break;
    case VK_ICO_CLEAR               : v=L"ICO_CLEAR"; break;
    case VK_PACKET                  : v=L"PACKET"; break;
    case VK_OEM_RESET               : v=L"OEM_RESET"; break;
    case VK_OEM_JUMP                : v=L"OEM_JUMP"; break;
    case VK_OEM_PA1                 : v=L"OEM_PA1"; break;
    case VK_OEM_PA2                 : v=L"OEM_PA2"; break;
    case VK_OEM_PA3                 : v=L"OEM_PA3"; break;
    case VK_OEM_WSCTRL              : v=L"OEM_WSCTRL"; break;
    case VK_OEM_CUSEL               : v=L"OEM_CUSEL"; break;
    case VK_OEM_ATTN                : v=L"OEM_ATTN"; break;
    case VK_OEM_FINISH              : v=L"OEM_FINISH"; break;
    case VK_OEM_COPY                : v=L"OEM_COPY"; break;
    case VK_OEM_AUTO                : v=L"OEM_AUTO"; break;
    case VK_OEM_ENLW                : v=L"OEM_ENLW"; break;
    case VK_OEM_BACKTAB             : v=L"OEM_BACKTAB"; break;
    case VK_ATTN                    : v=L"ATTN"; break;
    case VK_CRSEL                   : v=L"CRSEL"; break;
    case VK_EXSEL                   : v=L"EXSEL"; break;
    case VK_EREOF                   : v=L"EREOF"; break;
    case VK_PLAY                    : v=L"PLAY"; break;
    case VK_ZOOM                    : v=L"ZOOM"; break;
    case VK_NONAME                  : v=L"NONAME"; break;
    case VK_PA1                     : v=L"PA1"; break;
    case VK_OEM_CLEAR               : v=L"OEM_CLEAR"; break;
    default:
        v=(TCHAR)MapVirtualKey(vk, MAPVK_VK_TO_CHAR);
    }
    return v;
}

CString HotKeyToString(int modifier, int vk) {    
    CString mod;
    if(modifier&MOD_WIN)
        mod+=L"WIN+";
    if(modifier&MOD_CONTROL)
        mod+=L"CTRL+";
    if(modifier&MOD_ALT)
        mod+=L"ALT+";
    if(modifier&MOD_SHIFT)
        mod+=L"SHIFT+";

    CString c=getNameFromVK(vk);
           
    return mod+c;
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


void getSkinList(std::vector<CString> &skins) {
    GetSubFolderList(L"skins",skins);
}

void getPluginList(std::vector<PluginDesc> &plugins) {
    //plugins.push_back(L"Filesystem");         plugins.back().desc=L"Give access to local filesystem";
    //plugins.push_back(L"IndexedFiles");       plugins.back().desc=L"This returns startmenu items and other indexed items from the options dialog";
    //plugins.push_back(L"Websites");           plugins.back().desc=L"";
    //plugins.push_back(L"FileHistory");
    plugins.push_back(L"Network");            plugins.back().desc=L"Add shared drives objects";
    plugins.push_back(L"Contacts");           plugins.back().desc=L"Add your Gmail contacts (requires some configuration in Gmail pane )";
    plugins.push_back(L"ExplorerSelection");  plugins.back().desc=L"Add a currentselection object that's the currently selected item in Windows Explorer";
    plugins.push_back(L"Windows");            plugins.back().desc=L"Add objects for the currently opened window";

    plugins.push_back(L"EmailFile");          plugins.back().desc=L"Allow to send file to some contact";
    plugins.push_back(L"EmailText");          plugins.back().desc=L"Allow to send a text to some contact";
    plugins.push_back(L"WebsiteSearch");      plugins.back().desc=L"Trigger browser searches";

    std::vector<CString> pluginsfolders;
    GetSubFolderList(L"plugins",pluginsfolders);
    for(std::vector<CString>::iterator it=pluginsfolders.begin();it!=pluginsfolders.end();it++) {
        CString key(*it);
        CString pluginxml=L"plugins\\"+*it+"\\plugin.xml";

        pugi::xml_document d;
        if(!d.load_file(pluginxml))
            continue;

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


CString getGUID() {
    CLSID  clsid;
    CoCreateGuid(&clsid);
    OLECHAR szGuid[40]={0}; int nCount = ::StringFromGUID2(clsid, szGuid, 40);
    return szGuid;
}

int lastEditHotkey=0;
int lastEditHotkeyModifier=0;

LRESULT CALLBACK HotKeyEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg == WM_GETDLGCODE) {
        return DLGC_WANTALLKEYS;
    } else if(msg==WM_KEYDOWN) { 

        //CString tmp; tmp.Format(L"X : %x \n", hWnd);
        //OutputDebugString(tmp);

        // ignore dead keys
        if(wParam==VK_CAPITAL)
            return TRUE;
        if(wParam==VK_SHIFT)
            return TRUE;
        if(wParam==VK_CONTROL)
            return TRUE;
        if(wParam==VK_MENU)
            return TRUE;

        //if(wParam>VK_MENU) {
        lastEditHotkeyModifier=0;
        CString mod;
        BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
        if(bShift) {
            lastEditHotkeyModifier|=MOD_SHIFT;
        }
        BOOL bCtl = GetKeyState(VK_CONTROL) & 0x8000;
        if(bCtl) {
            lastEditHotkeyModifier|=MOD_CONTROL;
        }
        BOOL bAlt = GetKeyState(VK_MENU) & 0x8000;
        if(bAlt) {
            lastEditHotkeyModifier|=MOD_ALT;
        }
        BOOL blWin= GetKeyState(VK_LWIN) & 0x8000;
        BOOL brWin= GetKeyState(VK_RWIN) & 0x8000;
        if(blWin ||brWin) {
            lastEditHotkeyModifier|=MOD_WIN;
        }

        lastEditHotkey=wParam;

        CString txt=HotKeyToString(lastEditHotkeyModifier, lastEditHotkey);
        SetWindowText(hWnd,txt);
        //ListView_SetItemText(hListView, ((NMLVDISPINFO*)lParam)->item.iItem, 0, (LPWSTR)txt.GetString()); 
        // cancel just after giving the control the time to close with enter to differenciate
        //SetFocus(::GetParent(hWnd));
        return TRUE;
    } else if(msg==WM_SYSKEYDOWN) {
        
        if(wParam==VK_RETURN) {
            return TRUE;
        }

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

            int x=12;
            int y=70;
            //getSkinList
            HWND hsskins=CreateWindow(L"STATIC",
                         L"Skin :",
                         WS_VISIBLE|WS_CHILD,
                         x, y, 150, 20,
                         hWnd,
                         0,
                         0,
                         0);
            HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
		    SendMessage(hsskins, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
            y+=20;

            HWND hcbskin=CreateWindow(L"COMBOBOX",
                         L"", 
                         CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 
                         x, y, 153, 20,
                         hWnd,
                         (HMENU)IDC_SKINCB,
                         0,
                         0);
            SendMessage(hcbskin, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
            
            std::vector<CString> skins;
            getSkinList(skins);
            for(int i=0;i<skins.size();i++) {
                SendMessage(hcbskin,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)skins[i].GetString()); 
                if(g_pQatapult->m_skin==skins[i])
                    SendMessage(hcbskin,(UINT)CB_SETCURSEL,(WPARAM) i,(LPARAM)i); 
            }
            return TRUE;
        }
        case WM_SAVESETTINGS:
        {            
            // save hotkey
            hotkeycode = lastEditHotkey; hotkeymodifiers = lastEditHotkeyModifier;

            SetSettingsInt(L"hotKeys", L"toggleKey", hotkeycode);
            SetSettingsInt(L"hotKeys", L"toggleModifier", hotkeymodifiers);
            UnregisterHotKey(g_pUI->getHWND(), 1);
            RegisterHotKey(g_pUI->getHWND(), 1, hotkeymodifiers, hotkeycode);                
            
            // save skin
            TCHAR skinname[1024];
            GetDlgItemText(hWnd,IDC_SKINCB, skinname, sizeof(skinname));
            SetSettingsString(L"general",L"skin",skinname);
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
                g_pQatapult->reload();
                //g_pUI->InvalidateIndex();
                ::EndDialog(hWnd, 0);
            } else if(wParam==IDCANCEL) {
                ::EndDialog(hWnd, 0);
                g_pQatapult->show();
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
    addRule(Type(L"FILE",true), Type(L"FILEVERB"), new FileVerbRule); 
                
    TextItemSource *t;

    t=new TextItemSource(L"QATAPULTVERB");
    addSource(t);
    t->addItem(L"Quit",L"icons\\exit.png");
    t->addItem(L"Reload",L"icons\\reload.png");
    t->addItem(L"Options",L"icons\\options.png");
    //t->def=true;
    addRule(Type(L"FILE", false, Array(Type::Predicat(L"rfilename", L"QATAPULT.EXE"))), Type(L"QATAPULTVERB"), new QatapultRule);

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

    // sources
    t=new TextItemSource(L"SOURCEVERB");
    addSource(t);
    t->addItem(L"Open",L"icons\\open.png");
    addRule(Type(L"SOURCE"),Type(t->type),new SourceRule(this));

    // commands 
    /*t=new TextItemSource(L"COMMANDVERB");
    addSource(t);
    t->addItem(L"Save",L"icons\\open.png");
    t->addItem(L"Run after delay",L"icons\\open.png");
    t->addItem(L"Run at time",L"icons\\open.png");
    addRule(Type(L"COMMAND"),Type(t->type),new CommandRule);*/

    // empty
    m_emptysource=t=new TextItemSource(L"EMPTY");
    t->addItem(L"",L"");
    addSource(m_emptysource);

    // pseudo source for object created as return
    m_inputsource=new Source(L"INPUTSOURCE");
    addSource(m_inputsource);
    //m_history.m_inputsource = m_inputsource;


    // hotkey
    hotkeycode=GetSettingsInt(L"hotKeys", L"toggleKey",VK_SPACE);
    hotkeymodifiers=GetSettingsInt(L"hotKeys", L"toggleModifier",MOD_SHIFT);      

    // define hotkey
    RegisterHotKey(g_pUI->getHWND(), 1, hotkeymodifiers, hotkeycode);

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

    m_args.push_back(RuleArg());
    m_args.back().m_results.push_back(SourceResult());

    m_queries.push_back(L"");
    
    onQueryChange(L"");
    
    // create dialogs
    createSettingsDlg();

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
}
/*
int SHIFT = MOD_SHIFT;
int CTRL  = MOD_CONTROL;
int ALT   = MOD_ALT;
*/
int appendvk    = VK_OEM_COMMA;
int appendmod   = 0;

int makecommandvk    = VK_RETURN;
int makecommandmod   = MOD_CONTROL;

int validatevk  = VK_RETURN;
int validatemod = 0;
    
int escapevk    = VK_ESCAPE;
int escapemod   = 0;

int nextpanevk  = VK_TAB;
int nextpanemod = 0;

int prevpanevk  = VK_TAB;
int prevpanemod = MOD_SHIFT;

int exitsourcevk = VK_LEFT;
int exitsourcemod = 0;
    
int expandvk = VK_RIGHT; // expand or complete a result or a source
int expandmod = 0;

int nextresultvk = VK_DOWN;
int nextresultmod = 0;

int nextpageresultvk = VK_NEXT;
int nextpageresultmod = 0;

int prevresultvk = VK_UP;
int prevresultmod = 0;

int prevpageresultvk = VK_PRIOR;
int prevpageresultmod = 0;

int toggleeditmodevk = 'E';
int toggleeditmodemod = MOD_CONTROL;
/*
class EditShortcutDlg : public CDialogImpl<EditShortcutDlg>
{
public:
    EditShortcutDlg(int x,int y) {
        m_x=x;
        m_y=y;
    }
    enum { IDD = IDD_EDITSHORTCUTDLG };
 
    BEGIN_MSG_MAP(ShortcutDlg)
        //MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        //MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    END_MSG_MAP()
};*/


// edit shortcut, type any key combination, use that combination
class ShortcutDlg : public CDialogImpl<ShortcutDlg>
{
public:
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(ShortcutDlg)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        COMMAND_HANDLER(0, IDCANCEL, OnCancel)
        COMMAND_HANDLER(0, IDOK, OnOK)
        NOTIFY_HANDLER(0, NM_KILLFOCUS, OnKillFocus)
    END_MSG_MAP()

    int  icount;
    CListViewCtrl listview;
    HWND hListView;

    LRESULT OnOK(WPARAM wParam1, WPARAM wParam2, HWND lParam, BOOL& bHandled)
    {
        return S_OK;
    }
    LRESULT OnCancel(WPARAM wParam1, WPARAM wParam2, HWND lParam, BOOL& bHandled)
    {
        return S_OK;
    }
    LRESULT OnKillFocus(int wParam, LPNMHDR lParam, BOOL &bHandled)
    {
        return S_OK;
    }
    void addItem(const CString &hotkey, const CString &desc) {
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=(LPWSTR)hotkey.GetString();
        lvi.mask=LVFIF_TEXT;
        lvi.iItem=10000;
        int item=ListView_InsertItem(hListView, &lvi);
        ListView_SetItemText(hListView, item, 1, (LPWSTR)desc.GetString());
    }
    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (uMsg == WM_GETDLGCODE) {
		    bHandled=TRUE;
            return DLGC_WANTALLKEYS;
        }
        return S_OK;
    }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    { 
        RECT rcClient;                       // The parent window's client area.
        GetClientRect(&rcClient);

        int y=0;
        /*text.C

        edit.Create(m_hWnd, CRect(0,y,100,y+20), L"", WS_CHILD|WS_VISIBLE|WS_BORDER|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
        OldHotKeyEditProc=SubclassWindowX(edit.m_hWnd, HotKeyEditProc);        
        HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
		edit.SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
        */

        icount=0;

        CRect r;
        INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        rcClient.top+=50;
        listview.Create(m_hWnd, rcClient, L"", WS_CHILD | LVS_REPORT | /*LVS_EDITLABELS |*/ WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |LVS_EX_FULLROWSELECT );
        hListView=listview.m_hWnd;

        ::GetClientRect(hListView, &r);

        LVCOLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText=L"Hotkey";
        lvc.cx=100;
        ListView_InsertColumn(hListView, 0, &lvc);

        lvc.pszText=L"Description";
        lvc.cx=r.Width()-100;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(hListView, 1, &lvc);
        
        // must look like this visually
        addItem(HotKeyToString(hotkeymodifiers,   hotkeycode),       L"Show/hide Qatapult");
        addItem(HotKeyToString(validatemod,       validatevk),       L"Execute the current action if possible");
        addItem(HotKeyToString(appendmod,         appendvk),         L"Stack the current selection and select another object (multi selection )");
        addItem(HotKeyToString(escapemod,         escapevk),         L"Cancel a result, cancel a pane, close if empty");
        addItem(HotKeyToString(nextpanemod,       nextpanevk),       L"Move to next pane");
        addItem(HotKeyToString(prevpanemod,       prevpanevk),       L"Move to previous pane");
        addItem(HotKeyToString(exitsourcemod,     exitsourcevk),     L"Exit a source (on a source result only )");
        addItem(HotKeyToString(expandmod,         expandvk),         L"Expand a result or enter a source");
        addItem(HotKeyToString(nextresultmod,     nextresultvk),     L"Show the result pane or focus the next result");
        addItem(HotKeyToString(prevresultmod,     prevresultvk),     L"Select the previous result");
        addItem(HotKeyToString(nextpageresultmod, nextpageresultvk), L"Next page of results");
        addItem(HotKeyToString(prevpageresultmod, prevpageresultvk), L"Previous page of results");
        addItem(HotKeyToString(toggleeditmodemod, toggleeditmodevk), L"Toggle edit mode");

        return S_OK;
    }
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        
        if(((NMHDR*)lParam)->code==NM_CLICK && ((NMHDR*)lParam)->idFrom == 0) {            

        }

        /*static int i=0; i++;
        CString fmt; fmt.Format(L"%d : %d\n",i,((NMHDR*)lParam)->code);
        OutputDebugString(fmt);

        if(((NMHDR*)lParam)->code==NM_CLICK && ((NMHDR*)lParam)->idFrom == 0) {
            NMITEMACTIVATE *lpia=(NMITEMACTIVATE*)lParam;          

        } else if(((NMHDR*)lParam)->code==NM_KILLFOCUS && ((NMHDR*)lParam)->idFrom == 0) {                    
            //OutputDebugString(L"KILLFOCUS\n");
            return S_OK;
        } else if(((NMHDR*)lParam)->code==NM_SETFOCUS && ((NMHDR*)lParam)->idFrom == 0) {                    
            //OutputDebugString(L"SETFOCUS\n");
            return S_OK;
        } else if(((NMHDR*)lParam)->code==LVN_BEGINLABELEDIT) {            
            lastEditHotkeyModifier=-1;
            lastEditHotkey=-1;
            HWND hEdit=ListView_GetEditControl(hListView);
            //::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) | ES_MULTILINE | ES_WANTRETURN);
            WNDPROC OldHotKeyEditProc=SubclassWindowX(hEdit, HotKeyEditProc);
        } else if(((NMHDR*)lParam)->code==LVN_ENDLABELEDIT) {            
            switch( ((NMLVDISPINFO*)lParam)->item.iItem ) {
                case 0:  hotkeycode       = lastEditHotkey; hotkeymodifiers   = lastEditHotkeyModifier; break;
                case 1:  validatevk       = lastEditHotkey; validatemod       = lastEditHotkeyModifier; break;
                case 2:  appendvk         = lastEditHotkey; appendmod         = lastEditHotkeyModifier; break;
                case 3:  escapevk         = lastEditHotkey; escapemod         = lastEditHotkeyModifier; break;
                case 4:  prevpanevk       = lastEditHotkey; prevpanevk        = lastEditHotkeyModifier; break;
                case 5:  exitsourcevk     = lastEditHotkey; exitsourcemod     = lastEditHotkeyModifier; break;
                case 6:  expandvk         = lastEditHotkey; expandmod         = lastEditHotkeyModifier; break;
                case 7:  nextresultvk     = lastEditHotkey; nextresultmod     = lastEditHotkeyModifier; break;
                case 8:  prevresultvk     = lastEditHotkey; prevresultmod     = lastEditHotkeyModifier; break;
                case 9:  nextpageresultvk = lastEditHotkey; nextpageresultmod = lastEditHotkeyModifier; break;
                case 10: prevpageresultvk = lastEditHotkey; prevpageresultmod = lastEditHotkeyModifier; break;
                case 11: toggleeditmodevk = lastEditHotkey; toggleeditmodemod = lastEditHotkeyModifier; break;
            }
            //CString fmt;
            //fmt.Format(L"LVN_ENDLABELEDIT %s\n", ((NMLVDISPINFO*)lParam)->item.pszText);
            //OutputDebugString(fmt);
        } else if(((NMHDR*)lParam)->code==NM_RETURN) {
            return S_FALSE;
        }*/
        return S_OK;
    }
};

struct Record {
    void save() {
    }
    //std::vector<CString>       columns;
    std::map<CString, CString> values;
};

struct Table {
    Table(const CString &name) {
        m_name=name;
        int rc = sqlite3_open("databases\\"+CStringA(name)+".db", &db);
        
        /*char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE "+CStringA(name)+"(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        rc = sqlite3_prepare_v2(db, "SELECT * FROM "+CStringA(name)+";", -1, &queryall, &unused);*/
    }
    int query(std::vector<Record> &records) {
        int rc = sqlite3_prepare_v2(db,"SELECT * FROM "+CStringA(m_name)+";",-1, &queryall, &unused);
        int i=0;
        while((rc=sqlite3_step(queryall))==SQLITE_ROW) {

            int columns=sqlite3_column_count(queryall);

            records.push_back(Record());
            for(int j=0;j<columns;j++) {
                int                  type  = sqlite3_column_type(queryall,j);
                if(type==SQLITE_TEXT) {
                    const char          *name  = sqlite3_column_name(queryall,j);
                    const unsigned char *value = sqlite3_column_text(queryall,j);
                    records.back().values[UTF8toUTF16(name)] = UTF8toUTF16(value);
                }
            }

            i++;
        }

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(queryall);
        return i;
    }

    CString       m_name;
    sqlite3_stmt *queryall;
    const char   *unused;
    int           rc;
    sqlite3      *db;
};

CString capitalize(const CString &s) {
    CString tmp=s.Left(1).MakeUpper() + s.Mid(1);
    return tmp;
}

class WebsiteSearchDlg : public CDialogImpl<ShortcutDlg>
{
public:
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(WebsiteSearchDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    END_MSG_MAP()

    HWND hListView;

    WebsiteSearchDlg() {
    }

    void addItem(Record &r) {
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=L"";//(LPWSTR)r.values[L"key"].GetString();
        lvi.mask=LVFIF_TEXT;
        lvi.iItem=10000;
        int item=ListView_InsertItem(hListView, &lvi);

        int i=0;
        for(std::map<CString, CString>::iterator it=r.values.begin(); it!=r.values.end(); it++, i++) {            
            ListView_SetItemText(hListView, item, i, (LPWSTR)it->second.GetString());
        }
            //ListView_SetItemText(hListView, item, 1, (LPWSTR)href.GetString());
        //ListView_SetItemText(hListView, item, 2, (LPWSTR)query.GetString());
    }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    { 
        RECT rcClient;                       // The parent window's client area.
        GetClientRect(&rcClient);

        CRect r;
        INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        // Create the list-view window in report view with label editing enabled.
        hListView = ::CreateWindow(WC_LISTVIEW, 
                                        L"",
                                        WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE | WS_BORDER,
                                        rcClient.left, rcClient.top,
                                        rcClient.right - rcClient.left,
                                        rcClient.bottom - rcClient.top,
                                        m_hWnd,
                                        0,
                                        0,
                                        NULL); 



        ::GetClientRect(hListView, &r);
        /*
        LVCOLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText=L"Icon name";
        lvc.cx=100;
        ListView_InsertColumn(hListView, 0, &lvc);

        lvc.pszText=L"Url";
        lvc.cx=100;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(hListView, 1, &lvc);

        lvc.pszText=L"Query";
        lvc.cx=r.Width()-200;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(hListView, 2, &lvc);
        */
        Table table("websites");
        std::vector<Record> records;
        table.query(records);

        int i=0;
        Record &rec=records.front();
        for(std::map<CString,CString>::iterator col=records.front().values.begin(); col!=records.front().values.end(); col++, i++) {
            LVCOLUMN lvc;
            lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
            lvc.fmt = LVCFMT_LEFT;
            CString str = capitalize(col->first);
            lvc.pszText=(LPWSTR)str.GetString();
            lvc.cx=100;
            ListView_InsertColumn(hListView, i, &lvc);
        }

        for(std::vector<Record>::iterator it=records.begin(); it!=records.end(); it++) {
            addItem(*it);
        }

        return S_OK;
    }
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return S_OK;
    }
};


PluginsDlg       pluginsdlg;
SearchFoldersDlg searchfolderdlg;
ShortcutDlg      shortcutdlg;
WebsiteSearchDlg websiteSearchDlg;

void clearRuleArg(RuleArg &r) {
    r.m_results.clear();
    r.m_results.push_back(SourceResult());
}

void Qatapult::reset() {
    m_systray.Destroy();

    if(pluginsdlg.IsWindow())
        pluginsdlg.DestroyWindow();

    if(searchfolderdlg.IsWindow())
        searchfolderdlg.DestroyWindow();
    
    if(shortcutdlg.IsWindow())
        shortcutdlg.DestroyWindow();

    if(websiteSearchDlg.IsWindow())
        websiteSearchDlg.DestroyWindow();

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
    clearResults(m_nextresults);

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
                for(pugi::xpath_node_set::const_iterator itelt=elts.begin(); itelt!=elts.end(); itelt++) {
                    CHAR name[256];
                    CHAR op[256];
                    CStringA str(itelt->node().child_value());
                    sscanf_s(str, "%[^~=]", name, sizeof(name));
                    sscanf_s(str.Mid(strlen(name)), "%[~=]", op, sizeof(op));
                    CString value(str.Mid(strlen(name)+strlen(op)));
                    preds.push_back(Type::Predicat(name,value,op));
                }
                
                r->m_types.push_back(Type(type,false,preds));
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

extern BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Qatapult::createSettingsDlg() {
    m_hwndsettings=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), 0, (DLGPROC)SettingsDlgProc);
    //ShowWindow(m_hwndsettings, SW_SHOW);
    HWND hTreeView=GetDlgItem(m_hwndsettings, IDC_TREE);    

    pluginsdlg.Create(m_hwndsettings);
    pluginsdlg.SetWindowPos(0, 160, 11, 0, 0, SWP_NOSIZE);

    HWND hwndGmail=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GMAILCONTACTS), m_hwndsettings, (DLGPROC)DlgProc);
    SetWindowPos(hwndGmail, 0, 160, 0, 0, 0, SWP_NOSIZE);

    //HWND hwndEmail=CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EMAIL), m_hwndsettings, (DLGPROC)DlgProc);
    //SetWindowPos(hwndEmail, 0, 160, 0, 0, 0, SWP_NOSIZE);

    searchfolderdlg.Create(m_hwndsettings);
    searchfolderdlg.SetWindowPos(0, 160, 11, 0, 0, SWP_NOSIZE);

    shortcutdlg.Create(m_hwndsettings);
    shortcutdlg.SetWindowPos(0, 160, 11, 0, 0, SWP_NOSIZE);
    shortcutdlg.ShowWindow(SW_SHOW);

    //websiteSearchDlg.Create(m_hwndsettings);
    //websiteSearchDlg.SetWindowPos(0, 160, 11, 0, 0, SWP_NOSIZE);

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

    tviis.item.pszText = L"Shortcuts";
    tviis.item.lParam=(LPARAM)shortcutdlg.m_hWnd;
    HTREEITEM htreeShortcuts=TreeView_InsertItem(hTreeView, &tviis);

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

    //tviis.item.pszText = L"Website search";
    //tviis.item.lParam=(LPARAM)websiteSearchDlg.m_hWnd;
    //HTREEITEM htreeSearch=TreeView_InsertItem(hTreeView, &tviis);

    BOOL b=TreeView_SelectItem(hTreeView, htreeG);
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

void Qatapult::collectItems(const CString &q, const uint pane, std::vector<RuleArg> &args, std::vector<SourceResult> &results, int def) {
    clearResults(results);

    // collect all active rules (match could have an args that tell how much to match )
    // i should probably ignore the current pane for the match or just match until pane-1  

    // collect displayable items
    if(pane>=m_customsources.size() || m_customsources[pane]==0) {
        std::vector<Rule *> activerules;
        if(getActiveRules(pane,args,activerules)==0) {
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

        // get results for each sources
        // - sources need to filter by themselve from the 'activesources' arguments => this allows sources to output different types at once
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
        results[i].source()->rate(q,&results[i]);
    }
}

int Qatapult::resultSourceCmp(SourceResult &r1, SourceResult &r2) {
    return r1.rank() > r2.rank();
}

struct Results {
    int                        request;
    int                        pane;
    std::vector<SourceResult>  results;
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

    collectItems(q, m_pane, m_args, presults->results, 0);

    PostMessage(getHWND(), WM_PUSHRESULT,(WPARAM)presults,!select);
}
void Qatapult::showNextArg() {
    // check if there is extra args
    if(m_args.size()>0) {
        Results *presults=new Results;
        presults->request=m_request;
        presults->pane=m_pane+1;

        collectItems(L"", m_pane+1, m_args, presults->results, 1);

        PostMessage(getHWND(), WM_PUSHRESULT2,(WPARAM)presults,0);
    }
}

// TODO : move to rulearg ??
void copySourceResult(RuleArg &ra, SourceResult &r) {
    ra.m_results.back()=r;
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
    if(m_args[pane].m_results.size()==0) m_args[pane].m_results.push_back(SourceResult());
    copySourceResult(m_args[pane],r);
}

void Qatapult::setRetArg(uint pane, SourceResult &r) {
    ensureArgsCount(m_retArgs,pane+1,EA_NO_REMOVE_EXTRA);
    if(m_retArgs[pane].m_results.size()==0) m_retArgs[pane].m_results.push_back(SourceResult());
    copySourceResult(m_retArgs[pane],r);
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
    
    o->drawItem(g, &m_args[i].item(e), RectF(float(x), float(y), float(w), float(h)));

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

    o->drawItem(g, &m_results[i], RectF(float(x), float(y), float(w), float(h)));

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
        getResObject(i)->drawListItem(g,&m_results[i],RectF(float(x),float(y+40*p),float(rw),float(40)),m_fontsize,m_focusedresult==i,g_textcolor,m_resultbgcolor,m_resultfocuscolor);
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
    m_customsources.clear();

    clearResults(m_results);
    clearResults(m_nextresults);
    invalidate();
}

void Qatapult::exec() {
    // collect all active rules
    // might need to disambiguate here ?
    for(uint i=0;i<m_rules.size();i++) {
        if(m_rules[i]->match(m_args, m_args.size())>1) {
            // found one rule
            hide();

            m_retArgs.clear();
            if(m_rules[i]->execute(m_args)) {

                //saveCommand(new CommandObject(m_args, m_inputsource));

                for(uint a=0;a<m_args.size(); a++) {
                    for(uint r=0;r<m_args[a].m_results.size(); r++) {
                        if(m_args[a].source())
                            m_args[a].source()->validate(&m_args[a].item(r));
                    }
                }
                    
                clearPanes();

                // if there is ret args copy them and show interface
                if(m_retArgs.size() != 0) {
                    m_args=m_retArgs;
                    m_pane=m_args.size();                        
                    invalidate();
                    show();
                }
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
LRESULT Qatapult::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {   
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
    
    if(msg == WM_COPYDATA)
    {
        COPYDATASTRUCT *cpd=(COPYDATASTRUCT*)lParam;
        CString str;
        str.SetString((WCHAR*)cpd->lpData, cpd->cbData/sizeof(WCHAR));

        CComVariant ret;
        m_commandsHost.Eval(str, &ret);
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
            // remove all results and add new ones
            clearResults(m_results);
            for(int i=0;i<p->results.size();i++) {
                m_results.push_back(p->results[i]);
            }
            std::sort(m_results.begin(), m_results.end(), resultSourceCmp);

            // select the first result if any
            if(lParam==0)
                onSelChange(m_results.size()>0?&m_results.front():&SourceResult());

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

            // remove all results and add new ones
            clearResults(m_nextresults);
            for(int i=0;i<p->results.size();i++) {
                m_nextresults.push_back(p->results[i]);
            }
            std::sort(m_nextresults.begin(), m_nextresults.end(), resultSourceCmp);

            // initialize the second result, let empty if nothing match to prevent tabbing to it
            if(m_nextresults.size()>0)
                setResult(m_pane+1,m_nextresults.front());

            invalidate();
        }
        delete p;
        return TRUE;
    }
    else if(msg == WM_KEYDOWN && (wParam == appendvk && mod == appendmod) )
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
        clearResults(m_nextresults);
        invalidate();            
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && (wParam == makecommandvk && mod == makecommandmod) )
    {
        /*saveCommand(new CommandObject(m_args, m_inputsource));

        clearPanes();

        SourceResult sr;
        sr.object() = m_commandhistory.back();
        onSelChange(&sr);
        */
        CommandObject *c=new CommandObject(m_args, m_inputsource);
        clearPanes();
        onSelChange(&SourceResult(c));
    }
    else if(msg == WM_KEYDOWN && wParam == validatevk && mod == validatemod)
    {
        if(m_editmode==1 && bShift) {
            m_input.appendAtCaret(L"\n");
            invalidate();
            return FALSE;
        }
        
        exec();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == escapevk && mod == escapemod)
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
    else if(msg == WM_KEYDOWN && wParam == nextpanevk && mod == nextpanemod)
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
        onQueryChange(m_queries[m_pane],false);

        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == prevpanevk && mod == prevpanemod )
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
    else if(msg == WM_KEYDOWN && wParam == VK_DELETE)
    {
        bool bCtrl=!!(GetKeyState(VK_CONTROL)&0x8000);
        m_input.del(bCtrl);
        onQueryChange(m_input.m_text);
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_HOME)
    {
        if(m_editmode==1) {
            m_input.home();
            invalidate();
        }
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == VK_END)
    { 
        if(m_editmode==1) {
            m_input.end();
            invalidate();
        }
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
    else if(msg==WM_KEYDOWN &&  wParam == toggleeditmodevk && mod == toggleeditmodemod)
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
    else if(msg == WM_KEYDOWN && wParam == exitsourcevk && mod == exitsourcemod && m_pane<m_customsources.size())
    {
        m_customsources[m_pane]=0;
        m_input.setText(L"");
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN && wParam == expandvk && mod == expandmod)
    {
        SourceResult *r=getSelectedItem();
        if(!r)
            return FALSE;
        CString q;
        Source *s=r->source()->getSource(*r,q);
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

    else if(msg == WM_KEYDOWN && m_editmode!=1 && (wParam == nextresultvk && mod == nextresultmod) || (wParam == nextpageresultvk && mod == nextpageresultmod) )
    {	
        if(wParam == nextresultvk && mod == nextresultmod) {
            m_focusedresult++;
        } else if(wParam == nextpageresultvk && mod == nextpageresultmod) {
            m_focusedresult+=m_visibleresultscount;
        }

        if(m_focusedresult>=m_results.size())
            m_focusedresult=m_results.size()-1;

        if(m_focusedresult>=m_resultspos+m_visibleresultscount)
            m_resultspos=m_focusedresult-m_visibleresultscount+1;
    
        if(m_results.size()>0)
            onSelChange(&m_results[m_focusedresult]);
        invalidate();

        return FALSE;
    }
    else if(msg == WM_KEYDOWN && m_editmode!=1 && (wParam == prevresultvk && mod == prevresultmod) || (wParam == prevpageresultvk && mod == prevpageresultmod))
    {	
        if(wParam == prevresultvk && mod == prevresultmod)
            m_focusedresult--;
        else if(wParam == prevpageresultvk && mod == prevpageresultmod)
            m_focusedresult-=m_visibleresultscount;

        if(m_focusedresult<0)
            m_focusedresult=0;

        if(m_focusedresult<m_resultspos)
            m_resultspos=m_focusedresult;

        if(m_results.size()>m_focusedresult)
            onSelChange(&m_results[m_focusedresult]);
        invalidate();
        return FALSE;
    }
    else if(msg == WM_KEYDOWN)
    {
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
        } else if(wParam<VK_SPACE) {
            ;
        }

        // all keys not assigned to a shortcut must be send to translatemessage
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
        ShowWindow(m_hwnd, SW_SHOW);
    }
    else if(msg==WM_COMMAND)
    {
        if(wParam==ID_SYSTRAY_SHOW) {
            show();
        } else if(wParam==ID_SYSTRAY_QUIT) {
            PostQuitMessage(0);
        } else if(wParam==ID_SYSTRAY_OPTIONS) {    
            CenterWindow(m_hwndsettings);
            ShowWindow(m_hwndsettings,SW_SHOW);
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