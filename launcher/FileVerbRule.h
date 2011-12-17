
struct FileVerbRule : Rule {
    FileVerbRule() : Rule(L"FILE",L"FILEVERB") {}
    bool execute(std::vector<SourceResult> &args) {
        CString path=(*m_pArgs)[0].source->getString(args[0].key+L"/path");

        CString fp(path); fp.TrimRight(L"\\");
        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString f=fp.Mid(fp.ReverseFind(L'\\')+1);

        CString verb=args[1].key;
        CComPtr<IContextMenu> pCM;
        getContextMenu(d, f, &pCM);

        HMENU hmenu=CreatePopupMenu();
        pCM->QueryContextMenu(hmenu, 0, 0, 0xFFFF, CMF_DEFAULTONLY);
        
        ProcessCMCommand(pCM, args[1].id);
        return true;
    }
    HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, int verb) {
       CMINVOKECOMMANDINFO ici;
       ZeroMemory(&ici, sizeof(ici));
       ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
       ici.lpVerb = (LPCSTR)verb;
       ici.nShow = SW_SHOWNORMAL;

       return pCM->InvokeCommand(&ici);
    }
};