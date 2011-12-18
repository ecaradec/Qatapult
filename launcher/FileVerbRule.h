
struct FileVerbRule : Rule {
    FileVerbRule() : Rule(L"FILE",L"FILEVERB") {}
    bool execute(std::vector<SourceResult> &args) {
        CString path=(*m_pArgs)[0].source->getString(args[0].key+L"/path");

        CString fp(path); fp.TrimRight(L"\\");
        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString f=fp.Mid(fp.ReverseFind(L'\\')+1);

        if((*m_pArgs)[1].key == L"Open") {
            ShellExecute(0, L"open", path, 0, 0, SW_SHOWDEFAULT);
        } else if((*m_pArgs)[1].key == L"Edit") {
            ShellExecute(0, L"edit", path, 0, 0, SW_SHOWDEFAULT);
        } else if((*m_pArgs)[1].key == L"RunAs") {
            ShellExecute(0, L"runas", path, 0, 0, SW_SHOWDEFAULT);
        } else if((*m_pArgs)[1].key == L"Properties") {
            ShellExecute(0, L"properties", path, 0, 0, SW_SHOWDEFAULT);
        } else if((*m_pArgs)[1].key == L"Delete") {
            SHFILEOPSTRUCT sffo={0};
            int l=path.GetLength();
            TCHAR *str=path.GetBufferSetLength(l+1);
            str[l+1]=0; // add extra 0
            sffo.wFunc=FO_DELETE;
            sffo.pFrom=str;
            sffo.fFlags=FOF_ALLOWUNDO;            
            SHFileOperation(&sffo);
        }
        return true;
    }
};