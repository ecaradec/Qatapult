#pragma once
#include "Source.h"
#include "getItemVerbs.h"

struct Info {
    Source                    *source;
    std::vector<SourceResult> *results;
};

struct FileVerbSource : Source {
    FileVerbSource() : Source(L"FILEVERB") {
    }
    ~FileVerbSource() {
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        // try cache support
        (*m_pArgs)[0].source->getSubResults(query, (*m_pArgs)[0].key+L"/verb", results);
        for(int i=0;i<results.size();i++)
            results[i].source=this;

        // no cache support
        // this is better that way, optimization on the startmenu doesn't imply structural
        // changes on everything that output files.
        if(results.size()==0) {
            // path or pidl query might be useful
            //CString path=(*m_pArgs)[0].source->getString(query, (*m_pArgs)[0].key+L"/path");
            CString path=(*m_pArgs)[0].expand; path.TrimRight(L"\\");
            CString d=path.Left(path.ReverseFind(L'\\'));
            CString f=path.Mid(path.ReverseFind(L'\\')+1).MakeUpper();

            std::vector<Command> commands;
            getItemVerbs(d, f, commands);

            for(int i=0;i<commands.size();i++) {
                SourceResult r;
                r.key=path+L"/"+ItoS(i);
                r.display=commands[i].display;
                r.expand=path;
                r.id=commands[i].id;
                r.source=this;
                results.push_back(r);
            }
        }
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r) {
        CString verb=(*m_pArgs)[0].source->getString(r->key+L"/icon");

        if(verb==L"") {
            CString path=(*m_pArgs)[0].expand;
            CString d=path.Left(path.ReverseFind(L'\\'));
            CString f=path.Mid(path.ReverseFind(L'\\')+1).MakeUpper();

            std::vector<Command> commands;
            getItemVerbs(d, f, commands);

            for(int i=0;i<commands.size();i++)                
                if(commands[i].display==r->display) {
                    verb=commands[i].verb;
                    break;
                }
        }

        Gdiplus::Bitmap *bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\"+verb+L".png");
        if(bmp->GetLastStatus()!=Gdiplus::Ok) {
            delete bmp;
            bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\defaultverb.png");
        }
        return bmp;
    }
};
