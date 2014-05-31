#pragma once
//#include <atlrx.h>

struct Type {
    // only equality first
    struct Predicat {
        Predicat(const CString &n, const CString &op, const CString &v):m_name(n), m_value(v) {
            if(op==L"=")
                m_operator=0;
            else if(op==L"~=") {
                m_operator=1;
            }
        }
        int             m_operator;
        CString         m_name;
        CString         m_value;
    };
    /*Type(const std::map<CString,CString> &values) {
        
    }*/
    Type(const CString &type, bool multi=false, std::vector<Predicat> &predicates=std::vector<Predicat>()) {
        m_type=type;
        m_predicates=predicates;
        m_multi=multi;
    }
    Type(const CString &type, const CString &icon, bool multi=false, std::vector<Predicat> &predicates=std::vector<Predicat>()) {
        m_type=type;
        m_icon=icon;
        m_multi=multi;
        m_predicates=predicates;
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

        // there can't be multiple types in the results : the first one is the reference type and there is always at least one
        CString type=o->m_results[0].object()->type;
        if(type == L"")
            return false;

        // no match if the argument doesn't match this type               
        if(type != m_type)
            return false;

        // match
        return true;
    }
    
    bool                  m_multi;
    CString               m_type;
    CString               m_icon;
    std::vector<Predicat> m_predicates;
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