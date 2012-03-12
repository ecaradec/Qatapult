#pragma once
struct CollecterScript : IDispatchImpl<ICollecterScript,&__uuidof(ICollecterScript),&CAtlModule::m_libid,0xFFFF,0xFFFF>, CComObjectRoot {
    BEGIN_COM_MAP(CollecterScript)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ICollecterScript)
    END_COM_MAP()

    STDMETHOD(addObject)(BSTR type, BSTR key, IDispatch *args) {
        m_pSourceResults->push_back(SourceResult(L"",L"",L"",m_pSrc,0,0,0));

        if(CString(type)==L"FILE")
            m_pSourceResults->back().object=new FileObject(CString(key),m_pSrc,L"",L"",L"");
        else if(CString(type)==L"CONTACT")
            m_pSourceResults->back().object=new ContactObject(CString(key),m_pSrc,L"",L"");
        else if(CString(type)==L"TEXT")
            m_pSourceResults->back().object=new TextObject(CString(key),m_pSrc);
        else
            m_pSourceResults->back().object=new Object(CString(key),CString(type),m_pSrc,L"");
        
        CComQIPtr<IDispatchEx> pargs(args);

        DISPID dispid=DISPID_STARTENUM;
        while(pargs->GetNextDispID(fdexEnumAll,dispid,&dispid)==S_OK) {
            CComBSTR name;
            pargs->GetMemberName(dispid,&name);

            DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
            CComVariant ret;
            pargs->InvokeEx(dispid,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYGET,&dispparamsNoArgs,&ret,0,0);

            CString n(name);
            CString r(ret);
            if(n==L"text")
                m_pSourceResults->back().display=r;
            else if(n==L"expand")
                m_pSourceResults->back().expand=r;
            else if(n==L"bonus")
                m_pSourceResults->back().bonus=ret.intVal;

            m_pSourceResults->back().object->values[CString(name)]=CString(ret);
        }
        
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
