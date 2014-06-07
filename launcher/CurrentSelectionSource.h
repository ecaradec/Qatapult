#pragma once
#include "Source.h"

#include <atlbase.h>
#include <atlcom.h>

#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <exdisp.h>
#include "KVPack.h"

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
    }
    ~CurrentSelectionSource() {
    }
    virtual void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"FILE")==activetypes.end())
            return;

        CString q(query); q.MakeUpper();

        if(FuzzyMatch(L"Current Selection",q)) { 
            CString path=getExplorerSelection(g_foregroundWnd);

            pack.begin(KV_Map);
                pack.writePairString(L"type",L"FILE");
                pack.writePairUint32(L"source",(uint32)this);
                pack.writePairString(L"key",L"Current Selection");
                pack.writePairString(L"path",path);
                pack.writePairString(L"expand",L"Current Selection");
                pack.writePairString(L"filename",path.Mid(path.ReverseFind(L'\\')+1));
                pack.writePairString(L"text",L"Current Selection");
                pack.writePairUint32(L"bonus",(uint32)0);
                pack.writePairUint32(L"uses",(uint32)0);
            pack.end(); 
        }
    }
};