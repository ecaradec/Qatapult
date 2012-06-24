

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Mon Jun 04 09:26:35 2012
 */
/* Compiler settings for qatapult.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __qatapult_h_h__
#define __qatapult_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IQatapultScript_FWD_DEFINED__
#define __IQatapultScript_FWD_DEFINED__
typedef interface IQatapultScript IQatapultScript;
#endif 	/* __IQatapultScript_FWD_DEFINED__ */


#ifndef __ICollecterScript_FWD_DEFINED__
#define __ICollecterScript_FWD_DEFINED__
typedef interface ICollecterScript ICollecterScript;
#endif 	/* __ICollecterScript_FWD_DEFINED__ */


#ifndef __IPainterScript_FWD_DEFINED__
#define __IPainterScript_FWD_DEFINED__
typedef interface IPainterScript IPainterScript;
#endif 	/* __IPainterScript_FWD_DEFINED__ */


#ifndef __IQatapultScript_FWD_DEFINED__
#define __IQatapultScript_FWD_DEFINED__
typedef interface IQatapultScript IQatapultScript;
#endif 	/* __IQatapultScript_FWD_DEFINED__ */


#ifndef __ICollecterScript_FWD_DEFINED__
#define __ICollecterScript_FWD_DEFINED__
typedef interface ICollecterScript ICollecterScript;
#endif 	/* __ICollecterScript_FWD_DEFINED__ */


#ifndef __IPainterScript_FWD_DEFINED__
#define __IPainterScript_FWD_DEFINED__
typedef interface IPainterScript IPainterScript;
#endif 	/* __IPainterScript_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IQatapultScript_INTERFACE_DEFINED__
#define __IQatapultScript_INTERFACE_DEFINED__

/* interface IQatapultScript */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IQatapultScript;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E0516B01-92D9-4501-84DB-9E9B478E475F")
    IQatapultScript : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE match( 
            /* [in] */ BSTR str,
            /* [in] */ BSTR query,
            /* [retval][out] */ VARIANT_BOOL *b) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getArgItemCount( 
            /* [in] */ INT c,
            /* [retval][out] */ INT *v) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getArgValue( 
            /* [in] */ INT c,
            /* [in] */ VARIANT name_or_index,
            /* [defaultvalue][in] */ BSTR name,
            /* [retval][out] */ VARIANT *v) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_argscount( 
            /* [retval][out] */ INT *v) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getFocus( 
            /* [retval][out] */ INT *v) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getStatus( 
            /* [in] */ int i,
            /* [retval][out] */ BSTR *text) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getQuery( 
            /* [in] */ int i,
            /* [retval][out] */ BSTR *text) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_resultscount( 
            /* [retval][out] */ INT *i) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_textmode( 
            /* [retval][out] */ VARIANT_BOOL *b) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_resultsvisible( 
            /* [retval][out] */ VARIANT_BOOL *b) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE showmenu( 
            INT x,
            INT y) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_crawlprogress( 
            /* [retval][out] */ INT *i) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getResValue( 
            /* [in] */ INT c,
            /* [in] */ BSTR name,
            /* [retval][out] */ VARIANT *v) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE setVisibleResults( 
            /* [in] */ INT i) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_firstResult( 
            /* [retval][out] */ INT *i) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_focusedResult( 
            /* [retval][out] */ INT *i) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE invalidate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getObject( 
            /* [in] */ BSTR __MIDL__IQatapultScript0000,
            /* [retval][out] */ IDispatch **pp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getDynWrapper( 
            /* [retval][out] */ IDispatch **pp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE trace( 
            /* [in] */ BSTR str) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getBuffer( 
            /* [in] */ INT l,
            /* [retval][out] */ BSTR *pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE setInput( 
            /* [in] */ IDispatch *p) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE setSkinSize( 
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE run( 
            /* [in] */ VARIANT args) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE show( 
            /* [in] */ VARIANT args) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IQatapultScriptVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IQatapultScript * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IQatapultScript * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IQatapultScript * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IQatapultScript * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IQatapultScript * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IQatapultScript * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IQatapultScript * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *match )( 
            IQatapultScript * This,
            /* [in] */ BSTR str,
            /* [in] */ BSTR query,
            /* [retval][out] */ VARIANT_BOOL *b);
        
        HRESULT ( STDMETHODCALLTYPE *getArgItemCount )( 
            IQatapultScript * This,
            /* [in] */ INT c,
            /* [retval][out] */ INT *v);
        
        HRESULT ( STDMETHODCALLTYPE *getArgValue )( 
            IQatapultScript * This,
            /* [in] */ INT c,
            /* [in] */ VARIANT name_or_index,
            /* [defaultvalue][in] */ BSTR name,
            /* [retval][out] */ VARIANT *v);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_argscount )( 
            IQatapultScript * This,
            /* [retval][out] */ INT *v);
        
        HRESULT ( STDMETHODCALLTYPE *getFocus )( 
            IQatapultScript * This,
            /* [retval][out] */ INT *v);
        
        HRESULT ( STDMETHODCALLTYPE *getStatus )( 
            IQatapultScript * This,
            /* [in] */ int i,
            /* [retval][out] */ BSTR *text);
        
        HRESULT ( STDMETHODCALLTYPE *getQuery )( 
            IQatapultScript * This,
            /* [in] */ int i,
            /* [retval][out] */ BSTR *text);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_resultscount )( 
            IQatapultScript * This,
            /* [retval][out] */ INT *i);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_textmode )( 
            IQatapultScript * This,
            /* [retval][out] */ VARIANT_BOOL *b);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_resultsvisible )( 
            IQatapultScript * This,
            /* [retval][out] */ VARIANT_BOOL *b);
        
        HRESULT ( STDMETHODCALLTYPE *showmenu )( 
            IQatapultScript * This,
            INT x,
            INT y);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_crawlprogress )( 
            IQatapultScript * This,
            /* [retval][out] */ INT *i);
        
        HRESULT ( STDMETHODCALLTYPE *getResValue )( 
            IQatapultScript * This,
            /* [in] */ INT c,
            /* [in] */ BSTR name,
            /* [retval][out] */ VARIANT *v);
        
        HRESULT ( STDMETHODCALLTYPE *setVisibleResults )( 
            IQatapultScript * This,
            /* [in] */ INT i);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_firstResult )( 
            IQatapultScript * This,
            /* [retval][out] */ INT *i);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_focusedResult )( 
            IQatapultScript * This,
            /* [retval][out] */ INT *i);
        
        HRESULT ( STDMETHODCALLTYPE *invalidate )( 
            IQatapultScript * This);
        
        HRESULT ( STDMETHODCALLTYPE *getObject )( 
            IQatapultScript * This,
            /* [in] */ BSTR __MIDL__IQatapultScript0000,
            /* [retval][out] */ IDispatch **pp);
        
        HRESULT ( STDMETHODCALLTYPE *getDynWrapper )( 
            IQatapultScript * This,
            /* [retval][out] */ IDispatch **pp);
        
        HRESULT ( STDMETHODCALLTYPE *trace )( 
            IQatapultScript * This,
            /* [in] */ BSTR str);
        
        HRESULT ( STDMETHODCALLTYPE *getBuffer )( 
            IQatapultScript * This,
            /* [in] */ INT l,
            /* [retval][out] */ BSTR *pstr);
        
        HRESULT ( STDMETHODCALLTYPE *setInput )( 
            IQatapultScript * This,
            /* [in] */ IDispatch *p);
        
        HRESULT ( STDMETHODCALLTYPE *setSkinSize )( 
            IQatapultScript * This,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *run )( 
            IQatapultScript * This,
            /* [in] */ VARIANT args);
        
        HRESULT ( STDMETHODCALLTYPE *show )( 
            IQatapultScript * This,
            /* [in] */ VARIANT args);
        
        END_INTERFACE
    } IQatapultScriptVtbl;

    interface IQatapultScript
    {
        CONST_VTBL struct IQatapultScriptVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQatapultScript_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IQatapultScript_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IQatapultScript_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IQatapultScript_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IQatapultScript_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IQatapultScript_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IQatapultScript_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IQatapultScript_match(This,str,query,b)	\
    ( (This)->lpVtbl -> match(This,str,query,b) ) 

#define IQatapultScript_getArgItemCount(This,c,v)	\
    ( (This)->lpVtbl -> getArgItemCount(This,c,v) ) 

#define IQatapultScript_getArgValue(This,c,name_or_index,name,v)	\
    ( (This)->lpVtbl -> getArgValue(This,c,name_or_index,name,v) ) 

#define IQatapultScript_get_argscount(This,v)	\
    ( (This)->lpVtbl -> get_argscount(This,v) ) 

#define IQatapultScript_getFocus(This,v)	\
    ( (This)->lpVtbl -> getFocus(This,v) ) 

#define IQatapultScript_getStatus(This,i,text)	\
    ( (This)->lpVtbl -> getStatus(This,i,text) ) 

#define IQatapultScript_getQuery(This,i,text)	\
    ( (This)->lpVtbl -> getQuery(This,i,text) ) 

#define IQatapultScript_get_resultscount(This,i)	\
    ( (This)->lpVtbl -> get_resultscount(This,i) ) 

#define IQatapultScript_get_textmode(This,b)	\
    ( (This)->lpVtbl -> get_textmode(This,b) ) 

#define IQatapultScript_get_resultsvisible(This,b)	\
    ( (This)->lpVtbl -> get_resultsvisible(This,b) ) 

#define IQatapultScript_showmenu(This,x,y)	\
    ( (This)->lpVtbl -> showmenu(This,x,y) ) 

#define IQatapultScript_get_crawlprogress(This,i)	\
    ( (This)->lpVtbl -> get_crawlprogress(This,i) ) 

#define IQatapultScript_getResValue(This,c,name,v)	\
    ( (This)->lpVtbl -> getResValue(This,c,name,v) ) 

#define IQatapultScript_setVisibleResults(This,i)	\
    ( (This)->lpVtbl -> setVisibleResults(This,i) ) 

#define IQatapultScript_get_firstResult(This,i)	\
    ( (This)->lpVtbl -> get_firstResult(This,i) ) 

#define IQatapultScript_get_focusedResult(This,i)	\
    ( (This)->lpVtbl -> get_focusedResult(This,i) ) 

#define IQatapultScript_invalidate(This)	\
    ( (This)->lpVtbl -> invalidate(This) ) 

#define IQatapultScript_getObject(This,__MIDL__IQatapultScript0000,pp)	\
    ( (This)->lpVtbl -> getObject(This,__MIDL__IQatapultScript0000,pp) ) 

#define IQatapultScript_getDynWrapper(This,pp)	\
    ( (This)->lpVtbl -> getDynWrapper(This,pp) ) 

#define IQatapultScript_trace(This,str)	\
    ( (This)->lpVtbl -> trace(This,str) ) 

#define IQatapultScript_getBuffer(This,l,pstr)	\
    ( (This)->lpVtbl -> getBuffer(This,l,pstr) ) 

#define IQatapultScript_setInput(This,p)	\
    ( (This)->lpVtbl -> setInput(This,p) ) 

#define IQatapultScript_setSkinSize(This,w,h)	\
    ( (This)->lpVtbl -> setSkinSize(This,w,h) ) 

#define IQatapultScript_run(This,args)	\
    ( (This)->lpVtbl -> run(This,args) ) 

#define IQatapultScript_show(This,args)	\
    ( (This)->lpVtbl -> show(This,args) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IQatapultScript_INTERFACE_DEFINED__ */


#ifndef __ICollecterScript_INTERFACE_DEFINED__
#define __ICollecterScript_INTERFACE_DEFINED__

/* interface ICollecterScript */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ICollecterScript;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("240BA9C5-C64D-4DA5-AC5F-65099A1CA927")
    ICollecterScript : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE addObject( 
            /* [in] */ BSTR type,
            /* [in] */ BSTR key,
            /* [in] */ IDispatch *p) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE expects( 
            /* [in] */ BSTR type,
            /* [retval][out] */ VARIANT_BOOL *b) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICollecterScriptVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICollecterScript * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICollecterScript * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICollecterScript * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICollecterScript * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICollecterScript * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICollecterScript * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICollecterScript * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *addObject )( 
            ICollecterScript * This,
            /* [in] */ BSTR type,
            /* [in] */ BSTR key,
            /* [in] */ IDispatch *p);
        
        HRESULT ( STDMETHODCALLTYPE *expects )( 
            ICollecterScript * This,
            /* [in] */ BSTR type,
            /* [retval][out] */ VARIANT_BOOL *b);
        
        END_INTERFACE
    } ICollecterScriptVtbl;

    interface ICollecterScript
    {
        CONST_VTBL struct ICollecterScriptVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICollecterScript_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICollecterScript_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICollecterScript_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICollecterScript_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICollecterScript_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICollecterScript_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICollecterScript_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICollecterScript_addObject(This,type,key,p)	\
    ( (This)->lpVtbl -> addObject(This,type,key,p) ) 

#define ICollecterScript_expects(This,type,b)	\
    ( (This)->lpVtbl -> expects(This,type,b) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICollecterScript_INTERFACE_DEFINED__ */


#ifndef __IPainterScript_INTERFACE_DEFINED__
#define __IPainterScript_INTERFACE_DEFINED__

/* interface IPainterScript */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPainterScript;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("03E86E34-8D63-49A7-86A0-023110989767")
    IPainterScript : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE drawBitmap( 
            /* [in] */ BSTR text,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE drawInput( 
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE drawText( 
            /* [in] */ BSTR text,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE drawItem( 
            /* [in] */ INT arg,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE drawSubItem( 
            /* [in] */ INT arg,
            /* [in] */ INT e,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE drawResItem( 
            /* [in] */ INT arg,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE drawEmphased( 
            /* [in] */ BSTR text,
            /* [in] */ BSTR highlight,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h,
            /* [defaultvalue][in] */ INT flag = 1) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE drawResults( 
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE fillRectangle( 
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h,
            /* [in] */ DWORD color) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_resultScrollbarColor( 
            /* [in] */ DWORD c) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_resultBgColor( 
            /* [in] */ DWORD c) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_resultFocusColor( 
            /* [in] */ DWORD c) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_textcolor( 
            /* [retval][out] */ DWORD *c) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_textcolor( 
            /* [in] */ DWORD c) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_fontsize( 
            /* [in] */ float f) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_textalign( 
            /* [in] */ DWORD c) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_fontfamily( 
            /* [in] */ BSTR str) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_textRenderingHint( 
            /* [in] */ DWORD c) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_stringTrimming( 
            /* [in] */ DWORD c) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPainterScriptVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPainterScript * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPainterScript * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPainterScript * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPainterScript * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPainterScript * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPainterScript * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPainterScript * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *drawBitmap )( 
            IPainterScript * This,
            /* [in] */ BSTR text,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *drawInput )( 
            IPainterScript * This,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *drawText )( 
            IPainterScript * This,
            /* [in] */ BSTR text,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *drawItem )( 
            IPainterScript * This,
            /* [in] */ INT arg,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *drawSubItem )( 
            IPainterScript * This,
            /* [in] */ INT arg,
            /* [in] */ INT e,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *drawResItem )( 
            IPainterScript * This,
            /* [in] */ INT arg,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *drawEmphased )( 
            IPainterScript * This,
            /* [in] */ BSTR text,
            /* [in] */ BSTR highlight,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h,
            /* [defaultvalue][in] */ INT flag);
        
        HRESULT ( STDMETHODCALLTYPE *drawResults )( 
            IPainterScript * This,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h);
        
        HRESULT ( STDMETHODCALLTYPE *fillRectangle )( 
            IPainterScript * This,
            /* [in] */ INT x,
            /* [in] */ INT y,
            /* [in] */ INT w,
            /* [in] */ INT h,
            /* [in] */ DWORD color);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_resultScrollbarColor )( 
            IPainterScript * This,
            /* [in] */ DWORD c);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_resultBgColor )( 
            IPainterScript * This,
            /* [in] */ DWORD c);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_resultFocusColor )( 
            IPainterScript * This,
            /* [in] */ DWORD c);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_textcolor )( 
            IPainterScript * This,
            /* [retval][out] */ DWORD *c);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_textcolor )( 
            IPainterScript * This,
            /* [in] */ DWORD c);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_fontsize )( 
            IPainterScript * This,
            /* [in] */ float f);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_textalign )( 
            IPainterScript * This,
            /* [in] */ DWORD c);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_fontfamily )( 
            IPainterScript * This,
            /* [in] */ BSTR str);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_textRenderingHint )( 
            IPainterScript * This,
            /* [in] */ DWORD c);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_stringTrimming )( 
            IPainterScript * This,
            /* [in] */ DWORD c);
        
        END_INTERFACE
    } IPainterScriptVtbl;

    interface IPainterScript
    {
        CONST_VTBL struct IPainterScriptVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPainterScript_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPainterScript_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPainterScript_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPainterScript_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPainterScript_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPainterScript_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPainterScript_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPainterScript_drawBitmap(This,text,x,y,w,h)	\
    ( (This)->lpVtbl -> drawBitmap(This,text,x,y,w,h) ) 

#define IPainterScript_drawInput(This,x,y,w,h)	\
    ( (This)->lpVtbl -> drawInput(This,x,y,w,h) ) 

#define IPainterScript_drawText(This,text,x,y,w,h)	\
    ( (This)->lpVtbl -> drawText(This,text,x,y,w,h) ) 

#define IPainterScript_drawItem(This,arg,x,y,w,h)	\
    ( (This)->lpVtbl -> drawItem(This,arg,x,y,w,h) ) 

#define IPainterScript_drawSubItem(This,arg,e,x,y,w,h)	\
    ( (This)->lpVtbl -> drawSubItem(This,arg,e,x,y,w,h) ) 

#define IPainterScript_drawResItem(This,arg,x,y,w,h)	\
    ( (This)->lpVtbl -> drawResItem(This,arg,x,y,w,h) ) 

#define IPainterScript_drawEmphased(This,text,highlight,x,y,w,h,flag)	\
    ( (This)->lpVtbl -> drawEmphased(This,text,highlight,x,y,w,h,flag) ) 

#define IPainterScript_drawResults(This,x,y,w,h)	\
    ( (This)->lpVtbl -> drawResults(This,x,y,w,h) ) 

#define IPainterScript_fillRectangle(This,x,y,w,h,color)	\
    ( (This)->lpVtbl -> fillRectangle(This,x,y,w,h,color) ) 

#define IPainterScript_put_resultScrollbarColor(This,c)	\
    ( (This)->lpVtbl -> put_resultScrollbarColor(This,c) ) 

#define IPainterScript_put_resultBgColor(This,c)	\
    ( (This)->lpVtbl -> put_resultBgColor(This,c) ) 

#define IPainterScript_put_resultFocusColor(This,c)	\
    ( (This)->lpVtbl -> put_resultFocusColor(This,c) ) 

#define IPainterScript_get_textcolor(This,c)	\
    ( (This)->lpVtbl -> get_textcolor(This,c) ) 

#define IPainterScript_put_textcolor(This,c)	\
    ( (This)->lpVtbl -> put_textcolor(This,c) ) 

#define IPainterScript_put_fontsize(This,f)	\
    ( (This)->lpVtbl -> put_fontsize(This,f) ) 

#define IPainterScript_put_textalign(This,c)	\
    ( (This)->lpVtbl -> put_textalign(This,c) ) 

#define IPainterScript_put_fontfamily(This,str)	\
    ( (This)->lpVtbl -> put_fontfamily(This,str) ) 

#define IPainterScript_put_textRenderingHint(This,c)	\
    ( (This)->lpVtbl -> put_textRenderingHint(This,c) ) 

#define IPainterScript_put_stringTrimming(This,c)	\
    ( (This)->lpVtbl -> put_stringTrimming(This,c) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPainterScript_INTERFACE_DEFINED__ */



#ifndef __QatapultLib_LIBRARY_DEFINED__
#define __QatapultLib_LIBRARY_DEFINED__

/* library QatapultLib */
/* [helpstring][version][uuid] */ 





EXTERN_C const IID LIBID_QatapultLib;
#endif /* __QatapultLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


