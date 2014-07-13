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
/*
FileObject::FileObject(const CString &k, Source *s, const CString &text, const CString &expand, const CString &path) :Object(k,L"FILE",s,text) {
    values[L"expand"]=expand;
    values[L"path"]=path;
    m_jumboDrawMethod=0;
}*/