#include "stdafx.h"
#include "simpleini.h"

bool sendEmail(const TCHAR *to, const TCHAR *subject, const TCHAR *body, const TCHAR *attach) {
    CString from=GetSettingsString(L"gmailContacts", L"email");
    CString username=GetSettingsString(L"gmailContacts", L"username");
    CString password=GetSettingsString(L"gmailContacts", L"password");
    CString server=GetSettingsString(L"gmailContacts", L"server");

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

