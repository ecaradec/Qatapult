#pragma once
#include "TextObject.h"

struct TextSource : Source {
    TextSource() : Source(L"TEXT",L"Text (Catalog )") {
        m_ignoreemptyquery=true;
        m_prefix=L'\'';
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &r, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(L"TEXT")==activetypes.end())
            return;

         CString q(query);

        // TOFIX : 
        //int id=CString(query)[0]==_T('\''); // clear the ranking if this is prefixed
        //r.push_back(SourceResult(new Object(query, query, this, query)));
        
/*        CString text(query);
        if(q[0]==_T('\''))
            text=q.Mid(1);*/

        r.push_back(new TextObject(query,this));
        if(query[0]==_T('\''))
            r.back().bonus()=1000;
    }
    void rate(const CString &q, Object *r) {
        //if(r->display())
        //    r->rank()=0;
    }
};