#pragma once
struct Object;

struct PredicateParser {
    PredicateParser(const TCHAR *expr);
    bool match(Object *o);
    bool isIdentifier(TCHAR *str);
    bool isChar(TCHAR *str, TCHAR c);
    CString readIdentifier(TCHAR **str);
    bool readToken(TCHAR **str, TCHAR *m);
    CString readOperator(TCHAR **str);
    CString readValue(TCHAR **str);
    CString readRegEx(TCHAR **str);

    std::vector<std::pair<CString,CString> > conds;
};

