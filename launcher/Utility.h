#pragma once
inline bool FileExists(const CString &f) {
    return GetFileAttributes(f)!=INVALID_FILE_ATTRIBUTES;
}

inline CString getFileContent(const TCHAR *path) {
    FILE *f=_tfopen(path,L"rt");
    if(!f)
        return L"";
    long l=_filelength(f->_file);
    char *text=(char*)malloc(l+1);
    int l2=fread(text, sizeof(char), l, f);
    fclose(f);
    text[l2]=0;
    CString tmp(UTF16toUTF8(text));
    free(text);
    return tmp;
}

inline CString fuzzyfyArg(const CString &arg) {
    CString tmp=L"%";
    for(int i=0;i<arg.GetLength();i++) {
        tmp+=CString(arg[i])+L"%";
    }
    return tmp;
}


inline CString Format(TCHAR * format, ... )
{
   va_list args;
   int len;

   va_start( args, format );
   len = _vsctprintf( format, args ) // _vscprintf doesn't count
                               + 1; // terminating '\0'   
   CString tmp;
   _vstprintf(tmp.GetBufferSetLength(len), len, format, args );
   tmp.ReleaseBuffer();
   return tmp;
}
