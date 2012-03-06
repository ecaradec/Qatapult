#pragma once
#include "AlphaGUI.h"

struct PainterScript : IDispatchImpl<IPainterScript,&__uuidof(IPainterScript),&CAtlModule::m_libid,0xFFFF,0xFFFF>, CComObjectRoot {
    BEGIN_COM_MAP(PainterScript)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPainterScript)
    END_COM_MAP()

    STDMETHOD(drawBitmap)(BSTR text,INT x, INT y, INT w, INT h) {
        m_pUI->drawBitmap(CString(text),x,y,w,h);
        return S_OK;
    }
    STDMETHOD(drawInput)(INT x, INT y, INT w, INT h) {
        m_pUI->drawInput(x,y,w,h);
        return S_OK;
    }
    STDMETHOD(drawText)(BSTR text,INT x, INT y, INT w, INT h) {        
        m_pUI->drawText(CString(text),x,y,w,h);
        return S_OK;
    }
    STDMETHOD(drawItem)(INT arg, INT x, INT y, INT w, INT h) {        
        m_pUI->drawItem(arg,x,y,w,h);
        return S_OK;
    }
    STDMETHOD(drawEmphased)(BSTR text, BSTR highlight, INT x, INT y, INT w, INT h, INT flag) {
        m_pUI->drawEmphased(CString(text),CString(highlight),flag,x,y,w,h);
        return S_OK;
    }    
    STDMETHOD(drawResults)(INT x,INT y,INT w,INT h)
    {
        m_pUI->drawResults(x,y,w,h);
        return S_OK;
    }
    STDMETHOD(get_textcolor)(DWORD *c) {
        *c=m_pUI->m_textcolor;
        return S_OK;
    }
    STDMETHOD(put_textcolor)(DWORD c) {
        m_pUI->m_textcolor=c;
        return S_OK;
    }
    STDMETHOD(put_fontsize)(float f) {
        m_pUI->m_fontsize=f;
        return S_OK;
    }
    STDMETHOD(put_textalign)(DWORD v) {
        m_pUI->m_textalign=v;
        return S_OK;
    }
    STDMETHOD(put_fontfamily)(BSTR v) {
        g_fontfamily=CString(v);
        return S_OK;
    }

    static PainterScript *Make(AlphaGUI *pUI) {
        CComObject<PainterScript> *pImpl=0;
        CComObject<PainterScript>::CreateInstance(&pImpl);
        pImpl->m_pUI=pUI;
        return pImpl;
    }

    AlphaGUI *m_pUI;
};
