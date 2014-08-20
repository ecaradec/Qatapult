#pragma once
inline CString getShortcutPath(const CString &lnk) {

    CComPtr<IShellLink> pSL;

    HRESULT hr=pSL.CoCreateInstance(CLSID_ShellLink);
    
    CComQIPtr<IPersistFile> pPF(pSL);
    hr=pPF->Load(lnk,STGM_READ);

    TCHAR path[MAX_PATH];
    int l=sizeof(path);
    hr=pSL->GetPath(path,l,0,SLGP_RAWPATH);
    CString tmp=path;

    if(hr!=S_OK) {
        PIDLIST_ABSOLUTE pidl=0;
        hr=pSL->GetIDList(&pidl);

        CComPtr<IShellFolder> pSF;
        SHGetDesktopFolder(&pSF);

        STRRET strret;
        hr=pSF->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strret);
        tmp=(strret.pOleStr);
        CoTaskMemFree(strret.pOleStr);
    }
    
    // ignore links to namespaced objects
    if(tmp.Left(2)==L"::")
        return lnk;
    return tmp;
}