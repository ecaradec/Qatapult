#include "SimpleOptDialog.h"

// how to handle tab ?
//class WebsiteSearchDlg : public SimpleOptDialog
//{
//public:
//    enum { IDD = IDD_EMPTY };
// 
//    BEGIN_MSG_MAP(WebsiteSearchDlg)
//        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)        
//        CHAIN_MSG_MAP(SimpleOptDialog)
//    END_MSG_MAP()    
//
//    WebsiteSearchDlg() {
//    }
//    void save(Record &r) {
//        websites.save(r);
//    }
//    bool del(Record &r) { 
//        return websites.del(r);
//    }
//    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    { 
//        SimpleOptDialog::OnInitDialog(uMsg,wParam,lParam,bHandled);
//
//        columns=Array(CString(L"text"),CString(L"href"),CString(L"searchHref"));
//        
//        int y=0;
//        int x=0;
//        int listViewHeight=rcClient.Height()-(space+btnHeight+3*(lineHeight+space));
//
//        // websites list
//        x=0;
//        addLabel(L"Websites",      0, y, titleWidth, lineHeight*2);   x+=titleWidth;
//        addButton(L"New",          0, x, y, 100,        btnHeight); x+=110;
//        addButton(L"Delete",       1, x, y, 100,        btnHeight); x+=110;
//        addButton(L"Save",         2, x, y, 100,        btnHeight); x+=110;        
//
//        y+=space+btnHeight;
//        x=titleWidth;
//        hListView=addListView(x, y, uiWidth, listViewHeight); y+=listViewHeight+space;
//
//        // display, href, icon, key, searchhref
//        addColumn(L"Title",0, 100);
//        addColumn(L"Href", 1, 125);
//        addColumn(L"Query",2, uiWidth-225-space);
//        
//        // website infos        
//        x=0; addLabel(L"Title", x, y, titleWidth, lineHeight); x+=titleWidth; addEdit(L"text",       x, y, uiWidth, lineHeight); y+=lineHeight+space;
//        x=0; addLabel(L"Url",   x, y, titleWidth, lineHeight); x+=titleWidth; addEdit(L"href",       x, y, uiWidth, lineHeight); y+=lineHeight+space;
//        x=0; addLabel(L"Query", x, y, titleWidth, lineHeight); x+=titleWidth; addEdit(L"searchHref", x, y, uiWidth, lineHeight); y+=lineHeight+space;
//
//        CRect r;
//        ::GetClientRect(hListView, &r);
//
//        records.clear();
//        websites.query(records);
//
//        int i=0;
//        for(std::vector<Record>::iterator it=records.begin(); it!=records.end(); it++) {
//            addItem(*it, columns );
//        }
//
//        enableEdition(false);
//        /*if(records.size()==0) {
//            enableEdition(false);
//        } else {
//            selectLVItem(0);
//        }*/
//
//        return S_OK;
//    }
//};
