#pragma once
struct FileVerbRule : Rule {
    FileVerbRule() {
    }
    bool execute(std::vector<SourceResult> &args) {
        CString path=args[0].source->getString(args[0],L"path");

        // fullpath, director, filename should came from the source
        // but this also means that there will be more requirements for a source
        // to be valid with every rule
        CString fp(path); fp.TrimRight(L"\\");
        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString f=fp.Mid(fp.ReverseFind(L'\\')+1);
        
        if(args[1].object->key == L"Open") {
            ShellExecute(0, 0, path, 0, 0, SW_SHOWDEFAULT); // null is better than open (otherwise it won't work on items that don't have the open verb )
        } else if(args[1].object->key == L"Edit") {
            ShellExecute(0, L"edit", path, 0, 0, SW_SHOWDEFAULT);
        } else if(args[1].object->key == L"RunAs") {
            ShellExecute(0, L"runas", path, 0, 0, SW_SHOWDEFAULT);
        } else if(args[1].object->key == L"Properties") {
            ShellExecute(0, L"properties", path, 0, 0, SW_SHOWDEFAULT);
        } else if(args[1].object->key == L"Delete") {
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
            
            CString command = args[1].source->getString(args[1],L"command");            
            command.Replace(L"%p", fp); // full path
            command.Replace(L"%d", d);  // directory
            command.Replace(L"%f", f);  // filename

            CString workdir= args[1].source->getString(args[1],L"workdir");
            workdir.Replace(L"%p", fp); // full path
            workdir.Replace(L"%d", d);  // directory
            workdir.Replace(L"%f", f);  // filename

            CString cargs= args[1].source->getString(args[1],L"args");
            cargs.Replace(L"%p", fp); // full path
            cargs.Replace(L"%d", d);  // directory
            cargs.Replace(L"%f", f);  // filename

            ShellExecute(0, 0, command, cargs, workdir, SW_SHOWDEFAULT);
        }
        return true;
    }
};