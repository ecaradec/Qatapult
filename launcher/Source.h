#pragma once
struct Source {
    Source(const CString& t) {
        m_name=t;
        type=t;
    }
    Source(const CString& t, const CString &n) {
        m_name=n;
        type=t;
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
    virtual void collect(const TCHAR *query, std::vector<SourceResult> &args, std::vector<SourceResult> &r, int def) {
        CString q(query); q.MakeUpper();
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(CString(it->second.display).MakeUpper().Find(q)!=-1) {
                r.push_back(it->second);
            }
        }
    }
    // validate
    virtual void validate(SourceResult *r) {
        m_index[r->key].bonus+=10;
        save();
    }

    virtual void save() {
        CString dir;
        GetCurrentDirectory(4096, dir.GetBufferSetLength(4096)); dir.ReleaseBuffer();
        int i=0;
        CString nb;
        for(std::map<CString, SourceResult>::iterator it=m_index.begin(); it!=m_index.end();it++) {
            nb.Format(L"%d",i);
            WritePrivateProfileString(m_name,nb+"_key",it->second.key, dir+L"\\db.ini"); // I should add a 'key' member : a string probably
            WritePrivateProfileString(m_name,nb+"_bonus",ItoS(it->second.bonus), dir+L"\\db.ini");
            i++;
        }
    }
    virtual void load() {
        CString dir;
        GetCurrentDirectory(4096, dir.GetBufferSetLength(4096)); dir.ReleaseBuffer();

        // the key could be an md5 or sha of some value
        for(int i=0;;i++) {
            CString key;
            GetPrivateProfileString(m_name, ItoS(i)+L"_key", L"", key.GetBufferSetLength(4096), 4096, dir+L"\\db.ini");
            if(key==L"")
                break;
            int bonus=GetPrivateProfileInt(m_name, ItoS(i)+L"_bonus", 0, dir+L"\\db.ini");
            m_index[key].bonus=bonus;
        }
    }

    // unused yet
    // get named data of various types
    virtual bool getData(const TCHAR *name, char *buff, int len) { return false; }
    virtual void release(SourceResult *r) {}
    virtual void rate(SourceResult *r) {}

    CString                         m_name;
    CString                         type;
    CString                         defaultQuery;
    std::map<CString, SourceResult> m_index;
};
