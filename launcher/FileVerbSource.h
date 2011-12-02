#pragma once
#include "Source.h"
#include "getItemVerbs.h"

struct Info {
    Source                    *source;
    std::vector<SourceResult> *results;
};

struct FileVerbSource : Source {
    FileVerbSource() : Source(L"FILEVERB") {
        load();
    }
    ~FileVerbSource() {
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {
        CString fp((*m_pArgs)[0].source->getString((*m_pArgs)[0].key, L"path") ); fp.TrimRight(L"\\");

        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString f=fp.Mid(fp.ReverseFind(L'\\')+1);

        (*m_pArgs)[0].source->getSubResults((*m_pArgs)[0].key, L"VERBS", results);

        /*std::vector<Command> *commands=0;
        (*m_pArgs)[0].source->getData((*m_pArgs)[0].key, L"VERBS", (char*)&commands, sizeof(commands));

        for(std::vector<Command>::iterator it=commands->begin();it!=commands->end();it++) {
            if(CString(it->display).MakeUpper().Find(CString(query).MakeUpper())!=-1) {
                results.push_back(SourceResult(ItoS(it->id), it->display, it->display, this, it->id, 0, 0));
            }
        }*/
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r) {
        std::vector<Command> *commands=0;
        (*m_pArgs)[0].source->getData((*m_pArgs)[0].key, L"VERBS", (char*)&commands, sizeof(commands));
        
        CString verb;
        for(int i=0;i<commands->size();i++)
            if((*commands)[i].id==r->id) {
                verb=(*commands)[i].verb;
                break;
            }

        Gdiplus::Bitmap *bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\"+verb+L".png");
        if(bmp->GetLastStatus()!=Gdiplus::Ok) {
            delete bmp;
            bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\defaultverb.png");
        }
        return bmp;
    }

    std::vector<Command> m_commands;
};
