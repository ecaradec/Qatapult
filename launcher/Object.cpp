#include "stdafx.h"
#include "Object.h"
#include "Source.h"
#include "ShellLink.h"
#include "Record.h"

Object::~Object() {
}
/*void Object::persists(pugi::xml_node &xml) {
    pugi::xml_node obj=xml.append_child("object");
    obj.attribute("type").set_value(type);
    obj.attribute("key").set_value(key);

    for(std::map<CString,CString>::iterator it=values.begin();it!=values.end();it++) {
        pugi::xml_node n=obj.append_child(UTF16toUTF8(it->first));
        n.set_value(UTF16toUTF8(it->second));
    }
}*/

CString escapeXml(const CString &xml) {
    CString tmp(xml);
    tmp.Replace(L">",L"&gt;");
    return tmp;
}

CString escapeJson(const CString &xml) {
    CString tmp(xml);
    tmp.Replace(L"\\",L"\\\\");
    tmp.Replace(L"\"",L"\\\"");
    return tmp;
}

CString Object::toXML() {
    CString tmp;
    tmp=L"<object type='"+escapeXml(type)+L"' key='"+escapeXml(key)+L"'>\n";
    //tmp+=L"  <key>"+key+L"</key>\n";
    for(std::map<CString,CString>::iterator it=values.begin();it!=values.end();it++)
        tmp+=L"  <"+it->first+L">"+escapeXml(it->second)+L"</"+it->first+L">\n";
    tmp+="</object>\n";
    return tmp;
}
CString Object::toJSON() {
    CString tmp;
    tmp=L"{";

    int s=0;
    tmp+=L"\"type\":\""+escapeJson(type)+L"\",";
    tmp+=L"\"key\":\""+escapeJson(key)+L"\",";
    for(std::map<CString,CString>::iterator it=values.begin();it!=values.end();it++) {
        tmp+=L"\""+escapeJson(it->first)+L"\":\""+escapeJson(it->second)+L"\"";
        s++;
        if(s!=values.size())
            tmp+=L",";        
    }
    tmp+=L"}";
    return tmp;
}

CString Object::getString(const TCHAR *val_) {
    CString v(val_);
    if(v==L"type")
        return type;
    else if(v==L"status")
        return values[L"text"];
    else if(v==L"expand" && values.find(L"expand")==values.end())
        return values[L"text"];
    return values[val_];
}
Gdiplus::Bitmap *Object::getIcon(long flags) {
    if(values[L"icon"]!=L"")
        return Gdiplus::Bitmap::FromFile(values[L"icon"]);

    if(source->m_icon!=L"")
        return Gdiplus::Bitmap::FromFile(source->m_icon);

    Gdiplus::Bitmap *bmp=Gdiplus::Bitmap::FromFile(L"icons\\"+key+L".png");
    if(bmp)
        return bmp;

    return Gdiplus::Bitmap::FromFile(L"icons\\default.png");
}
void Object::drawItem(Graphics &g, SourceResult *sr, RectF &r) {        
    if(!m_icon)
        m_icon.reset( getIcon(ICON_SIZE_LARGE) );
    if(m_icon)
        g.DrawImage(m_icon.get(), r);
}

void Object::drawListItem(Graphics &g, SourceResult *sr, RectF &r, float fontSize, bool selected, DWORD textcolor, DWORD bgcolor, DWORD focuscolor) {    
    Gdiplus::Font itemlistFont(g_fontfamily, fontSize, FontStyleBold, UnitPoint);
    Gdiplus::Font itemscoreFont(g_fontfamily, fontSize);

    if(selected)
        g.FillRectangle(&SolidBrush(Color(focuscolor)), r);
    else
        g.FillRectangle(&SolidBrush(Color(bgcolor)), r);

    if(!m_smallicon)
        m_smallicon.reset(getIcon(ICON_SIZE_SMALL));
        
    if(m_smallicon)
        g.DrawImage(m_smallicon.get(), RectF(r.X+10, r.Y, r.Height, r.Height)); // height not a bug, think a minute
        
    REAL x=r.X+r.Height+5+10;
        
    CString str(sr->object()->getString(L"text"));
    if(str[0]==source->m_prefix)
        str=str.Mid(1);

    g.DrawString(str, -1, &itemlistFont, RectF(x, r.Y+5.0f, r.Width, 14.0f), &source->sfitemlist, &SolidBrush(Color(textcolor)));
        
    StringFormat sfscore;
    sfscore.SetAlignment(StringAlignmentNear);
    g.DrawString(ItoS(sr->rank()), -1, &itemscoreFont, RectF(r.X+r.Height+5+10, r.Y+25, r.Width, r.Height), &sfscore, &SolidBrush(Color(textcolor)));

    Font pathfont(g_fontfamily, fontSize);
    StringFormat sfpath;
    sfpath.SetTrimming(StringTrimmingEllipsisPath);
    CString path(sr->object()->getString(L"path"));
    path.TrimRight(L'\\');
    g.DrawString(path.Left(path.ReverseFind(L'\\')), -1, &pathfont, RectF(r.X+r.Height+40, r.Y+25, r.Width-(r.X+r.Height+40), 14), &sfpath, &SolidBrush(Color(textcolor)));
}


FileObject::FileObject(Record &r,Source *s) {
    type=L"FILE";
    source=s;
    values=r.values;
    ivalues=r.ivalues;
}
FileObject::FileObject(const CString &k, Source *s, const CString &text, const CString &expand, const CString &path) :Object(k,L"FILE",s,text) {
    values[L"expand"]=expand;
    values[L"path"]=path;
}
FileObject::FileObject(const FileObject &f):Object(f) {
}
CString FileObject::getString(const TCHAR *val_) {
    CString val(val_);

    if(val==L"rdirectory") {
        CString fp(getString(L"rpath"));
        return fp.Left(fp.ReverseFind(L'\\'));
    } else if(val==L"directory") {
        CString fp(getString(L"path"));
        return fp.Left(fp.ReverseFind(L'\\'));
    } else if(val==L"rfilename") {
        CString fp(getString(L"rpath"));
        return fp.Mid(fp.ReverseFind(L'\\')+1);
    } else if(val==L"filename") {
        CString fp(getString(L"path")); 
        return fp.Mid(fp.ReverseFind(L'\\')+1);
    } else if(val==L"rpath") {
        CString path=getString(L"path");
        if(path.Right(4)==L".lnk")
            return getShortcutPath(getString(L"path"));
        else
            return path;
    } else if(val==L"status") {
        if(getString(L"path").Right(4)==L".lnk")        
            return Object::getString(L"text");
        else
            return getString(L"path");
    }

    return Object::getString(val);
}
Gdiplus::Bitmap *FileObject::getIcon(long flags) {    
    // allows to override the default system icon
    if(values[L"icon"]!=L"")
        return Gdiplus::Bitmap::FromFile(values[L"icon"]);

    return ::getIcon(values[L"path"],flags);
}

        