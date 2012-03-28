struct ScriptRule : Rule {    
    ScriptRule(ActiveScriptHost *pHost, const CString &script)
        :m_script(script),m_pHost(pHost) {
    }
    bool execute(std::vector<SourceResult> &args) {
        CComVariant v;
        m_pHost->Eval(m_script,&v);
        return true;
    }
    CString m_script;
    ActiveScriptHost *m_pHost;
};
