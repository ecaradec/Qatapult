#include "stdafx.h"

typedef unsigned char uint8;
typedef unsigned long uint32;

#include "KVPack.h"

TEST(KVPack_getValue)
{
    KVPack pack;
    uint8 *b=pack.beginBlock();
    uint8 *o=pack.beginBlock();
    pack.pack(L"value",L"b");
    pack.endBlock(o);
    pack.endBlock(b);

    CHECK_EQUAL(CString(L"b"), KVObject(o).getString(L"value"));
}

TEST(KVPack_getMissingValueReturnsNull)
{
    KVPack pack;
    uint8 *b=pack.beginBlock();
    uint8 *o=pack.beginBlock();
    pack.pack(L"value",L"b");
    pack.endBlock(o);
    pack.endBlock(b);

    CHECK_EQUAL((TCHAR*)0, KVObject(o).getString(L"othervalue"));
}

TEST(KVPack_intType)
{
    KVPack pack;
    uint8 *b=pack.beginBlock();
    uint8 *o=pack.beginBlock();
    pack.pack(L"value",123);
    pack.endBlock(o);
    pack.endBlock(b);

    //CHECK_EQUAL(CString(L"123"), KVObject(o).getString(L"value"));
    CHECK_EQUAL(123, KVObject(o).getInt(L"value"));
}
