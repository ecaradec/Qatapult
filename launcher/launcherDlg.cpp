
// launcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "launcherDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Define rules :
// 
// FILE > FILE_VERB > ARG ou RUN (mixed : first result is run, others are anything )
// SOURCE > OBJECT > OBJECT_VERB
// SOURCE can be implicit c:\ imply FILESYSTEM

//WCHAR open[L"FILE", L"OPEN", 0];
//WCHAR sendto[L"FILE", L"SENDTO", L"FOLDER", 0];
//int iop=0;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// ClauncherDlg dialog




ClauncherDlg::ClauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ClauncherDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ClauncherDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_queryWnd);
    DDX_Control(pDX, IDC_LIST1, m_resultsWnd);
}

BEGIN_MESSAGE_MAP(ClauncherDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_EN_CHANGE(IDC_EDIT1, &ClauncherDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


struct Rule {
    virtual ~Rule() =0 {}
    virtual void  filter(const TCHAR *q) = 0;
    virtual Rule *validate() = 0;
};
struct FileVerbRule : Rule {
    FileVerbRule(const CString &filepath, CListBox *pLB):m_resultsWnd(pLB) {
        CString d=filepath.Left(filepath.ReverseFind(L'\\'));
        CString filename=filepath.Right(filepath.ReverseFind(L'\\'));
        
        CComPtr<IShellFolder> psfDesktop, psfWinFiles;
        PIDLIST_RELATIVE pidlFolder, pidlItem;
        DWORD eaten;
        m_pContextMenu=0;
        
        HRESULT hr;
        if(FAILED(hr = SHGetDesktopFolder(&psfDesktop)))
            return;

        if(FAILED(hr=psfDesktop->ParseDisplayName(0, 0, (LPOLESTR)CStringW(d).GetBuffer(), 0, &pidlFolder, NULL)))
            return;

        if(FAILED(hr = psfDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder, (LPVOID *) &psfWinFiles)))
            return;
        
        // sometimes the given filename is not parsable under the given IShellFolder but is parsable under the desktop folder
        if(SUCCEEDED(hr = psfWinFiles->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(filename).GetBuffer(), &eaten, &pidlItem, 0)))
        {
            if(FAILED(hr=psfWinFiles->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)&m_pContextMenu)))
                return;
        }
        else if(SUCCEEDED(hr= psfDesktop->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(filename).GetBuffer(), &eaten, &pidlItem, 0)))
        {
            if(FAILED(hr=psfDesktop->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)&m_pContextMenu)))
                return;
        }
        if(m_pContextMenu) {
            CMenu m;
            m.CreatePopupMenu();
            m_pContextMenu->QueryContextMenu(m.GetSafeHmenu(), 0, 0, 0xFFFF, CMF_EXTENDEDVERBS);
            int i=0;
            CString s;
            for(int i=0;i<m.GetMenuItemCount();i++) {
            
                MENUITEMINFO mii;
                memset(&mii, 0, sizeof(mii));
                mii.cbSize=sizeof(MENUITEMINFO);
                WCHAR buff[1024]={0};
                mii.dwTypeData=(LPWSTR)&buff;
                mii.cch=sizeof(buff);
                mii.fMask=MIIM_STRING|MIIM_SUBMENU|MIIM_ID;
                bool b=m.GetMenuItemInfo(i, &mii,TRUE);

                int c=m.GetMenuString(i, s, MF_BYPOSITION);
                s.Replace(L"&&",L"__EAMP__");
                s.Replace(L"&",L"");
                s.Replace(L"__EAMP__",L"&");

                if(mii.hSubMenu)
                    s+=" [MENU]";

                if(s.GetLength()>0)
                    sources.push_back(s);
            }
            //m.TrackPopupMenu(0, 0, 0, this, 0);
        }

        // add some homemade rules
        sources.push_back(L"Open");
        sources.push_back(L"Edit");
        sources.push_back(L"Properties");
        sources.push_back(L"Sendto");
    }
    void  filter(const TCHAR *query) {
        m_resultsWnd->ResetContent();
        CString q(query);
        for(int i=0;i<sources.size();i++) {
            CString str(sources[i]);
            if(str.MakeUpper().Find(q.MakeUpper())!=-1)
                m_resultsWnd->AddString(sources[i]);
        }
    }
    Rule *validate() {
        return 0;
    }
    IContextMenu        *m_pContextMenu;
    CListBox            *m_resultsWnd;
    std::vector<CString> sources;
};

struct FileRule : Rule {
    FileRule(CListBox *pLB):m_resultsWnd(pLB)  {}
    void  filter(const TCHAR *query) {
        CString q(query);
        m_resultsWnd->ResetContent();
        if(q==L"")
            q=L"";
        
        HANDLE h;
        WIN32_FIND_DATA w32fd;
        h=FindFirstFile(q+L"*", &w32fd);
        bool b=(h!=INVALID_HANDLE_VALUE);
        while(b) {        
            CString d=q.Left(q.ReverseFind(L'\\'));
            CString expandStr;
        
        
            if(CString(w32fd.cFileName)==L".") {
                CString noslash=q.Left(q.ReverseFind(L'\\'));
                CString foldername=noslash.Mid(noslash.ReverseFind(L'\\')+1);

                results.push_back(noslash+L"\\"+foldername);
                m_resultsWnd->AddString(foldername);
            } else if(CString(w32fd.cFileName)==L"..") {
            } else {
                bool isdirectory=(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
                if(isdirectory)
                    expandStr = CString(d+L"\\"+w32fd.cFileName+L"\\");
                else
                    expandStr = CString(d+L"\\"+w32fd.cFileName);

                results.push_back(expandStr);
                m_resultsWnd->AddString(CString(isdirectory?L"[D]":L"[F]")+w32fd.cFileName);
            }
            b=FindNextFile(h, &w32fd);
        }
        FindClose(h);
    }
    Rule *validate() { return new FileVerbRule(results.front(), m_resultsWnd); }

    std::vector<CString> results;
    CListBox            *m_resultsWnd;
};

struct SourcesRule : Rule {
    SourcesRule(CListBox *pLB):m_resultsWnd(pLB) {
        sources.push_back(L"File");
        sources.push_back(L"Text");
        sources.push_back(L"Window");
        sources.push_back(L"Email");
        sources.push_back(L"Delicious");
    }
    void  filter(const TCHAR *query) {
        m_resultsWnd->ResetContent();
        CString q(query);
        for(int i=0;i<sources.size();i++) {
            CString str(sources[i]);
            if(str.MakeUpper().Find(q.MakeUpper())!=-1)
                m_resultsWnd->AddString(sources[i]);
        }
    }
    Rule *validate() {
        return new FileRule(m_resultsWnd);
    }
    CListBox            *m_resultsWnd;
    std::vector<CString> sources;
};

// if there is more than one file, use filegrouprule
// if there are command args like sendto, use sendtoargrule ?
// rules could pass their args to the next element


// ClauncherDlg message handlers

BOOL ClauncherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    m_rules.push_back(new SourcesRule(&m_resultsWnd));

    m_mode=0;

    SetWindowText(L"Object mode");

    m_queryWnd.SetWindowText(L"");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void ClauncherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ClauncherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ClauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void ClauncherDlg::OnEnChangeEdit1()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.


    // TODO:  Add your control notification handler code here
    // object mode
    CString q;
    m_queryWnd.GetWindowText(q);

    /*std::vector<std::vector<CString> > validrules; 
    std::vector<CString> r; r.push_back(L"FILE"); r.push_back(L"OPEN"); validrules.push_back(r);
    std::vector<CString> r; r.push_back(L"FILE"); r.push_back(L"EDIT"); validrules.push_back(r);
   
    std::vector<CString> objects;
    for(int i=0;i<validrules.size();i++) {
        objects.push_back(validrules[i]);
    }
    
    // then make each rule unique and search on each simultaneously ?
    // Le premier element est décidé implicitement en fonction de la syntaxe (2eme temps )
    // en realité il faudrait taper FILE d'abord pour orienter explicitement la source

    FILE -> OPEN
         -> EDIT
         -> PROPERTY
         -> SENDTO -> FOLDER
    WINDOW

    c'est une sorte d'automate guidée par des  regles, chaque niveau doit exclure les précédents

    {"FILE" : {"OPEN":[],
               "EDIT":[],
               "PROPERTY":[],
               "SENDTO" : ["FOLDER"]
              },
     "WINDOW"

    */

    /*
    if(m_mode==0) {
        if(q.Left(3)==L"c:\\")
            SearchObject(q);
        else
            m_resultsWnd.ResetContent();
    } else if(m_mode==1) { //verb mode

        q.MakeUpper();

        m_resultsWnd.ResetContent();
        m_verbsfilteredresults.clear();
        
        for(int i=0;i<m_verbsresults.size();i++) {
            VerbResult verb=m_verbsresults[i];
            CString verbUC=verb.m_expandStr; verbUC.MakeUpper();
            if(q.GetLength()==0 || verbUC.Find(q)!=-1)
                m_verbsfilteredresults.push_back(verb);
        }

        for(int i=0;i<m_verbsfilteredresults.size(); i++)
            m_resultsWnd.AddString(m_verbsfilteredresults[i].m_expandStr);

    }*/

    m_rules.back()->filter(q);
}

void ClauncherDlg::SearchObject(const CString &q) {
    m_lastquery=q;

    m_resultsWnd.ResetContent();
    m_results.clear();
    
    HANDLE h;
    WIN32_FIND_DATA w32fd;
    h=FindFirstFile(q+L"*", &w32fd);
    bool b=(h!=INVALID_HANDLE_VALUE);
    while(b) {        
        CString d=q.Left(q.ReverseFind(L'\\'));
        CString expandStr;
        
        
        if(CString(w32fd.cFileName)==L".") {
            CString noslash=q.Left(q.ReverseFind(L'\\'));
            CString foldername=noslash.Mid(noslash.ReverseFind(L'\\')+1);

            m_results.push_back(ObjectResult(noslash+L"\\", w32fd));
            m_resultsWnd.AddString(foldername);
        } else if(CString(w32fd.cFileName)==L"..") {
        } else {
            bool isdirectory=(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            if(isdirectory)
                expandStr = CString(d+L"\\"+w32fd.cFileName+L"\\");
            else
                expandStr = CString(d+L"\\"+w32fd.cFileName);

            m_results.push_back(ObjectResult(expandStr, w32fd));
            m_resultsWnd.AddString(CString(isdirectory?L"[D]":L"[F]")+w32fd.cFileName);
        }
        b=FindNextFile(h, &w32fd);
    }
    FindClose(h);

    /*for(int i=0;i<m_results.size();i++) {
        bool isdirectory=(m_results[i].m_w32fd & FILE_ATTRIBUTE_DIRECTORY);
        m_resultsWnd.AddString(CString(isdirectory?L"[D]":L"[F]")+m_results[i].m_w32fd.cFileName);
    }*/
}

void ClauncherDlg::CacheVerbs() {
    
    CString d=m_object.m_expandStr.Left(m_object.m_expandStr.ReverseFind(L'\\'));
    CString filename=m_object.m_w32fd.cFileName;
        
    CComPtr<IShellFolder> psfDesktop, psfWinFiles;
    PIDLIST_RELATIVE pidlFolder, pidlItem;
    DWORD eaten;
    m_pContextMenu=0;
        
    HRESULT hr;
    if(FAILED(hr = SHGetDesktopFolder(&psfDesktop)))
        return;

    if(FAILED(hr=psfDesktop->ParseDisplayName(0, 0, (LPOLESTR)CStringW(d).GetBuffer(), 0, &pidlFolder, NULL)))
        return;

    if(FAILED(hr = psfDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder, (LPVOID *) &psfWinFiles)))
        return;
        
    // sometimes the given filename is not parsable under the given IShellFolder but is parsable under the desktop folder
    if(SUCCEEDED(hr = psfWinFiles->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(filename).GetBuffer(), &eaten, &pidlItem, 0)))
    {
        if(FAILED(hr=psfWinFiles->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)&m_pContextMenu)))
            return;
    }
    else if(SUCCEEDED(hr= psfDesktop->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(filename).GetBuffer(), &eaten, &pidlItem, 0)))
    {
        if(FAILED(hr=psfDesktop->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)&m_pContextMenu)))
            return;
    }
    if(m_pContextMenu) {
        CMenu m;
        m.CreatePopupMenu();
        m_pContextMenu->QueryContextMenu(m.GetSafeHmenu(), 0, 0, 0xFFFF, CMF_EXTENDEDVERBS);
        int i=0;
        CString s;
        for(int i=0;i<m.GetMenuItemCount();i++) {
            
            MENUITEMINFO mii;
            memset(&mii, 0, sizeof(mii));
            mii.cbSize=sizeof(MENUITEMINFO);
            WCHAR buff[1024]={0};
            mii.dwTypeData=(LPWSTR)&buff;
            mii.cch=sizeof(buff);
            mii.fMask=MIIM_STRING|MIIM_SUBMENU|MIIM_ID;
            bool b=m.GetMenuItemInfo(i, &mii,TRUE);

            int c=m.GetMenuString(i, s, MF_BYPOSITION);
            s.Replace(L"&&",L"__EAMP__");
            s.Replace(L"&",L"");
            s.Replace(L"__EAMP__",L"&");

            if(mii.hSubMenu)
                s+=" [MENU]";

            if(s.GetLength()>0)
                m_verbsresults.push_back(VerbResult(s, mii.wID));
        }
        //m.TrackPopupMenu(0, 0, 0, this, 0);
    }
}


HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, UINT idCmdOffset)
{
   CMINVOKECOMMANDINFO ici;
   ZeroMemory(&ici, sizeof(ici));
   ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
   ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCmdOffset);
   ici.nShow = SW_SHOWNORMAL;

   return pCM->InvokeCommand(&ici);
}

BOOL ClauncherDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && GetFocus() == &m_queryWnd)
    {	
        m_rules.push_back(m_rules.back()->validate());
        m_queryWnd.SetWindowText(L"");
        return FALSE;
    }	
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && GetFocus() == &m_queryWnd)
    {        
        delete m_rules.back();
        m_rules.pop_back();
        if(m_rules.size()==0)
            EndDialog(0);
        else
            m_queryWnd.SetWindowText(L"");
        return FALSE;
    }

    /*if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && GetFocus() == &m_queryWnd)
    {	
        // we're in the edit control return false so that the dialog doesn't change the message.
        if(m_mode==0) {
            if(m_results.size()>0) {

                CString expandStr;
                CString d=m_results[0].m_expandStr.Left(m_results[0].m_expandStr.ReverseFind(L'\\'));
                CString filename=m_results[0].m_w32fd.cFileName;

                expandStr = CString(m_results[0].m_expandStr);

                m_queryWnd.SetWindowText(expandStr);
                m_queryWnd.SetSel(expandStr.GetLength(), expandStr.GetLength());
            }
        } else if(m_mode==1) {
            // I need to get actually displayed results somewhere
            // => expand to the first result
            CString expandStr=m_verbsfilteredresults.front().m_expandStr;
            m_queryWnd.SetWindowText(expandStr);
            m_queryWnd.SetSel(expandStr.GetLength(), expandStr.GetLength());
        }
        return FALSE;
    }
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && GetFocus() == &m_queryWnd)
    {	
        if(m_mode==0) {
            if(m_results.size()>0) {
                m_mode=1;
                m_object=m_results[0];
                SetWindowText(m_object.m_expandStr);

                m_verbsresults.clear();
                CacheVerbs();

                m_queryWnd.SetWindowText(L""); // this trigger the EN_CHANGE message
            }   
        } else if(m_mode==1) {
            VerbResult v=m_verbsfilteredresults.front();
            ProcessCMCommand(m_pContextMenu, v.m_id);
        }
        return FALSE;
    }	
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && GetFocus() == &m_queryWnd)
    {	
        if(m_mode==0) {
            EndDialog(0);
        } else if(m_mode==1) {
            m_mode=0;
            m_queryWnd.SetWindowText(m_lastquery);
            m_queryWnd.SetSel(m_lastquery.GetLength(), m_lastquery.GetLength());
            //SearchObject(m_object.m_expandStr);
        } 
        return FALSE;
    }
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DOWN)
    {	
        m_resultsWnd.SetFocus();
        return FALSE;
    }	*/
    
    return CDialog::PreTranslateMessage(pMsg);
}
