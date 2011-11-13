struct StartMenuRule : Rule {
    StartMenuRule(ClauncherDlg *pLB):m_pUI(pLB)  {
    }
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        std::vector<CString> lnks;
        FindFilesRecursively(L"C:\\ProgramData\\Microsoft\\Windows\\Start Menu", L"*.lnk", lnks);

        for(int i=0;i<lnks.size();i++) {
            CString str(lnks[i]);
            PathRemoveExtension(str.GetBuffer()); str.ReleaseBuffer();
            str=PathFindFileName(str.GetBuffer()); str.ReleaseBuffer();

            /*CString q(query);
            int first=str.Find(q.GetAt(0));
            if(first==-1) first=0;
            int last=str.Find(q.Right(1), first);
            if(last==-1) last=first;
            CString test=str.Mid(first,last);
            int l=levenshtein_distance(CStringA(query).MakeUpper().GetString(), CStringA(test).MakeUpper().GetString());*/
            //CString s; s.Format(L"%3.3d %s", l, test.GetString());
            if(CString(str).MakeUpper().Find(CString(query).MakeUpper())!=-1) {
                RuleResult r;
                r.expandStr=lnks[i];
                r.display=str;
                r.rule=this;
                results.push_back(r);
            }
        }
    }
    Rule *validate() {
        RuleResult *r=m_pUI->GetSelectedItem();

        g_history.push_back(*r);

        return new FileVerbRule(r->expandStr, m_pUI); 
    }
    Gdiplus::Bitmap *getIcon(RuleResult *r) {
        return ::getIcon(r->expandStr);
    }

    ClauncherDlg            *m_pUI;
};
