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
                pack.begin(KV_Map);
                    pack.writePairString(L"type",L"SOURCE");
                    pack.writePairUint32(L"source",(uint32)this);
                    pack.writePairUint32(L"subsource",(uint32)*it);
                    pack.writePairString(L"key",(*it)->m_name);
                    pack.writePairString(L"text",(*it)->m_name);
                    pack.writePairString(L"status",(*it)->m_name);
                    pack.writePairString(L"expand",(*it)->m_name);
                    pack.writePairUint32(L"bonus",(uint32)0);
                    pack.writePairUint32(L"uses",(uint32)0);
                pack.end();
            }        
        }
    }
    std::vector<Source*> m_sources;
};