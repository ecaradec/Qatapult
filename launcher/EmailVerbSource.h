#pragma once

struct EmailVerbSource : Source {
    EmailVerbSource() : Source(L"EMAILVERB") {
        m_index[L"EmailTo"]=SourceResult(new Object(L"EmailTo", L"EmailTo", this, L"EmailTo"));
    }
};
