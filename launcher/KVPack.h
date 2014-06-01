#pragma once
#include "PredicateParser.h"

enum KVTypes {
    KVT_String,
    KVT_Integer
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
        uint8 *endOfObject = m_pObj+*(uint32*)m_pObj;
        uint8 *firstKV     = m_pObj+sizeof(uint32);

        for(uint8 *pk=firstKV; pk<endOfObject; pk+=*(uint32*)pk) {

            uint32 koffset = sizeof(uint32)+sizeof(uint32);
            uint32 toffset = sizeof(uint32)+*(uint32*)(pk+sizeof(uint32));
            uint32 voffset = toffset+1;

            if(*(pk+toffset)==KVT_String && _tcscmp(k, (TCHAR*)(pk+koffset))==0) {
                return (TCHAR*)(pk+voffset);
            }
            //_tprintf("%s=>%s\n",pk+koffset, pk+voffset);
        }
        return 0;
    }
    int getInt(const TCHAR *k) {
        uint8 *endOfObject = m_pObj+*(uint32*)m_pObj;
        uint8 *firstKV     = m_pObj+sizeof(uint32);

        for(uint8 *pk=firstKV; pk<endOfObject; pk+=*(uint32*)pk) {

            uint32 koffset = sizeof(uint32)+sizeof(uint32);
            uint32 toffset = sizeof(uint32)+*(uint32*)(pk+sizeof(uint32));
            uint32 voffset = toffset+1;

            if(*(pk+toffset)==KVT_Integer && _tcscmp(k, (TCHAR*)(pk+koffset))==0) {
                return *(uint32*)(pk+voffset);
            }
            //_tprintf("%s=>%s\n",pk+koffset, pk+voffset);
        }
        return 0;
    }

    /*int getInt(const TCHAR *k) {
        TCHAR *v=getString(k);
        if(v==0) return 0;
        return _ttoi(v);
    }*/
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
        //return pack(k,ItoS(i));
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

    /*TCHAR *getString(uint8 *pobj, const TCHAR *k) {

        uint8 *endOfObject = pobj+*(uint32*)pobj;
        uint8 *firstKV     = pobj+sizeof(uint32);

        for(uint8 *pk=firstKV; pk<endOfObject; pk+=*(uint32*)pk) {

            uint32 koffset = sizeof(uint32)+sizeof(uint32);
            uint32 toffset = sizeof(uint32)+*(uint32*)(pk+sizeof(uint32));
            uint32 voffset = toffset+1;

            if(_tcscmp(k, (TCHAR*)(pk+koffset))==0) {
                return (TCHAR*)(pk+voffset);
            }
            //_tprintf("%s=>%s\n",pk+koffset, pk+voffset);
        }
        return 0;
    }*/

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
};