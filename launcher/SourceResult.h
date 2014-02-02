#pragma once
#include "Object.h"

struct Source;
struct Object;


struct SourceResult {
    void clear() {
        m_object=0;
    }
    SourceResult() {
        clear();
    }
    SourceResult(Object *o) {
        clear();
        m_object.reset(o);
    }
    std::shared_ptr<Object> &object() {
        return m_object;
    }
    Source *source() {
        if(!m_object.get())
            return 0;
        return m_object->source;
    }
    CString display() {
        if(!m_object.get())
            return "";
        CString t=m_object->getString(L"text");
        if(t==L"")
            return m_object->getString(L"display");
        return t;
    }
    CString expand() {
        if(!m_object.get())
            return "";
        return m_object->getString(L"expand");
    }
    CString &iconname() {
        return m_object->m_iconname;
    }
    int &bonus() {
        return m_object->m_bonus;
    }
    int &uses() {
        return m_object->m_uses;
    }
    int &rank() {
        return m_object->m_rank;
    }
    // add all new values to Object class
    std::shared_ptr<Object>          m_object;
};

struct RuleArg {
    RuleArg() { 
    }
    RuleArg(const RuleArg &s) {
        *this=s; // copy all fields stupidly
    }
    // easy access to the current object on RuleArg
    std::shared_ptr<Object> &object(int i=0) {
        return item(i).m_object;
    }
    Source *source(int i=0) {
        return item(i).source();
    }
    CString display(int i=0) {
        return item(i).display();
    }
    CString expand(int i=0) {
        return item(i).display();
    }
    CString &iconname(int i=0) {
        return item(i).iconname();
    }
    int &bonus(int i=0) {
        return item(i).bonus();
    }
    int &uses(int i=0) {
        return item(i).uses();
    }
    int &rank(int i=0) {
        return item(i).rank();
    }
    bool isEmpty() {
        return m_results.size()==0;
    }

    SourceResult &item(int i) {
        return m_results[m_results.size()-1-i];
    }

    std::vector<SourceResult> m_results;
};


