#pragma once
#include "geticon.h"
#include "FileVerbSource.h"

struct FileSource : Source {
    FileSource() : Source(L"File",L"FILE") {
        type=L"FILE";
        load();
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int flags) {
        CString q(query); 
        if(q.Find(L":\\")!=1 && q.Find(L"\\\\")==-1)
            return;
        CString Q(q); Q.MakeUpper();

        // network works for \\FREEBOX\Disque dur\ but not for \\FREEBOX\
        // need a way to enumerate servers and shares
        
        CString d=q.Left(q.ReverseFind(L'\\'));
        CString f=q.Mid(q.ReverseFind(L'\\')+1).MakeUpper();

        HANDLE h;
        WIN32_FIND_DATA w32fd;
        h=FindFirstFile(d+L"\\*", &w32fd);
        bool b=(h!=INVALID_HANDLE_VALUE);
        while(b) {                    
            CString expand;
        
            if(CString(w32fd.cFileName)==L".") {
                CString noslash=q.Left(q.ReverseFind(L'\\'));
                CString foldername=noslash.Mid(noslash.ReverseFind(L'\\')+1);

                if(CString(foldername).MakeUpper().Find(f)!=-1) {
                    SourceResult r;
                    r.display=L"[D]"+foldername;
                    r.expand=noslash+L"\\";
                    r.source=this;
                    r.rank=10;
                    results.push_back(r);
                }
            } else if(CString(w32fd.cFileName)==L"..") {
            } else {
                bool isdirectory=!!(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
                if(isdirectory)
                    expand = CString(d+L"\\"+w32fd.cFileName+L"\\");
                else
                    expand = CString(d+L"\\"+w32fd.cFileName);

                if(CString(w32fd.cFileName).MakeUpper().Find(f)!=-1) {
                    SourceResult r;
                    r.display=CString(isdirectory?L"[D]":L"[F]")+w32fd.cFileName;
                    r.expand=expand;
                    r.source=this;
                    r.rank=10;
                    results.push_back(r);
                }
            }
            b=!!FindNextFile(h, &w32fd);
        }
        FindClose(h);
    }
    Gdiplus::Bitmap *getIcon(std::vector<SourceResult> &args,SourceResult *r) {
        return ::getIcon(r->expand);
    }
    // ail pas de details pour avoir le path ici ?
    // it doesn't work here ??*
    // a temporary index feel really sucky
    virtual bool getData(const TCHAR *itemkey, const TCHAR *name, char *buff, int len) { 
        /*X *x=(X*)m_index[itemkey].data;
        if(CString(name)==L"VERBS") {
             *((std::vector<Command>**)buff)=&((X*)m_index[itemkey].data)->m_commands;
        }else if(CString(name)==L"PATH") {
            wcscpy((TCHAR*)buff, (r->expand.GetString());
        }*/
        return false; 
    }

    //ClauncherDlg            *m_pUI;
};