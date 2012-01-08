#include "http.h"
#include <time.h>

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
                ShellExecute(0, L"open", L"https://accounts.google.com/o/oauth2/auth?response_type=code&client_id=152444811162-mhjnj3csgt4km2icp0uni71d3n3assln.apps.googleusercontent.com&redirect_uri=urn:ietf:wg:oauth:2.0:oob&scope=https://www.google.com/m8/feeds", 0, 0, SW_SHOW);
            } else if(wParam==IDC_GMAIL_APPLY) {
                CHAR code[1024];
                GetDlgItemTextA(hWnd, IDC_CODE, code, sizeof(code));

                char data[4096];
                sprintf(data, "code=%s&client_id=%s&client_secret=%s&redirect_uri=urn:ietf:wg:oauth:2.0:oob&grant_type=authorization_code", clientId, clientSecret, code);

                CStringA res;
                HttpSubmit(L"https://accounts.google.com/o/oauth2/token", data, &res);


                char accessToken[1024];
                int expireIn;
                char refreshToken[1024];
                sscanf(res, 
                       "{\n\"access_token\" : \"%[^\"]\",\n  \"token_type\" : \"Bearer\",\n  \"expires_in\" : %d,\n  \"refresh_token\" : \"%[^\"]\"\n}", 
                       accessToken, &expireIn, refreshToken);

                WCHAR curDir[MAX_PATH];
                GetCurrentDirectory(MAX_PATH, curDir);
                WritePrivateProfileStringA("QSLLContacts", "accessToken", accessToken, CStringA(curDir)+"\\settings.ini");
                WritePrivateProfileStringA("QSLLContacts", "refreshToken", refreshToken, CStringA(curDir)+"\\settings.ini");
                
                g_pUI->InvalidateIndex();
            }
        return TRUE;
    }
    return FALSE;
}

// those kind of sources could have a simplified load and save ?
struct ContactSource : DBSource {    
    ContactSource() : DBSource(L"contacts", L"CONTACT") {
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE contacts(key TEXT PRIMARY KEY ASC, display TEXT, email TEXT, bonus INTEGER)", 0, 0, &zErrMsg);       
    }
    void parseGmailContacts(const char *xml) {
        char *zErrMsg = 0;
        FILE *f=fopen("contacts.xml", "w+");
        fwrite(xml,  strlen(xml), 1, f);
        fclose(f);

        WCHAR curDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, curDir);

        char accessToken[1024];
        GetPrivateProfileStringA("QSLLContacts", "accessToken", "", accessToken, sizeof(accessToken), CStringA(curDir)+"\\settings.ini");

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(xml);        

        pugi::xpath_node_set tools = doc.select_nodes("/feed/entry");

        // https://www.google.com/m8/feeds/photos/media/emmanuel.caradec%40gmail.com/2/Ek0KduuCgrDXaZrhFaol7Q?access_token=ya29.AHES6ZSjqhlK84cgFupCXYrCM4fN4pa5I3QYyqy41U3ak9NzlaOq4sA        
        // https://www.google.com/m8/feeds/photos/media/emmanuel.caradec@gmail.com/2?access_token=ya29.AHES6ZSjqhlK84cgFupCXYrCM4fN4pa5I3QYyqy41U3ak9NzlaOq4sA        
        
        // there can only exists one single mark
        //int mark;
        //sqlite3_exec(db, "SELECT mark FROM contacts LIMIT 1;", getIntCB, &mark, &zErrMsg);
        
        // begin
        int rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);
        sqlite3_stmt *stmt=0;
        const char *unused=0;        

        for (pugi::xpath_node_set::const_iterator it = tools.begin(); it != tools.end(); ++it)
        {
            pugi::xpath_node node = *it;
                                
            CStringA photo=node.node().child("link").attribute("href").value();
            char photoHref[4096];
            sprintf(photoHref, "%s?access_token=%s", photo, accessToken);                       
            
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
        
        WritePrivateProfileStringA("QSLLContacts", "updated-min", doc.child("feed").child_value("updated"), CStringA(curDir)+"\\settings.ini");
    }
    void crawl() {
        WCHAR curDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, curDir);

        char accessToken[1024];
        GetPrivateProfileStringA("QSLLContacts", "accessToken", "", accessToken, sizeof(accessToken), CStringA(curDir)+"\\settings.ini");
        if(strlen(accessToken)>0) {            
            CStringA res;
            int retry=0;

            TCHAR updateMin[256];
            GetPrivateProfileString(L"QSLLContacts", L"updated-min", L"2007-03-16T00:00:00", updateMin, sizeof(updateMin), CString(curDir)+"\\settings.ini");
            while(HttpGet(L"https://www.google.com/m8/feeds/contacts/default/full?max-results=9999&access_token="+CString(accessToken)+L"&updated-min="+updateMin, &res)==401) {

                char refreshToken[4096];
                GetPrivateProfileStringA("QSLLContacts", "refreshToken", "", refreshToken, sizeof(refreshToken), CStringA(curDir)+"\\settings.ini");

                char data[4096];
                sprintf(data, "client_id=%s&client_secret=%s&refresh_token=%s&grant_type=refresh_token", clientId, clientSecret, refreshToken);
                CStringA res;
                HttpSubmit(L"https://accounts.google.com/o/oauth2/token", data, &res);

                int expireIn;
                sscanf(res, 
                       "{\n\"access_token\" : \"%[^\"]\",\n  \"token_type\" : \"Bearer\",\n  \"expires_in\" : %d\n}", 
                       accessToken, &expireIn);

                WritePrivateProfileStringA("QSLLContacts", "accessToken", accessToken, CStringA(curDir)+"\\settings.ini");
                retry++;
                if(retry>1)
                    break;
            }
            
            parseGmailContacts(res);
        }
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        if(sr->icon)
            g.DrawImage(sr->icon, RectF(r.X+25, r.Y+45, 100, 100));

        RectF r1(r);
        r1.Y+=5;
        r1.X+=5;
        r1.Width-=10;
        r1.Height-=10;
        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentNear);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);
        Gdiplus::Font f(L"Arial", 12.0f); 

        RectF rOut;
        RectF rclip(r1);
        rclip.Width=9999;
        g.MeasureString(sr->display, sr->display.GetLength(), &f, rclip, &rOut);
        r1.Height=rclip.Height;
        //g.DrawString(sr->display, sr->display.GetLength(), &f, r1, &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));
        drawUnderlined(g, sr->display, m_pUI->getQuery(), r1, StringAlignmentNear, 12.0f);

        Gdiplus::Font f2(L"Arial", 7.0f); 
        r1.Y+=rOut.Height;
        CString email=getString(sr->key+L"/email");        
        g.DrawString(email, email.GetLength(), &f2, r1, &sfcenter, &SolidBrush(Color(0x88FFFFFF)));
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        if(GetFileAttributes("photos\\"+r->key+".jpg")!=INVALID_FILE_ATTRIBUTES)
            return Gdiplus::Bitmap::FromFile(L"photos\\"+r->key+".jpg");
        return Gdiplus::Bitmap::FromFile(L"icons\\contact.png");
    }
};
