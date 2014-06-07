#pragma once
#include "PredicateParser.h"
/*
enum KVTypes {
    KVT_Key, // required ?
    KVT_String,
    KVT_Integer,
    KVT_Object
};

struct KVObject {
    uint8 *m_pObj;
    KVObject() {
        m_pObj=0;
    }
    KVObject(uint8 *pObj) {
        m_pObj=pObj;
    }
    TCHAR *getString(const TCHAR *k) {
        return (TCHAR*)find(k,KVT_String);
    }
    int getInt(const TCHAR *k) {
        uint32 *i=(uint32*)find(k,KVT_Integer);
        if(i!=0)
            return *i;
        return 0;
    }
    uint8 *getObject(const TCHAR *k) {
        return (uint8*)find(k,KVT_Object);
    }
    uint8 *find(const TCHAR *k, uint8 type) {
        uint8 *endOfObject = m_pObj+*(uint32*)m_pObj;
        uint8 *firstKV     = m_pObj+sizeof(uint32);

        for(uint8 *pk=firstKV; pk<endOfObject; pk+=*(uint32*)pk) {

            uint32 koffset = sizeof(uint32)+sizeof(uint32);
            uint32 toffset = sizeof(uint32)+*(uint32*)(pk+sizeof(uint32));
            uint32 voffset = toffset+1;

            if(type == *(pk+toffset) && _tcscmp(k, (TCHAR*)(pk+koffset))==0) {
                return (uint8*)(pk+voffset);
            }
        }
        return 0;
    }
};

// type[this_is_the_key6=5]
struct KVPack {
    //std::vector<uint8*> objects;
    uint8 *buff;
    uint8 *pos;
    KVPack() {
        pos=buff=(uint8*)malloc(10*1024*1024);
    }
    void clear() {
        if(!buff) return;
        free(buff);
        buff=0;
    }
    void packKey(const TCHAR *k) {
        uint8 *key=beginBlock();
            pack(k);
        endBlock(key);
    }

    uint8 *pack(const TCHAR *k, const TCHAR *v) {
        uint8 *p=pos;
        uint8 *pkv=beginBlock();

            uint8 *pkvk=beginBlock();
                pack(k);
            endBlock(pkvk);
            pack((uint8)KVT_String);
            pack(v);
        
        endBlock(pkv);
        return p;
    }
    uint8 *pack(const TCHAR *k, uint32 v) {
        
        uint8 *p=pos;
        uint8 *pkv=beginBlock();

            uint8 *pkvk=beginBlock();
                pack(k);
            endBlock(pkvk);
            pack((uint8)KVT_Integer);
            pack(v);
        
        endBlock(pkv);

        return p;
    }
    uint8 *pack(const TCHAR *str) {
        uint8 *p=pos;
        uint32 l=(uint32)_tcslen(str);
        _tcsncpy((TCHAR*)pos,str,l+1);
        pos+=(l+1)*sizeof(TCHAR);
        return pos;
    }
    uint8 *pack(uint32 i) {
        uint8 *p=pos;
        *(uint32*)pos=i;
        pos+=sizeof(uint32);
        return p;
    }
    uint8 *pack(uint8 i) {
        uint8 *p=pos;
        *pos=i;
        pos+=sizeof(uint8);
        return p;
    }

    uint8 *beginBlock() {        
        return (uint8*)pack(uint32(0));
    }
    void endBlock(uint8 *b) {
        *(uint32*)b=pos-b;
        //objects.push_back(b);
    }

    uint8 *beginTable() {
        return (uint8*)pack(uint32(0));
    }
    void endTable(uint8 *b) {
        *(uint32*)b=pos-b;
    }

    void find(const TCHAR *expr, std::vector<uint8*> &res) {
        PredicateParser parser(expr);
        uint8 *endOfObject = buff+*(uint32*)buff;
        uint8 *firstObject = buff+sizeof(uint32);

        for(uint8 *pobj=firstObject; pobj<endOfObject; pobj+=*(uint32*)pobj) {
            uint8 *endOfObject = pobj+*(uint32*)pobj;
            uint8 *firstKV     = pobj+sizeof(uint32);

            if( parser.match(*this,pobj) )
                res.push_back(pobj);            
        }
    }

    void find(TCHAR *key, TCHAR *val, std::vector<uint8*> &res) {
        uint8 *endOfObject = buff+*(uint32*)buff;
        uint8 *firstObject = buff+sizeof(uint32);

        for(uint8 *pobj=firstObject; pobj<endOfObject; pobj+=*(uint32*)pobj) {
            uint8 *endOfObject = pobj+*(uint32*)pobj;
            uint8 *firstKV     = pobj+sizeof(uint32);

            for(uint8 *pk=firstKV; pk<endOfObject; pk+=*pk) {

                uint32 koffset = sizeof(uint32)+sizeof(uint32);
                uint32 toffset = sizeof(uint32)+*(uint32*)(pk+sizeof(uint32));
                uint32 voffset = toffset+1;

                if(_tcscmp(key, (TCHAR*)(pk+koffset))==0 && _tcscmp(val, (TCHAR*)(pk+voffset))==0) {
                    //_tprintf("%s=>%s\n",pk+koffset, pk+voffset);
                    res.push_back(pobj);
                }
            }
        }
    }

    void print() {
        uint8 *endOfObject = buff+*(uint32*)buff;
        uint8 *firstObject = buff+sizeof(uint32);

        for(uint8 *pobj=firstObject; pobj<endOfObject; pobj+=*(uint32*)pobj) {
            print(pobj);
        }
    }
    void print(uint8 *pobj) {
        uint8 *endOfObject = pobj+*(uint32*)pobj;
        uint8 *firstKV     = pobj+sizeof(uint32);
        TCHAR buff[4096*4];
        for(uint8 *pk=firstKV; pk<endOfObject; pk+=*(uint32*)pk) {
            uint32 koffset = sizeof(uint32)+sizeof(uint32);
            uint32 voffset = sizeof(uint32)+*(uint32*)(pk+sizeof(uint32));

            _tprintf(_T("%s=>%s\n"),pk+koffset, pk+voffset);
            _stprintf(buff, _T("%s=>%s\n"),pk+koffset, pk+voffset);
            OutputDebugString(buff);
        }
    }
    uint8 *begin() {
        return buff+sizeof(uint32);
    }
    uint8 *end() {
        return buff+*(uint32*)buff;
    }
    uint8 *next(uint8 *obj) {
        return obj+*(uint32*)obj;
    }
};*/


//#include <vector>
/*typedef unsigned char uint8;
typedef unsigned long uint32;


struct KVPack {
    enum Type {
        String,
        Integer,
        Map,
        Pair,
        Array
    };
    KVPack() {
        buff=(uint8*)malloc(10*1024*1024);
        p=buff;
    }
    void begin(Type type) {
        stack.push_back((uint32*)p);
        *(uint32*)p=0xFEFEFEFE; // clean
        p+=sizeof(uint32);
        *p=type;
        p+=sizeof(uint8);
    }
    void end() {
        *stack.back() = p - (uint8*)stack.back();
        stack.pop_back();
    }
    void writeString(const TCHAR *str) {
        begin(KVPack::String); 
            _tcscpy((TCHAR*)p, str);
            p+=(1+_tcslen(str))*sizeof(TCHAR);
            *(p-1)=0;
        end();
    }
    void writeUint32(uint32 i) {
        begin(KVPack::Integer); 
            memcpy(p, &i, sizeof(uint32));
            p+=sizeof(uint32);
        end();
    }
    void writePairString(const TCHAR *k, const TCHAR *v) {
        begin(KVPack::Pair);
            writeString(k);
            writeString(v);
        end();
    }
    void writePairUint32(const TCHAR *k, uint32 v) {
        begin(KVPack::Pair);
            writeString(k);
            writeUint32(v);
        end();
    }
    void pad(int space) {
        while(space--) printf(" ");
    }
    void print(uint8 *pos=0, int level=0) {
        if(pos==0) return;
        //if(!pos) pos=buff;
        uint32 l=*(uint32*)pos;
        uint8 *e=pos+l;
        pos+=sizeof(uint32);
        uint8  t=*pos;
        pos+=sizeof(uint8);        
        
        pad(level); printf("%d:",l);
        switch(t) {
            case String:
                printf("#String: %s\n", pos);
                break;
            case Integer:
                printf("#Integer: %d\n", *(uint32*)pos);
                break;
            case Map:
                printf("#Map:\n");
                for(uint8 *pp=pos; pp<e; pp+=*(uint32*)pp)
                    print(pp,level+1);
                break;
            case Pair:
                printf("#Pair:\n");
                for(uint8 *pp=pos; pp<e; pp+=*(uint32*)pp)
                    print(pp,level+1);
                break;
            case Array:
                printf("#Array:\n");                
                for(uint8 *pp=pos; pp<e; pp+=*(uint32*)pp)
                    print(pp,level+1);
                break;
        }
        pos=e;
    }
    uint8 *getObject(uint8 *p, TCHAR *key) {
        uint8 *endmap=p+*(uint32*)p;
        uint8 *pmap=p+sizeof(uint32)+sizeof(uint8);
        
        for(; pmap<endmap; pmap+=*(uint32*)pmap) {
            uint8 *endpair=pmap+*(uint32*)pmap;
            uint8 *ppair=pmap+sizeof(uint32)+sizeof(uint8);
            for(; ppair<endpair; ppair+=*(uint32*)ppair) {

                uint8 *pstr;
                pstr=ppair+sizeof(uint32)+sizeof(uint8);
                ppair+=*(uint32*)ppair;                

                int t=*(uint8*)(ppair+sizeof(uint32));
                uint8 *pval=ppair+sizeof(uint32)+sizeof(uint8); 
                
                if(_tcscmp(key,(TCHAR*)pstr)==0)
                    return ppair;

                ppair+=*(uint32*)ppair;
            }
        }
        return 0;
    }
    TCHAR *getString(uint8 *o, TCHAR *k) {
        uint8 *v=getObject(o,k);
        if(v==0) return 0;
        return (TCHAR*)(v+sizeof(uint32)+sizeof(uint8));
    }
    uint32 getInt(uint8 *o, TCHAR *k) {
        uint8 *v=getObject(o,k);
        if(v==0) return 0;
        return *(uint32*)(v+sizeof(uint32)+sizeof(uint8));
    }
    uint8 *p;
    uint8* buff;
    std::vector<uint32*> stack;
};

// divide in kvreader and kvwriter
struct KVObject {
    KVObject(uint8* o_) :o(o_) {}
    uint8 *getObject(TCHAR *key) {
        uint8 *endmap=o+*(uint32*)o;
        uint8 *pmap=o+sizeof(uint32)+sizeof(uint8);
        
        for(; pmap<endmap; pmap+=*(uint32*)pmap) {
            uint8 *endpair=pmap+*(uint32*)pmap;
            uint8 *ppair=pmap+sizeof(uint32)+sizeof(uint8);
            for(; ppair<endpair; ppair+=*(uint32*)ppair) {

                uint8 *pstr;
                pstr=ppair+sizeof(uint32)+sizeof(uint8);
                ppair+=*(uint32*)ppair;                

                int t=*(uint8*)(ppair+sizeof(uint32));
                uint8 *pval=ppair+sizeof(uint32)+sizeof(uint8); 
                
                if(_tcscmp(key,(TCHAR*)pstr)==0)
                    return ppair;

                ppair+=*(uint32*)ppair;
            }
        }
        return 0;
    }
    TCHAR *getString(TCHAR *k) {
        uint8 *v=getObject(k);
        if(v==0) return 0;
        return (TCHAR*)(v+sizeof(uint32)+sizeof(uint8));
    }
    uint32 getInt(TCHAR *k) {
        uint8 *v=getObject(k);
        if(v==0) return 0;
        return *(uint32*)(v+sizeof(uint32)+sizeof(uint8));
    }
    uint8 *o;
};*/


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
    uint8 *pobj;
};

struct KVPack {
    KVPack() {
        buff=(uint8*)malloc(10*1024*1024);
        p=buff;
    }
    void begin(KV_Type type) {
        stack.push_back((uint32*)p);
        *(uint32*)p=0xFEFEFEFE; // clean
        p+=sizeof(uint32);
        *p=type;
        p+=sizeof(uint8);
    }
    void end() {
        *stack.back() = p - (uint8*)stack.back();
        stack.pop_back();
    }
    void writeString(const TCHAR *str) {
        begin(KV_String); 
            _tcscpy((TCHAR*)p, str);
            p+=(1+_tcslen(str))*sizeof(TCHAR);
            *(p-1)=0;
        end();
    }
    void writeUint32(uint32 i) {
        begin(KV_Integer); 
            memcpy(p, &i, sizeof(uint32));
            p+=sizeof(uint32);
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
            memcpy(p,o.pobj,o.length());
            p+=o.length();
        end();
    }
    void clear() {
        free(buff);
    }
    KVObject root() {
        return KVObject(buff);
    }
    uint8 *p;
    uint8* buff;
    std::vector<uint32*> stack;
};
