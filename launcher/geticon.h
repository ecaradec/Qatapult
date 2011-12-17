#pragma once
#include <CommonControls.h>
#include <Commctrl.h>

int getNextHigherIconSize(int v) {    
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

inline Gdiplus::Bitmap *getIcon(SHFILEINFO &sh) {      
    IImageList *pil=0;
    CLSID clsid;
    CLSIDFromString(L"{46EB5926-582E-4017-9FDF-E8998DAA0950}", &clsid);
    //SHGetImageList(SHIL_EXTRALARGE, clsid, (void**)&pil);   
    SHGetImageList(SHIL_JUMBO, clsid, (void**)&pil);   
    
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
    GetBitmapBits(iconinfo.hbmColor, l, pixels);
    
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

    for(int y=0;y<bmColor.bmHeight;y++)
        for(int x=0;x<bmColor.bmWidth;x++) {
            DWORD pixel=pixels[x+y*bmColor.bmWidth];
            icon->SetPixel(x,y,pixel);
        }

    g3.ReleaseHDC(hdc3);

    pil->Release();

    return icon;
}

inline Gdiplus::Bitmap *getIcon(const CString &path) {
    // get hicon
    SHFILEINFO sh;
    SHGetFileInfo(path, FILE_ATTRIBUTE_NORMAL, &sh, sizeof(sh), SHGFI_SYSICONINDEX);        
    return getIcon(sh);
}

inline Gdiplus::Bitmap *getIcon(ITEMIDLIST *pidl) {
    // get hicon
    SHFILEINFO sh;
    SHGetFileInfo((LPCWSTR)pidl, FILE_ATTRIBUTE_NORMAL, &sh, sizeof(sh), SHGFI_PIDL|SHGFI_SYSICONINDEX);        
    return getIcon(sh);
}