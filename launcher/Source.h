#pragma once
struct Source {
    Source(const CString& t) {
        m_name=t;
        type=t;
        m_ignoreemptyquery=false;

        CString dir;
        GetCurrentDirectory(4096, dir.GetBufferSetLength(4096)); dir.ReleaseBuffer();
        m_db = dir+L"\\db.ini";
    }
    Source(const CString& t, const CString &n) {
        m_name=n;
        type=t;
        m_ignoreemptyquery=false;

        CString dir;
        GetCurrentDirectory(4096, dir.GetBufferSetLength(4096)); dir.ReleaseBuffer();
        m_db = dir+L"\\db.ini";
    }

    virtual ~Source() =0 {}
    // get icon
    virtual Gdiplus::Bitmap *getIcon(SourceResult *r) { return 0; }
    // draw
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        if(sr->icon)
            g.DrawImage(sr->icon, RectF(r.X+10, r.Y+10, 128, 128));

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        Gdiplus::Font f(L"Arial", 8.0f);
        g.DrawString(sr->display, sr->display.GetLength(), &f, RectF(r.X, r.Y+r.Height-15, r.Width, 20), &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
    }
    // get results
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &results, int def) {        
        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(CString(it->second.display).MakeUpper().Find(q)!=-1) {
                results.push_back(it->second);
            }
        }
    }
    // validate
    virtual void validate(SourceResult *r) {
        if(m_index.find(r->key) == m_index.end())
            return;
        m_index[r->key].bonus+=10;
        save(); // saving everything each time make everything a bit slows
    }

    virtual bool saveItem(int i, SourceResult *r) {
        // there is no need to write items with a 0 bonus
        // but there will be one, once it turned to an index
        //if(r->bonus == 0)
        //    return false;
        CString nb; nb.Format(L"%d",i);
        WritePrivateProfileString(m_name,nb+L"_source",r->source->m_name, m_db);
        WritePrivateProfileString(m_name,nb+L"_key",r->key, m_db);
        WritePrivateProfileString(m_name,nb+L"_display",r->display, m_db);
        WritePrivateProfileString(m_name,nb+L"_expand",r->expand, m_db);
        WritePrivateProfileString(m_name,nb+L"_bonus",ItoS(r->bonus), m_db);
        WritePrivateProfileString(m_name,nb+L"_id",ItoS(r->id), m_db);
        return true;
    }

    virtual bool loadItem(int i) {
        CString key;
        GetPrivateProfileString(m_name, ItoS(i)+L"_key", L"", key.GetBufferSetLength(4096), 4096, m_db); key.ReleaseBuffer();
        if(key==L"")
            return false;
        m_index[key].key=key;
        m_index[key].source=this;
        GetPrivateProfileString(m_name, ItoS(i)+L"_display", L"", m_index[key].display.GetBufferSetLength(256), 256, m_db); m_index[key].display.ReleaseBuffer();
        GetPrivateProfileString(m_name, ItoS(i)+L"_expand", L"", m_index[key].expand.GetBufferSetLength(256), 256, m_db); m_index[key].expand.ReleaseBuffer();
        m_index[key].bonus=GetPrivateProfileInt(m_name, ItoS(i)+L"_bonus", 0, m_db);
        m_index[key].id=GetPrivateProfileInt(m_name, ItoS(i)+L"_id", 0, m_db);        
        return true;
    }

    virtual void save() {
        int i=0;
        CString nb;
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(saveItem(i, &it->second))
                i++;                        
        }
    }
    virtual void load() {
        // the key could be an md5 or sha of some value
        for(int i=0;;i++) {
            if(!loadItem(i))
                break;
        }
    }

    virtual void updateIndex(std::map<CString,SourceResult> *index) {
        // update new data with bonus, then copy the new index
        // may be the bonuses shouldn't be saved there ? just collected separatly ? 
        for(std::map<CString, SourceResult>::iterator it=index->begin(); it!=index->end();it++) {
            it->second.bonus=m_index[it->first].bonus;
        }
        m_index.clear();
        m_index=*index;
    }

    virtual void crawl(std::map<CString,SourceResult> *index) {}

    // unused yet
    // get named data of various types
    virtual bool getSubResults(const TCHAR *itemkey, std::vector<SourceResult> &results) { return false; }
    virtual bool getString(const TCHAR *itemkey, CString &str) { return false; }
    virtual bool getInt(const TCHAR *itemkey, int &i) { return false; }

    virtual bool getData(const TCHAR *itemkey, const TCHAR *name, char *buff, int len) { return false; }
    virtual void release(SourceResult *r) {}
    virtual void rate(SourceResult *r) {}

    bool                            m_ignoreemptyquery;
    CString                         m_name;
    CString                         type;
    CString                         defaultQuery;
    CString                         m_db;
    std::map<CString, SourceResult> m_index;
    std::vector<SourceResult>      *m_pArgs;
};
