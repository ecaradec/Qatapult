#pragma once
#include "HotKeyEditProc.h"
#include "GeneralDlg.h"
#include "EmailDlg.h"
#include "ActionsDlg.h"
#include "PluginDlg.h"
//#include "ShortcutDlg.h"
#include "WebsiteSearchDlg.h"

struct SettingsDlg;
extern SettingsDlg* settingsdlg;

struct SettingsDlg : CDialogImpl<SettingsDlg> {
    enum {
        IDD = IDD_SETTINGS
    };

	BEGIN_MSG_MAP(ActionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDOK, BN_CLICKED, OnOk)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
        COMMAND_HANDLER(IDC_APPLY, BN_CLICKED, OnApply)
        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
        NOTIFY_HANDLER(ID_TAB, TCN_SELCHANGE, OnSelChange)
	END_MSG_MAP()

    enum {
        ID_TAB
    };

    GeneralDlg       m_generalDlg;
    SearchFoldersDlg m_searchfolderdlg;
    ActionsDlg       m_actionsDlg;
    CTreeViewCtrl    m_treeview;
    CTabCtrl         m_tabctrl;

    void OnFinalMessage(HWND) {
        delete settingsdlg;
        settingsdlg=0;
    }
    LRESULT OnSelChange(int id, LPNMHDR lpnhhdr, BOOL& bHandled) {
        m_generalDlg.ShowWindow(m_tabctrl.GetCurSel()==0?SW_SHOW:SW_HIDE);
        m_searchfolderdlg.ShowWindow(m_tabctrl.GetCurSel()==1?SW_SHOW:SW_HIDE);
        m_actionsDlg.ShowWindow(m_tabctrl.GetCurSel()==2?SW_SHOW:SW_HIDE);
        return S_OK;
    }
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {   
        CRect r;
        GetClientRect(&r);
        r.bottom-=40;
        m_tabctrl.Create(*this, r, L"", WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, 0, ID_TAB);
        m_tabctrl.AddItem(L"General");
        m_tabctrl.AddItem(L"Search folders");
        m_tabctrl.AddItem(L"Actions");

        m_tabctrl.AdjustRect(FALSE, &r);

        m_generalDlg.Create(*this);
        m_generalDlg.MoveWindow(&r);

        m_searchfolderdlg.Create(*this);
        m_searchfolderdlg.MoveWindow(&r);
        m_actionsDlg.Create(*this);
        m_actionsDlg.MoveWindow(&r);

        m_generalDlg.ShowWindow(SW_SHOWNORMAL);

        SetStdFontOfDescendants(*this);

        return S_OK;
    }

    LRESULT OnOk(WORD hiWord, WORD loWord, HWND lParam, BOOL& bHandled) {
        SendMessageToDescendants(WM_SAVESETTINGS,0,0,FALSE);
        settings.save_file("settings.xml");
        DestroyWindow();
        g_pQatapult->reload();        
        return S_OK;
    }

    LRESULT OnApply(WORD hiWord, WORD loWord, HWND lParam, BOOL& bHandled) {
        SendMessageToDescendants(WM_SAVESETTINGS,0,0,FALSE);
        settings.save_file("settings.xml");
        g_pQatapult->reload();        
        return S_OK;
    }

    LRESULT OnCancel(WORD hiWord, WORD loWord, HWND lParam, BOOL& bHandled) {
        DestroyWindow();
        g_pQatapult->show();        
        return S_OK;
    }
    LRESULT OnSelChanged(int id, LPNMHDR lpnhhdr, BOOL& bHandled) {
        NMTREEVIEW *nmtv=(NMTREEVIEW*)lpnhhdr;
        EnumChildWindows(m_hWnd, ToggleSettingsEWProc, nmtv->itemNew.lParam);
        return S_OK;
    }
    static BOOL CALLBACK ToggleSettingsEWProc(HWND hwnd, LPARAM lParam) {
        TCHAR className[256];
        GetClassName(hwnd, className, sizeof(className));
        if(_tcscmp(L"#32770", className)==0)
            ::ShowWindow(hwnd, (hwnd==(HWND)lParam)?SW_SHOW:SW_HIDE);
        return TRUE;
    }
};
