#pragma once
#include "Qatapult.h"

extern IDispatch *getDynamicWrapper();

struct QatapultScript : IDispatchImpl<IQatapultScript,&__uuidof(IQatapultScript),&CAtlModule::m_libid,0xFFFF,0xFFFF>, CComObjectRoot {
    BEGIN_COM_MAP(QatapultScript)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IQatapultScript)
    END_COM_MAP()

    STDMETHOD(match)(BSTR text,BSTR query,VARIANT_BOOL *b) {
        *b=::FuzzyMatch(CString(text),CString(query));
        return S_OK;
    }    
    STDMETHOD(getArgValue)(INT c, BSTR name, VARIANT *v) {
        CComVariant(m_pUI->getArgString(c,name)).Detach(v);
        return S_OK;
    }
    STDMETHOD(get_argscount)(INT *v) { 
        *v=m_pUI->getArgsCount();
        return S_OK;
    }
    STDMETHOD(getFocus)(INT *v) {
        *v=m_pUI->m_pane; 
        return S_OK;
    }
    STDMETHOD(getStatus)(INT i, BSTR *s) {
        if(i<m_pUI->m_status.size())
            *s=m_pUI->m_status[i].AllocSysString();
        return S_OK;
    }
    STDMETHOD(getQuery)(INT i,BSTR *s) {
        if(i==m_pUI->m_queries.size())
            *s=m_pUI->m_input.m_text.AllocSysString();
        else if(i>m_pUI->m_queries.size())
            *s=CString(L"").AllocSysString();
        else
            *s=m_pUI->m_queries[i].AllocSysString();
        return S_OK;
    }
    STDMETHOD(get_textmode)(VARIANT_BOOL *b) {
        *b=(m_pUI->m_editmode==1)?VARIANT_TRUE:VARIANT_FALSE;
        return S_OK;
    }
    STDMETHOD(get_resultscount)(INT *v) {
        *v=m_pUI->m_results.size();
        return S_OK;
    }
    STDMETHOD(get_resultsvisible)(VARIANT_BOOL *b) {
        // DEPRECATED
        *b=VARIANT_TRUE;
        //*b=m_pUI->m_resultsvisible?VARIANT_TRUE:VARIANT_FALSE;
        return S_OK;
    }
    STDMETHOD(showmenu)(INT x, INT y) {
        m_pUI->showMenu(x,y);
        return S_OK;
    }
    STDMETHOD(get_crawlprogress)(INT *p) {
        *p=m_pUI->m_crawlprogress;
        return S_OK;
    }
    STDMETHOD(getResValue)(INT i, BSTR name, VARIANT *v) {
        CComVariant(m_pUI->getResString(i,CString(name))).Detach(v);
        return S_OK;
    }
    STDMETHOD(setVisibleResults)(INT p) {
        m_pUI->m_visibleresultscount=p;
        return S_OK;
    }
    STDMETHOD(get_firstResult)(INT *p) {
        *p=m_pUI->m_resultspos;
        return S_OK;
    }
    STDMETHOD(get_focusedResult)(INT *p) {
        *p=m_pUI->m_focusedresult;
        return S_OK;
    }
    STDMETHOD(invalidate)() {
        m_pUI->Invalidate();
        return S_OK;
    }
    STDMETHOD(getObject)(BSTR q, IDispatch **p) {
        return ::CoGetObject(q, NULL, IID_IDispatch, (void**)p);
    }
    STDMETHOD(trace)(BSTR str) {
        OutputDebugString(CStringW(str)+L"\n");
        return S_OK;
    }
    STDMETHOD(getDynWrapper)(IDispatch **pp) {
        *pp=getDynamicWrapper();
        (*pp)->AddRef();
        return S_OK;
    }
    STDMETHOD(getBuffer)(INT l, BSTR *pstr) {
        *pstr=SysAllocStringLen(NULL,l);
        (*pstr)[0]=0;
        return S_OK;
    }
    STDMETHOD(setInput)(IDispatch *p) {
        m_pUI->setRetArg(0, getResultFromIDispatch(L"",L"",p,m_pUI->m_inputsource));
        return S_OK;
    }
    static QatapultScript *Make(Qatapult *pUI) {
        CComObject<QatapultScript> *pImpl=0;
        CComObject<QatapultScript>::CreateInstance(&pImpl);
        pImpl->m_pUI=pUI;
        return pImpl;
    }
    Qatapult *m_pUI;
};
