#pragma once
struct Source {
    virtual ~Source() =0 {}
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &args, std::vector<SourceResult> &r, int def=0) {}
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r) { return 0; }

    CString type;
    CString defaultQuery;
};
