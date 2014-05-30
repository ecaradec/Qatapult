#pragma once
#include "Source.h"

struct SourceOfSources : Source {
    SourceOfSources() : Source(L"SOURCE",L"Source of sources") {
        m_icon=L"icons\\source.png";
    }
    virtual void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"SOURCE")==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
            if(FuzzyMatch((*it)->m_name,q)) {
                uint8 *obj=pack.beginBlock();
                pack.pack(L"type",L"SOURCE");
                pack.pack(L"source",(uint32)this);
                pack.pack(L"subsource",(uint32)*it);
                pack.pack(L"key",(*it)->m_name);
                pack.pack(L"text",(*it)->m_name);
                pack.pack(L"status",(*it)->m_name);
                pack.pack(L"expand",(*it)->m_name);
                pack.pack(L"bonus",(uint32)0);
                pack.pack(L"uses",(uint32)0);
                pack.endBlock(obj);
            }        
        }
    }
    std::vector<Source*> m_sources;
};