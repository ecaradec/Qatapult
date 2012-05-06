
struct SourceRule : Rule {
    SourceRule(UI *pUI):m_pUI(pUI) {
    }
    bool execute(std::vector<RuleArg> &args) {
        CString q;
        Source *s=(*m_pArgs)[0].source()->getSource((*m_pArgs)[0].item(0),q);
        m_pUI->SetCurrentSource(0,s,q);
        m_pUI->Show();
        return true;
    }
    UI *m_pUI;
};
