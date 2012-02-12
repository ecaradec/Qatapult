
struct FileVerbRule : Rule {
    FileVerbRule() {
    }
    bool execute(std::vector<SourceResult> &args) {
        CString path=(*m_pArgs)[0].source->getString(args[0],L"path");

        // fullpath, director, filename should came from the source
        // but this also means that there will be more requirements for a source
        // to be valid with every rule
        CString fp(path); fp.TrimRight(L"\\");
        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString f=fp.Mid(fp.ReverseFind(L'\\')+1);
        
        if((*m_pArgs)[1].key == L"Open") {
            ShellExecute(0, 0, path, 0, 0, SW_SHOWDEFAULT); // null is better than open (otherwise it won't work on items that don't have the open verb )
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
        } else {
            if((GetFileAttributes(fp)&FILE_ATTRIBUTE_DIRECTORY)!=0) {
                d=fp;
            }
            
            CString command = (*m_pArgs)[1].source->getString((*m_pArgs)[1],L"command");            
            command.Replace(L"%p", fp); // full path
            command.Replace(L"%d", d);  // directory
            command.Replace(L"%f", f);  // filename

            CString workdir= (*m_pArgs)[1].source->getString((*m_pArgs)[1],L"workdir");
            workdir.Replace(L"%p", fp); // full path
            workdir.Replace(L"%d", d);  // directory
            workdir.Replace(L"%f", f);  // filename

            CString args= (*m_pArgs)[1].source->getString((*m_pArgs)[1],L"args");
            args.Replace(L"%p", fp); // full path
            args.Replace(L"%d", d);  // directory
            args.Replace(L"%f", f);  // filename

            ShellExecute(0, 0, command, args, workdir, SW_SHOWDEFAULT);
        }
        return true;
    }
};