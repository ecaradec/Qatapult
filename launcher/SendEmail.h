
bool sendEmail(const TCHAR *to, const TCHAR *subject, const TCHAR *body, const TCHAR *attach) {
    WCHAR curDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, curDir);

    TCHAR from[1024];
    GetPrivateProfileString(L"QSLLContacts", L"email", L"", from, sizeof(from), CString(curDir)+"\\settings.ini");
    TCHAR username[1024];
    GetPrivateProfileString(L"QSLLContacts", L"username", L"", username, sizeof(username), CString(curDir)+"\\settings.ini");
    TCHAR password[1024];
    GetPrivateProfileString(L"QSLLContacts", L"password", L"", password, sizeof(password), CString(curDir)+"\\settings.ini");
    TCHAR server[1024];
    GetPrivateProfileString(L"QSLLContacts", L"server", L"", server, sizeof(server), CString(curDir)+"\\settings.ini");

    TCHAR buff[4096];
    if(attach)
        wsprintf(buff, L"-to %s -f %s -subject \"%s\" -body \"%s\" -attach \"%s\" -u %s -pw %s -debug -log blat.log -server %s", to, from, subject, body, attach, username, password, server);
    else
        wsprintf(buff, L"-to %s -f %s -subject \"%s\" -body \"%s\" -u %s -pw %s -debug -log blat.log -server %s", to, from, subject, body, username, password, server);

    SHELLEXECUTEINFO sei={0};
    sei.cbSize=sizeof(SHELLEXECUTEINFO);
    sei.lpFile=L"blat";
    sei.lpParameters=buff;
    sei.nShow=SW_HIDE;
    sei.fMask=SEE_MASK_NOCLOSEPROCESS;

    ShellExecuteEx(&sei);

    WaitForSingleObject(sei.hProcess, INFINITE);
    DWORD processExitCode=-1;
    GetExitCodeProcess(sei.hProcess, &processExitCode);
    if(processExitCode==1) {
        // retry
        ShellExecuteEx(&sei);

        WaitForSingleObject(sei.hProcess, INFINITE);
        processExitCode=-1;
        GetExitCodeProcess(sei.hProcess, &processExitCode);
        if(processExitCode==1) {
            MessageBox(0, L"Can't send email ! Check your SMTP configuration.", L"QSLL", MB_OK);
        }
    }
    return processExitCode==0;
}

