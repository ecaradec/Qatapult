
struct QuitVerbSource : Source {
    QuitVerbSource() : Source(L"QUITVERB") {        
        m_index[L"Quit (QSLL )"]=SourceResult(L"Quit (QSLL )", L"Quit (QSLL )", L"Quit (QSLL )", this, 0, 0, m_index[L"Quit (QSLL )"].bonus);
        m_ignoreemptyquery=true;
    }
};

struct QuitRule : Rule {
    QuitRule() : Rule(L"QUITVERB") {}
    virtual bool execute(std::vector<SourceResult> &args) {
        PostQuitMessage(0);
        return true;
    }    
};