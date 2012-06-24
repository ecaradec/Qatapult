#pragma once

struct EmailVerbSource : Source {
    EmailVerbSource() : Source(L"EMAILVERB") {
        m_index[L"EmailTo"]=SourceResult(L"EmailTo", L"EmailTo", L"EmailTo", this);
    }
};
