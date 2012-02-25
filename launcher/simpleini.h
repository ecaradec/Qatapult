#pragma once
#include "utf8.h"
    
inline CString ItoS(int i) {
    CString tmp; tmp.Format(L"%d",i);
    return tmp;
}

extern CString settingsini;
extern pugi::xml_document settings;
extern pugi::xml_document settingsWT; // settings for the working thread

inline CStringA GetSettingsStringA(const CHAR *key, const CHAR *value, const CHAR *defval="") {
    CStringA q; q.Format("settings/%s/%s",key,value);
    CStringA tmp=settings.select_single_node(q).node().child_value();
    if(tmp.GetLength()==0)
        return defval;
    return tmp;
}

inline CString GetSettingsString(const TCHAR *key, const TCHAR *value, const TCHAR *defval=_T("")) {
    CString q; q.Format(L"/settings/%s/%s",key,value);
    CStringA tmp=settings.select_single_node(UTF16toUTF8(q)).node().child_value();
    if(tmp.GetLength()==0)
        return defval;
    return UTF8toUTF16(tmp);
}

inline void SetSettingsStringA(const CHAR *key, const CHAR *value, const CHAR *v) {
    CStringA q; q.Format("settings/%s/%s",key,value);
    pugi::xml_node node=settings.select_single_node(q).node();
    pugi::xml_node firstchild = node.first_child();    
    if( !firstchild )
        node.append_child(pugi::node_pcdata).set_value(v);
    else
        firstchild.set_value(v);
    settings.save_file("settings.xml");
}

inline void SetSettingsString(const TCHAR *key, const TCHAR *value, const TCHAR *v) {
    CString q; q.Format(L"settings/%s/%s",key,value);
    pugi::xml_node node=settings.select_single_node(UTF16toUTF8(q)).node();
    pugi::xml_node firstchild = node.first_child();
    if( !firstchild )
        node.append_child(pugi::node_pcdata).set_value(UTF16toUTF8(v));
    else
        firstchild.set_value(UTF16toUTF8(v));
    settings.save_file("settings.xml");
}

inline int GetSettingsInt(const TCHAR *key, const TCHAR *value, int defval) {
    CString tmp=GetSettingsString(key,value);
    if(tmp.GetLength()!=0)
        return _ttoi(tmp);
    return defval;
}

inline void SetSettingsInt(const TCHAR *key, const TCHAR *value, int v) {
    SetSettingsString(key,value,ItoS(v));
}
