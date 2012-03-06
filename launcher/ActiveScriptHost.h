// ActiveScriptHost.cpp : Defines the entry point for the console application.
#pragma once
#include <atlsafe.h>
#include "activdbg.h"
#include "activscp.h"

bool inline Assert(bool pred, char *file, int line, TCHAR *msg)
{
    if(pred)
        return true;

    //printf("Assertion failed at %s(%d) : ",file,line);
    //_tprintf(_T("%s\n"),msg);
    
    return false;
}

bool inline AssertSucceeded(HRESULT hr, char *file, int line, TCHAR *msg)
{
    if(SUCCEEDED(hr))
    {
        return true;
    }

    //printf("Assertion failed at %s(%d) with hr=%x: ",file,line,hr);
    //_tprintf(_T("%s\n"),msg);
    
    return false;
}

#define Q_ASSERT(x) ( Assert( x, __FILE__, __LINE__, _T(#x) ) )
#define Q_ASSERT_SUCCEEDED(x) AssertSucceeded( x, __FILE__, __LINE__, _T(#x) )

struct ActiveScriptHost : IActiveScriptSite, IActiveScriptSiteDebug
{
    ActiveScriptHost()
    {
    }
    ~ActiveScriptHost()
    {
        Reset();
    }
    void Reset() {
        m_pDDH=0;
        m_pPDM=0;
        m_pDA=0;
        m_pAS=0;

        for(std::map<CString, IDispatch*>::iterator it=m_objects.begin(); it!=m_objects.end(); it++)
            it->second->Release();
    }
    HRESULT Initialize(const wchar_t *appName, const wchar_t *engineName)
    {
        HRESULT hr;  

        // debug        
        DWORD dwAppCookie=0;

        // debugdochelper
        if(m_pPDM.CoCreateInstance(CLSID_ProcessDebugManager)==S_OK) {            
            if(!Q_ASSERT_SUCCEEDED(hr=m_pPDM->GetDefaultApplication(&m_pDA)))
                return hr;
            if(!Q_ASSERT_SUCCEEDED(hr=m_pDA->SetName(appName)))
                return hr;
            if(!Q_ASSERT_SUCCEEDED(hr=m_pPDM->AddApplication(m_pDA, &dwAppCookie)))
                return hr;
        }

        // activescriptsite initialisation
        if(!Q_ASSERT_SUCCEEDED(hr=m_pAS.CoCreateInstance(engineName)))
            return hr;

        CComQIPtr<IActiveScriptParse> pASP(m_pAS);
        if(!Q_ASSERT_SUCCEEDED(hr=pASP->InitNew()))
            return hr;
        if(!Q_ASSERT_SUCCEEDED(hr=m_pAS->SetScriptSite(this)))
            return hr;
        if(!Q_ASSERT_SUCCEEDED(hr=m_pAS->SetScriptState(SCRIPTSTATE_CONNECTED)))
            return hr;

        return S_OK;
    }
    HRESULT ParseScriptText(const wchar_t *code, const wchar_t *name)
    {
        CComPtr<IDebugDocumentHelper> pDDH;
        DWORD context=0;
        HRESULT hr=0;
        
        if(m_pPDM) {
            if(!Q_ASSERT_SUCCEEDED(m_pPDM->CreateDebugDocumentHelper(0, &pDDH)))
                return hr;
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->Init(m_pDA, name, name, TEXT_DOC_ATTR_READONLY)))
                return hr;
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->Attach(0)))
                return hr;
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->AddUnicodeText(code)))
                return hr;
            DWORD len=wcslen(code);
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->DefineScriptBlock(0, len, m_pAS, FALSE, &context)))
                return hr;

            m_DDHList[context]=pDDH.p;
            pDDH.p->AddRef();
        }        

        CComQIPtr<IActiveScriptParse> pASP(m_pAS);
        if(!Q_ASSERT_SUCCEEDED(hr=pASP->ParseScriptText(code,NULL,NULL,NULL,context, 0,SCRIPTTEXT_HOSTMANAGESSOURCE|SCRIPTTEXT_ISVISIBLE|SCRIPTTEXT_ISPERSISTENT,NULL,NULL)))
            return hr;
        return S_OK;
    }
    HRESULT Eval(const wchar_t *expr, VARIANT *pV)
    {
        CComPtr<IDebugDocumentHelper> pDDH;
        DWORD context=0;
        HRESULT hr=0;
        
        if(m_pPDM) {
            if(!Q_ASSERT_SUCCEEDED(m_pPDM->CreateDebugDocumentHelper(0, &pDDH)))
                return hr;          
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->Init(m_pDA, L"expression", L"expression", TEXT_DOC_ATTR_READONLY)))
                return hr;
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->Attach(0)))
                return hr;
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->AddUnicodeText(expr)))
                return hr;
            DWORD len=wcslen(expr);
            if(!Q_ASSERT_SUCCEEDED(hr=pDDH->DefineScriptBlock(0, len, m_pAS, FALSE, &context)))
                return hr;


            m_DDHList[context]=pDDH.p;
            pDDH.p->AddRef();
        }

        CComQIPtr<IActiveScriptParse> pASP(m_pAS);
        if(!Q_ASSERT_SUCCEEDED(hr=pASP->ParseScriptText(expr,NULL,NULL,NULL,context,0,SCRIPTTEXT_HOSTMANAGESSOURCE|SCRIPTTEXT_ISVISIBLE|SCRIPTTEXT_ISEXPRESSION,pV,NULL))) {
            return hr;
        }
        return S_OK;        
    }
    HRESULT AddObject(const wchar_t *name, IDispatch *p)
    {      
        HRESULT hr;
        m_objects[CString(name)]=p;
        p->AddRef();
        if(!Q_ASSERT_SUCCEEDED(hr=m_pAS->AddNamedItem(name, SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE)))
            return hr;
        return S_OK;
    }
    HRESULT Run(LPCOLESTR method, SAFEARRAY **ppAry, VARIANT *pR)
    {
        if(!Q_ASSERT((*ppAry)->cDims==1 && (*ppAry)->rgsabound[0].cElements>0))
            return S_FALSE;

        HRESULT hr;
        CComPtr<IDispatch> pSD;
        m_pAS->GetScriptDispatch(NULL, &pSD);

        CComSafeArray<VARIANT> ary;
        for(int i=(*ppAry)->rgsabound[0].cElements-1; i!=-1; i--)
        {
            ary.Add(((VARIANT*)(*ppAry)->pvData)[i]);
        }

        if(!Q_ASSERT_SUCCEEDED(hr=pSD.InvokeN(method, (VARIANT*)ary.m_psa->pvData, ary.m_psa->rgsabound[0].cElements, pR))) {
            return hr;
        }

        return S_OK;
    }
    virtual void OnError(int line, int pos, wchar_t *txt)
    {
        OutputDebugString(txt);
    }

    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
    {
        *ppvObject = 0;
        if( InlineIsEqualGUID(riid, __uuidof(IUnknown)))
        {
            *ppvObject = (IActiveScriptSite*)this;
            static_cast<IUnknown*>(*ppvObject)->AddRef();
            return S_OK;
        }
        else if( InlineIsEqualGUID(riid, __uuidof(IActiveScriptSite)))   
        {
            *ppvObject = (IActiveScriptSite*)this;
            static_cast<IActiveScriptSite*>(*ppvObject)->AddRef();
            return S_OK;
        }
        else if(InlineIsEqualGUID(riid, __uuidof(IActiveScriptSiteDebug)) && m_pPDM)   
        {
            *ppvObject = (IActiveScriptSiteDebug*)this;
            static_cast<IActiveScriptSiteDebug*>(*ppvObject)->AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }
    ULONG __stdcall AddRef()
    {
        return 1;
    }
    ULONG __stdcall Release()
    {
        return 1;
    }
    STDMETHODIMP GetLCID ( LCID *plcid)
    {
        return S_OK;  
    }
    STDMETHODIMP GetItemInfo ( LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti)
    {
        if(m_objects.find(CString(pstrName))!=m_objects.end())
        {
            *ppiunkItem=m_objects[CString(pstrName)];
            (*ppiunkItem)->AddRef();
            return S_OK;
        }
        return TYPE_E_ELEMENTNOTFOUND;
    }
    STDMETHODIMP GetDocVersionString ( BSTR *pbstrVersion)
    {
        return S_OK;
    }
    STDMETHODIMP OnScriptTerminate ( const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo)
    {
        return S_OK;
    }
    STDMETHODIMP OnStateChange ( SCRIPTSTATE ssScriptState)
    {
        return S_OK;
    }
    STDMETHODIMP OnScriptError ( IActiveScriptError *pASE)
    {
        HRESULT hr;

        CComBSTR  parseError;
        DWORD     ctx;
        ULONG     charPos;
        ULONG     lineNum;
        ULONG     numChar;
        ULONG     column;
        EXCEPINFO expi;

        // not a parse error, onscripterror is called despite pfCallOnScriptErrorWhenContinuing=FALSE in OnScriptErrorDebug
        if(FAILED(pASE->GetSourceLineText(&parseError)))
            return S_OK;

        if(!Q_ASSERT_SUCCEEDED(hr=pASE->GetSourcePosition(&ctx, &lineNum, (LONG*)&charPos)))
            return hr;

        if(!Q_ASSERT_SUCCEEDED(hr=pASE->GetExceptionInfo(&expi)))
            return hr;


        wchar_t buff[4096];
        wsprintfW(buff, L"%s\n%s\n\n%s\n", expi.bstrSource, expi.bstrDescription, parseError.m_str);

        OnError(lineNum, charPos, buff);
        
        return S_OK;
    }
    STDMETHODIMP OnEnterScript ()
    {
        return S_OK;
    }
    STDMETHODIMP OnLeaveScript ( )
    {
        return S_OK;
    }

    STDMETHODIMP GetDocumentContextFromPosition(DWORD dwSourceContext,ULONG uCharacterOffset,ULONG uNumChars,IDebugDocumentContext **ppsc)
    {
        if(!m_pPDM)
            return S_FALSE;

        ULONG ulStartPos=0;
        HRESULT hr;
        IDebugDocumentHelper *pDDH=m_DDHList[dwSourceContext];
        if(!Q_ASSERT_SUCCEEDED(hr=pDDH->GetScriptBlockInfo(dwSourceContext, 0, &ulStartPos,0)))
            return hr;           
        if(!Q_ASSERT_SUCCEEDED(hr=pDDH->CreateDebugDocumentContext(ulStartPos + uCharacterOffset, uNumChars, ppsc)))
            return hr;
        return S_OK;
    }
    STDMETHODIMP GetApplication( IDebugApplication **ppda)
    {
        *ppda=m_pDA;
        (*ppda)->AddRef();
        return S_OK;
    }
    STDMETHODIMP GetRootApplicationNode( IDebugApplicationNode **ppdanRoot)
    {
        return S_OK;
    }
    STDMETHODIMP OnScriptErrorDebug( IActiveScriptErrorDebug *pErrorDebug, BOOL *pfEnterDebugger, BOOL *pfCallOnScriptErrorWhenContinuing)
    {
        HRESULT hr;

        *pfEnterDebugger = TRUE;                    // debugger
        *pfEnterDebugger = FALSE;
        //*pfCallOnScriptErrorWhenContinuing = TRUE;
        *pfCallOnScriptErrorWhenContinuing = FALSE;
            
        EXCEPINFO ei;
        if(!Q_ASSERT_SUCCEEDED(hr=pErrorDebug->GetExceptionInfo(&ei)))
            return hr;
        
        CComPtr<IDebugDocumentContext> pDDC;
        if(!Q_ASSERT_SUCCEEDED(hr=pErrorDebug->GetDocumentContext(&pDDC)))
            return hr;
        
        CComPtr<IDebugDocument> pDD;
        if(!Q_ASSERT_SUCCEEDED(hr=pDDC->GetDocument(&pDD)))
            return hr;

        ULONG pos;
        ULONG len;
        CComQIPtr<IDebugDocumentText> pDDT(pDD);
        if(!Q_ASSERT_SUCCEEDED(hr=pDDT->GetPositionOfContext(pDDC, &pos, &len)))
            return hr;

        ULONG line;
        ULONG offset;
        if(!Q_ASSERT_SUCCEEDED(hr=pDDT->GetLineOfPosition(pos, &line, &offset)))
            return hr;
       
        wchar_t brokenStatement[4096]={0};
        if(!Q_ASSERT_SUCCEEDED(hr=pDDT->GetText(pos, brokenStatement, 0, &len, len)))
            return hr;

        wchar_t buff[4096];
        wsprintfW(buff, L"%s (%d)\n%s\n\n%s\n", ei.bstrSource, line, ei.bstrDescription, brokenStatement);

        OnError(0, 0, buff);
                
        return S_OK;
    }

    std::map<CString, IDispatch*> m_objects;
    std::map<DWORD,IDebugDocumentHelper*> m_DDHList;    

    CComPtr<IDebugDocumentHelper> m_pDDH;
    CComPtr<IProcessDebugManager> m_pPDM;
    CComPtr<IDebugApplication>    m_pDA;
    CComPtr<IActiveScript>        m_pAS;
};
