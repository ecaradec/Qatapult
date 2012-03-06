#pragma once

struct QuitVerbSource : Source {
    QuitVerbSource() : Source(L"QUITVERB") {        
        m_index[L"Quit (Qatapult )"]=SourceResult(L"Quit (Qatapult )", L"Quit (Qatapult )", L"Quit (Qatapult )", this, 0, 0, m_index[L"Quit (Qatapult )"].bonus);
        m_ignoreemptyquery=true;
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\exit.png");
    }
};

// commented for use with textitemsource
struct QuitRule : Rule {
    QuitRule() {}
    virtual bool execute(std::vector<SourceResult> &args) {
        if(args[0].display==L"Quit (Q)")
            PostQuitMessage(0);
        else if(args[0].display==L"Reload (Q)")
            g_pUI->Reload();

        return true;
    }
};