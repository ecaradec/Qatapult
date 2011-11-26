
// launcher.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "launcher.h"
#include "launcherDlg.h"
#include <gdiplus.h>
#include "AlphaGUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ClauncherApp

BEGIN_MESSAGE_MAP(ClauncherApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// ClauncherApp construction

ClauncherApp::ClauncherApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only ClauncherApp object

ClauncherApp theApp;


// ClauncherApp initialization

BOOL ClauncherApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    AlphaGUI gui;

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        bool translate=true;
        if (msg.hwnd == gui.m_dlg.GetSafeHwnd() ||::IsChild(gui.m_dlg.GetSafeHwnd(), msg.hwnd)) {
            if(msg.message==WM_KEYDOWN && (msg.wParam==VK_LEFT || msg.wParam==VK_RIGHT)) {
                translate=false;
            }

            gui.OnKeyboardMessage(msg.message, msg.wParam, msg.lParam);
            if(!translate)
                continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    
	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
    
    UnregisterClass(L"GUI",0);
    Gdiplus::GdiplusShutdown(gdiplusToken);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

