#include "stdafx.h"

struct GdiplusInit {
    GdiplusInit() {
        ULONG_PTR gdiplusToken;
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }
} gdiplusinit;