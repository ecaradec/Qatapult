#include <stdafx.h>
#include "FileObject.h"
#include "ShellLink.h"
#include "Record.h"
#include "geticon.h"

FileObject::FileObject(Record &r,Source *s) {
    type=L"FILE";
    source=s;
    values=r.values;
    ivalues=r.ivalues;
    m_jumboDrawMethod=0;
}

FileObject::FileObject(const CString &k, Source *s, const CString &text, const CString &expand, const CString &path) :Object(k,L"FILE",s,text) {
    values[L"expand"]=expand;
    values[L"path"]=path;
    m_jumboDrawMethod=0;
}

CString FileObject::getString(const TCHAR *val_) {
    CString val(val_);

    if(val==L"rdirectory") {
        CString fp(getString(L"rpath"));
        return fp.Left(fp.ReverseFind(L'\\'));
    } else if(val==L"directory") {
        CString fp(getString(L"path"));
        return fp.Left(fp.ReverseFind(L'\\'));
    } else if(val==L"rfilename") {
        CString fp(getString(L"rpath"));
        return fp.Mid(fp.ReverseFind(L'\\')+1);
    } else if(val==L"filename") {
        CString fp(getString(L"path")); 
        return fp.Mid(fp.ReverseFind(L'\\')+1);
    } else if(val==L"rpath") {
        CString path=getString(L"path");
        if(path.Right(4)==L".lnk")
            return getShortcutPath(getString(L"path"));
        else
            return path;
    } else if(val==L"status") {
        if(getString(L"path").Right(4)==L".lnk")        
            return Object::getString(L"text");
        else
            return getString(L"path");
    }

    return Object::getString(val);
}

int GetIconIndex(PCTSTR pszFile)
{
    SHFILEINFO sfi;
    SHGetFileInfo(pszFile, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
    return sfi.iIcon;
}

HICON GetJumboIcon(int iImage)
{
  CComPtr<IImageList> pil;
  SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&pil);

  HICON hico;
  pil->GetIcon(iImage, ILD_TRANSPARENT, &hico);
  return hico;
}
/*
bool hasJumboIcon(TCHAR *path) {
    CComPtr<IShellItem> pSI;
    SHCreateItemFromParsingName(path, 0, __uuidof(IShellItem), (void**)&pSI);
    

    CComQIPtr<IShellItemImageFactory> pSIIF(pSI);
    HBITMAP hbmp;
    HRESULT hr=pSIIF->GetImage(CSize(1,1), SIIGBF_THUMBNAILONLY, &hbmp);
    

    BITMAP bm;
    GetObject(hbmp,sizeof(bm), &bm); 
    return hbmp!=0;
}*/


void detectRealJumboSize(DWORD *pixels, int w, int h, int &cx, int &cy) {
    // read each pixel to detect if the icon is 256 pixels
    // it's a very slow way of detecting if icons are 256
    // but it doesn't seems there is any other
    // 
    int leftMax=0;
    int bottomMax=0;
    for(int y=0;y<h;y++)
        for(int x=0;x<w;x++) {
            DWORD pixel=pixels[x+y*w];
            if(pixel!=0) {
                leftMax=max(leftMax, x);
                bottomMax=max(bottomMax, y);
            }
        }

    cx=getNextHigherIconSize(leftMax);
    cy=getNextHigherIconSize(bottomMax);
}

void FileObject::drawIcon(Graphics &g, RectF &r) {
    // fun but costly
    /*CString iconPath=L"icons\\"+getString(L"rfilename")+L".png";
    if( m_icon.get() || GetFileAttributes(iconPath)!=INVALID_FILE_ATTRIBUTES ) {
        
        if(!m_icon.get())
            m_icon.reset(Gdiplus::Bitmap::FromFile(iconPath));

        if(m_icon)
            g.DrawImage(m_icon.get(), r);

        return;
    }*/

    // function is based on
    // http://blogs.msdn.com/b/oldnewthing/archive/2014/01/20/10490951.aspx
    
    if(r.Width<=64) {
        if(!m_smallicon)
            m_smallicon.reset(getIcon(values[L"path"],ICON_SIZE_SMALL));

        if(m_smallicon)
            g.DrawImage(m_smallicon.get(), r);

        return;
    }

    // the function is a bit complex because :
    // - I didn't find a way to detect if jumbo size was supported for a file
    // - some file have issues with alpha mask that is not correctly rendered when transfering to gdiplus (like command.exe)
    // - IImageList::Draw seems to have some premultiplication issues is the rendering is less good than the gdiplus one
    // => but it works    
    
    if(m_jumboDrawMethod==0) {
        IImageList *pil=0;

        SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&pil);
        // XP support
        if(pil==0)
            SHGetImageList(SHIL_LARGE, IID_IImageList, (void**)&pil); 
        
        SHFILEINFO sh;
        SHGetFileInfo(values[L"path"], FILE_ATTRIBUTE_NORMAL, &sh, sizeof(sh), SHGFI_SYSICONINDEX|SHGFI_SHELLICONSIZE);

        HICON hicon;
        HRESULT hr=pil->GetIcon(sh.iIcon, ILD_TRANSPARENT|ILD_PRESERVEALPHA, &hicon);
    
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
        if(GetBitmapBits(iconinfo.hbmColor, l, pixels)==0) {
            m_jumboDrawMethod=1; // try the default way just in case
        } else {
            int cx, cy;
            detectRealJumboSize(pixels, bmColor.bmWidth, bmColor.bmHeight, cx, cy);

            if(cx==256) {
                m_jumboDrawMethod=1;
            } else {
                m_jumboDrawMethod=2;
                // draw icon
                m_icon.reset(new Gdiplus::Bitmap(cx, cy, PixelFormat32bppARGB));
                Gdiplus::Graphics g3(m_icon.get());
                HDC hdc3=g3.GetHDC();

                for(int y=0;y<cx;y++) {
                    for(int x=0;x<cy;x++) {
                        DWORD pixel=pixels[x+y*bmColor.bmWidth];
                        m_icon->SetPixel(x,y,pixel);
                        //CString tmp; tmp.Format(L"%2x",(pixel&0xFF));
                        //OutputDebugString(tmp);
                    }
                    //OutputDebugString(L"\n");
                }

                g3.ReleaseHDC(hdc3);
            }
        }         

        pil->Release();

        DestroyIcon(hicon);
    }
    
    if(m_jumboDrawMethod==2) {
        if(m_icon)
            g.DrawImage(m_icon.get(), r);
    } else {
        HRESULT hr;
        HDC hdc=g.GetHDC();

        CComPtr<IImageList> pil;

        SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&pil);
        // XP support
        if(pil==0)
            SHGetImageList(SHIL_LARGE, IID_IImageList, (void**)&pil); 

        IMAGELISTDRAWPARAMS ildp = { sizeof(ildp) };
        ildp.himl = IImageListToHIMAGELIST(pil);
        ildp.i = GetIconIndex(values[L"path"]);
        ildp.hdcDst = hdc;
        ildp.x = r.X;
        ildp.y = r.Y;
        ildp.cx = r.Width;
        ildp.cy = r.Height;
        ildp.rgbBk = CLR_NONE;
        ildp.fStyle = ILD_TRANSPARENT|ILD_SCALE;
        pil->Draw(&ildp);

        g.ReleaseHDC(hdc);
    }
}