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

        uint8 *pobj=pack.beginBlock();
        pack.pack(L"type",L"TEXT");
        pack.pack(L"source",(uint32)this);
        pack.pack(L"key",text);
        pack.pack(L"text",text);
        pack.pack(L"status",text);
        pack.pack(L"expand",query);
        pack.pack(L"prefixed",prefixed?1:0);
        pack.pack(L"uses",(uint32)0);
        pack.endBlock(pobj);     
    }
    void rate(const CString &q, Object *r) {
        if(r->getString(L"prefixed")==L"0")
            r->m_rank=0;
        else
            r->m_rank=1000;
    }
};