#include "SourceOfSources.h"

struct SourceRule : Rule {
    SourceRule(UI *pUI):m_pUI(pUI) {
    }
    bool execute(std::vector<RuleArg> &args) {
        CString q;
        Source *sourceofsources=(*m_pArgs)[0].source();
        Source *s=sourceofsources->getSubSource((*m_pArgs)[0].item(0).m_object.get(),q);
        m_pUI->setCurrentSource(0,s,q);
        m_pUI->show();
        m_pUI->onQueryChange(L"",true);
        return true;
    }
    UI *m_pUI;
};
