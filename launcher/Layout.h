#pragma once

struct Layout {
    Layout(HWND hwnd, int m=5) {
        maxHeight=0;
        CWindow w(hwnd);
        w.GetClientRect(&r);
        r.DeflateRect(m,m,0,m);
        pos=r.TopLeft();
    }
    enum {
        NOMOVE=0,
        MOVE=1
    };
    CRect   r;
    int     maxHeight;
    CPoint  pos;
    CRect getSpace(int w, int h, bool flags=MOVE) {
        CRect cpos(pos, CSize(w,h));
        if(flags&MOVE) {
            pos.x+=cpos.Width()+5;        
            maxHeight=max(h,maxHeight);
        }
        return cpos;
    }
    void clearRow() {
        pos.x=r.left;
        pos.y+=(maxHeight+7);
        maxHeight=0;
    }
};

extern CFont g_defaultfont;

extern void SetStdFontOfDescendants(CWindow w);