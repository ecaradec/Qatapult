#pragma once
#include "Source.h"
#include "getItemVerbs.h"

struct FileVerbSource : Source {
    FileVerbSource() : Source(L"FILEVERB", L"File verbs (Catalog )") {
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
            if(FuzzyMatch(it->second.display,q)) {
                results.push_back(it->second);
                results.back().object=new Object(it->first,this,it->second.display);
            }
        }
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        Gdiplus::Bitmap *bmp=Gdiplus::Bitmap::FromFile(L"icons\\"+r->object->key+L".png");
        if(bmp->GetLastStatus()!=Gdiplus::Ok) {
            delete bmp;
            bmp=Gdiplus::Bitmap::FromFile(L"icons\\defaultverb.png");
        }
        return bmp;
    }
    struct Extra {
        CString command;
        CString args;
        CString workdir;
    };
    std::map<CString, Extra> m_extras;
};
