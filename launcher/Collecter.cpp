#include "stdafx.h"
#include "Collecter.h"
#include "JScriptSource.h"

HRESULT CollecterScript::addObject(BSTR type, BSTR key, IDispatch *args) {

    SourceResult sr;
    
    CComQIPtr<IDispatch> pdispArgs(args);

    CComQIPtr<IDispatchEx> pargs(args);

    uint8 *pobj=m_pPack->beginBlock();
    m_pPack->pack(L"source",(uint32)m_pSrc);
    m_pPack->pack(L"type",CString(type));
    m_pPack->pack(L"key",CString(key));

    DISPID dispid=DISPID_STARTENUM;
    while(pargs->GetNextDispID(fdexEnumAll,dispid,&dispid)==S_OK) {
        CComBSTR name;
        pargs->GetMemberName(dispid,&name);

        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        CComVariant ret;
        pargs->InvokeEx(dispid,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET,&dispparamsNoArgs,&ret,0,0);

        CString n(name);
        CString r(ret);        
            
        m_pPack->pack(n, r);
    }
    m_pPack->pack(L"uses", (uint32)m_pSrc->getUses(CString(key)));
    m_pPack->endBlock(pobj);

    return S_OK;
}

HRESULT CollecterScript::expects(BSTR type, VARIANT_BOOL *b) {
    *b=(m_pActiveTypes->size()==0 || m_pActiveTypes->find(CString(type))!=m_pActiveTypes->end());
    return S_OK;
}

CollecterScript *CollecterScript::Make(JScriptSource *psrc, KVPack *pPack, std::map<CString,bool> *activetypes) {
    CComObject<CollecterScript> *pImpl=0;
    CComObject<CollecterScript>::CreateInstance(&pImpl);        

    pImpl->m_pSrc=psrc;
    pImpl->m_pActiveTypes=activetypes;
    pImpl->m_pPack=pPack;
    return pImpl;
}