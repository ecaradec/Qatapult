struct Type {
    Type(const CString &type) {
        m_type=type;
    }
    Type(const CString &type, const CString &icon) {
        m_type=type;
        m_icon=icon;
    }
    CString m_type;
    CString m_icon;
};

// a simple solution to mark the difference between sources and keywords
//Type Keyword(const CString &type, const CString &icon) {
//    return Type();
//}

struct Rule {
    Rule(){}
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

    int match(std::vector<SourceResult> &args, int l) {
        uint i; 
        for(i=0;i<args.size() && i<m_types.size() ;i++) {
            if(args[i].object && args[i].object->type!=m_types[i].m_type)
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