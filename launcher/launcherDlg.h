
// launcherDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>

struct Rule;

// ClauncherDlg dialog
class ClauncherDlg : public CDialogEx
{
// Construction
public:
	ClauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LAUNCHER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    CEdit m_queryWnd;
    CListBox m_resultsWnd;
    afx_msg void OnEnChangeEdit1();

    BOOL PreTranslateMessage(MSG* pMsg);

    void SearchObject(const CString &q);
    void CacheVerbs();

    int                  m_mode;
    
    // object results
    struct ObjectResult {
        ObjectResult(){}
        ObjectResult(CString e, WIN32_FIND_DATA &w32fd) : m_expandStr(e), m_w32fd(w32fd) {}
        CString         m_expandStr;
        WIN32_FIND_DATA m_w32fd;
    };
    std::vector<ObjectResult> m_results;

    ObjectResult        m_object;
    CString             m_lastquery;

    // verb results
    struct VerbResult {
        VerbResult(){}
        VerbResult(CString e, int id) : m_expandStr(e), m_id(id) {}

        CString m_expandStr;
        int     m_id;
    };
    std::vector<VerbResult> m_verbsresults;
    std::vector<VerbResult> m_verbsfilteredresults;
    CString                 m_lastverbquery;

    IContextMenu           *m_pContextMenu;



    Rule *m_pCurRule;
    std::vector<Rule*> m_rules;
};
