#pragma once
struct KVPack;

struct PredicateParser {
    PredicateParser(const TCHAR *expr);
    bool match(KVPack &pack, uint8 *o);
    bool isIdentifier(TCHAR *str);
    bool isChar(TCHAR *str, TCHAR c);
    CString readIdentifier(TCHAR **str);
    void readToken(TCHAR **str, TCHAR *m);
    CString readOperator(TCHAR **str);
    CString readValue(TCHAR **str);
    CString readRegEx(TCHAR **str);

    std::vector<std::pair<CString,CString> > conds;
};

