#include "stdafx.h"
#include "getItemVerbs.h"

bool getContextMenu(const CString &d, const CString &f, IContextMenu **ppCM) {
   CComPtr<IShellFolder> psfDesktop, psfWinFiles;
    PIDLIST_RELATIVE pidlFolder, pidlItem;
    DWORD eaten;
        
    HRESULT hr;
    if(FAILED(hr = SHGetDesktopFolder(&psfDesktop)))
        return false;

    if(FAILED(hr=psfDesktop->ParseDisplayName(0, 0, (LPOLESTR)CStringW(d).GetBuffer(), 0, &pidlFolder, NULL)))
        return false;

    if(FAILED(hr = psfDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder, (LPVOID *) &psfWinFiles)))
        return false;
        
    // sometimes the given filename is not parsable under the given IShellFolder but is parsable under the desktop folder
    if(SUCCEEDED(hr = psfWinFiles->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(f).GetBuffer(), &eaten, &pidlItem, 0)))
    {
        if(FAILED(hr=psfWinFiles->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)ppCM)))
            return false;
    }
    else if(SUCCEEDED(hr= psfDesktop->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(f).GetBuffer(), &eaten, &pidlItem, 0)))
    {
        if(FAILED(hr=psfDesktop->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)ppCM)))
            return false;
    }
    return true;
}

HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, UINT idCmdOffset) {
    CMINVOKECOMMANDINFO ici;
    ZeroMemory(&ici, sizeof(ici));
    ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
    ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCmdOffset);
    ici.nShow = SW_SHOWNORMAL;

    return pCM->InvokeCommand(&ici);
}

void getItemVerbs(const CString &d, const CString &f, std::vector<Command> &commands) {
    CComPtr<IContextMenu> pCM;
    getContextMenu(d,f,&pCM);
    if(pCM) {
        HMENU hmenu=CreatePopupMenu();
        pCM->QueryContextMenu(hmenu, 0, 0, 0xFFFF, CMF_EXPLORE);
        int i=0;
        CString s;
        for(int i=0;i<::GetMenuItemCount(hmenu);i++) {
            
            MENUITEMINFO mii;
            memset(&mii, 0, sizeof(mii));
            mii.cbSize=sizeof(MENUITEMINFO);
            WCHAR buff[1024]={0};
            mii.dwTypeData=(LPWSTR)&buff;
            mii.cch=sizeof(buff);
            mii.fMask=MIIM_STRING|MIIM_SUBMENU|MIIM_ID;
            bool b=!!::GetMenuItemInfo(hmenu, i, TRUE, &mii);

            int c=::GetMenuString(hmenu, i, s.GetBufferSetLength(256), 256, MF_BYPOSITION); s.ReleaseBuffer();
            s.Replace(L"&&",L"__EAMP__");
            s.Replace(L"&",L"");
            s.Replace(L"__EAMP__",L"&");

            if(mii.hSubMenu)
                continue;
            
            if(s.GetLength()>0) {
                //WCHAR buff[256]={0};
                pCM->GetCommandString(mii.wID, GCS_VERBW, 0, (LPSTR)buff, sizeof(buff));
                
                if(wcscmp(buff, L"cut")==0 ||
                   wcscmp(buff, L"copy")==0 ||
                   wcscmp(buff, L"paste")==0)
                   continue;

                Command cmd;
                cmd.display=s;
                cmd.id=mii.wID;
                cmd.verb=buff;
                commands.push_back(cmd);
            }
        }
        DestroyMenu(hmenu);
        //m.TrackPopupMenu(0, 0, 0, this, 0);
    }
}
