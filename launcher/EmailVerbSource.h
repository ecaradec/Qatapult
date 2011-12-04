#pragma once

struct EmailVerbSource : Source {
    EmailVerbSource() : Source(L"EMAILVERB") {
        m_index[L"EmailTo (Compose )"]=SourceResult(L"EmailTo (Compose )", L"EmailTo (Compose )", L"EmailTo (Compose )", this, 0, 0, 0);
        m_index[L"EmailTo (Send )"]=SourceResult(L"EmailTo (Send )", L"EmailTo (Send )", L"EmailTo (Send )", this, 0, 0, 0);
    }
};
