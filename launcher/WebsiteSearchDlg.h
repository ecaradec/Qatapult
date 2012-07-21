#include "SimpleOptDialog.h"

// how to handle tab ?
class WebsiteSearchDlg : public SimpleOptDialog
{
public:
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(WebsiteSearchDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)        
        CHAIN_MSG_MAP(SimpleOptDialog)
    END_MSG_MAP()    

    WebsiteSearchDlg() {
    }
    void save(Record &r) {
        websites.save(r);
    }
    bool del(Record &r) { 
        return websites.del(r);
    }
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    { 
        SimpleOptDialog::OnInitDialog(uMsg,wParam,lParam,bHandled);

        columns=Array(CString(L"text"),CString(L"href"),CString(L"searchHref"));
        
        CRect r;
        RECT rcClient;                       // The parent window's client area.
        GetClientRect(&rcClient);

        int y=0;
        int x=0;

        r=rcClient;
        r.bottom=20;
        
        addLabel(L"Title", 0, y, 50, 20); addEdit(L"text", 55, y, 200, 20);                 y+=25;
        addLabel(L"Url",   0, y, 50, 20); addEdit(L"href",   55, y, rcClient.right-55, 20);   y+=25;
        addLabel(L"Query", 0, y, 50, 20); addEdit(L"searchHref", 55, y, rcClient.right-55, 20);   y+=25;

        x=rcClient.right-320;        
        addButton(L"New",   0, x, y, 100, 25); x+=110;
        addButton(L"Delete",1, x, y, 100, 25); x+=110;
        addButton(L"Save",  2, x, y, 100, 25); x+=110;        

        y+=30;
        hListView=addListView(rcClient.left, y, rcClient.right - rcClient.left, rcClient.bottom - y);

        ::GetClientRect(hListView, &r);

        // display, href, icon, key, searchhref
        addColumn(L"Title",0, 100);
        addColumn(L"Href", 1, 125);
        addColumn(L"Query",2, rcClient.right-225);

        records.clear();
        websites.query(records);

        int i=0;
        for(std::vector<Record>::iterator it=records.begin(); it!=records.end(); it++) {
            addItem(*it, columns );
        }

        if(records.size()==0) {
            enableEdition(false);
        } else {
            selectLVItem(0);
        }

        return S_OK;
    }
};
