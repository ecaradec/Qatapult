#pragma once
#include "makeObject.h"
#include "qatapult_h.h"

struct JScriptSource;

struct CollecterScript : IDispatchImpl<ICollecterScript,&__uuidof(ICollecterScript),&CAtlModule::m_libid,0xFFFF,0xFFFF>, CComObjectRoot {
    BEGIN_COM_MAP(CollecterScript)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ICollecterScript)
    END_COM_MAP()

    STDMETHOD(addObject)(BSTR type, BSTR key, IDispatch *args);
    STDMETHOD(expects)(BSTR type, VARIANT_BOOL *b);
    static CollecterScript *Make(JScriptSource *psrc, KVPack *pPack, std::map<CString,bool> *activetypes);

    JScriptSource             *m_pSrc;
    std::map<CString,bool>    *m_pActiveTypes;
    KVPack                    *m_pPack;
};
