#pragma once

// http://www.jeffkastner.com/2010/01/blat-stunnel-and-gmail/
struct EmailFileVerbRule : Rule {
    EmailFileVerbRule() {
    }
    virtual bool execute(std::vector<RuleArg> &args) {
        CString path=args[0].object()->getString(L"path");
        CString email=args[2].object()->getString(L"email");        
        CString filename=path.Right(path.GetLength() - (path.ReverseFind(L'\\')+1));

        sendEmail(email, filename, L"Here is your file", path);
        return true;
    }
};