#pragma once

void CenterWindow(HWND hwnd) {
    CRect workarea;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

    CRect r;
    GetWindowRect(hwnd, r);
    
    SetWindowPos(hwnd, 0, (workarea.left+workarea.right)/2-r.Width()/2, max(workarea.top, (workarea.top+workarea.bottom)/2 - r.Height()/2), 0, 0, SWP_NOSIZE);
}

// commented for use with textitemsource
struct QatapultRule : Rule {
    QatapultRule() {}
    virtual bool execute(std::vector<SourceResult> &args) {
        if(args[1].display()==L"Quit")
            PostQuitMessage(0);
        else if(args[1].display()==L"Reload")
            g_pUI->Reload();
        else if(args[1].display()==L"Options") {
            ::CenterWindow(g_pQatapult->m_hwndsettings);
            ShowWindow(g_pQatapult->m_hwndsettings,SW_SHOW);
            ShowWindow(g_pQatapult->m_hwnd, SW_HIDE);
        }
        return true;
    }
};