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
    STDMETHOD(getArgValue)(INT c, VARIANT name_or_index, BSTR name, VARIANT *v) {        
        if(name_or_index.vt==VT_BSTR)
            CComVariant(m_pUI->getArgString(c,0, name_or_index.bstrVal)).Detach(v);
        else if(name_or_index.vt==VT_I4)
            CComVariant(m_pUI->getArgString(c,name_or_index.intVal, name)).Detach(v);
        return S_OK;
    }
    STDMETHOD(getArgItemCount)(INT c, INT *v) {
        *v=(c<m_pUI->m_args.size()) ? m_pUI->m_args[c].m_results.size() : 0;
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
        m_pUI->invalidate();
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
    STDMETHOD(run)(VARIANT args) {
        if(args.vt!=VT_DISPATCH) return S_FALSE;
        
        KVPack pack;
        kvFromVariant(pack,args.pdispVal);

        PostMessage(m_pUI->m_hwnd, WM_RUNRULE, (WPARAM)pack.buff, 0);
        return S_OK;
    }
    STDMETHOD(show)(VARIANT args) {
        if(args.vt!=VT_DISPATCH) return S_FALSE;

        KVPack pack;
        kvFromVariant(pack,args.pdispVal);

        PostMessage(m_pUI->m_hwnd, WM_SHOWRULE, (WPARAM)pack.buff, 0);
        return S_OK;
    }
    uint8 *kvFromVariant(KVPack &pack, IDispatch *disp) {
        CComQIPtr<IDispatch> pdispArgs(disp);
        CComQIPtr<IDispatchEx> pargs(disp);
        
        pack.begin(KV_Map);
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
                pack.writePairString(n, r);
            } else if(ret.vt==VT_I4) {
                pack.writePairUint32(n, ret.intVal);
            } else if(ret.vt==VT_DISPATCH) {
                kvFromVariant(pack,ret.pdispVal);
            }
        }
        pack.end();

        return pack.buff;
    }
    STDMETHOD(setSkinSize)(INT w, INT h) {
        m_pUI->m_buffer.Destroy();
        m_pUI->m_buffer.Create(640,800,32,PixelFormat32bppARGB);
        return S_OK;
    }
    STDMETHOD(exec)(BSTR path, BSTR args,BSTR dir) {
        //TCHAR buff[4096];
        //GetCurrentDirectory(sizeof(buff),buff);
        ShellExecute(0, 0, CString(path), CString(args), CString(dir), SW_SHOWDEFAULT);
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
