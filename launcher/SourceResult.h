#pragma once

struct Source;
struct Object;

struct SourceResult {
    SourceResult() { 
        m_results.push_back(Item());
        rank=0;
        smallicon=0;
        dirty=false;
    }
    SourceResult(const CString &_key, const CString &_display, const CString &_expand, Source *_s, int _id=0, void *_data=0, int _uses=0) { 
        //key=_key;
        m_results.push_back(Item(_s,0,_display,_expand,0, _uses, _id, _data));
        rank=0;
        smallicon=0;
        dirty=false;
    }
    SourceResult(const SourceResult &s) {
        *this=s; // copy all fields stupidly
    }

    // must save        
    Object *&object(int i=0) {
        return item(i).object;
    }
    Gdiplus::Bitmap *&icon(int i=0) {
        return item(i).icon;
    }
    Source *&source(int i=0) {
        return item(i).source;
    }
    CString &display(int i=0) {
        return item(i).display;
    }
    CString &expand(int i=0) {
        return item(i).expand;
    }
    int &bonus(int i=0) {
        return item(i).bonus;
    }
    int &uses(int i=0) {
        return item(i).uses;
    }
    int &id(int i=0) {
        return item(i).uses;
    }
    void *&data(int i=0) {
        return item(i).data;
    }

    struct Item {
        Item() {
            source=0;
            object=0;
            bonus=0;
            uses=0;
            id=0;
            data=0;
            icon=0;
        }

        Item(Source *s_, Object *o_, const CString &d_, const CString &e_, int b_, int u_, int id_, void *data_) {
            source=s_;
            object=o_;
            id=id_;
            expand=e_;
            bonus=b_;
            uses=u_;
            id=id_;
            data=data_;
            icon=0;
        }
        Source  *source;
        Object  *object;
        CString  display;
        CString  expand;
        int      bonus;
        int      uses;
        int      id;
        void    *data; // must be cloned if needed
        Gdiplus::Bitmap *icon;
    };
    Item &item(int i) {
        return m_results[m_results.size()-1-i];
    }

    std::vector<Item> m_results;

    // temporary for automatic
    CString  iconname;

    // no saving required (and no copy )
    int      rank;
    bool     dirty;    
    Gdiplus::Bitmap *smallicon;
};


