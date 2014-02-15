#pragma once
#include "Object.h"

struct FileObject : Object {
    FileObject(const CString &k, Source *s, const CString &text, const CString &expand, const CString &path);
    FileObject(const FileObject& f);
    FileObject(Record &r,Source *s);
    FileObject *clone();
    CString getString(const TCHAR *val_);
    void drawIcon(Graphics &g, RectF &r);

    int m_jumboDrawMethod;
};
