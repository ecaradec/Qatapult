#pragma once
#include "Object.h"

struct FileObject : Object {
    //FileObject(const CString &k, Source *s, const CString &text, const CString &expand, const CString &path);
    FileObject(const FileObject& f);
    FileObject(Record &r,Source *s);
    FileObject *clone();

    int m_jumboDrawMethod;
};
