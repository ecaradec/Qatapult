
struct WindowSource : Source {
    WindowSource() : Source(L"WINDOW",L"Windows (Catalog )") {
        m_icon=L"icons\\window.png";
    }
    virtual void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"WINDOW")==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        std::vector<HWND> windowList;
        EnumWindows(collectWindows,(LPARAM)&windowList);
        for(std::vector<HWND>::iterator it=windowList.begin(); it!=windowList.end(); it++) {
            if(!::IsWindowVisible(*it))
                continue;
            TCHAR title[MAX_PATH]={0};
            GetWindowText(*it,title,sizeof(title));
            if(FuzzyMatch(title,q)) {                
                uint8 *pobj=pack.beginBlock();
                pack.pack(L"type",L"WINDOW");
                pack.pack(L"source",(uint32)this);
                pack.pack(L"key",title);
                pack.pack(L"text",title);
                pack.pack(L"status",title);
                pack.pack(L"expand",title);
                pack.pack(L"hwnd",(uint32)*it);
                pack.pack(L"bonus",(uint32)0);
                pack.pack(L"uses",(uint32)0);
                pack.endBlock(pobj);     
            }
        }
    }
    static BOOL __stdcall collectWindows(HWND hwnd, LPARAM lparam) {
        std::vector<HWND> *hwndlist=(std::vector<HWND>*)lparam;
        hwndlist->push_back(hwnd);
        return TRUE;
    }
};
