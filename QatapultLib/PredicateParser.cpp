#include <stdafx.h>
#include "predicateparser.h"
#include "Object.h"
#include "Utility.h"

PredicateParser::PredicateParser(const TCHAR *expr) {
    TCHAR *p=(TCHAR*)expr;
        
    CString attr;
    CString op;
    CString v;
    CString type=readIdentifier(&p);

    conds.push_back(std::pair<CString,std::vector<CString> >(_T("type"),Array(type) ));

    if(!readToken(&p, L"["))
        return;
    while(1) {
        attr=readIdentifier(&p);
        op=readOperator(&p);
        v=readValue(&p);

        conds.push_back(std::pair<CString,std::vector<CString> >(attr,Explode(v,L'|')));

        if(*p!=L',') {
            break;
        }
        p++;
    }
    readToken(&p, L"]");
}

bool PredicateParser::match(Object *o) {
    for(std::vector<std::pair<CString,std::vector<CString> > >::iterator it=conds.begin(); it!=conds.end(); it++) {
        CString v(o->getString(it->first));
        if( std::find(it->second.begin(), it->second.end(), v) == it->second.end() )
            return false;
    }
    return true;
}

bool PredicateParser::isIdentifier(TCHAR *str) {
    return str[0]>=L'a' && str[0]<=L'z';
}

bool PredicateParser::isChar(TCHAR *str, TCHAR c) {
    return *str==c;
}

CString PredicateParser::readIdentifier(TCHAR **str) {
    CString tmp;
    while((**str>=L'a' && **str<=L'z') || (**str>=L'A' && **str<=L'Z') || **str==L'/' || **str==L'_' || (**str>=L'0' && **str<=L'9')) {
        tmp+=**str;
        (*str)++;
    }
    return tmp;
}
bool PredicateParser::readToken(TCHAR **str, TCHAR *m) {
    while(**str == *m && **str!=0 && *m!=0) {
        m++;
        (*str)++;
    }
    return *m==0;
}

CString PredicateParser::readOperator(TCHAR **str) {
    if((*str)[0]==L'~' && (*str)[1]==L'=') {
        (*str)+=2;
        return L"~=";
    } else if((*str)[0]==L'=') {
        (*str)+=1;
        return L"=";
    }
    return L"";
}
CString PredicateParser::readValue(TCHAR **str) {
    CString tmp;
    while(**str>=L'0' && **str<=L'9' ||
            **str>=L'a' && **str<=L'z' ||
            **str>=L'A' && **str<=L'Z' ||
            **str==L'.' ||
            **str==L'|' ) {
        tmp+=**str;
        (*str)++;
        if(**str==0) return tmp;
    }
    return tmp;
}
CString PredicateParser::readRegEx(TCHAR **str) {
    CString tmp;
    tmp+=**str;        
    (*str)++;  // read '/'
    if(**str==0) return tmp;
    while(**str!='/') {
        if(**str == L'\\') {
            (*str)++;
            if(**str==0) return tmp;
        }
        tmp+=**str;
        (*str)++;
        if(**str==0) return tmp;
    }
    tmp+=**str;
    (*str)++; // read extra '/'
    return tmp;
}