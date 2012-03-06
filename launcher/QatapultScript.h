#pragma once
#include "AlphaGUI.h"

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
        *b=m_pUI->m_resultsvisible?VARIANT_TRUE:VARIANT_FALSE;
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
    static QatapultScript *Make(AlphaGUI *pUI) {
        CComObject<QatapultScript> *pImpl=0;
        CComObject<QatapultScript>::CreateInstance(&pImpl);
        pImpl->m_pUI=pUI;
        return pImpl;
    }
    AlphaGUI *m_pUI;
};
