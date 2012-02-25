#pragma once
inline CString getShortcutPath(const CString &lnk) {

    HRESULT hres;
    CComPtr<IShellLink> pSL;
    WIN32_FIND_DATA wfd;

    pSL.CoCreateInstance(CLSID_ShellLink);
    
    CComQIPtr<IPersistFile> pPF(pSL);
    pPF->Load(lnk,STGM_READ);

    PIDLIST_ABSOLUTE pidl=0;
    pSL->GetIDList(&pidl);

    CComPtr<IShellFolder> pSF;
    SHGetDesktopFolder(&pSF);

    STRRET strret;
    pSF->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strret);
    CString tmp(strret.pOleStr);
    CoTaskMemFree(strret.pOleStr);

    // ignore links to namespaced objects
    if(tmp.Left(2)==L"::")
        return lnk;
    return tmp;
}