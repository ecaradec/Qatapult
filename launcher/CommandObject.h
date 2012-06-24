#include "makeObject.h"

struct CommandObject : Object {
    CommandObject(std::vector<RuleArg> &ra, Source *s) {
        m_args = ra;
        source = s;
        type = L"COMMAND";                
        CString name;
        for(int i=0;i<ra.size();i++) {
            if(i!=0) name+=L" > ";
            name+=ra[i].display();
        }

        key = name;
        values[L"text"]=name;
    }
    CString toXML() {
        CString tmp=L"<command>\n";
        for(std::vector<RuleArg>::iterator it=m_args.begin(); it!=m_args.end(); it++) {
            tmp+=it->object()->toXML();
        }
        tmp+=L"</command>\n";
        return tmp;
    }
    CString toJSON() {
        int i=0;
        CString tmp=L"[";
        for(std::vector<RuleArg>::iterator it=m_args.begin(); it!=m_args.end(); it++) {
            tmp+=it->object()->toJSON();
            i++;
            if(i!=m_args.size()) tmp+=L",";
        }
        tmp+=L"]";
        return tmp;
    }
    void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        m_args[0].object()->drawItem(g, 0, r);
    }
    void drawListItem(Graphics &g, SourceResult *sr, RectF &r, float fontsize, bool b, DWORD textcolor, DWORD bgcolor, DWORD focuscolor) {
        m_args[0].object()->drawListItem(g, 0, r, fontsize, b, textcolor, bgcolor, focuscolor);
    }
    std::vector<RuleArg> m_args;
};

