#pragma once
/*struct SourcesRule : Source {
    SourcesRule(ClauncherDlg *pLB):m_pUI(pLB) {
        m_sources.push_back(new HistorySource(m_pUI));
        m_sources.push_back(new StartMenuSource(m_pUI));
        m_sources.push_back(new FileSource(m_pUI));        
    }
    ~SourcesRule() {
        for(int i=0;i<m_sources.size(); i++) {
            delete m_sources[i];
        }
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results) {
        for(int i=0;i<m_sources.size(); i++) {
            m_sources[i]->collect(query, results);
        }
    }
    Source *validate() {
        SourceResult *r=m_pUI->GetSelectedItem();
        return r->source->validate();
    }
    ClauncherDlg           *m_pUI;
    std::vector<Source*>      m_sources;
};*/