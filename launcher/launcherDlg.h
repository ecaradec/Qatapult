
// launcherDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include <list>

struct Source;

struct CMyListBox : CListBox {
    // CMyListBox is my owner-drawn list box derived from CListBox. This 
    // example draws an item's text centered vertically and horizontally. The 
    // list box control was created with the following code:
    //   pmyListBox->Create(
    //      WS_CHILD|WS_VISIBLE|WS_BORDER|WS_HSCROLL|WS_VSCROLL|
    //      LBS_SORT|LBS_MULTIPLESEL|LBS_OWNERDRAWVARIABLE,
    //      myRect, pParentWnd, 1);
    //
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
    {
        if(lpDrawItemStruct->itemID==-1)
            return;

       ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);

       CString strText; 
       GetText(lpDrawItemStruct->itemID, strText);

       CDC dc;

       dc.Attach(lpDrawItemStruct->hDC);

       // Save these value to restore them when done drawing.
       COLORREF crOldTextColor = dc.GetTextColor();
       COLORREF crOldBkColor = dc.GetBkColor();

       // If this item is selected, set the background color 
       // and the text color to appropriate values. Also, erase
       // rect by filling it with the background color.
       if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
          (lpDrawItemStruct->itemState & ODS_SELECTED))
       {
          dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
          dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
          dc.FillSolidRect(&lpDrawItemStruct->rcItem, 
             ::GetSysColor(COLOR_HIGHLIGHT));
       }
       else
          dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);

       // If this item has the focus, draw a red frame around the
       // item's rect.
       if ((lpDrawItemStruct->itemAction | ODA_FOCUS) &&
          (lpDrawItemStruct->itemState & ODS_FOCUS))
       {
          CBrush br(RGB(255, 0, 0));
          dc.FrameRect(&lpDrawItemStruct->rcItem, &br);
          /*CRect r(lpDrawItemStruct->rcItem);
          //r.right=10;
          dc.FillSolidRect(&r, 0x0088FF);*/
       }

       // Draw the text.
       dc.DrawText(
          strText,
          strText.GetLength(),
          &lpDrawItemStruct->rcItem,
          DT_SINGLELINE|DT_VCENTER);

       // Reset the background color and the text color back to their
       // original values.
       dc.SetTextColor(crOldTextColor);
       dc.SetBkColor(crOldBkColor);

       dc.Detach();
    }
};

struct Source;

struct SourceResult {
    SourceResult() { icon=0; source=0; data=0; }
    CString  expandStr;
    CString  display;
    Source  *source;
    int      id;
    void    *data;
    
    // should move to launchdlg subclass
    Gdiplus::Bitmap *icon; 
};

struct KeyHook {
    virtual ~KeyHook() = 0 {}
    virtual LRESULT OnKeyboardMessage(UINT msg, WPARAM wparam, LPARAM lparam) = 0;
    virtual void OnSelChange(SourceResult *r) = 0;
};

// ClauncherDlg dialog
class ClauncherDlg : public CDialogEx
{
// Construction
public:
	ClauncherDlg(KeyHook *ph, CWnd* pParent = NULL);	// standard constructor

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
    afx_msg void OnHotKey(UINT nHotkey, UINT key1, UINT key2);
    afx_msg void OnShowWindow(BOOL b, UINT nStatus);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLbnSelChange();

    BOOL PreTranslateMessage(MSG* pMsg);

    SourceResult *GetSelectedItem();

    KeyHook                    *m_pKH;
    ULONG_PTR                   m_gdiplusToken;
    CEdit                       m_queryWnd;
    CMyListBox                  m_resultsWnd;
    int                         m_mode;
    std::vector<SourceResult>     m_results;
    CString                     m_lastquery;
    CString                     m_lastverbquery;
    IContextMenu               *m_pContextMenu;
    Source                       *m_pCurRule;
    std::vector<Source*>          m_sources;
};
