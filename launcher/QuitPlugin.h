
struct QuitVerbSource : Source {
    QuitVerbSource() : Source(L"QUITVERB") {        
        m_index[L"Quit (Qatapult )"]=SourceResult(L"Quit (Qatapult )", L"Quit (Qatapult )", L"Quit (Qatapult )", this, 0, 0, m_index[L"Quit (Qatapult )"].bonus);
        m_ignoreemptyquery=true;
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\exit.png");
    }
};

struct QuitRule : Rule {
    QuitRule() : Rule(L"QUITVERB") {}
    virtual bool execute(std::vector<SourceResult> &args) {
        PostQuitMessage(0);
        return true;
    }
};