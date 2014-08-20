#include "stdafx.h"
#include "QatapultRunner.h"
#include "Qatapult.h"

void QatapultRunner::Run() {
    INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    Qatapult gui; 
    
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
    }
    
    UnregisterClass(L"GUI",0);
    gui.reset();
}