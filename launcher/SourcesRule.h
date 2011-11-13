
struct SourcesRule : Rule {
    SourcesRule(ClauncherDlg *pLB):m_pUI(pLB) {
        m_rules.push_back(new HistoryRule(m_pUI));
        m_rules.push_back(new StartMenuRule(m_pUI));
        m_rules.push_back(new FileRule(m_pUI));        
    }
    ~SourcesRule() {
        for(int i=0;i<m_rules.size(); i++) {
            delete m_rules[i];
        }
    }
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        for(int i=0;i<m_rules.size(); i++) {
            m_rules[i]->collect(query, results);
        }
    }
    Rule *validate() {
        RuleResult *r=m_pUI->GetSelectedItem();
        return r->rule->validate();
    }
    ClauncherDlg           *m_pUI;
    std::vector<Rule*>      m_rules;
};