#pragma once
#include "Qatapult.h"

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
        m_pUI->drawItem(arg,0,x,y,w,h);
        return S_OK;
    }
    STDMETHOD(drawSubItem)(INT arg, INT e, INT x, INT y, INT w, INT h) {        
        m_pUI->drawItem(arg,e,x,y,w,h);
        return S_OK;
    }
    STDMETHOD(drawResItem)(INT i, INT x, INT y, INT w, INT h) {        
        m_pUI->drawResItem(i,x,y,w,h);
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
    STDMETHOD(fillRectangle)(INT x,INT y,INT w,INT h,DWORD c)
    {
        m_pUI->fillRectangle(x,y,w,h,c);
        return S_OK;
    }
    STDMETHOD(get_textcolor)(DWORD *c) {
        *c=g_textcolor;
        return S_OK;
    }
    STDMETHOD(put_textcolor)(DWORD c) {
        g_textcolor=c;
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
    STDMETHOD(put_textRenderingHint)(DWORD c) {
        m_pUI->m_textrenderinghint=c;
        return S_OK;
    }
    STDMETHOD(put_stringTrimming)(DWORD c) {
        m_pUI->m_stringtrimming=c;
        return S_OK;
    }
    STDMETHOD(put_resultScrollbarColor)(DWORD c) {
        m_pUI->m_resultscrollbarcolor=c;
        return S_OK;
    }
    STDMETHOD(put_resultFocusColor)(DWORD c) {
        m_pUI->m_resultfocuscolor=c;
        return S_OK;
    }
    STDMETHOD(put_resultBgColor)(DWORD c) {
        m_pUI->m_resultbgcolor=c;
        return S_OK;
    }

    static PainterScript *Make(Qatapult *pUI) {
        CComObject<PainterScript> *pImpl=0;
        CComObject<PainterScript>::CreateInstance(&pImpl);
        pImpl->m_pUI=pUI;
        return pImpl;
    }

    Qatapult *m_pUI;
};
