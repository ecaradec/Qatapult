
// http://www.jeffkastner.com/2010/01/blat-stunnel-and-gmail/
struct EmailVerbRule : Rule {
    EmailVerbRule() : Rule(L"TEXT", L"EMAILVERB",L"CONTACT") {
    }
    virtual bool execute(std::vector<SourceResult> &args) {
        CString email=args[2].source->getString(args[2].key+L"/email");       

        CString content(args[0].display);
        CString subject;
        CString body=" ";
        int subjectEnd=content.Find(L".");
        if(subjectEnd==-1) {
            subject=content;
        } else {
            subject=content.Left(subjectEnd);
            body=content.Mid(subjectEnd+1);
        }

        sendEmail(email, subject, body, 0);
        return true;
    }
};
