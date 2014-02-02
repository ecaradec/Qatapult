#pragma once
#include "Source.h"

struct SourceObject : Object {
    SourceObject(const CString &name, const CString &type, Source *tg, Source *s) : Object(name, type, s, name) {
        subsource=tg;
    }
};

struct SourceOfSources : Source {
    SourceOfSources() : Source(L"SOURCE",L"Source of sources") {
        m_icon=L"icons\\source.png";
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"SOURCE")==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
            if(FuzzyMatch((*it)->m_name,q)) {
                results.push_back(new SourceObject((*it)->m_name, L"SOURCE", *it, this));
            }        
        }
    }
    std::vector<Source*> m_sources;
};