#include "Layout.h"

struct GeneralDlg : CDialogImpl<GeneralDlg> {
    BEGIN_MSG_MAP(ActionsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SAVESETTINGS, OnSaveSettings)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
        COMMAND_HANDLER(IDC_OPEN_PLUGIN_FOLDER,BN_CLICKED, OnOpenPluginFolder)
    END_MSG_MAP()

    enum {
        IDC_OPEN_PLUGIN_FOLDER=1
    };

    HotKeyEdit m_hkedit;

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {        
        int x=12;
        int y=10;
        
        Layout l(*this);

        CStatic hkLbl;
        hkLbl.Create(*this, l.getSpace(80,22), L"Hotkey : ", WS_VISIBLE|WS_CHILD|SS_CENTERIMAGE|SS_RIGHT);                
        m_hkedit.Create(*this, l.getSpace(150,22), L"", WS_VISIBLE|WS_CHILD, WS_EX_CLIENTEDGE);
        m_hkedit.setHotKey(hotkeys[0].mod, hotkeys[0].vk);

        l.clearRow();

        CStatic skinLbl;
        skinLbl.Create(*this, l.getSpace(80,22), L"Skin : ", WS_VISIBLE|WS_CHILD|SS_CENTERIMAGE|SS_RIGHT);

        CComboBox skinCb;
        skinCb.Create(*this, l.getSpace(150,22), L"", CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 0, IDC_SKINCB);
            
        std::vector<CString> skins;
        getSkinList(skins);
        for(int i=0;i<skins.size();i++) {
            skinCb.AddString(skins[i]);
            if(g_pQatapult->m_skin==skins[i])
                skinCb.SetCurSel(i);
        }
        
        l.clearRow();
        
        CStatic pluginsLbl;
        pluginsLbl.Create(*this, l.getSpace(80,25), L"Plugins : ", WS_VISIBLE|WS_CHILD|SS_CENTERIMAGE|SS_RIGHT);

        CButton pluginsBtn;
        pluginsBtn.Create(*this, l.getSpace(150,25), L"Open Plugin Folder", WS_CHILD | WS_VISIBLE, 0, IDC_OPEN_PLUGIN_FOLDER);
        
        SetStdFontOfDescendants(*this);
        return S_OK;
    }
    LRESULT OnSaveSettings(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        // save hotkey
        hotkeys[HK_SHOW].vk=m_hkedit.m_vk;
        hotkeys[HK_SHOW].mod=m_hkedit.m_mod;

        SetSettingsInt(CString(L"hotKeys/")+hotkeys[HK_SHOW].name, L"vk", hotkeys[HK_SHOW].vk);
        SetSettingsInt(CString(L"hotKeys/")+hotkeys[HK_SHOW].name, L"mod", hotkeys[HK_SHOW].mod);
                
        UnregisterHotKey(g_pUI->getHWND(), 1);
        RegisterHotKey(g_pUI->getHWND(), 1, hotkeys[HK_SHOW].mod, hotkeys[HK_SHOW].vk);  
        
        // save skin
        CString skinname;
        GetDlgItemText(IDC_SKINCB, skinname);
        SetSettingsString(L"general",L"skin",skinname);
        return S_OK;
    }

    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (uMsg == WM_GETDLGCODE) {
		    bHandled=TRUE;
            return DLGC_WANTALLKEYS;
        }
        return S_OK;
    }
    LRESULT OnOpenPluginFolder(short id, short code, HWND lParam, BOOL& bHandled)
    {
        ShellExecuteW(0, 0, L"plugins", 0, 0, SW_SHOWNORMAL);
        return S_OK;
    }
    enum { IDD = IDD_EMPTY };
};
