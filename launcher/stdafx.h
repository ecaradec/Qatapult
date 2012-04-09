
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
/*
#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif
*/

#define GDIPVER 0x0110

#include "targetver.h"

// #define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
// #define _AFX_ALL_WARNINGS

#include <windows.h>         // MFC core and standard components

#include <commctrl.h>
#include <commoncontrols.h>
#include <atlstr.h>
#include <shlobj.h>
#include <atlimage.h>

#include <io.h>

#include <Winhttp.h>
#include <Shlwapi.h>
#include <windowsx.h>
#include <dispex.h>

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <map>
#include <vector>
#include <list>
#include <algorithm>

#include <gdiplus.h>

using namespace Gdiplus;

#include "sqlite3/sqlite3.h"

typedef unsigned int uint;

#pragma comment(lib,"Comctl32.lib")


#define     SubclassWindowX(hwnd, lpfn)       \
              ((WNDPROC)SetWindowLongPtr((hwnd), GWLP_WNDPROC, (LPARAM)(WNDPROC)(lpfn)))

#include <atlapp.h>
#include <atlwinx.h>
#include <atldlgs.h>