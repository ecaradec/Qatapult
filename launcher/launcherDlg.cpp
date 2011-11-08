
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


HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, UINT idCmdOffset)
{
   CMINVOKECOMMANDINFO ici;
   ZeroMemory(&ici, sizeof(ici));
   ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
   ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCmdOffset);
   ici.nShow = SW_SHOWNORMAL;

   return pCM->InvokeCommand(&ici);
}

struct Rule {
    virtual ~Rule() =0 {}
    virtual void collect(const TCHAR *query, std::vector<RuleResult> &r) =0;
    virtual Rule *validate() = 0;

    CString defaultQuery;
};
struct FileVerbRule : Rule {
    FileVerbRule(const CString &filepath, CListBox *pLB):m_resultsWnd(pLB) {        
        CString fp(filepath); fp.TrimRight(L"\\");
        pLB->GetParent()->SetWindowText(fp);

        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString filename=fp.Mid(fp.ReverseFind(L'\\')+1);
        
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

                if(s.GetLength()>0) {
                    Command c;
                    c.expandStr=s;
                    c.display=s;
                    c.id=mii.wID;
                    m_commands.push_back(c);
                }
            }
            //m.TrackPopupMenu(0, 0, 0, this, 0);
        }

        // add some homemade rules
        /*sources.push_back(L"Open");
        sources.push_back(L"Edit");
        sources.push_back(L"Properties");
        sources.push_back(L"Sendto");*/
    }
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        for(std::list<Command>::iterator it=m_commands.begin();it!=m_commands.end();it++) {
            if(CString(it->display).MakeUpper().Find(CString(query).MakeUpper())!=-1) {
                RuleResult r;
                r.display=it->display;
                r.expandStr=it->display;
                r.id=it->id;
                r.rule=this;
                results.push_back(r);
            }
        }
    }
    Rule *validate() {
        int id=m_resultsWnd->GetCaretIndex();
        m_resultsWnd->GetSelItems(1, &id);
        RuleResult *r=(RuleResult*)m_resultsWnd->GetItemDataPtr(id);
        ProcessCMCommand(m_pContextMenu, r->id);
        return 0;
    }
    IContextMenu        *m_pContextMenu;
    CListBox            *m_resultsWnd;
    struct Command {
        CString expandStr;
        CString display;
        int     id;
    };
    std::list<Command> m_commands;
};

struct FileRule : Rule {
    FileRule(CListBox *pLB):m_resultsWnd(pLB)  {
        defaultQuery=L"c:\\";
    }
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        CString q(query);
        if(q.Find(L":\\")!=1)
            return;
        
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

                RuleResult r;
                r.display=L"[D]"+foldername;
                r.expandStr=noslash+L"\\";
                r.rule=this;
                results.push_back(r);
            } else if(CString(w32fd.cFileName)==L"..") {
            } else {
                bool isdirectory=(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
                if(isdirectory)
                    expandStr = CString(d+L"\\"+w32fd.cFileName+L"\\");
                else
                    expandStr = CString(d+L"\\"+w32fd.cFileName);

                RuleResult r;
                r.display=CString(isdirectory?L"[D]":L"[F]")+w32fd.cFileName;
                r.expandStr=expandStr;
                r.rule=this;
                results.push_back(r);
            }
            b=FindNextFile(h, &w32fd);
        }
        FindClose(h);
    }
    Rule *validate() {
        int sel=m_resultsWnd->GetCaretIndex();
        m_resultsWnd->GetSelItems(1, &sel);

        RuleResult *r=(RuleResult*)m_resultsWnd->GetItemDataPtr(sel);        
        return new FileVerbRule(r->expandStr, m_resultsWnd); 
    }

    CListBox            *m_resultsWnd;
};

void FindFilesRecursively(LPCTSTR lpFolder, LPCTSTR lpFilePattern, std::vector<CString> &files)
{
    TCHAR szFullPattern[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    // first we are going to process any subdirectories
    PathCombine(szFullPattern, lpFolder, _T("*"));
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && CString(FindFileData.cFileName)!=L"." && CString(FindFileData.cFileName)!=L"..")
            {
                // found a subdirectory; recurse into it
                PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
                FindFilesRecursively(szFullPattern, lpFilePattern, files);
            }
        } while(FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
    // now we are going to look for the matching files
    PathCombine(szFullPattern, lpFolder, lpFilePattern);
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // found a file; do something with it
                PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
                files.push_back(szFullPattern);
                //_tprintf_s(_T("%s\n"), szFullPattern);
            }
        } while(FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
}

struct StartMenuRule : Rule {
    StartMenuRule(CListBox *pLB):m_resultsWnd(pLB)  {
    }
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        std::vector<CString> lnks;
        FindFilesRecursively(L"C:\\ProgramData\\Microsoft\\Windows\\Start Menu", L"*.lnk", lnks);

        for(int i=0;i<lnks.size();i++) {
            CString str(lnks[i]);
            PathRemoveExtension(str.GetBuffer()); str.ReleaseBuffer();
            str=PathFindFileName(str.GetBuffer()); str.ReleaseBuffer();
            if(CString(str).MakeUpper().Find(CString(query).MakeUpper())!=-1) {
                RuleResult r;
                r.expandStr=lnks[i];
                r.display=str;
                r.rule=this;
                results.push_back(r);
            }
        }
    }
    Rule *validate() {
        int sel=m_resultsWnd->GetCaretIndex();
        m_resultsWnd->GetSelItems(1, &sel);

        Result *r=(Result*)m_resultsWnd->GetItemDataPtr(sel);        
        return new FileVerbRule(r->expandStr, m_resultsWnd); 
    }
    struct Result {
        CString expandStr;
        CString display;
    };
    std::list<Result>    results;
    CListBox            *m_resultsWnd;
};


struct SourcesRule : Rule {
    SourcesRule(CListBox *pLB):m_resultsWnd(pLB) {
        // several sources could be displayed and filtered simultaneously :
        // startmenu, history, contact, window, bookmarks, etc...
        // with optional exclusion
        /*RuleResult c;
        c.expandStr=c.display=L"File"; sources.push_back(c);
        c.expandStr=c.display=L"StartMenu"; sources.push_back(c);
        c.expandStr=c.display=L"Text"; sources.push_back(c);
        c.expandStr=c.display=L"Window"; sources.push_back(c);
        c.expandStr=c.display=L"Email"; sources.push_back(c);
        c.expandStr=c.display=L"Delicious"; sources.push_back(c);*/

        m_rules.push_back(new StartMenuRule(m_resultsWnd));
        m_rules.push_back(new FileRule(m_resultsWnd));
    }
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        /*for(std::vector<RuleResult>::iterator it=sources.begin();it!=sources.end();it++) {
            if(CString(it->display).MakeUpper().Find(CString(query).MakeUpper()) == -1)
                continue;
            RuleResult r;
            r.expandStr=it->expandStr;
            r.display=it->display;
            results.push_back(r);
        }
        smr.collect(query, results);
        FileRule fr(m_resultsWnd);
        fr.collect(query,results);*/
        for(int i=0;i<m_rules.size(); i++) {
            m_rules[i]->collect(query, results);
        }

    }
    Rule *validate() {
        /*int sel=m_resultsWnd->GetCaretIndex();
        m_resultsWnd->GetSelItems(1, &sel);
        CString s;
        m_resultsWnd->GetText(sel, s);

        if(s==L"File")
            return new FileRule(m_resultsWnd);
        else if(s=="StartMenu")
            return new StartMenuRule(m_resultsWnd);
        
        MessageBox(0, L"Source not implemented", L"Source not implemented", MB_OK);*/
        
        int sel=m_resultsWnd->GetCaretIndex();
        m_resultsWnd->GetSelItems(1, &sel);
        RuleResult *r=(RuleResult*)m_resultsWnd->GetItemDataPtr(sel);
        return r->rule->validate();
    }
    CListBox               *m_resultsWnd;
    std::vector<Rule*>      m_rules;
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
    CString q;
    m_queryWnd.GetWindowText(q);

    m_resultsWnd.ResetContent();
    m_results.clear();
    m_rules.back()->collect(q, m_results);
    for(int i=0;i<m_results.size();i++) {
        int id=m_resultsWnd.AddString(m_results[i].display);
        m_resultsWnd.SetItemDataPtr(id, &m_results[i]); // ok if we don't add anything to results later
    }


    /*Rule *r=m_rules.back()->filter(q);
    if(r!=0)
    {
        m_rules.push_back(r);
        m_queryWnd.SetWindowText(q);
        m_queryWnd.SetSel(q.GetLength(), q.GetLength());
    }*/
}

BOOL ClauncherDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {	
        Rule *r=m_rules.back()->validate();
        if(r==0) {
            m_queryWnd.SetWindowText(L"");
            return FALSE;
        }

        m_rules.push_back(r);
        m_queryWnd.SetWindowText(r->defaultQuery);
        m_queryWnd.SetSel(r->defaultQuery.GetLength(), r->defaultQuery.GetLength());

        return FALSE;
    }	
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
    {        
        delete m_rules.back();
        m_rules.pop_back();
        if(m_rules.size()==0) {
            EndDialog(0);
            return FALSE;
        }
        
        Rule *r=m_rules.back();
        m_queryWnd.SetWindowText(r->defaultQuery);
        m_queryWnd.SetSel(r->defaultQuery.GetLength(), r->defaultQuery.GetLength());
        return FALSE;
    }
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
    {        
        int id=m_resultsWnd.GetCaretIndex();
        m_resultsWnd.GetSelItems(1, &id);
        RuleResult *r=(RuleResult*)m_resultsWnd.GetItemDataPtr(id);
        CString expandStr(r->expandStr);
        m_queryWnd.SetWindowText(expandStr);
        m_queryWnd.SetSel(expandStr.GetLength(), expandStr.GetLength());
        m_queryWnd.SetFocus();
        
        return FALSE;
    }
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DOWN)
    {	
        m_resultsWnd.SetFocus();
        return FALSE;
    }	
    else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_UP && GetFocus() == &m_resultsWnd && m_resultsWnd.GetCaretIndex() == 0)
    {	
        m_queryWnd.SetFocus();
        return FALSE;
    }	
    return CDialog::PreTranslateMessage(pMsg);
}
