
struct Rule {
    Rule(const CString &arg1type) {
        m_types.push_back(arg1type);
    }
    Rule(const CString &arg1type, const CString &verbtype) {
        m_types.push_back(arg1type);
        m_types.push_back(verbtype);
    }
    Rule(const CString &arg1type, const CString &verbtype, const CString &arg2type) {
        m_types.push_back(arg1type);
        m_types.push_back(verbtype);
        m_types.push_back(arg2type);
    }
    int match(std::vector<SourceResult> &args, int l) {
        uint i; 
        for(i=0;i<args.size();i++) {
            if(args[i].source && args[i].source->type!=m_types[i])
                break;
        }

        // if the command match everything in the buffer or more : perfect match
        // => if l is large enough a perfect match is the only possibility
        if(i>=m_types.size())
            return 2;

        // if the command match at least as much as the required length : partial match
        if(i>=l)
            return 1;

        return 0;
    }
    virtual bool execute(std::vector<SourceResult> &args) { return true; }

    std::vector<CString> m_types;
    std::vector<SourceResult> *m_pArgs;
};