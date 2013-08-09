#include "stdafx.h"
#include "SourceResult.h"
#include "Object.h"
#include "ContactSource.h"
#include "TextObject.h"

SourceResult getResultFromIDispatch(const CString &type_, const CString &key_, IDispatch *args,Source *src) {
    SourceResult sr;
    
    CComQIPtr<IDispatch> pdispArgs(args);
    CComVariant vtype, vkey;
    CString type, key;
    type = pdispArgs.GetPropertyByName(L"type",&vtype)==S_OK ? vtype : type_;
    key  = pdispArgs.GetPropertyByName(L"key",&vkey)==S_OK ? vkey : key_;

    if(type==L"FILE") {
        
        CComVariant expand;
        pdispArgs.GetPropertyByName(L"expand",&expand);

        CComVariant path;
        pdispArgs.GetPropertyByName(L"path",&path);

        sr.object().reset(new FileObject(key,src,L"",expand,path));
    } else if(type==L"CONTACT")
        sr.object().reset(new ContactObject(key,src,L"",L""));
    else if(type==L"TEXT")
        sr.object().reset(new TextObject(key,src));
    else
        sr.object().reset(new Object(key,type,src,L""));

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
            sr.display()=r;
        else if(n==L"expand")
            sr.expand()=r;
        else if(n==L"bonus")
            sr.bonus()=ret.intVal;

        sr.object()->values[CString(name)]=CString(ret);
    }
    return sr;    
}

SourceResult getResultFromFilePath(const CString &path, Source *s) {
    CString filename=path.Right(path.GetLength() - (path.ReverseFind(L'\\')+1));
    
    return SourceResult(new FileObject(path, s, filename, filename, path));
}
/*
<object>
  <key>b162fd99e69ab1e6347390aebddcc62b</key>
  <expand>Mozilla Firefox</expand>
  <icon></icon>
  <path>C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Mozilla Firefox.lnk</path>
  <text>Mozilla Firefox</text>
</object>
*/
SourceResult getResultFromXML(pugi::xml_node &xml, Source *src) {
    SourceResult sr;
    
    CString type=UTF8toUTF16( xml.attribute("type").value() );
    CString key=UTF8toUTF16( xml.attribute("key").value() );
    sr.object().reset(new Object(key,type,src,L""));

    pugi::xpath_node_set ns=xml.select_nodes("*");
    for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++) {        
        sr.object()->values[ UTF8toUTF16(it->node().name()) ] = UTF8toUTF16( it->node().value() );
    }
    
   /* 
    if(type==L"FILE")
        sr.object()=new FileObject(key,src,L"",L"",L"");
    else if(type==L"CONTACT")
        sr.object()=new ContactObject(key,src,L"",L"");
    else if(type==L"TEXT")
        sr.object()=new TextObject(key,src);
    else
        sr.object()=new Object(key,type,src,L"");

        */
    return sr;
}