
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
                pack.begin(KV_Map);
                    pack.writePairString(L"type",L"WINDOW");
                    pack.writePairUint32(L"source",(uint32)this);
                    pack.writePairString(L"key",title);
                    pack.writePairString(L"text",title);
                    pack.writePairString(L"status",title);
                    pack.writePairString(L"expand",title);
                    pack.writePairUint32(L"hwnd",(uint32)*it);
                    pack.writePairUint32(L"bonus",(uint32)0);
                    pack.writePairUint32(L"uses",(uint32)0);
                pack.end();  
            }
        }
    }
    static BOOL __stdcall collectWindows(HWND hwnd, LPARAM lparam) {
        std::vector<HWND> *hwndlist=(std::vector<HWND>*)lparam;
        hwndlist->push_back(hwnd);
        return TRUE;
    }
};
