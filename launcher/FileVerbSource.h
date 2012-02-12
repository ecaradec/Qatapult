#pragma once
#include "Source.h"
#include "getItemVerbs.h"

struct FileVerbSource : Source {
    FileVerbSource() : Source(L"FILEVERB") {
        int itemcount=GetSettingsInt(L"FileVerbs", L"count", 0);

        int b=itemcount+10;
        m_index[L"Open"]=SourceResult(L"Open", L"Open", L"Open", this, 0, 0, b--);
        m_index[L"Edit"]=SourceResult(L"Edit", L"Edit", L"Edit", this, 0, 0, b--);
        m_index[L"RunAs"]=SourceResult(L"RunAs", L"RunAs", L"RunAs", this, 0, 0, b--);
        m_index[L"Delete"]=SourceResult(L"Delete", L"Delete", L"Delete", this, 0, 0, b--);
        m_index[L"Properties"]=SourceResult(L"Properties", L"Properties", L"Properties", this, 0, 0, b--);
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
    CString getString(SourceResult &sr,const TCHAR *val_) {
        return sr.key;
    }
    struct Extra {
        CString command;
        CString args;
        CString workdir;
    };
    std::map<CString, Extra> m_extras;
};
