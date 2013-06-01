#pragma once
#include "Record.h"

struct SimpleOptDialog : public CDialogImpl<SimpleOptDialog> {
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(SimpleOptDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        COMMAND_CODE_HANDLER(BN_CLICKED,OnBnClicked)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
    END_MSG_MAP()

    HWND hListView;
    CListViewCtrl              listview;
    std::map<CString,CEdit*>   m_editCtrls;
    std::map<CString,CButton*> m_buttons;
    int                        currentitem;
    HGDIOBJ                    hfDefault;
    std::vector<Record>        records;
    std::vector<CString>       columns;

    SimpleOptDialog() {
    }

    virtual void save(Record &r) {
    }
    virtual bool del(Record &r) { 
        return true;
    }

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    { 
        hfDefault=GetStockObject(DEFAULT_GUI_FONT);
        return S_OK;
    }
    
    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {        
        bHandled=TRUE;
        return DLGC_WANTTAB;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        for(std::map<CString,CEdit*>::iterator it=m_editCtrls.begin(); it!= m_editCtrls.end(); it++) {
            it->second->DestroyWindow();
            delete it->second;
        }
        records.clear();

        for(std::map<CString,CButton*>::iterator it=m_buttons.begin(); it!= m_buttons.end(); it++) {
            it->second->DestroyWindow();
            delete it->second;
        }

        return S_OK;
    }
    void addItem(Record &r,std::vector<CString> &items) {
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=L"";
        lvi.mask=LVFIF_TEXT;
        lvi.iItem=10000;
        int item=ListView_InsertItem(hListView, &lvi);

        for(int i=0;i<items.size();i++) {
            CString v=r.values[items[i]];
            ListView_SetItemText(hListView, item, i, (LPWSTR)v.GetString());
        }
    }
    void addColumn(const CString &col,int i, int w) {
        LVCOLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;
        CString str = capitalize(col);
        lvc.pszText=(LPWSTR)str.GetString();
        lvc.cx=w;
        ListView_InsertColumn(hListView, i, &lvc);
    }
    void addLabel(const CString &name, int x, int y, int w, int h) {
        CStatic *label=new CStatic;
        label->Create(m_hWnd,CRect(x,y,x+w,y+h),name,WS_VISIBLE|WS_CHILD);
        label->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));
    }
    void addEdit(const CString &name, int x, int y, int w, int h) {        
        CEdit *edit=new CEdit;
        m_editCtrls[name]=edit;
        
        edit->Create(m_hWnd,CRect(x,y,x+w,y+h),L"",WS_VISIBLE|WS_CHILD|WS_TABSTOP,WS_EX_CLIENTEDGE);
        edit->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));        
    }
    void addButton(const CString &name, int id, int x, int y, int w, int h) {
        CButton *btn=new CButton;
        m_buttons[name]=btn;
        btn->Create(m_hWnd,CRect(x,y,x+w,y+h),name,WS_VISIBLE|WS_CHILD,0,id);
        btn->SendMessage(WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE,0));        
    }
    HWND addListView(int x, int y, int w, int h) {
        // Create the list-view window in report view with label editing enabled.
        HWND hListView = ::CreateWindow(WC_LISTVIEW, 
                                        L"",
                                        WS_CHILD | LVS_REPORT | WS_VISIBLE | WS_BORDER | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
                                        x,y,w,h,
                                        m_hWnd,
                                        0,
                                        0,
                                        NULL); 
        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT);
        return hListView;
    }
    void selectLVItem(int item) {
        ListView_SetItemState(hListView,item,LVIS_FOCUSED|LVIS_SELECTED,0xF);
    }
    void setLVItemText(int item, int subitem, const WCHAR *text) {
        ListView_SetItemText(hListView,item,subitem,(LPWSTR)text);
    }
    CString getLVItemText(int item, int subitem) {
        CString tmp;
        ListView_GetItemText(hListView, item, subitem, tmp.GetBufferSetLength(256), 256);
        tmp.ReleaseBuffer();
        return tmp;
    }
    CString getEditFieldText(const CString &name) {
        CString txt;
        m_editCtrls[name]->GetWindowText(txt.GetBufferSetLength(256), 256);
        txt.ReleaseBuffer();
        return txt;
    }
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if(((NMHDR*)lParam)->idFrom == 0 && ((NMHDR*)lParam)->code == LVN_ITEMCHANGED)
        {
            LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

            if((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_SELECTED) != (pnmv->uOldState & LVIS_SELECTED))
            {
                if (pnmv->uNewState & LVIS_SELECTED) {
                    onSelectItem(pnmv->iItem);
                }
            }
        }
        return S_OK;
    }
    LRESULT OnBnClicked(WPARAM wParam1, WPARAM wParam2, HWND lParam, BOOL& bHandled)
    {
        if(wParam2==0) { // new
            enableEdition(true);            
            records.push_back(Record());
            currentitem=records.size()-1;
            addItem(records.back(), columns );
            ListView_SetItemState(hListView,currentitem,LVIS_FOCUSED|LVIS_SELECTED,0xF);
        } else if(wParam2==1) { // delete
            if(del(records[currentitem])) {
                records.erase(records.begin()+currentitem);
                ListView_DeleteItem(hListView,currentitem);
                if(records.size()!=0) {
                    selectLVItem(min(currentitem,records.size()-1));
                } else {
                    enableEdition(false);
                }
            }
        } else if(wParam2==2) { // save
            Record &r=records[currentitem];
            for(int i=0;i<columns.size();i++) {
                r.values[columns[i]]=getEditFieldText(columns[i]);
                setLVItemText(currentitem,i,r.values[columns[i]]);
            }
            save(records[currentitem]);
        }
        
        return S_OK;
    }
    virtual void enableEdition(bool b) {
        m_buttons[L"Save"]->EnableWindow(b);
        m_buttons[L"Delete"]->EnableWindow(b);

        for(int i=0;i<columns.size();i++) {
            m_editCtrls[columns[i]]->EnableWindow(b);
            if(!b)
                m_editCtrls[columns[i]]->SetWindowText(L"");
        }
    }
    virtual void onSelectItem(int item) {
        currentitem=item;
        for(int i=0;i<columns.size();i++) {
            m_editCtrls[columns[i]]->SetWindowText(getLVItemText(currentitem,i));
        }
    }
};