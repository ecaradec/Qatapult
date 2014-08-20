#include "PluginList.h"
//
//class PluginsDlg : public SimpleOptDialog
//{
//public:
//    enum { IDD = IDD_EMPTY };
// 
//    BEGIN_MSG_MAP(PluginsDlg)
//        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
//        MESSAGE_HANDLER(WM_SAVESETTINGS, OnSaveSettings)
//    END_MSG_MAP()
//
//    std::vector<PluginDesc> m_plugins;
//    HWND                    m_hListView;
//
//    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {         
//        SimpleOptDialog::OnInitDialog(uMsg,wParam,lParam,bHandled);
//        int x=0;
//        int y=0;
//
//        // Create the list-view window in report view with label editing enabled.
//        m_hListView = CreateWindow(WC_LISTVIEW, 
//                                      L"",
//                                      WS_CHILD | LVS_REPORT | WS_VISIBLE | WS_BORDER,
//                                      x, y,
//                                      rcClient.Width(),
//                                      rcClient.Height(),
//                                      m_hWnd,
//                                      0,
//                                      0,
//                                      NULL); 
//
//        ListView_SetExtendedListViewStyle(m_hListView, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
//
//        LVCOLUMN lvc;
//        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
//        lvc.pszText=L"Active";
//        lvc.fmt = LVCFMT_CENTER;
//        lvc.cx=20;
//        ListView_InsertColumn(m_hListView, 0, &lvc);
//
//        lvc.pszText=L"Name";
//        lvc.cx=120;
//        lvc.fmt = LVCFMT_LEFT;
//        ListView_InsertColumn(m_hListView, 1, &lvc);
//
//        lvc.pszText=L"Description";
//        lvc.cx=rcClient.right - rcClient.left - 160;
//        lvc.fmt = LVCFMT_LEFT;
//        ListView_InsertColumn(m_hListView, 2, &lvc);
//
//        int z=0;
//        Qatapult *pQ=(Qatapult*)lParam;
//        int i=0;
//        getPluginList(m_plugins);
//        for(int i=0;i<m_plugins.size();i++) {
//            LVITEM lvi;
//            memset(&lvi, 0, sizeof(lvi));
//            lvi.pszText=(LPWSTR)L"";
//            lvi.mask=LVFIF_TEXT;
//            lvi.iItem=i;
//            int iitem=ListView_InsertItem(m_hListView, &lvi);
//            ListView_SetItemText(m_hListView, iitem, 1, (LPWSTR)m_plugins[i].name.GetString());
//
//            ListView_SetItemText(m_hListView, iitem, 2, (LPWSTR)m_plugins[i].desc.GetString());
//
//            bool bChecked=pQ->isSourceEnabled(UTF16toUTF8(m_plugins[i].key));
//            ListView_SetCheckState(m_hListView, iitem, bChecked);
//        }
//
//        //hcbox=CreateWindow(L"BUTTON", (*it)->m_name, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 0, i, 300, 25, hWnd, 0, 0, 0);
//        //HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
//		//SendMessage(hcbox, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));*/
//        return TRUE;    // let the system set the focus
//    }
//    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {
//        m_plugins.clear();
//        m_hListView=0;
//        return TRUE;
//    }
//    LRESULT OnSaveSettings(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {
//        for(int i=0;i<m_plugins.size();i++) {
//            SetSettingsInt(L"sources/"+m_plugins[i].key, L"enabled", !!ListView_GetCheckState(m_hListView,i));
//        }
//        return TRUE;
//    }
//};
