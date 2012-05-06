
// types de command
// file,copyto,file => copy $p0.path $p2.path
// text,clip => clip $p0.text
// parseur : chercher les *, renvoyer la chaine, 
struct ShellExecuteRule : Rule {    
    ShellExecuteRule(const CString &cmd,const CString &args, const CString &workdir):m_command(cmd),m_args(args),m_workdir(workdir) {
        m_command.Trim();
    } // the type must be precised later
    bool execute(std::vector<RuleArg> &args) {
        CString cmd=expand(m_command,args);
        CString arg=expand(m_args,args);
        CString workdir=expand(m_workdir,args);

        TCHAR pf[MAX_PATH];
        SHGetSpecialFolderPath(0, pf, CSIDL_PROGRAM_FILES, FALSE);

        OutputDebugString(Format(L"shellexecute '%s' '%s' '%s'\n",cmd,arg,workdir));
        ShellExecute(0, 0, cmd, arg, workdir, SW_SHOWDEFAULT);
        return true;
    }
    CString expand(const CString &str, std::vector<RuleArg> &args) {
        CString tmp;
        for(int i=0;i<str.GetLength();i++) {
            TCHAR c=str[i];
            if(c=='$') {
                i++;
                CString id;
                CString arg;
                while(str[i]>='0' && str[i]<='9') {
                    id+=str[i];
                    i++;
                }
                i++;
                while(str[i]>='a' && str[i]<='z') {
                    arg+=str[i];
                    i++;
                }
                                
                CString val=args[_ttoi(id)].source()->getString(args[_ttoi(id)].item(0),arg);
                tmp+=val;
                tmp+=str[i];
            } else {
                tmp+=c;
            }
        }
        return tmp;
    }
    CString m_command;
    CString m_args;
    CString m_workdir;
};