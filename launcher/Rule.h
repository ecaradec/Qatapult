#pragma once
#include <atlrx.h>

struct Type {
    // only equality first
    struct Predicat {
        Predicat(const CString &n, const CString &v, const CString &op=L"="):m_name(n), m_value(v) {
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

    Type(const CString &type, std::vector<Predicat> &predicates=std::vector<Predicat>()) {
        m_type=type;
        m_predicates=predicates;
    }
    Type(const CString &type, const CString &icon, std::vector<Predicat> &predicates=std::vector<Predicat>()) {
        m_type=type;
        m_icon=icon;
        m_predicates=predicates;
    }
    bool match(Object *o) {
        if(o->type != m_type)
            return false;
        for(std::vector<Predicat>::iterator it=m_predicates.begin(); it!=m_predicates.end(); it++) {
            CString n(o->getString(it->m_name));
            CString N(n); N.MakeUpper();
            switch(it->m_operator) {
            case 0: {
                //CString txt; txt.Format(L"Testing equal predicate : %s = %s => %s", N, it->m_value, (N == it->m_value)?L"true":L"false");
                //OutputDebugString(txt);
                if(N != it->m_value ) {
                    return false;
                }
                break;
            }
            case 1:
                CAtlRegExp<>    re;
                CAtlREMatchContext<> m;
                BOOL b=re.Parse( it->m_value ); 
                if( !re.Match(n, &m) )
                    return false;
                break;
            }
        }
        return true;
    }
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
    int match(std::vector<SourceResult> &args, int l) {
        uint i; 
        for(i=0;i<args.size() && i<m_types.size() ;i++) {
            if(args[i].object() && !m_types[i].match(args[i].object()))
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
    virtual bool execute(std::vector<SourceResult> &args) { return true; }

    // add an default icon here ???
    std::vector<Type>          m_types;
    std::vector<SourceResult> *m_pArgs;
};