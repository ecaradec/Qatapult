#pragma once
#include "makeObject.h"

struct CollecterScript : IDispatchImpl<ICollecterScript,&__uuidof(ICollecterScript),&CAtlModule::m_libid,0xFFFF,0xFFFF>, CComObjectRoot {
    BEGIN_COM_MAP(CollecterScript)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ICollecterScript)
    END_COM_MAP()

    STDMETHOD(addObject)(BSTR type, BSTR key, IDispatch *args) {
        m_pSourceResults->push_back(getResultFromIDispatch(type,key,args,m_pSrc));        
        return S_OK;
    }
    STDMETHOD(expects)(BSTR type, VARIANT_BOOL *b) {
        *b=(m_pActiveTypes->size()==0 || m_pActiveTypes->find(CString(type))!=m_pActiveTypes->end());
        return S_OK;
    }

    static CollecterScript *Make(Source *psrc,std::vector<SourceResult> *pSourceResults, std::map<CString,bool> *activetypes) {
        CComObject<CollecterScript> *pImpl=0;
        CComObject<CollecterScript>::CreateInstance(&pImpl);        

        pImpl->m_pSrc=psrc;
        pImpl->m_pActiveTypes=activetypes;
        pImpl->m_pSourceResults=pSourceResults;
        return pImpl;
    }

    Source                    *m_pSrc;
    std::map<CString,bool>    *m_pActiveTypes;
    std::vector<SourceResult> *m_pSourceResults;
};
