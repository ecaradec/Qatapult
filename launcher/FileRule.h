#include "geticon.h"

struct FileRule : Rule {
    FileRule(ClauncherDlg *pLB):m_pUI(pLB)  {
        defaultQuery=L"c:\\";
    }
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        CString q(query);
        if(q.Find(L":\\")!=1 && q.Find(L"\\\\")==-1)
            return;

        // network works for \\FREEBOX\Disque dur\ but not for \\FREEBOX\
        // need a way to enumerate servers and shares
        
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
        RuleResult *r=m_pUI->GetSelectedItem();
        return new FileVerbRule(r->expandStr, m_pUI); 
    }
    Gdiplus::Bitmap *getIcon(RuleResult *r) {
        return ::getIcon(r->expandStr);
    }


    ClauncherDlg            *m_pUI;
};