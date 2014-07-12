#pragma once
#include "TextObject.h"

struct TextSource : Source {
    TextSource() : Source(L"TEXT",L"Text (Catalog )") {
        m_ignoreemptyquery=true;
        m_prefix=L'\'';
    }
    void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"TEXT")==activetypes.end())
            return;

        CString q(query);

        bool prefixed=_tcslen(query)>0 && (query[0]==_T('\''));
        CString text(query);
        if(prefixed) {
            text.Mid(1);
        }

        pack.begin(KV_Map);
            pack.writePairString(L"type",L"TEXT");
            pack.writePairUint32(L"source",(uint32)this);
            pack.writePairString(L"key",text);
            pack.writePairString(L"text",text);
            pack.writePairString(L"status",text);
            pack.writePairString(L"expand",query);
            pack.writePairUint32(L"prefixed",prefixed?1:0);
            pack.writePairUint32(L"uses",(uint32)0);
        pack.end();
    }
    void rate(const CString &q, Object *r) {
        if(r->getInt(L"prefixed")==0)
            r->m_rank=0;
        else
            r->m_rank=1000;
    }
};