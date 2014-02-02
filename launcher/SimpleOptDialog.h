#pragma once
#include "Record.h"
#include <functional>

struct Button : CWindowImpl<Button, CButton> {
    BEGIN_MSG_MAP(Button)
        
        REFLECTED_COMMAND_CODE_HANDLER(BN_CLICKED, OnClick)
        //COMMAND_HANDLER(0, BN_CLICKED, OnClick)        
        //MESSAGE_HANDLER(WM_LBUTTONUP, OnClick)
    END_MSG_MAP()

    Button(HWND hwnd, const CString &text, int x, int y, int w, int h) {        
        Create(hwnd,CRect(x,y,x+w,y+h),text,WS_VISIBLE|WS_CHILD|BS_NOTIFY,0,1);
        //SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));        
    }
    LRESULT OnClick(WORD hiWParam, WORD loWParam, HWND lParam, BOOL& bHandled) {
        bHandled=FALSE;
        if(GetState()&BST_PUSHED && m_onClick) {
            m_onClick(*this);
        }
        return S_OK;
    }
    std::function<void(CWindow&)> m_onClick;
};
struct ListView : CWindowImpl<ListView, CListViewCtrl>  {
    BEGIN_MSG_MAP(ListView)
    END_MSG_MAP()

    ListView(HWND hwnd, int x, int y, int w, int h) {
        Create(hwnd, CRect(CPoint(x,y), CSize(w,h)), L"", WS_CHILD | /*LVS_LIST |*/ LVS_REPORT | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_SINGLESEL, WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT);
    }
    void addColumn(const CString &col,int i, int w) {
        LVCOLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;
        CString str = capitalize(col);
        lvc.pszText=(LPWSTR)str.GetString();
        lvc.cx=w;
        ListView_InsertColumn(m_hWnd, i, &lvc);
    }    

    int addItem() {
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=L"";
        lvi.mask=LVFIF_TEXT|LVIF_NORECOMPUTE;
        lvi.iItem=ListView_GetItemCount(m_hWnd);
        int item=ListView_InsertItem(m_hWnd, &lvi);
        return item;
    }

    void setItemText(int r, int c, const CString &text) {
        ListView_SetItemText(m_hWnd, r, c, (LPWSTR)text.GetString());
    }
};

struct Label : CWindowImpl<Label, CStatic>  {
    BEGIN_MSG_MAP(Label)
    END_MSG_MAP()

    Label(HWND hwnd, const CString &txt, int x, int y, int w, int h) {
        Create(hwnd, CRect(CPoint(x,y), CSize(w,h)), txt, WS_VISIBLE|WS_CHILD);
        //label->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
    }
};

struct Edit : CWindowImpl<Edit, CEdit>  {
    BEGIN_MSG_MAP(Edit)
    END_MSG_MAP()

    Edit(HWND hwnd, int x, int y, int w, int h, int ssStyle=0, int id=0) {
        Create(hwnd,CRect(x,y,x+w,y+h),L"",WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL|ssStyle,WS_EX_CLIENTEDGE,id);
        //edit->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
    }
};

struct MultilineEdit : CWindowImpl<MultilineEdit, CEdit>  {
    BEGIN_MSG_MAP(MultilineEdit)
    END_MSG_MAP() 
    
    MultilineEdit(HWND hwnd, int x, int y, int w, int h, int ssStyle=0, int id=0) {
        Create(hwnd,CRect(x,y,x+w,y+h),L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ssStyle,WS_EX_CLIENTEDGE,id);
        //edit->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
    }
};


struct TabCtrl : CWindowImpl<TabCtrl, CTabCtrl>  {
    BEGIN_MSG_MAP(TabCtrl)        
        //REFLECT_COMMAND_CODE(BN_CLICKED)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    TabCtrl(HWND hwnd, int x, int y, int w, int h, int id) {
        Create(hwnd,CRect(x,y,x+w,y+h),L"",WS_VISIBLE|WS_CHILD,0,id);
        //SendMessage(hTabCntrl, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));   
    }
    HWND addTab(const CString &txt, int id) {
        TCITEM tabitem;
    
        memset(&tabitem, 0, sizeof(tabitem));	
    
        tabitem.mask = TCIF_TEXT;
	    tabitem.cchTextMax = 2;  

        // tabs
        tabitem.pszText = (LPWSTR)txt.GetString();
        SendMessage(TCM_INSERTITEM, (WPARAM)id, (LPARAM)&tabitem);

        CRect tabRect;
        ::GetClientRect(m_hWnd,&tabRect);
        TabCtrl_AdjustRect(m_hWnd, FALSE, &tabRect);

        // shell tab
        return CreateWindow(L"#32770", L"", WS_CHILD|WS_VISIBLE, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), m_hWnd,(HMENU)id,0,0);    
    }
};

//struct SimpleOptDialog : public CDialogImpl<SimpleOptDialog> {
//    enum { IDD = IDD_EMPTY };
// 	
//    BEGIN_MSG_MAP(SimpleOptDialog)        
//        REFLECT_COMMAND_CODE(BN_CLICKED)
//        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
//        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)        
//        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
//    ALT_MSG_MAP(1)
//        COMMAND_CODE_HANDLER(BN_CLICKED,OnBnClicked)
//        REFLECT_NOTIFICATIONS()
//    END_MSG_MAP()
//    
//    HWND hListView;
//    CListViewCtrl              listview;
//    std::map<CString,CEdit*>   m_editCtrls;
//    std::map<CString,CButton*> m_buttons;
//    int                        currentitem;
//    HGDIOBJ                    hfDefault;
//    std::vector<Record>        records;
//    std::vector<CString>       columns;
//
//    // measure of positions
//    CRect                      rcClient;
//    int                        largeBtnWidth;
//    //int                        largeBtnHeight;
//    int                        lineHeight;
//    int                        btnWidth;
//    int                        btnHeight;
//    int                        space;
//    int                        titleWidth;
//    int                        uiWidth;
//    int                        dlgWidth;   
//    int                        lastControlId;    
//
//    
//    SimpleOptDialog() {
//        lastControlId=1000;
//    }
//
//    virtual void save(Record &r) {
//    }
//    virtual bool del(Record &r) { 
//        return true;
//    }
//
//    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {        
//        GetClientRect(&rcClient);
//        
//        dlgWidth        = rcClient.Width();
//        lineHeight      = 20;
//        largeBtnWidth   = 100;
//        //largeBtnHeight  = 25;
//        btnWidth        = lineHeight;
//        btnHeight       = 25;
//        space           = 10;
//        titleWidth      = 70;
//        uiWidth         = rcClient.Width()-titleWidth;        
//
//        hfDefault=GetStockObject(DEFAULT_GUI_FONT);
//
//        ModifyStyle(DS_SHELLFONT, WS_VSCROLL|WS_HSCROLL);
//
//        return S_OK;
//    }
//    
//    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {        
//        bHandled=TRUE;
//        return DLGC_WANTTAB;
//    }
//
//    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {
//        for(std::map<CString,CEdit*>::iterator it=m_editCtrls.begin(); it!= m_editCtrls.end(); it++) {
//            it->second->DestroyWindow();
//            delete it->second;
//        }
//        records.clear();
//
//        for(std::map<CString,CButton*>::iterator it=m_buttons.begin(); it!= m_buttons.end(); it++) {
//            it->second->DestroyWindow();
//            delete it->second;
//        }
//
//        return S_OK;
//    }
//    void addItem(Record &r,std::vector<CString> &items) {
//        LVITEM lvi;
//        memset(&lvi, 0, sizeof(lvi));
//        lvi.pszText=L"";
//        lvi.mask=LVFIF_TEXT;
//        lvi.iItem=ListView_GetItemCount(hListView);
//        int item=ListView_InsertItem(hListView, &lvi);
//
//        for(int i=0;i<items.size();i++) {
//            CString v=r.values[items[i]];
//            ListView_SetItemText(hListView, item, i, (LPWSTR)v.GetString());
//        }
//    }
//    void addItem(int pos, std::vector<CString> &items) {
//        LVITEM lvi;
//        memset(&lvi, 0, sizeof(lvi));
//        lvi.pszText=L"";
//        lvi.mask=LVFIF_TEXT|LVIF_NORECOMPUTE;
//        lvi.iItem=ListView_GetItemCount(hListView);
//        int item=ListView_InsertItem(hListView, &lvi);
//
//        for(int i=0;i<items.size();i++) {
//            ListView_SetItemText(hListView, item, i, (LPWSTR)items[i].GetString());
//        }
//    }
//    
//    int addItem() {
//        LVITEM lvi;
//        memset(&lvi, 0, sizeof(lvi));
//        lvi.pszText=L"";
//        lvi.mask=LVFIF_TEXT|LVIF_NORECOMPUTE;
//        lvi.iItem=ListView_GetItemCount(hListView);
//        int item=ListView_InsertItem(hListView, &lvi);
//        return item;
//    }
//
//    void setItemText(int r, int c, const CString &text) {
//        ListView_SetItemText(hListView, r, c, (LPWSTR)text.GetString());
//    }
//
//    void addColumn(const CString &col,int i, int w) {
//        LVCOLUMN lvc;
//        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
//        lvc.fmt = LVCFMT_LEFT;
//        CString str = capitalize(col);
//        lvc.pszText=(LPWSTR)str.GetString();
//        lvc.cx=w;
//        ListView_InsertColumn(hListView, i, &lvc);
//    }
//    void addLabel(const CString &name, int x, int y, int w, int h, HWND hwnd=0) {
//        CStatic *label=new CStatic;
//        label->Create(hwnd?hwnd:m_hWnd,CRect(x,y,x+w,y+h),name,WS_VISIBLE|WS_CHILD);
//        label->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
//    }
//    void addLabel2(const CString &name, int id, int x, int y, int w, int h, HWND hwnd=0) {
//        CStatic *label=new CStatic;
//        label->Create(hwnd?hwnd:m_hWnd,CRect(x,y,x+w,y+h),name,WS_VISIBLE|WS_CHILD,0,id);
//        label->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
//    }
//    void addSeparator(int x, int y, int w, int h) {
//        CStatic *label=new CStatic;
//        label->Create(m_hWnd,CRect(x,y,x+w,y+h),L"",WS_VISIBLE|WS_CHILD|SS_ETCHEDHORZ);
//        label->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
//    }
//    void addEdit(const CString &name, int x, int y, int w, int h) {        
//        CEdit *edit=new CEdit;
//        m_editCtrls[name]=edit;        
//        edit->Create(m_hWnd,CRect(x,y,x+w,y+h),L"",WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL,WS_EX_CLIENTEDGE);
//        edit->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));        
//    }
//    /*void addEdit3(int id, int x, int y, int w, int h, HWND hwnd=0, int ssStyle=0) {        
//        CEdit *edit=new CEdit;
//        edit->Create(hwnd?hwnd:m_hWnd,CRect(x,y,x+w,y+h),L"",WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL|ssStyle,WS_EX_CLIENTEDGE, id);
//        edit->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
//    } */       
//    void addMultilineEdit(const CString &name, int id, int x, int y, int w, int h, HWND hwnd=0) {
//        CEdit *edit=new CEdit;
//        edit->Create(hwnd?hwnd:m_hWnd,CRect(x,y,x+w,y+h),L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL,WS_EX_CLIENTEDGE,id);
//        edit->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));        
//    }
//    void addButton(const CString &strid, int id, int x, int y, int w, int h, HWND hwnd=0) {
//        CButton *btn=new CButton;
//        m_buttons[strid]=btn;
//        btn->Create(hwnd?hwnd:m_hWnd,CRect(x,y,x+w,y+h),strid,WS_VISIBLE|WS_CHILD|BS_NOTIFY,0,id);
//        btn->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));        
//    }
//    void addButton3(const CString &text, int id, int x, int y, int w, int h, HWND hwnd=0) {
//        CButton *btn=new CButton;        
//        btn->Create(hwnd?hwnd:m_hWnd,CRect(x,y,x+w,y+h),text,WS_VISIBLE|WS_CHILD|BS_NOTIFY,0,id);
//        btn->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));        
//    }    
//    HWND addTabControl(int id, int x, int y, int w, int h, HWND hwnd=0) {
//        // tabs
//        HWND hTabCntrl=CreateWindowEx(
//            0,                      // extended style
//            WC_TABCONTROL,          // tab control constant
//            L"",                     // text/caption
//            WS_CHILD | WS_VISIBLE,  // is a child control, and visible
//            x,                      // X position - device units from left
//            y,                      // Y position - device units from top
//            w,                // Width - in device units
//            h,          // Height - in device units
//            hwnd?hwnd:m_hWnd,       // parent window
//            (HMENU)id,   // no menu
//            0,                      // instance
//            NULL                    // no extra junk
//        );        
//        
//        SendMessage(hTabCntrl, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));                
//        return hTabCntrl;
//    }
//    HWND addTab(int tabid, int id, const CString &label) {
//        TCITEM tabitem;
//    
//        memset(&tabitem, 0, sizeof(tabitem));	
//    
//        tabitem.mask = TCIF_TEXT;
//	    tabitem.cchTextMax = 2;  
//
//        // tabs
//        tabitem.pszText = (LPWSTR)label.GetString();
//        SendMessage(GetDlgItem(tabid), TCM_INSERTITEM, id, (LPARAM)&tabitem);
//
//        CRect tabRect;
//        ::GetClientRect(GetDlgItem(tabid),&tabRect);
//        TabCtrl_AdjustRect(GetDlgItem(tabid), FALSE, &tabRect);
//
//        // shell tab
//        return CreateWindow(L"#32770", L"", WS_CHILD|WS_VISIBLE, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), GetDlgItem(tabid),(HMENU)id,0,0);        
//    }
//    HWND addListView(int x, int y, int w, int h) {
//        // Create the list-view window in report view with label editing enabled.
//        hListView = ::CreateWindowEx(WS_EX_CLIENTEDGE,
//                                          WC_LISTVIEW, 
//                                          L"",
//                                          WS_CHILD | /*LVS_LIST |*/ LVS_REPORT | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
//                                          x,y,w,h,
//                                          m_hWnd,
//                                          0,
//                                          0,
//                                          NULL); 
//        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT);
//        return hListView;
//    }
//    void selectLVItem(int item) {
//        ListView_SetItemState(hListView,item,LVIS_FOCUSED|LVIS_SELECTED,0xF);
//    }
//    void setLVItemText(int item, int subitem, const WCHAR *text) {
//        ListView_SetItemText(hListView,item,subitem,(LPWSTR)text);
//    }
//    CString getLVItemText(int item, int subitem) {
//        CString tmp;
//        ListView_GetItemText(hListView, item, subitem, tmp.GetBufferSetLength(256), 256);
//        tmp.ReleaseBuffer();
//        return tmp;
//    }
//    CString getEditFieldText(const CString &name) {
//        if(m_editCtrls.find(name)!=m_editCtrls.end()) {
//            CString txt;
//            m_editCtrls[name]->GetWindowText(txt.GetBufferSetLength(256), 256);
//            txt.ReleaseBuffer();
//        }
//        return L"";
//    }
//    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {
//        if(((NMHDR*)lParam)->idFrom == 0 && ((NMHDR*)lParam)->code == LVN_ITEMCHANGED)
//        {
//            LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
//
//            if((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_SELECTED) != (pnmv->uOldState & LVIS_SELECTED))
//            {
//                if (pnmv->uNewState & LVIS_SELECTED) {                    
//                    onSelectItem(pnmv->iItem);
//                }
//            }
//        }
//        return S_OK;
//    }
//    LRESULT OnBnClicked(WPARAM wParam1, WPARAM wParam2, HWND lParam, BOOL& bHandled)
//    {
//        if(wParam2==0) { // new
//            enableEdition(true);            
//            records.push_back(Record());
//            currentitem=records.size()-1;
//            addItem(records.back(), columns );
//            ListView_SetItemState(hListView,currentitem,LVIS_FOCUSED|LVIS_SELECTED,0xF);
//        } else if(wParam2==1) { // delete
//            if(del(records[currentitem])) {
//                records.erase(records.begin()+currentitem);
//                ListView_DeleteItem(hListView,currentitem);
//                if(records.size()!=0) {
//                    selectLVItem(min(currentitem,records.size()-1));
//                } else {
//                    enableEdition(false);
//                }
//            }
//        } else if(wParam2==2) { // save
//            Record &r=records[currentitem];
//            for(int i=0;i<columns.size();i++) {
//                r.values[columns[i]]=getEditFieldText(columns[i]);
//                setLVItemText(currentitem,i,r.values[columns[i]]);
//            }
//            save(records[currentitem]);
//        }
//        
//        return S_OK;
//    }
//    virtual void enableEdition(bool b) {
//        m_buttons[L"Save"]->EnableWindow(b);
//        m_buttons[L"Delete"]->EnableWindow(b);
//
//        for(int i=0;i<columns.size();i++) {            
//            m_editCtrls[columns[i]]->EnableWindow(b);
//            if(i==0)
//                m_editCtrls[columns[i]]->SetFocus();
//            if(!b)
//                m_editCtrls[columns[i]]->SetWindowText(L"");
//        }
//    }
//    virtual void onSelectItem(int item) {
//        enableEdition(true);
//
//        currentitem=item;
//        for(int i=0;i<columns.size();i++) {
//            if(m_editCtrls.find(columns[i]) != m_editCtrls.end())
//                m_editCtrls[columns[i]]->SetWindowText(getLVItemText(currentitem,i));
//        }
//    }
//};