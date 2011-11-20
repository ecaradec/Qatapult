
// launcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "launcherDlg.h"
#include "afxdialogex.h"
#include "FindFileRecursively.h"
#include "Source.h"
#include "FileVerbSource.h"
#include "FileSource.h"
#include "HistorySource.h"
#include "StartMenuSource.h"
#include <gdiplus.h>
#include <atlimage.h>

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




ClauncherDlg::ClauncherDlg(KeyHook *ph, CWnd* pParent)
	: CDialogEx(ClauncherDlg::IDD, pParent), m_pKH(ph)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ClauncherDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_resultsWnd);
}

BEGIN_MESSAGE_MAP(ClauncherDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_LBN_SELCHANGE(IDC_LIST1, &ClauncherDlg::OnLbnSelChange)
    ON_WM_HOTKEY()
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


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

    //m_sources.push_back(new SourcesRule(this));

    m_mode=0;

    SetWindowText(L"Object mode");

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

SourceResult *ClauncherDlg::GetSelectedItem()
{
    int sel=m_resultsWnd.GetCaretIndex();
    m_resultsWnd.GetSelItems(1, &sel);
    return (SourceResult*)m_resultsWnd.GetItemDataPtr(sel);
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

BOOL ClauncherDlg::PreTranslateMessage(MSG* pMsg)
{
    return CDialog::PreTranslateMessage(pMsg);
}

void ClauncherDlg::OnHotKey(UINT nHotkey, UINT key1, UINT key2)
{
    /*if(nHotkey==1) {
        if(IsWindowVisible())
            ShowWindow(SW_HIDE);
        else
            ShowWindow(SW_SHOW);
    }*/
}

void ClauncherDlg::OnShowWindow(BOOL b, UINT nStatus)
{
   /* if(b) {
        for(int i=0;i<m_sources.size()!=0; i++) {
            delete m_sources[i];
        }
        m_sources.clear();

        m_sources.push_back(new SourcesRule(this));

        m_queryWnd.SetWindowText(L"");
        m_queryWnd.SetFocus();
    }*/
}

void ClauncherDlg::OnLbnSelChange()
{
    int sel=m_resultsWnd.GetCaretIndex();
    SourceResult *r=(SourceResult*)m_resultsWnd.GetItemDataPtr(sel);

    m_pKH->OnSelChange(r);
}