#include "stdafx.h"
#include "SourceResult.h"
#include "Object.h"
#include "ContactSource.h"
#include "TextObject.h"
#include "FileObject.h"

SourceResult getResultFromFilePath(const CString &path, Source *s) {
    CString filename=path.Right(path.GetLength() - (path.ReverseFind(L'\\')+1));
    CString lpath=path;
    if( (GetFileAttributes(path)&FILE_ATTRIBUTE_DIRECTORY)!=0 && path.Right(1)!=L"\\") {
        lpath=lpath+"\\";
    }
    
    KVPack pack;
    pack.begin(KV_Map);
        pack.writePairString(L"type",L"FILE");
        pack.writePairUint32(L"source",(uint32)s);
        pack.writePairString(L"key",lpath);
        pack.writePairString(L"text",filename);
        pack.writePairString(L"expand",filename);
        pack.writePairString(L"path",lpath);
        pack.writePairUint32(L"uses",0);
        pack.writePairUint32(L"bonus",(uint32)0);
    pack.end();

    return SourceResult(new Object(pack.root().pobj,true));
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
    pugi::xpath_node_set ns=xml.select_nodes("*");
    
    KVPack pack;
    pack.begin(KV_Map);
        pack.writePairUint32(L"source",(uint32)src);
    for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++) {        
        pack.writePairString(UTF8toUTF16(it->node().name()), UTF8toUTF16( it->node().value() ));
    }        
    pack.end();

    sr.object().reset(new Object(pack.root().pobj,true));

    return sr;
}