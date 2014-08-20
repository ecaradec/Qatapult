#include "stdafx.h"

typedef unsigned char uint8;
typedef unsigned long uint32;

#include "..\QatapultLib\KVPack.h"

// find search in all subblock inside a block
SUITE(KVPack)
{
    TEST(getValue)
    {
        KVPack pack;
        pack.begin(KV_Map);
            pack.writePairString(L"value",L"b");
            pack.writePairString(L"value2",L"2");
        pack.end();

        CHECK_EQUAL(CString(L"b"), KVObject(pack.buff).getString(L"value"));
        CHECK_EQUAL(CString(L"2"), KVObject(pack.buff).getString(L"value2"));
    }

    TEST(getMissingValueReturnsNull)
    {
        KVPack pack;
        pack.begin(KV_Map);
            pack.writePairString(L"value",L"b");
        pack.end();

        CHECK_EQUAL((TCHAR*)0, KVObject(pack.buff).getString(L"othervalue"));
    }

    TEST(intType)
    {
        KVPack pack;
        pack.begin(KV_Map);
            pack.writePairUint32(L"value",123);
        pack.end();

        //CHECK_EQUAL(CString(L"123"), KVObject(o).getString(L"value"));
        CHECK_EQUAL(123, KVObject(pack.buff).getInt(L"value"));
        CHECK_EQUAL(0, KVObject(pack.buff).getInt(L"value_doesnt_exists"));
    }

    TEST(objectType)
    {
        KVPack pack;
        pack.begin(KV_Map);    
            pack.begin(KV_Pair);
                pack.writeString(L"value");
                pack.begin(KV_Map);
                    pack.writePairString(L"0",L"100");
                    pack.writePairString(L"1",L"101");
                pack.end();
            pack.end();
        pack.end();

        CHECK_EQUAL(CString(L"100"), KVObject(pack.buff).getValue(L"value").getString(L"0"));
    }

    TEST(writePairObject)
    {
        KVPack obj;
        obj.begin(KV_Map);    
            obj.writePairString(L"key",L"value");
        obj.end();


        KVPack pack;
        pack.begin(KV_Map);    
            pack.writePairObject(L"obj",obj.root());
        pack.end();

        CHECK_EQUAL(CString(L"value"), pack.root().getValue(L"obj").getString(L"key"));
    }

    // write a test for object larger than 256 characters
    TEST(readArrayOfObjects)
    {
        KVPack pack;
        pack.begin(KV_Array);
        for(int i=0;i<100;i++) {
            pack.begin(KV_Map);
                pack.writePairString(L"value",L"0");
                pack.writePairUint32(L"nb",0);
                pack.writePairString(L"text",L"a");
            pack.end();
        }
        pack.end();

    //    pack.debug();
    
        /*for(KVObject o=pack.root().first(); o!=pack.root().end(); o=o.next()) {
            CString s=o.getString(L"text");
            CString v=o.getString(L"value");
        }*/
    }
    TEST(objectToJSON)
    {
        KVPack pack;
        pack.begin(KV_Map);
            pack.writePairString(L"value",L"b");
            pack.begin(KV_Pair);
                pack.writeString(L"value2");
                pack.begin(KV_Map);
                    pack.writePairString(L"0",L"100");
                    pack.writePairString(L"1",L"101");
                pack.end();
            pack.end();
        pack.end();
        //_tprintf(L"%s", pack.root().toJSON());
        CHECK(CString(L"{\"value\":\"b\",\"value2\":{\"0\":\"100\",\"1\":\"101\"}}") == pack.root().toJSON());
    }

    TEST(objectToXML)
    {
        KVPack pack;
        pack.begin(KV_Map);
            pack.writePairString(L"value",L"b");
            pack.writePairString(L"value2",L"2");
        pack.end();
        //_tprintf(L"%s", pack.root().toXML());
        CHECK(CString(L"<value>b</value><value2>2</value2>") == pack.root().toXML());
    }

    void loadJSON(KVPack &pack, char *json) {
        
        char *errorPos = 0;
	    char *errorDesc = 0;
	    int errorLine = 0;
	    block_allocator allocator(1 << 10);
	            
        json_value *root = json_parse(json, &errorPos, &errorDesc, &errorLine, &allocator);
        if( root->type != JSON_ARRAY ) return;
        pack.begin(KV_Array);
        for (json_value *it = root->first_child; it; it = it->next_sibling) {
            if(it->type!=JSON_OBJECT) continue;
            pack.begin(KV_Map);
            for (json_value *it2 = it->first_child; it2; it2 = it2->next_sibling) {
                if(it2->type!=JSON_STRING) continue;                
                pack.writePairString(UTF8toUTF16(it2->name),UTF8toUTF16(it2->string_value));
            }
            pack.end();
        }
        pack.end();
    }

    TEST(jsonToObject)
    {        
        char json[]="[{\"value1\":\"a\",\"value2\":\"b\"}]";
        
        KVPack pack;
        loadJSON(pack,json);

        CHECK(pack.root().first().getString(L"value1") == CString(L"a"));
    }
}
