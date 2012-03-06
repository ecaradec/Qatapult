#pragma once
#include "Source.h"

#include <atlbase.h>
#include <atlcom.h>

#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <exdisp.h>

#pragma comment(lib, "Mpr.lib")

#include <lm.h>
#pragma comment(lib, "Netapi32.lib")

extern TCHAR g_szPath[MAX_PATH];
extern TCHAR g_szItem[MAX_PATH];

extern HWND g_foregroundWnd;

CString getExplorerSelection(HWND hwndFind);

struct CurrentSelectionSource : Source {
    CurrentSelectionSource() : Source(L"FILE", L"Current selection (Catalog )") {
        m_ignoreemptyquery=true;
        m_index[L"CurrentSelection"]=SourceResult(L"CurrentSelection", L"Current Selection", L"Current Selection", this, 0, 0, 0);
    }
    ~CurrentSelectionSource() {
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(FuzzyMatch(it->second.display,q)) {
                results.push_back(it->second);               
                results.back().object=new FileObject(it->second.display,
                                                     this,
                                                     it->second.display,
                                                     it->second.display,
                                                     getExplorerSelection(g_foregroundWnd));
            }
        }
    }
};