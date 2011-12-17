

// http://www.jeffkastner.com/2010/01/blat-stunnel-and-gmail/
struct EmailFileVerbRule : Rule {
    EmailFileVerbRule() : Rule(L"FILE", L"EMAILVERB",L"CONTACT") {
    }
    virtual bool execute(std::vector<SourceResult> &args) {
        CString path=args[0].source->getString(args[0].key+L"/path");
        CString email=args[2].source->getString(args[2].key+L"/email");        
        CString filename=path.Right(path.GetLength() - (path.ReverseFind(L'\\')+1));

        sendEmail(email, filename, L"Here is your file", path);
        return true;
    }
};