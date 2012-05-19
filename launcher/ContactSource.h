#pragma once
#include "http.h"
#include <time.h>
#include "Object.h"
#include "DBSource.h"


struct ContactObject : Object {
    ContactObject(const CString &k, Source *s, const CString &text, const CString &email);
    Gdiplus::Bitmap *getIcon(long flags);
    //void drawItem(Graphics &g, SourceResult *sr, RectF &r);
};

// those kind of sources could have a simplified load and save ?
struct ContactSource : DBSource {
    ContactSource();
    void parseGmailContacts(const char *xml) ;
    void crawl();
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes);
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags);
};
