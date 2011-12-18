#pragma once
#include "Source.h"
#include "getItemVerbs.h"

struct Info {
    Source                    *source;
    std::vector<SourceResult> *results;
};

struct FileVerbSource : Source {
    FileVerbSource() : Source(L"FILEVERB") {
        m_index[L"Open"]=SourceResult(L"Open", L"Open", L"Open", this, 0, 0, 10);
        m_index[L"RunAs"]=SourceResult(L"RunAs", L"RunAs", L"RunAs", this, 0, 0, 9);
        m_index[L"Delete"]=SourceResult(L"Delete", L"Delete", L"Delete", this, 0, 0, 8);
        m_index[L"Properties"]=SourceResult(L"Properties", L"Properties", L"Properties", this, 0, 0, 7);
    }
    ~FileVerbSource() {
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(CString(it->second.display).MakeUpper().Find(q)!=-1) {
                results.push_back(it->second);
            }
        }
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        Gdiplus::Bitmap *bmp=Gdiplus::Bitmap::FromFile(L"icons\\"+r->key+L".png");
        if(bmp->GetLastStatus()!=Gdiplus::Ok) {
            delete bmp;
            bmp=Gdiplus::Bitmap::FromFile(L"icons\\defaultverb.png");
        }
        return bmp;
    }
};
