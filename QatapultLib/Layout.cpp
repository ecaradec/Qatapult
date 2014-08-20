#include "stdafx.h"
#include "Layout.h"

CFont g_defaultfont;

void SetStdFontOfDescendants(CWindow w) {
    // configure font
    if(!g_defaultfont) {
        NONCLIENTMETRICS metrics;
        metrics.cbSize = sizeof(NONCLIENTMETRICS);
        ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
        
        g_defaultfont.CreateFontIndirectW(&metrics.lfMessageFont);
    }

    w.SendMessageToDescendants(WM_SETFONT, (WPARAM)g_defaultfont.m_hFont, (LPARAM)MAKELONG((WORD)TRUE, 0), TRUE);
}