#pragma once

// ok but create duplicates that should be filtered and is difficult to serialize
// should the last action be saved as the default on this item too ?
std::vector<SourceResult> g_history;

struct HistorySource : Source {
    HistorySource(ClauncherDlg *pLB):m_pUI(pLB)  {
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results) {
        for(int i=0;i<g_history.size();i++)
            if(CString(g_history[i].display).MakeUpper().Find(CString(query).MakeUpper())!=-1) {
                results.push_back(g_history[i]);
            }
    }
    ClauncherDlg            *m_pUI;
};

