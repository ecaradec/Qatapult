
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
    // send any command line to already running instance, or just fail
    CString cmdline(lpCmdLine);
    if(cmdline.GetLength()!=0) {
        HWND hwnd=FindWindow(L"STATIC", L"Qatapult");
        CString strDataToSend=cmdline;
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

    INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Qatapult gui; 

    //CMessageLoop ml;
    //ml.Run();

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        //if(IsDialogMessage(msg.hwnd,&msg))
        //    continue;

        int j=0;
        if(msg.message == WM_KEYDOWN ) {
            j=0;
            //CString tmp; tmp.Format(L"%x \n", msg.hwnd);
            //OutputDebugString(tmp);
        }

        if(gui.isAccelerator(msg.hwnd, msg.message,msg.wParam,msg.lParam)) {
            BOOL b=TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        /*LRESULT lres=DispatchMessage(&msg);        
        if(msg.message == WM_KEYDOWN && lres!=FALSE ) {
            BOOL b=TranslateMessage(&msg);
            DispatchMessage(&msg);
        }*/
    }
    
    UnregisterClass(L"GUI",0);
    gui.reset();
    Gdiplus::GdiplusShutdown(gdiplusToken);
    
    
    OleUninitialize();
    //CoUninitialize();
    return 0;
}
