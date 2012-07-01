   
struct PluginDesc {
    PluginDesc(){}
    PluginDesc(const TCHAR *n): name(n), key(n) {
    }
    CString key;
    CString name;
    CString desc;
};

void GetSubFolderList(const TCHAR *path, std::vector<CString> &subfolders) {
    TCHAR szFullPattern[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    // first we are going to process any subdirectories
    PathCombine(szFullPattern, path, _T("*"));
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && CString(FindFileData.cFileName)!=L"." && CString(FindFileData.cFileName)!=L"..")
                subfolders.push_back(FindFileData.cFileName);
        } while(FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
}

void getSkinList(std::vector<CString> &skins) {
    GetSubFolderList(L"skins",skins);
}

void getPluginList(std::vector<PluginDesc> &plugins) {
    //plugins.push_back(L"Filesystem");         plugins.back().desc=L"Give access to local filesystem";
    //plugins.push_back(L"IndexedFiles");       plugins.back().desc=L"This returns startmenu items and other indexed items from the options dialog";
    //plugins.push_back(L"Websites");           plugins.back().desc=L"";
    //plugins.push_back(L"FileHistory");
    plugins.push_back(L"Network");            plugins.back().desc=L"Add shared drives objects";
    plugins.push_back(L"Contacts");           plugins.back().desc=L"Add your Gmail contacts (requires some configuration in Gmail pane )";
    plugins.push_back(L"ExplorerSelection");  plugins.back().desc=L"Add a currentselection object that's the currently selected item in Windows Explorer";
    plugins.push_back(L"Windows");            plugins.back().desc=L"Add objects for the currently opened window";

    plugins.push_back(L"EmailFile");          plugins.back().desc=L"Allow to send file to some contact";
    plugins.push_back(L"EmailText");          plugins.back().desc=L"Allow to send a text to some contact";
    plugins.push_back(L"WebsiteSearch");      plugins.back().desc=L"Trigger browser searches";

    std::vector<CString> pluginsfolders;
    GetSubFolderList(L"plugins",pluginsfolders);
    for(std::vector<CString>::iterator it=pluginsfolders.begin();it!=pluginsfolders.end();it++) {
        CString key(*it);
        CString pluginxml=L"plugins\\"+*it+"\\plugin.xml";

        pugi::xml_document d;
        if(!d.load_file(pluginxml))
            continue;

        CStringA name=d.select_single_node("settings").node().child_value("name");
        CStringA desc=d.select_single_node("settings").node().child_value("description");        

        PluginDesc pd;
        pd.key=key;
        if(name==L"") {            
            pd.name=key;
            pd.desc=UTF8toUTF16(desc);
        } else {
            pd.name=UTF8toUTF16(name);
            pd.desc=UTF8toUTF16(desc);            
        }
        plugins.push_back(pd);
    }
}