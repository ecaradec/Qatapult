#pragma once
#include "PredicateParser.h"
//#include <atlrx.h>

struct Type {
    Type(const CString &type, bool multi=false) : m_predicate(type) {
        m_type=type;
        m_multi=multi;
    }
    bool match(RuleArg *o) {
        if(o->m_results.size()==0)
            return false;

        // no type at all: this is an empty object, it doesn't match anything
        if(o->m_results[0].object()==0)
            return false;

        // non multi results only match if the type is not multi
        if(!m_multi && o->m_results.size()>1)
            return false;

        if(!m_predicate.match(o->object().get()))
            return false;

        // match
        return true;
    }
    
    bool                  m_multi;
    CString               m_type;
    CString               m_icon;
    PredicateParser       m_predicate;
};

extern int rulescount;
struct Rule {
    Rule(){
    }
    Rule(const Type &arg1) {
        m_types.push_back(arg1);
    }
    Rule(const Type &arg1, const Type &arg2) {
        m_types.push_back(arg1);
        m_types.push_back(arg2);
    }
    Rule(const Type &arg1, const Type &arg2, const Type &arg3) {
        m_types.push_back(arg1);
        m_types.push_back(arg2);
        m_types.push_back(arg3);
    }
    virtual ~Rule() {
    }
    int match(std::vector<RuleArg> &args, int l) {
        // count how many args match the rule
        uint i; 
        for(i=0;i<args.size() && i<m_types.size() ;i++) {
            if(!m_types[i].match(&args[i]))
                break;
        }

        // if the command match everything in the buffer or more : perfect match
        // => if l is large enough a perfect match is the only possibility
        if(i>=m_types.size())
            return 2;

        // if the command match at least as much as the required length : partial match
        if(i>=uint(l))
            return 1;

        return 0;
    }
    virtual bool execute(std::vector<RuleArg> &args) { return true; }

    // add an default icon here ???
    std::vector<Type>     m_types;
    std::vector<RuleArg> *m_pArgs;
};