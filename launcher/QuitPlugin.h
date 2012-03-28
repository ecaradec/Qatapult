#pragma once

// commented for use with textitemsource
struct QatapultRule : Rule {
    QatapultRule() {}
    virtual bool execute(std::vector<SourceResult> &args) {
        if(args[1].display==L"Quit")
            PostQuitMessage(0);
        else if(args[1].display==L"Reload")
            g_pUI->Reload();

        return true;
    }
};