#include "stdafx.h"
#include "StartMenuSource.h"
#include "pugixml.hpp"
#include "resource.h"

 CString GetSpecialFolder(int csidl) {
     CString tmp;
     SHGetFolderPath(0, csidl, 0, SHGFP_TYPE_CURRENT, tmp.GetBufferSetLength(MAX_PATH)); tmp.ReleaseBuffer();
     return tmp;
 }
 

int SaveSearchFolders(HWND hListView) {
    WCHAR curDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, curDir);

    int itemcount=ListView_GetItemCount(hListView);
    pugi::xml_node settingsnode=settings.select_single_node("settings").node();
    settingsnode.remove_child("searchFolders");
    pugi::xml_node searchfolders=settingsnode.append_child("searchFolders");
    for(int i=0;i<itemcount;i++) {
        TCHAR path[MAX_PATH]={0};
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=path;
        lvi.cchTextMax=sizeof(path);
        lvi.iItem=i;
        lvi.mask=LVFIF_TEXT;
        ListView_GetItem(hListView, &lvi);
        if(CString(path)!=L"")
            searchfolders.append_child("folder").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(path));
    }
    settings.save_file("settings.xml");
    return itemcount;
}

BOOL CALLBACK SearchFolderDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int res;
    HWND hListView=GetDlgItem(hWnd, IDC_LIST3);
    CRect r;
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            // allows for full row selection
            ListView_SetExtendedListViewStyle(hListView, ListView_GetExtendedListViewStyle(hListView)| LVS_EX_FULLROWSELECT);

            GetClientRect(hListView, &r);

            LVCOLUMN lvc;
            lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
            lvc.pszText=L"Path";
            lvc.cx=r.Width()-60;
            ListView_InsertColumn(hListView, 0, &lvc);

            lvc.pszText=L"Depth";
            lvc.cx=60;
            lvc.fmt = LVCFMT_CENTER;
            ListView_InsertColumn(hListView, 1, &lvc);

            //lvc.pszText=L"Bonus";
            //lvc.cx=40;
            //ListView_InsertColumn(GetDlgItem(hWnd, IDC_LIST3), 0, &lvc);
            
            WCHAR curDir[MAX_PATH];
            GetCurrentDirectory(MAX_PATH, curDir);

            int i=0;

            pugi::xpath_node_set ns=settings.select_nodes("settings/searchFolders/folder");
            for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++,i++) {
                CStringW path=UTF8toUTF16(it->node().child_value());
                if(path==L"")
                    break;
                LVITEM lvi;
                memset(&lvi, 0, sizeof(lvi));
                lvi.pszText=(LPWSTR)path.GetString();
                lvi.mask=LVFIF_TEXT;
                lvi.iItem=i;
                int iitem=ListView_InsertItem(hListView, &lvi);
                ListView_SetItemText(hListView, iitem, 1, L"3");
            }

            LVITEM lvi;
            memset(&lvi, 0, sizeof(lvi));
            lvi.pszText=L"";
            lvi.mask=LVFIF_TEXT;
            lvi.iItem=i;
            res=ListView_InsertItem(hListView, &lvi);
        }
        return TRUE;
        case WM_COMMAND:
            if(wParam==10000) {
                int iitem=ListView_GetSelectionMark(hListView);
                ListView_DeleteItem(hListView, iitem);

                SaveSearchFolders(hListView);
            }
        return TRUE;        
        case WM_NOTIFY:
            if(((NMHDR*)lParam)->code==LVN_ENDLABELEDIT) {

                NMLVDISPINFO *nmdi=(NMLVDISPINFO*)lParam;                
                ListView_SetItem(hListView, &nmdi->item);

                int itemcount=SaveSearchFolders(hListView);

                // if the edited item was the last, add an empty one
                if(nmdi->item.iItem==(itemcount-1)) {
                    LVITEM lvi;
                    memset(&lvi, 0, sizeof(lvi));
                    lvi.pszText=L"";
                    lvi.mask=LVFIF_TEXT;
                    lvi.iItem=itemcount;
                    res=ListView_InsertItem(hListView, &lvi);
                }
            } else if(((NMHDR*)lParam)->code==NM_RCLICK) {
                HMENU hmenu=CreatePopupMenu();
                AppendMenu(hmenu, MF_STRING, 10000, L"Delete");
                DWORD pos=GetMessagePos();
                TrackPopupMenu(hmenu, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, hWnd, 0);
            }
            return TRUE;
    }
    return FALSE;
}

int uselev;