#pragma once

// http://www.jeffkastner.com/2010/01/blat-stunnel-and-gmail/
struct EmailVerbRule : Rule {
    EmailVerbRule() {
    }
    virtual bool execute(std::vector<RuleArg> &args) {
        CString email=args[2].object(0)->getString(L"email");       

        CString content(args[0].object(0)->getString(L"text"));
        CString subject;
        CString body=" ";
        int subjectEnd=content.Find(L".");
        if(subjectEnd==-1) {
            subject=content;
        } else {
            subject=content.Left(subjectEnd);
            body=content.Mid(subjectEnd+1);
        }
        
        OutputDebugString(Format(L"sendemail '%s' '%s' '%s'\n",email,subject,body));
        sendEmail(email, subject, body, 0);
        return true;
    }
};
