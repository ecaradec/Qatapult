
// edit shortcut, type any key combination, use that combination
class ShortcutDlg : public CDialogImpl<ShortcutDlg>
{
public:
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(ShortcutDlg)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        COMMAND_HANDLER(IDCANCEL,BN_CLICKED, OnCancel)
        COMMAND_HANDLER(IDOK,BN_CLICKED, OnOK)
        COMMAND_HANDLER(IDC_CHANGE,BN_CLICKED, OnClickChange)
        NOTIFY_HANDLER(0, NM_KILLFOCUS, OnKillFocus)
    END_MSG_MAP()

    int  icount;
    CListViewCtrl listview;
    CStatic       msg;
    CEdit         edit;
    CButton       change;
    int           currentitem;
    HWND hListView;

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        listview.DestroyWindow();
        msg.DestroyWindow();
        edit.DestroyWindow();
        change.DestroyWindow();

        return S_OK;
    }
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
    LRESULT OnClickChange(WPARAM wParam1, WPARAM wParam2, HWND lParam, BOOL& bHandled)
    {        
        hotkeys[currentitem].vk=lastEditHotkey;
        hotkeys[currentitem].mod=lastEditHotkeyModifier;

        SetSettingsInt(CString(L"hotKeys/")+hotkeys[currentitem].name, L"vk", hotkeys[currentitem].vk);
        SetSettingsInt(CString(L"hotKeys/")+hotkeys[currentitem].name, L"mod", hotkeys[currentitem].mod);
        
        CString hotkey=HotKeyToString(hotkeys[currentitem].mod, hotkeys[currentitem].vk);
        ListView_SetItemText(hListView, currentitem, 0, (LPWSTR)hotkey.GetString());
     
        if(currentitem==HK_SHOW) {
            UnregisterHotKey(g_pUI->getHWND(), 1);
            RegisterHotKey(g_pUI->getHWND(), 1, hotkeys[HK_SHOW].mod, hotkeys[HK_SHOW].vk);  
        }

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

        HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);

        int y=0;
        CRect rmsg(rcClient); rmsg.bottom=20;
        msg.Create(m_hWnd, rmsg, L"Here is the list of available shortcuts. You can also change them here if something is not to your taste.", WS_CHILD|WS_VISIBLE);
        msg.SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));

        y+=20;
        edit.Create(m_hWnd, CRect(0,y,100,y+20), L"", WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_DISABLED, WS_EX_CLIENTEDGE);
        OldHotKeyEditProc=SubclassWindowX(edit.m_hWnd, HotKeyEditProc);                
		edit.SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
        
        change.Create(m_hWnd, CRect(110,y,200,y+20), L"Change", WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_DISABLED,0,IDC_CHANGE);
        change.SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));

        icount=0;

        CRect r;

        rcClient.top+=50;
        listview.Create(m_hWnd, rcClient, L"", WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | /*LVS_EDITLABELS |*/ WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        hListView=listview.m_hWnd;
        ListView_SetExtendedListViewStyle(listview.m_hWnd,LVS_EX_FULLROWSELECT);

        ::GetClientRect(hListView, &r);

        LVCOLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText=L"Hotkey";
        lvc.cx=100;
        ListView_InsertColumn(hListView, 0, &lvc);

        lvc.pszText=L"Description";
        lvc.cx=r.Width()-120;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(hListView, 1, &lvc);
        
        // must look like this visually
        for(int i=0;i<HK_LAST;i++) {
            addItem(HotKeyToString(hotkeys[i].mod, hotkeys[i].vk), hotkeys[i].desc);
        }

        return S_OK;
    }
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if(((NMHDR*)lParam)->idFrom == 0 && ((NMHDR*)lParam)->code == LVN_ITEMCHANGED)
        {
            LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

            if((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_SELECTED) != (pnmv->uOldState & LVIS_SELECTED))
            {
                if (pnmv->uNewState & LVIS_SELECTED) {

                    lastEditHotkey=hotkeys[pnmv->iItem].vk;
                    lastEditHotkeyModifier=hotkeys[pnmv->iItem].mod;

                    CString hotkey;
                    ListView_GetItemText(listview.m_hWnd, pnmv->iItem, 0, hotkey.GetBufferSetLength(256), 256);
                    edit.SetWindowText(hotkey);
                    edit.EnableWindow(TRUE);
                    change.EnableWindow(TRUE);

                    currentitem=pnmv->iItem;
                    //PRINT("Item number %d has been selected\n", pnmv->iItem);
                }
            }
        }
        return S_OK;
    }
};
