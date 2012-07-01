
BOOL CALLBACK GeneralDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            int x=12;
            int y=10;
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
            // save skin
            TCHAR skinname[1024];
            GetDlgItemText(hWnd,IDC_SKINCB, skinname, sizeof(skinname));
            SetSettingsString(L"general",L"skin",skinname);
        }        
    }
    return FALSE;
}
