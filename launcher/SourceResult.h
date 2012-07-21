#pragma once
#include "Object.h"

struct Source;
struct Object;


struct SourceResult {
    void clear() {
        m_source=0;
        m_object=0;
        m_bonus=0;
        m_uses=0;
        m_id=0;
        m_data=0;
        m_rank=0;
    }
    SourceResult() {
        clear();
    }
    SourceResult(Object *o) {
        clear();
        m_display=o->getString(L"text");
        if(m_display==L"")
            m_display=o->getString(L"display");
        m_expand=o->getString(L"expand");        
        m_uses=o->ivalues[L"uses"];
        m_object.reset(o);

        m_source=o->source;
    }
    SourceResult(Source *s_, Object *o_, const CString &d_, const CString &e_, int b_, int u_, int id_, void *data_) {
        clear();
        m_source=s_;
        m_object.reset(o_);
        m_data=data_;
        m_expand=e_;
        m_bonus=b_;
        m_uses=u_;
        m_id=id_;
    }
    SourceResult(const CString &_key, const CString &_display, const CString &_expand, Source *_s, int _id=0, void *_data=0, int _uses=0) { 
        clear();
        m_source=_s;
        m_display=_display;
        m_expand=_expand;
        m_id=_id;
        m_data=_data;
        m_uses=_uses;
    }

    std::shared_ptr<Object> &object() {
        return m_object;
    }
    Source *&source() {
        return m_source;
    }
    CString &display() {
        return m_display;
    }
    CString &expand() {
        return m_expand;
    }
    CString &iconname() {
        return m_iconname;
    }
    int &bonus() {
        return m_bonus;
    }
    int &uses() {
        return m_uses;
    }
    int &id() {
        return m_uses;
    }
    void *&data() {
        return m_data;
    }
    int &rank() {
        return m_rank;
    }

    Source                          *m_source;
    std::shared_ptr<Object>          m_object;
    CString                          m_display;
    CString                          m_expand;
    int                              m_bonus;
    int                              m_uses;
    int                              m_id;
    void                            *m_data; // must be cloned if needed    

    // temporary for automatic
    CString                          m_iconname;

    // no saving required (and no copy )
    int                              m_rank;
};

struct RuleArg {
    RuleArg() { 
        //m_results.push_back(SourceResult());
    }
    RuleArg(const CString &_key, const CString &_display, const CString &_expand, Source *_s, int _id=0, void *_data=0, int _uses=0) { 
        //key=_key;
        m_results.push_back(SourceResult(_s,0,_display,_expand,0, _uses, _id, _data));
    }
    RuleArg(const RuleArg &s) {
        *this=s; // copy all fields stupidly
    }

    // easy access to the current object on RuleArg
    std::shared_ptr<Object> &object(int i=0) {
        return item(i).m_object;
    }
    Source *&source(int i=0) {
        return item(i).m_source;
    }
    CString &display(int i=0) {
        return item(i).m_display;
    }
    CString &expand(int i=0) {
        return item(i).m_expand;
    }
    CString &iconname(int i=0) {
        return item(i).m_iconname;
    }
    int &bonus(int i=0) {
        return item(i).m_bonus;
    }
    int &uses(int i=0) {
        return item(i).m_uses;
    }
    int &id(int i=0) {
        return item(i).m_uses;
    }
    void *&data(int i=0) {
        return item(i).m_data;
    }
    int &rank(int i=0) {
        return item(i).m_rank;
    }
    bool isEmpty() {
        return m_results.size()==0;
    }

    SourceResult &item(int i) {
        return m_results[m_results.size()-1-i];
    }

    std::vector<SourceResult> m_results;
};


