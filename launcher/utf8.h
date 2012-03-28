#pragma once

inline CStringW UTF8toUTF16(const CStringA& utf8)
{
    CStringW utf16;
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    if (len>1)
    { 
        wchar_t *ptr = utf16.GetBuffer(len-1);
        if (ptr) MultiByteToWideChar(CP_UTF8, 0, utf8, -1, ptr, len);
        utf16.ReleaseBuffer();
    }
    return utf16;
}
inline CStringA UTF16toUTF8(const CStringW& utf16)
{    
    CStringA utf8;
    int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
    if (len>1)
    { 
        char *ptr = utf8.GetBuffer(len-1);
        if (ptr) WideCharToMultiByte(CP_UTF8, 0, utf16, -1, ptr, len, 0, 0);
        utf8.ReleaseBuffer();
    }
    return utf8;
}