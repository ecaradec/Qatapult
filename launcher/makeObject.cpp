#include "stdafx.h"
#include "SourceResult.h"
#include "Object.h"
#include "ContactSource.h"
#include "TextObject.h"

SourceResult getResultFromIDispatch(const CString &type_, const CString &key, IDispatch *args,Source *src) {
    SourceResult sr(L"",L"",L"",src,0,0,0);
    
    CComQIPtr<IDispatch> pdispArgs(args);
    CComVariant vtype;
    CString type;
    if( pdispArgs.GetPropertyByName(L"type",&vtype)==S_OK ) {
        type=vtype;
    } else {
        type=type_;
    }

    if(type==L"FILE")
        sr.object=new FileObject(key,src,L"",L"",L"");
    else if(type==L"CONTACT")
        sr.object=new ContactObject(key,src,L"",L"");
    else if(type==L"TEXT")
        sr.object=new TextObject(key,src);
    else
        sr.object=new Object(key,type,src,L"");           

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
            sr.display=r;
        else if(n==L"expand")
            sr.expand=r;
        else if(n==L"bonus")
            sr.bonus=ret.intVal;

        sr.object->values[CString(name)]=CString(ret);
    }
    return sr;    
}
