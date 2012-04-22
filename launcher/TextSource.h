#pragma once
#include "TextObject.h"

struct TextSource : Source {
    TextSource() : Source(L"TEXT",L"Text (Catalog )") {
        m_ignoreemptyquery=true;
        m_prefix=L'\'';
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &r, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

         CString q(query);

        int id=CString(query)[0]==_T('\''); // clear the ranking if this is prefixed
        r.push_back(SourceResult(query, query, query, this, id, 0, 0));
        
        CString text(query);
        if(q[0]==_T('\''))
            text=q.Mid(1);

        r.back().object()=new TextObject(text,this);
    }
    void rate(const CString &q, SourceResult *r) {
        if(r->id()==0)
            r->rank=0;
    }
};