#include "stdafx.h"

typedef unsigned char uint8;
typedef unsigned long uint32;

#include "..\QatapultLib\KVPack.h"
#include "..\QatapultLib\UserSource.h"


// find search in all subblock inside a block
SUITE(UserSource)
{
    TEST(loadObjectsFromJSON)
    {
        UserSource source;

        char json[]="[{\"value1\":\"a\",\"value2\":\"b\"}]";
        
        source.loadJSON(json);

        CHECK(source.m_pack.root().first().getString(L"value1") == CString(L"a"));
        CHECK(source.m_pack.root().first().getString(L"value2") == CString(L"b"));
        CHECK(source.m_pack.root().first().getInt(L"uses") == 10);
        CHECK(source.m_pack.root().first().getInt(L"bonus") == 10);
        CHECK((UserSource*)source.m_pack.root().first().getInt(L"source")==&source);
    }
}
