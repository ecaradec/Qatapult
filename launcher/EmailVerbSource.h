#pragma once

struct EmailVerbSource : Source {
    EmailVerbSource() : Source(L"EMAILVERB") {
        m_index[L"EmailTo"]=SourceResult(L"EmailTo", L"EmailTo", L"EmailTo", this);
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\"+r->object->key+L".png");
    }
};
