#pragma once
#include <CommonControls.h>

inline Gdiplus::Bitmap *getIcon(const CString &path) {
    // get hicon
    SHFILEINFO sh;
    //SHGetFileInfo(L"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\IDE\\devenv.exe", FILE_ATTRIBUTE_NORMAL, &sh, sizeof(sh), SHGFI_SYSICONINDEX);                                
    HICON hicon;

    SHGetFileInfo(path, FILE_ATTRIBUTE_NORMAL, &sh, sizeof(sh), SHGFI_SYSICONINDEX);        
    IImageList *pil=0;
    CLSID clsid;
    CLSIDFromString(L"{46EB5926-582E-4017-9FDF-E8998DAA0950}", &clsid);
    SHGetImageList(SHIL_JUMBO, clsid, (void**)&pil);
    HRESULT hr=pil->GetIcon(sh.iIcon, ILD_TRANSPARENT|ILD_PRESERVEALPHA, &hicon);
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