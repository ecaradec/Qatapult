#include "stdafx.h"
#include "QatapultLib\PredicateParser.h"
#include "QatapultLib\Object.h"
#include "QatapultLib\Utility.h"
#include "QatapultLib\ArrayHelper.h"

typedef unsigned char uint8;
typedef unsigned long uint32;

SUITE(PredicateParser)
{

    TEST(Type)
    {
        PredicateParser p(L"FILE");
        KVPack pack;
        pack.begin(KV_Map);
            pack.writePairString(L"type",L"FILE");
            pack.writePairString(L"key",L"c:\\file.jpg");
            pack.writePairUint32(L"source",(uint32)0);
            pack.writePairString(L"path",L"c:\\file.jpg");
            pack.writePairString(L"expand",L"c:\\file.jpg");
            pack.writePairString(L"filename",L"file.jpg");
            pack.writePairString(L"text",L"");
            pack.writePairUint32(L"bonus",100);
            pack.writePairUint32(L"uses",(uint32)0);
        pack.end();

        Object o(pack.buff);
    
        CHECK(p.match(&o));
    
        pack.clear();
    }

    TEST(MatchFileExt)
    {
        PredicateParser p(L"FILE[fileext=jpg|png]");
        KVPack pack;
        pack.begin(KV_Map);
            pack.writePairString(L"type",L"FILE");
            pack.writePairString(L"path",L"c:\\file.jpg");
        pack.end();

        Object o(pack.buff);
    
        CHECK(p.match(&o));
    
        pack.clear();
    }    

    TEST(Explode)
    {
        CHECK(Array(CString(L"md"),CString(L"txt")) == Explode(L"md|txt", L'|'));
    }
}