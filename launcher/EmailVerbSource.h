#pragma once

struct EmailVerbSource : Source {
    EmailVerbSource() : Source(L"EMAILVERB") {
        m_index[L"EmailTo"]=SourceResult(L"EmailTo", L"EmailTo", L"EmailTo", this, 0, 0, 0);
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\"+r->key+L".png");
    }
};
