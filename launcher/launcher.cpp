
// launcher.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Qatapult.h"
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
    CString cmdline(lpCmdLine);
    int index=cmdline.Find(L"/run=");
    if(index!=-1) {
        HWND hwnd=FindWindow(L"STATIC", L"Qatapult");
        //PostMessage(hwnd, WM_USER+100, 0, 0);

        CString strDataToSend=cmdline.Mid(index+5);
        COPYDATASTRUCT cpd;
		cpd.dwData = 0;
		cpd.cbData = strDataToSend.GetLength()*sizeof(WCHAR);
		cpd.lpData = (void*)strDataToSend.GetBuffer(cpd.cbData);
		LPARAM copyDataResult = SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)&cpd);
		strDataToSend.ReleaseBuffer();
        return 0;
    }

    OleInitialize(0);
    //CoInitialize(0);

    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Qatapult gui; 

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        int j=0;
        if(msg.message == WM_KEYDOWN ) {
            j=0;
            //CString tmp; tmp.Format(L"%x \n", msg.hwnd);
            //OutputDebugString(tmp);
        }

        
        LRESULT lres=DispatchMessage(&msg);        
        if(msg.message == WM_KEYDOWN && lres!=FALSE ) {
            BOOL b=TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        //BOOL b=TranslateMessage(&msg);
        //DispatchMessage(&msg);
    }
    
    UnregisterClass(L"GUI",0);
    gui.reset();
    Gdiplus::GdiplusShutdown(gdiplusToken);
    
    
    OleUninitialize();
    //CoUninitialize();
    return 0;
}
