#include "stdafx.h"
#include "Collecter.h"
#include "JScriptSource.h"

HRESULT CollecterScript::addObject(BSTR type, BSTR key, IDispatch *args) {
    CComQIPtr<IDispatch> pdispArgs(args);
    CComQIPtr<IDispatchEx> pargs(args);

    m_pPack->begin(KV_Map);
    m_pPack->writePairUint32(L"source",(uint32)m_pSrc);
    m_pPack->writePairString(L"type",CString(type));
    m_pPack->writePairString(L"key",CString(key));

    DISPID dispid=DISPID_STARTENUM;
    while(pargs->GetNextDispID(fdexEnumAll,dispid,&dispid)==S_OK) {
        CComBSTR name;
        pargs->GetMemberName(dispid,&name);

        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        CComVariant ret;
        pargs->InvokeEx(dispid,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET,&dispparamsNoArgs,&ret,0,0);

        CString n(name);
        if(ret.vt==VT_BSTR) {
            CString r(ret);                    
            m_pPack->writePairString(n, r);
        } else if(ret.vt==VT_I4) {
            m_pPack->writePairUint32(n, ret.intVal);
        }
    }
    m_pPack->writePairUint32(L"uses", (uint32)m_pSrc->getUses(CString(key)));
    m_pPack->end();

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