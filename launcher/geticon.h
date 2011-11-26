#pragma once
#include <CommonControls.h>

inline Gdiplus::Bitmap *getIcon(SHFILEINFO &sh) {      
    IImageList *pil=0;
    CLSID clsid;
    CLSIDFromString(L"{46EB5926-582E-4017-9FDF-E8998DAA0950}", &clsid);
    SHGetImageList(SHIL_JUMBO, clsid, (void**)&pil);
    HICON hicon;
    HRESULT hr=pil->GetIcon(sh.iIcon, ILD_ASYNC|ILD_TRANSPARENT|ILD_PRESERVEALPHA, &hicon);
    
    IMAGEINFO im;
    pil->GetImageInfo(sh.iIcon, &im);
    int cx, cy;
    pil->GetIconSize(&cx,&cy);

    ICONINFO iconinfo;
    GetIconInfo(hicon, &iconinfo);

    BITMAP bmMask;
    GetObject(iconinfo.hbmMask, sizeof(BITMAP), &bmMask);
    BITMAP bmColor;
    GetObject(iconinfo.hbmColor, sizeof(BITMAP), &bmColor);
                
    DWORD pixels[256*256];
    LONG l=sizeof(pixels);
    GetBitmapBits(iconinfo.hbmColor, l, pixels);
                
    // draw icon
    Gdiplus::Bitmap *icon=new Gdiplus::Bitmap(bmColor.bmWidth, bmColor.bmHeight, PixelFormat32bppARGB);
    Gdiplus::Graphics g3(icon);
    HDC hdc3=g3.GetHDC();
    for(int y=0;y<bmColor.bmHeight;y++)
        for(int x=0;x<bmColor.bmWidth;x++)
            icon->SetPixel(x,y,pixels[x+y*bmColor.bmWidth]);
    //DrawIconEx(hdc3, 0, 0, hicon, 256, 256, 0, 0, DI_NORMAL);
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