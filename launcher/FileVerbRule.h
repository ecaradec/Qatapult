#pragma once

struct OpenFileRule : Rule {
    bool execute(std::vector<RuleArg> &args) {       
        for(int r=0;r<args[0].m_results.size();r++) {
            CString path=args[0].object(r)->getString(L"path");
            ShellExecute(0, 0, path, 0, 0, SW_SHOWDEFAULT); // null is better than open (otherwise it won't work on items that don't have the open verb )      
        }
        return true;
    }
};

struct RunAsFileRule : Rule {
    bool execute(std::vector<RuleArg> &args) {                    
        for(int r=0;r<args[0].m_results.size();r++) {
            CString path=args[0].object(r)->getString(L"path");
            ShellExecute(0, L"runas", path, 0, 0, SW_SHOWDEFAULT);
        }
        return true;
    }
};

struct PropertiesFileRule : Rule {
    bool execute(std::vector<RuleArg> &args) {                    
        for(int r=0;r<args[0].m_results.size();r++) {
            CString path=args[0].object(r)->getString(L"path");
            ShellExecute(0, L"properties", path, 0, 0, SW_SHOWDEFAULT);
        }
        return true;
    }
};

struct EditFileRule : Rule {
    bool execute(std::vector<RuleArg> &args) {                    
        for(int r=0;r<args[0].m_results.size();r++) {
            CString path=args[0].object(r)->getString(L"path");
            ShellExecute(0, L"edit", path, 0, 0, SW_SHOWDEFAULT);
        }
        return true;
    }
};

struct DeleteFileRule : Rule {
    bool execute(std::vector<RuleArg> &args) {
        for(int r=0;r<args[0].m_results.size();r++) {
            CString path=args[0].object(r)->getString(L"path");

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