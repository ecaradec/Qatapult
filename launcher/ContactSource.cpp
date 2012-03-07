#include "stdafx.h"
#include "ContactSource.h"
#include "resource.h"

//get the key from the developer console https://code.google.com/apis/console/b/0/?pli=1#project:152444811162:access
CStringA clientId="152444811162-mhjnj3csgt4km2icp0uni71d3n3assln.apps.googleusercontent.com";
CStringA clientSecret="0yZWReVI_5zq9lPvuUH5TO2h";

BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        return TRUE;
        case WM_COMMAND:
            if(wParam==IDC_AUTHORIZE_APP) {
                ShellExecute(0, L"open", L"https://accounts.google.com/o/oauth2/auth?response_type=code&client_id="+CString(clientId)+L"&redirect_uri=urn:ietf:wg:oauth:2.0:oob&scope=https://www.google.com/m8/feeds", 0, 0, SW_SHOW);
            } else if(wParam==IDC_GMAIL_APPLY) {
                CHAR code[1024];
                GetDlgItemTextA(hWnd, IDC_CODE, code, sizeof(code));

                char data[4096];
                sprintf_s(data, sizeof(data), "code=%s&client_id=%s&client_secret=%s&redirect_uri=urn:ietf:wg:oauth:2.0:oob&grant_type=authorization_code", code, clientId, clientSecret);

                CStringA res;
                HttpSubmit(L"https://accounts.google.com/o/oauth2/token", data, &res);


                char accessToken[1024];
                int expireIn;
                char refreshToken[1024];
                sscanf_s(res, 
                         "{\n\"access_token\" : \"%[^\"]\",\n  \"token_type\" : \"Bearer\",\n  \"expires_in\" : %d,\n  \"refresh_token\" : \"%[^\"]\"\n}", 
                         accessToken, sizeof(accessToken), &expireIn, sizeof(expireIn), refreshToken, sizeof(refreshToken));

                SetSettingsStringA("gmailContacts", "accessToken", accessToken);
                SetSettingsStringA("gmailContacts", "refreshToken", refreshToken);
                
                g_pUI->InvalidateIndex();
            }
        return TRUE;
    }
    return FALSE;
}

ContactObject::ContactObject(const CString &k, Source *s, const CString &text, const CString &email):Object(k,L"CONTACT",s,text) {
    values[L"expand"]=text;
    values[L"email"]=email;
}
ContactObject *ContactObject::clone() {
    return new ContactObject(*this);
}
Gdiplus::Bitmap *ContactObject::getIcon(long flags) {
    if(GetFileAttributes("photos\\"+key+".jpg")!=INVALID_FILE_ATTRIBUTES)
        return Gdiplus::Bitmap::FromFile(L"photos\\"+key+".jpg");
    return Gdiplus::Bitmap::FromFile(L"icons\\contact.png");
}
/*void ContactObject::drawItem(Graphics &g, SourceResult *sr, RectF &r) {
    float fontsize=10.0f;

    Gdiplus::Font f(g_fontfamily,fontsize);
    Gdiplus::Font f2(g_fontfamily, 7.0f);         

    float textheight=f.GetHeight(&g);
    bool h=(float(r.Width)/r.Height)>2;

    RectF ricon=getStdIconPos(r,h,f.GetHeight(&g)+f2.GetHeight(&g));
    RectF rtext0=getStdTextPos(r,h,f.GetHeight(&g));
    RectF rtext=getStdTitlePos(r,h,f.GetHeight(&g)+f2.GetHeight(&g));
    RectF rtext2=getStdSubTitlePos(r,h,f2.GetHeight(&g));

    StringFormat sfcenter;
    sfcenter.SetAlignment(StringAlignmentNear);
    sfcenter.SetFormatFlags(StringFormatFlagsNoWrap);        
    sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

    ricon.Y+=10;

    if(sr->icon)
        g.DrawImage(sr->icon, ricon);
        
    CString email=getString(L"email");        
    if(email==L"") {
        drawEmphased(g, sr->display, g_pUI->getQuery(g_pUI->getFocus()), rtext0, DE_UNDERLINE,StringAlignmentNear, fontsize);
    } else {
        drawEmphased(g, sr->display, g_pUI->getQuery(g_pUI->getFocus()), rtext, DE_UNDERLINE,StringAlignmentNear, fontsize);
        g.DrawString(email, email.GetLength(), &f2, rtext2, &sfcenter, &SolidBrush(Color(0x88FFFFFF)));
    }
}*/

ContactSource::ContactSource() : DBSource(L"CONTACT",L"Contacts (Catalog )", L"contacts") {
    char *zErrMsg = 0;
    sqlite3_exec(db, "CREATE TABLE contacts(key TEXT PRIMARY KEY ASC, display TEXT, email TEXT, bonus INTEGER)", 0, 0, &zErrMsg);
    sqlite3_free(zErrMsg);

    UpgradeTable(db,"contacts");
}
void ContactSource::parseGmailContacts(const char *xml) {
    char *zErrMsg = 0;

    CStringA accessToken=GetSettingsStringA("gmailContacts", "accessToken");

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load(xml);

    pugi::xpath_node_set tools = doc.select_nodes("/feed/entry");

    // https://www.google.com/m8/feeds/photos/media/emmanuel.caradec%40gmail.com/2/Ek0KduuCgrDXaZrhFaol7Q?access_token=ya29.AHES6ZSjqhlK84cgFupCXYrCM4fN4pa5I3QYyqy41U3ak9NzlaOq4sA        
    // https://www.google.com/m8/feeds/photos/media/emmanuel.caradec@gmail.com/2?access_token=ya29.AHES6ZSjqhlK84cgFupCXYrCM4fN4pa5I3QYyqy41U3ak9NzlaOq4sA        
        
    // there can only exists one single mark
    //int mark;
    //sqlite3_exec(db, "SELECT mark FROM contacts LIMIT 1;", getIntCB, &mark, &zErrMsg);
    //sqlite3_free(zErrMsg);
        
    // begin
    int rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);
    sqlite3_free(zErrMsg);
    sqlite3_stmt *stmt=0;
    const char *unused=0;

    for (pugi::xpath_node_set::const_iterator it = tools.begin(); it != tools.end(); ++it)
    {
        pugi::xpath_node node = *it;
                                
        CStringA photo=node.node().child("link").attribute("href").value();
        char photoHref[4096];
        sprintf_s(photoHref, sizeof(photoHref),"%s?access_token=%s", photo, accessToken);                       
            
        CStringA email=node.node().child("gd:email").attribute("address").value();
        CStringA title=node.node().child_value("title"); if(title=="") title=email;
        CStringA key=node.node().child_value("id");
        key.Replace('/', '_');
        key.Replace(':', '_');

        //if(GetFileAttributesA("photos\\"+key+".jpg")==INVALID_FILE_ATTRIBUTES)
        //    HttpDownload(photoHref, "photos\\"+key+".jpg");

        rc = sqlite3_prepare_v2(db,
                                "INSERT OR REPLACE INTO contacts (key, display, email, bonus) VALUES(?, ?, ?, coalesce((SELECT bonus FROM contacts WHERE key=?), 0));\n",
                                -1, &stmt, &unused);
        rc = sqlite3_bind_text(stmt, 1, key.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text(stmt, 2, title.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text(stmt, 3, email.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text(stmt, 4, key.GetString(), -1, SQLITE_STATIC);            
        //rc = sqlite3_bind_int(stmt, 5, mark+1);
        rc = sqlite3_step(stmt);
        const char *errmsg=sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
    }
    rc = sqlite3_exec(db, "END;", 0, 0, &zErrMsg);
    sqlite3_free(zErrMsg);
        
    SetSettingsStringA("gmailContacts", "updated-min", doc.child("feed").child_value("updated"));
}
void ContactSource::crawl() {
    WCHAR curDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, curDir);

    CStringA accessToken=GetSettingsStringA("gmailContacts", "accessToken");
    if(strlen(accessToken)>0) {            
        CStringA res;
        int retry=0;
            
        CStringA updateMin=GetSettingsStringA("gmailContacts", "updated-min", "");
        if(updateMin=="")
            updateMin="2007-03-16T00:00:00";
        while(HttpGet(L"https://www.google.com/m8/feeds/contacts/default/full?max-results=9999&access_token="+CString(accessToken)+L"&updated-min="+CString(updateMin), &res)==401) {

            CStringA refreshToken=GetSettingsStringA("gmailContacts", "refreshToken");

            char data[4096];
            sprintf_s(data, sizeof(data), "client_id=%s&client_secret=%s&refresh_token=%s&grant_type=refresh_token", clientId, clientSecret, refreshToken);
            CStringA res;
            HttpSubmit(L"https://accounts.google.com/o/oauth2/token", data, &res);

            int expireIn;
            sscanf_s(res, 
                    "{\n\"access_token\" : \"%[^\"]\",\n  \"token_type\" : \"Bearer\",\n  \"expires_in\" : %d\n}", 
                    accessToken, sizeof(accessToken), &expireIn, sizeof(expireIn));

            SetSettingsStringA("gmailContacts", "accessToken", accessToken);
            retry++;
            if(retry>1)
                break;
        }
            
        parseGmailContacts(res);
    }
}
void ContactSource::collect(const TCHAR *query, std::vector<SourceResult> &results, int def, std::map<CString,bool> &activetypes) {
    if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
        return;

    CString q(query);
    sqlite3_stmt *stmt=0;
    const char *unused=0;
    int rc;

    rc = sqlite3_prepare_v2(db,"SELECT key, display, email, bonus FROM contacts WHERE display LIKE ?;",-1, &stmt, &unused);
    rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
    int i=0;
    while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
        results.push_back(SourceResult(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),        // key
                                        UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),        // display
                                        UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),        // expand
                                        this,                         // source
                                        sqlite3_column_int(stmt,3),   // id
                                        0,                            // data
                                        sqlite3_column_int(stmt,4))); // bonus

        results.back().object=new ContactObject(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),
                                                this,
                                                UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),
                                                UTF8toUTF16((char*)sqlite3_column_text(stmt,2)));
    }

    const char *errmsg=sqlite3_errmsg(db) ;
    sqlite3_finalize(stmt);         
}
Gdiplus::Bitmap *ContactSource::getIcon(SourceResult *r, long flags) {
    return r->object->getIcon(flags);
}
