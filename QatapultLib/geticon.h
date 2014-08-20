#pragma once
#include <CommonControls.h>
#include <Commctrl.h>

inline int getNextHigherIconSize(int v) {    
    if(v<=16)
        return 16;
    if(v<=24)
        return 24;
    if(v<=32)
        return 32;
    if(v<=48)
        return 48;
    if(v<=64)
        return 64;
    if(v<=256)
        return 256;
    return v;
}
#define ICON_SIZE_SMALL 0
#define ICON_SIZE_LARGE 1

inline Gdiplus::Bitmap *getIcon(SHFILEINFO &sh, long flags) {
    IImageList *pil=0;
    CLSID clsid;
    CLSIDFromString(L"{46EB5926-582E-4017-9FDF-E8998DAA0950}", &clsid);
    //SHGetImageList(SHIL_EXTRALARGE, clsid, (void**)&pil);   
    if(flags==ICON_SIZE_SMALL)
        SHGetImageList(SHIL_LARGE, clsid, (void**)&pil);   
    else {
        SHGetImageList(SHIL_JUMBO, clsid, (void**)&pil);   
        // XP support
        if(pil==0)
            SHGetImageList(SHIL_LARGE, clsid, (void**)&pil); 
    }    
    
    HICON hicon;
    HRESULT hr=pil->GetIcon(sh.iIcon, ILD_ASYNC|ILD_TRANSPARENT|ILD_PRESERVEALPHA, &hicon);
    
    IMAGEINFO im;
    pil->GetImageInfo(sh.iIcon, &im);

    ICONINFO iconinfo;
    GetIconInfo(hicon, &iconinfo);

    BITMAP bmMask;
    GetObject(iconinfo.hbmMask, sizeof(BITMAP), &bmMask);
    BITMAP bmColor;
    GetObject(iconinfo.hbmColor, sizeof(BITMAP), &bmColor);
                
    DWORD pixels[256*256];
    LONG l=sizeof(pixels);
    if(GetBitmapBits(iconinfo.hbmColor, l, pixels)==0)
        return 0;
    
    int leftMax=0;
    int bottomMax=0;
    for(int y=0;y<bmColor.bmHeight;y++)
        for(int x=0;x<bmColor.bmWidth;x++) {
            DWORD pixel=pixels[x+y*bmColor.bmWidth];
            if(pixel!=0) {
                leftMax=max(leftMax, x);
                bottomMax=max(bottomMax, y);
            }
        }

    int cx=getNextHigherIconSize(leftMax);
    int cy=getNextHigherIconSize(bottomMax);

    // draw icon
    Gdiplus::Bitmap *icon=new Gdiplus::Bitmap(cx, cy, PixelFormat32bppARGB);
    Gdiplus::Graphics g3(icon);
    HDC hdc3=g3.GetHDC();

    for(int y=0;y<cx;y++) {
        for(int x=0;x<cy;x++) {
            DWORD pixel=pixels[x+y*bmColor.bmWidth];
            icon->SetPixel(x,y,pixel);
            //CString tmp; tmp.Format(L"%2x",(pixel&0xFF));
            //OutputDebugString(tmp);
        }
        //OutputDebugString(L"\n");
    }

    g3.ReleaseHDC(hdc3);

    pil->Release();

    DestroyIcon(hicon);

    return icon;
}

inline Gdiplus::Bitmap *getIcon(const CString &path, long flags=ICON_SIZE_LARGE) {    
    // get hicon
    SHFILEINFO sh;
    SHGetFileInfo(path, FILE_ATTRIBUTE_NORMAL, &sh, sizeof(sh), SHGFI_SYSICONINDEX);        
    return getIcon(sh,flags);
}

inline Gdiplus::Bitmap *getIcon(ITEMIDLIST *pidl, long flags=ICON_SIZE_LARGE) {
    // get hicon
    SHFILEINFO sh;
    SHGetFileInfo((LPCWSTR)pidl, FILE_ATTRIBUTE_NORMAL, &sh, sizeof(sh), SHGFI_PIDL|SHGFI_SYSICONINDEX);        
    return getIcon(sh,flags);
}