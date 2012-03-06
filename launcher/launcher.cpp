
// launcher.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AlphaGUI.h"
#include <psapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString GetProcessName( DWORD processID ) {
    TCHAR szProcessName[MAX_PATH] = L"<unknown>";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE, processID);

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) ) {
            GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }
    CloseHandle( hProcess );
    return szProcessName;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{    
    /*TCHAR path[MAX_PATH];
    GetModuleFileName(0, path, sizeof(path));

    DWORD aProcesses[1024], cbNeeded=0, cProcesses;
    unsigned int i;
    EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded );
    cProcesses = cbNeeded / sizeof(DWORD);
    for ( i = 0; i < cProcesses; i++ )
        if( aProcesses[i] != 0 ) {
            if(GetProcessName( aProcesses[i] )==path) {
                MessageBox(MB_OK, L"Qatapult is already running",L"Qatapult",MB_OK);
                return 0;
            }
        }*/

    CoInitialize(0);

    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    AlphaGUI gui; 

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UnregisterClass(L"GUI",0);
    gui.Reset();
    Gdiplus::GdiplusShutdown(gdiplusToken);
    CoUninitialize();
    return 0;
}
