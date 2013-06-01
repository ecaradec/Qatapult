/*
struct SourceOfSources : Source {
    SourceOfSources() : Source(L"SOURCE",L"Source of sources") {
        m_icon=L"icons\\source.png";
    }
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::vector<Source*>::iterator it=m_sources.begin(); it!=m_sources.end(); it++) {
            if(FuzzyMatch((*it)->m_name,q)) {
                SourceResult r;
                r.expand()=r.display()=(*it)->m_name;
                r.source()=this;
                r.data()=*it;
                results.push_back(r);
                results.back().object().reset(new Object((*it)->m_name,type,this,(*it)->m_name));                
            }        
        }
    }
    virtual Source *getSource(SourceResult &sr, CString &q) {
        q=L"";
        return (Source*)sr.data();
    }
    std::vector<Source*> m_sources;
};*/