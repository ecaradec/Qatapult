#include "Source.h"

#include <atlbase.h>
#include <atlcom.h>

#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <exdisp.h>

#pragma comment(lib, "Mpr.lib")

#include <lm.h>
#pragma comment(lib, "Netapi32.lib")

TCHAR g_szPath[MAX_PATH];
TCHAR g_szItem[MAX_PATH];

extern HWND g_foregroundWnd;

CString getExplorerSelection(HWND hwndFind)
{
	g_szPath[0] = TEXT('\0');
	g_szItem[0] = TEXT('\0');
    CString currentSelection;

	IShellWindows *psw;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL,IID_IShellWindows, (void**)&psw))) {
			VARIANT v;
			V_VT(&v) = VT_I4;
			IDispatch  *pdisp;
			BOOL fFound = FALSE;
			for (V_I4(&v) = 0; !fFound && psw->Item(v, &pdisp) == S_OK; V_I4(&v)++) {
					IWebBrowserApp *pwba;
					if (SUCCEEDED(pdisp->QueryInterface(IID_IWebBrowserApp, (void**)&pwba))) {
						HWND hwndWBA;
						if (SUCCEEDED(pwba->get_HWND((LONG_PTR*)&hwndWBA)) && hwndWBA == hwndFind
) {
								fFound = TRUE;
								IServiceProvider *psp;
								if (SUCCEEDED(pwba->QueryInterface(IID_IServiceProvider, (void**)&psp))) {
									IShellBrowser *psb;
									if (SUCCEEDED(psp->QueryService(SID_STopLevelBrowser,IID_IShellBrowser, (void**)&psb))) {
											IShellView *psv;
											if (SUCCEEDED(psb->QueryActiveShellView(&psv))) {
												IFolderView *pfv;
												if (SUCCEEDED(psv->QueryInterface(IID_IFolderView,(void**)&pfv))) {
														IPersistFolder2 *ppf2;
														if (SUCCEEDED(pfv->GetFolder(IID_IPersistFolder2,(void**)&ppf2))) {
																LPITEMIDLIST pidlFolder;
																if (SUCCEEDED(ppf2->GetCurFolder(&pidlFolder))) {


																	if (!SHGetPathFromIDList(pidlFolder, g_szPath)) {
																		lstrcpyn(g_szPath, TEXT("<not a directory>"), MAX_PATH);
																	}

																	CComPtr<IDataObject> pdo;
																	psv->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**)&pdo);

																	CComPtr<IEnumIDList> pEIDL;
																	if(SUCCEEDED(pfv->Items(SVGIO_SELECTION, IID_IEnumIDList, (void**)&pEIDL))) {

																		LPITEMIDLIST pidlItem;
																		ULONG l=1;
																		pEIDL->Reset();
																		while(1) {
																			if(FAILED(pEIDL->Next(1, &pidlItem, &l)))
																				break;
																			if(l==0)
																				break;


																			IShellFolder *psf;
																			if (SUCCEEDED(ppf2->QueryInterface(IID_IShellFolder,(void**)&psf))) {
																				STRRET str;
																				if (SUCCEEDED(psf->GetDisplayNameOf(pidlItem, SHGDN_FORPARSING,&str))) {
																					StrRetToBuf(&str, pidlItem, g_szItem, MAX_PATH);
                                                                                    currentSelection=g_szItem;

																					//OutputDebugString(g_szItem);
																					//OutputDebugString("\n");
																				}
																				psf->Release();
																			}
																			CoTaskMemFree(pidlItem);
																		}
																		
																		CoTaskMemFree(pidlFolder);
																	}																	
																}
																ppf2->Release();
															}
															pfv->Release();
													}
													psv->Release();
											}
											psb->Release();
										}
										psp->Release();
								}
							}
							pwba->Release();
					}
					pdisp->Release();
				}
				psw->Release();
		}
		//InvalidateRect(hwnd, NULL, TRUE);
    return currentSelection;
}

struct CurrentSelectionSource : Source {
    CurrentSelectionSource() : Source(L"FILE", L"Current selection (Catalog )") {
        m_ignoreemptyquery=true;
        m_index[L"CurrentSelection"]=SourceResult(L"CurrentSelection", L"Current Selection", L"Current Selection", this, 0, 0, 0);
    }
    ~CurrentSelectionSource() {
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(FuzzyMatch(it->second.display,q)) {
                results.push_back(it->second);               
                results.back().object=new FileObject(it->second.display,
                                                     this,
                                                     it->second.display,
                                                     it->second.display,
                                                     getExplorerSelection(g_foregroundWnd));
            }
        }
    }
};