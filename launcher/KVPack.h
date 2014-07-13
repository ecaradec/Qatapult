#pragma once
#include "PredicateParser.h"

typedef unsigned char uint8;
typedef unsigned long uint32;

enum KV_Type {
    KV_String,
    KV_Integer,
    KV_Map,
    KV_Pair,
    KV_Array
};

// change object for first, end, next, !=
struct KVObject {
    KVObject(uint8 *o) : pobj(o) {}
    uint32 length() {
        if(pobj==0) return 0;
        return *(uint32*)pobj;
    }
    uint32 type() {
        return *(pobj+sizeof(uint32));
    }
    KVObject end() {
        if(pobj==0) return 0;
        return KVObject(pobj+length());
    }
    uint8 *data() {
        if(pobj==0) return 0;
        return pobj+sizeof(uint32)+sizeof(uint8);
    }
    KVObject first() {
        return KVObject(data());
    }
    KVObject next() {
        if(!pobj) return 0;
        return pobj+length();
    }
    bool const operator==(const KVObject &rhs) const {
        return pobj==rhs.pobj;
    }
    bool const operator!=(const KVObject &rhs) const {
        return pobj!=rhs.pobj;
    }
    uint32 getInt(const TCHAR *key) {
        // current object has to be a map
        uint32 *i=(uint32*)getValue(key).data();
        if(!i) return 0;
        return *i;
    }
    const TCHAR *getString(const TCHAR *key) {
        // current object has to be a map
        static TCHAR buff[256];
        KVObject v=getValue(key);
        if(v.pobj && v.type()==KV_Integer) {            
             _stprintf(buff,L"%d",*(uint32*)v.data());
             return buff;
        }
        return (const TCHAR*)v.data();
    }
    KVObject getValue(const TCHAR *key) {
        // current object has to be a map
        return getPair(key).first().next();
    }
    KVObject getPair(const TCHAR *key) {
        // current object has to be a map
        for(KVObject p=first(); p!=end(); p=p.next()) {
            KVObject k=p.data();
            if(_tcscmp(key,(TCHAR*)k.data())==0)
                return p;
        }
        return 0;
    }
    KVObject findObject(const TCHAR *key) {
        // current object has to be an array
        for(KVObject k=first(); k!=end(); k=k.next()) {
            return k.getValue(key);
        }
        return KVObject(0);
    }
    KVObject clone() {
        uint8 *pClone=(uint8*)malloc(length());
        memcpy(pClone, pobj, length());            
        return KVObject(pClone);
    }
    void pad(int space) {
        while(space--) printf(" ");
    }
    void print(int level=0) {
        pad(level); printf("%d:",length());
        switch(type()) {
            case KV_String:
                printf("#String: %s\n", data());
                break;
            case KV_Integer:
                printf("#Integer: %d\n", *(uint32*)data());
                break;
            case KV_Map:
                printf("#Map:\n");
                for(KVObject k=first(); k!=end(); k=k.next())
                    k.print(level+1);
                break;
            case KV_Pair:
                printf("#Pair:\n");
                for(KVObject k=first(); k!=end(); k=k.next())
                    k.print(level+1);
                break;
            case KV_Array:
                printf("#Array:\n");                
                for(KVObject k=first(); k!=end(); k=k.next())
                    k.print(level+1);
                break;
        }
    }
    CString toXML() {
        CString str;
        switch(type()) {
            case KV_String:
                str+=CString((TCHAR*)data());
                break;
            case KV_Integer:
                str+=ItoS(*(uint32*)data());
                break;
            case KV_Map:
                for(KVObject k=first(); k!=end(); k=k.next())
                    str+=k.toXML();
                break;
            case KV_Pair:
                str+=L"<"+first().toXML()+L">";
                str+=first().next().toXML();
                str+=L"</"+first().toXML()+L">";
                break;
            case KV_Array:
                // not truly representable
                for(KVObject k=first(); k!=end(); k=k.next())
                    str+=k.toXML();
                break;
        }
        return str;
    }
    CString toJSON() {
        CString str;
        switch(type()) {
            case KV_String:
                str+=L"\""+CString((TCHAR*)data())+L"\"";
                break;
            case KV_Integer:
                str+=ItoS(*(uint32*)data());
                break;
            case KV_Map:
                str+=L"{";
                for(KVObject k=first(); k!=end(); k=k.next())
                    str+=k.toJSON()+(k.next()!=end()?L",":L"");
                str+=L"}";
                break;
            case KV_Pair:
                str+=first().toJSON();
                str+=L":";
                str+=first().next().toJSON();
                break;
            case KV_Array:
                str+=L"[";
                for(KVObject k=first(); k!=end(); k=k.next())
                    str+=k.toJSON()+(k.next()!=end()?L",":L"");
                str+=L"]";
                break;
        }
        return str;
    }
    uint8 *pobj;
};


#include <assert.h>
struct KVPack {
    KVPack() {
        len=1;
        buff=(uint8*)malloc(len);
        //buff=(uint8*)malloc(10*1024*1024);
        offset=0;
    }
    void begin(KV_Type type) {
        reserve(sizeof(uint32)+sizeof(uint8));

        stack.push_back(offset);
        *(uint32*)(buff+offset)=0xAAAAAAAA; // clean        
        offset+=sizeof(uint32);
        *(buff+offset)=type;
        offset+=sizeof(uint8);
    }
    void end() {
        uint32 *bstart=(uint32*)(buff+stack.back());
        assert(*bstart == 0xAAAAAAAA);

        *(uint32*)(buff+stack.back()) = offset - stack.back();
        stack.pop_back();
    }
    void writeString(const TCHAR *str) {
        begin(KV_String);
            reserve((1+_tcslen(str))*sizeof(TCHAR));
            _tcscpy((TCHAR*)(buff+offset), str);
            offset += (1+_tcslen(str))*sizeof(TCHAR);
            *(buff+offset-1)=0;
        end();
    }
    void writeUint32(uint32 i) {
        begin(KV_Integer); 
            reserve(sizeof(uint32));
            memcpy(buff+offset, &i, sizeof(uint32));
            offset+=sizeof(uint32);
        end();
    }
    void writePairString(const TCHAR *k, const TCHAR *v) {
        begin(KV_Pair);
            writeString(k);
            writeString(v);
        end();
    }
    void writePairUint32(const TCHAR *k, uint32 v) {
        begin(KV_Pair);
            writeString(k);
            writeUint32(v);
        end();
    }
    void writePairObject(const TCHAR *k, KVObject o) {
        begin(KV_Pair);
            writeString(k);
            reserve(o.length());
            memcpy(buff+offset,o.pobj,o.length());
            offset+=o.length();
        end();
    }
    void clear() {
        free(buff);
    }
    KVObject root() {
        return KVObject(buff);
    }

    void reserve(int s) {
        if(offset+s>len) {
            int l1=2*len;
            int l2=offset+s;
            len=max(l1,l2);
            buff=(uint8*)realloc(buff,len);
            //memset(buff+offset,0xAA,len-offset);
        }
    }

    int len;
    int offset;
    uint8* buff;
    std::vector<int> stack;
};
