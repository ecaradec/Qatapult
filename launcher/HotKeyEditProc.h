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

struct HotKeyEdit : CWindowImpl<HotKeyEdit, CEdit> {
    BEGIN_MSG_MAP(HotKeyEdit)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeydown)
        MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeydown)
        MESSAGE_HANDLER(WM_CHAR, OnChar)
    END_MSG_MAP()

    int m_mod;
    int m_vk;

    void setHotKey(int mod, int vk) {
        m_mod=mod;
        m_vk=vk;
        SetWindowText(HotKeyToString(mod,vk));
    }

    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        bHandled=TRUE;
        return DLGC_WANTALLKEYS;
    }
    LRESULT OnKeydown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
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
        int mod=0;
        BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
        if(bShift) {
            mod|=MOD_SHIFT;
        }
        BOOL bCtl = GetKeyState(VK_CONTROL) & 0x8000;
        if(bCtl) {
            mod|=MOD_CONTROL;
        }
        BOOL bAlt = GetKeyState(VK_MENU) & 0x8000;
        if(bAlt) {
            mod|=MOD_ALT;
        }
        BOOL blWin= GetKeyState(VK_LWIN) & 0x8000;
        BOOL brWin= GetKeyState(VK_RWIN) & 0x8000;
        if(blWin ||brWin) {
            mod|=MOD_WIN;
        }

        setHotKey(mod,wParam);

        return TRUE;
    }
    LRESULT OnSysKeydown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        bHandled=true;
        if(wParam==VK_RETURN) {
            return TRUE;
        }

        return TRUE;    
    }
    LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        bHandled=true;
        return TRUE;    
    }
};
