#pragma once
#include "FindFileRecursively.h"
#include "sqlite3/sqlite3.h"

// might be useful
// http://stackoverflow.com/questions/1744902/how-might-i-obtain-the-icontextmenu-that-is-displayed-in-an-ishellview-context-m
struct StartMenuSource : Source {
    StartMenuSource() : Source(L"FILE", L"STARTMENU") {
        std::vector<CString> lnks;
        FindFilesRecursively(L"C:\\ProgramData\\Microsoft\\Windows\\Start Menu", L"*.lnk", lnks);
        for(uint i=0;i<lnks.size();i++) {
            CString str(lnks[i]);
            PathRemoveExtension(str.GetBuffer()); str.ReleaseBuffer();
            str=PathFindFileName(str.GetBuffer()); str.ReleaseBuffer();
            m_index[lnks[i]] = SourceResult(lnks[i], str, lnks[i], this, i, 0);
        }

        load();
        /*for(int i=0;i<m_index.size(); i++) {
            WritePrivateProfileString("startmenu", "1_name", r->display);
            WritePrivateProfileString("startmenu", "1_name", r->display);
            WritePrivateProfileString("startmenu", "1_name", r->display);
            WritePrivateProfileString("startmenu", "1_name", r->display);
            WritePrivateProfileString("startmenu", "1_name", r->display);
        }*/

        // dumb code to scan the control panel, leaks included
        // ugly but works
        // I have an issue considering how to store various kinds of data for reusing later
        // here for icons it's ok if I store a itemidlist but how to persist it ??? and such others
        // issues
        // saving, loading, deleting, getting icons has to pass through the source. Should I get
        // args from the source as well ???? its possible via a getarg(id, value*) ???
        // where value is anything
        /*CComPtr<IShellFolder> pSF;
        SHGetDesktopFolder(&pSF);
        ULONG eaten=0;
        LPITEMIDLIST pidl=0;
        DWORD attr=0;
        pSF->ParseDisplayName(0, 0, L"::{26EE0668-A00A-44D7-9371-BEB064C98683}", &eaten, &pidl, &attr);
        CComPtr<IShellFolder> pCPSF;
        //pSF->GetUIObjectOf(0, 1, (LPCITEMIDLIST*)&pidl, IID_IShellFolder, 0, (void**)&pCPSF);
        pSF->BindToObject(pidl, 0, IID_IShellFolder, (void**)&pCPSF);

        CComPtr<IEnumIDList> pEIDL;
        pCPSF->EnumObjects(0, SHCONTF_FOLDERS|SHCONTF_NONFOLDERS, &pEIDL);

        LPITEMIDLIST pidl2;
        ULONG fetched=0;
        STRRET name;
        while(pEIDL->Next(1, &pidl2, &fetched)==S_OK) {         
            // TODO leak
            // TODO make absolute pidl 
            pCPSF->GetDisplayNameOf(pidl2, SHGDN_NORMAL, &name);
            
            CComPtr<IPersistFolder2> pSF2;
            pCPSF->BindToObject(pidl2, 0, IID_IPersistFolder2, (void**)&pSF2);
            
            LPITEMIDLIST pidlCF=0;
            pSF2->GetCurFolder(&pidlCF);

            m_results.push_back(SourceResult(CStringW(name.pOleStr), L"", this, 0, 0));
            m_results.back().icon=::getIcon(pidlCF);
        }*/
    }
    // getvalue name, buff, bufflen
    Gdiplus::Bitmap *getIcon(SourceResult *r) {
        return ::getIcon(r->expandStr);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int flags) {
        if(_tcslen(query)==0)
            return;
        Source::collect(query,args,results,flags);
    }
    ClauncherDlg                    *m_pUI;
};
