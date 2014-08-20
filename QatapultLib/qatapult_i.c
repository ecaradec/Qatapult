

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Tue Aug 19 18:06:28 2014
 */
/* Compiler settings for qatapult.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IQatapultScript,0xE0516B01,0x92D9,0x4501,0x84,0xDB,0x9E,0x9B,0x47,0x8E,0x47,0x5F);


MIDL_DEFINE_GUID(IID, IID_ICollecterScript,0x240BA9C5,0xC64D,0x4DA5,0xAC,0x5F,0x65,0x09,0x9A,0x1C,0xA9,0x27);


MIDL_DEFINE_GUID(IID, IID_IPainterScript,0x03E86E34,0x8D63,0x49A7,0x86,0xA0,0x02,0x31,0x10,0x98,0x97,0x67);


MIDL_DEFINE_GUID(IID, LIBID_QatapultLib,0xF5BB800F,0xDB3D,0x4036,0xBF,0x5C,0x47,0xD9,0x14,0x65,0x61,0x31);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



