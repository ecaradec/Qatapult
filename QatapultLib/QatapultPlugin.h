#pragma once
#include "SettingsDlg.h"

extern SettingsDlg *settingsdlg;

void CenterWindow(HWND hwnd) {
    CRect workarea;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

    CRect r;
    GetWindowRect(hwnd, r);
    
    SetWindowPos(hwnd, 0, (workarea.left+workarea.right)/2-r.Width()/2, max(workarea.top, (workarea.top+workarea.bottom)/2 - r.Height()/2), 0, 0, SWP_NOSIZE);
}

struct QuitQatapultRule : Rule {
    virtual bool execute(std::vector<RuleArg> &args) {
        PostQuitMessage(0);
        return true;
    }
};
struct ReloadQatapultRule : Rule {
    virtual bool execute(std::vector<RuleArg> &args) {
        g_pUI->reload();
        return true;
    }
};

struct OptionsQatapultRule : Rule {
    virtual bool execute(std::vector<RuleArg> &args) {
        g_pQatapult->createSettingsDlg();
        settingsdlg->CenterWindow();
        settingsdlg->ShowWindow(SW_SHOW);
        ShowWindow(g_pQatapult->m_hwnd, SW_HIDE);
        return true;
    }
};