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
    void pad(int space) {
        while(space--) printf(" ");
    }
    void dpad(int space) {
        while(space--) OutputDebugString(L" ");
    }
    void print(int level=0) {
        pad(level); 
        printf("%d:",length());
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
    void debug(int level=0) {
        dpad(level); 
        CString f; f.Format(L"%d",length());
        OutputDebugString(f);
        switch(type()) {
            case KV_String:
                f.Format(L"#String: %s\n", data());
                OutputDebugString(f);
                break;
            case KV_Integer:
                f.Format(L"#Integer: %d\n", *(uint32*)data());
                OutputDebugString(f);
                break;
            case KV_Map:
                f.Format(L"#Map:\n");
                OutputDebugString(f);
                for(KVObject k=first(); k!=end(); k=k.next())
                    k.debug(level+1);
                break;
            case KV_Pair:
                f.Format(L"#Pair:\n");
                OutputDebugString(f);
                for(KVObject k=first(); k!=end(); k=k.next())
                    k.debug(level+1);
                break;
            case KV_Array:
                f.Format(L"#Array:\n");                
                OutputDebugString(f);
                for(KVObject k=first(); k!=end(); k=k.next())
                    k.debug(level+1);
                break;
        }
    }
    uint8 *pobj;
};

// change stack to offsets
struct KVPack {
    KVPack() {
        len=10*1024*1024;
        //len=1024;
        buff=(uint8*)malloc(len);
        offset=0;
    }
    void begin(KV_Type type) {
        reserve(sizeof(uint32)+sizeof(uint8));
        stack.push_back(offset);
        *(uint32*)(buff+offset)=0x00000000; // clean
        offset+=sizeof(uint32);
        *(buff+offset)=type;
        offset+=sizeof(uint8);
    }
    void end() {
        *(buff+stack.back()) = offset - stack.back();
        stack.pop_back();
    }
    void writeString(const TCHAR *str) {
        int slen=_tcslen(str);        
        begin(KV_String);
            reserve(sizeof(TCHAR)*(slen+1));
            _tcsncpy((TCHAR*)(buff+offset), str, slen+1); // copy string including last 0 character
            offset+=(slen+1)*sizeof(TCHAR);            
        end();
    }
    void writeUint32(uint32 i) {        
        begin(KV_Integer); 
            reserve(sizeof(uint32));
            memcpy((buff+offset), &i, sizeof(uint32));
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

    int                  len;
    int                  offset;
    uint8               *buff;
    std::vector<int>     stack;
};
