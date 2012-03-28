
struct WindowSource : Source {
    WindowSource() : Source(L"WINDOW",L"Windows (Catalog )") {
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\window.png");
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        std::vector<HWND> windowList;
        EnumWindows(CollectWindows,(LPARAM)&windowList);
        for(std::vector<HWND>::iterator it=windowList.begin(); it!=windowList.end(); it++) {
            if(!::IsWindowVisible(*it))
                continue;
            TCHAR title[MAX_PATH]={0};
            GetWindowText(*it,title,sizeof(title));
            if(FuzzyMatch(title,q)) {
                results.push_back(SourceResult(title,title,title,this,0,*it));
                Object *o=new Object(ItoS((int)*it),type,this,title);
                o->values[L"title"]=title;
                o->values[L"hwnd"]=ItoS((int)*it);
                results.back().object=o;
            }
        }
    }
    static BOOL __stdcall CollectWindows(HWND hwnd, LPARAM lparam) {
        std::vector<HWND> *hwndlist=(std::vector<HWND>*)lparam;
        hwndlist->push_back(hwnd);
        return TRUE;
    }
};
