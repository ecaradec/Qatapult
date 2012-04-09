#include "stdafx.h"
#include "StartMenuSource.h"
#include "pugixml.hpp"

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


int uselev;