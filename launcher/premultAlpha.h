void PremultAlpha(CImage &img) {
    for(int y=0;y<img.GetHeight(); y++) 
        for(int x=0;x<img.GetWidth(); x++) {
            DWORD c=*(DWORD*)img.GetPixelAddress(x,y);            
            DWORD r=((c&0xFF));
            DWORD g=((c&0xFF00)>>8);
            DWORD b=((c&0xFF0000)>>16);
            DWORD a=((c&0xFF000000)>>24);
            
            DWORD R=int(r*a)>>8;
            DWORD G=int(g*a)>>8;
            DWORD B=int(b*a)>>8;
            DWORD A=int(a);

            DWORD cm=R + (G<<8) + (B<<16) + (A<<24);
            *(DWORD*)img.GetPixelAddress(x,y) = cm;
        }
}
void PremultAlpha(Bitmap &img) {
    for(uint y=0;y<img.GetHeight(); y++) 
        for(uint x=0;x<img.GetWidth(); x++) {
            DWORD c;
            img.GetPixel(x,y,(Color*)&c);
                
            DWORD r=((c&0xFF));
            DWORD g=((c&0xFF00)>>8);
            DWORD b=((c&0xFF0000)>>16);
            DWORD a=((c&0xFF000000)>>24);
            
            DWORD R=int(r*a)>>8;
            DWORD G=int(g*a)>>8;
            DWORD B=int(b*a)>>8;
            DWORD A=int(a);

            DWORD cm=R + (G<<8) + (B<<16) + (A<<24);
            img.SetPixel(x,y,Color(cm));
        }
}
    