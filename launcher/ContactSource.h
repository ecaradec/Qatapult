#include "http.h"

BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        return TRUE;
        case WM_COMMAND:
            if(wParam==IDC_AUTHORIZE_APP) {
                ShellExecute(0, L"open", L"https://accounts.google.com/o/oauth2/auth?response_type=code&client_id=152444811162-mhjnj3csgt4km2icp0uni71d3n3assln.apps.googleusercontent.com&redirect_uri=urn:ietf:wg:oauth:2.0:oob&scope=https://www.google.com/m8/feeds", 0, 0, SW_SHOW);
            } else if(wParam==IDOK) {
                CHAR code[1024];
                GetDlgItemTextA(hWnd, IDC_CODE, code, sizeof(code));

                char data[4096];
                sprintf(data, "code=%s&client_id=152444811162-mhjnj3csgt4km2icp0uni71d3n3assln.apps.googleusercontent.com&client_secret=0yZWReVI_5zq9lPvuUH5TO2h&redirect_uri=urn:ietf:wg:oauth:2.0:oob&grant_type=authorization_code", code);

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
                
                MessageBox(0, L"Hello", L"Everything should be ok, type quit and restart to load your contact. It's still a beta right ?", MB_OK);
                ::EndDialog(hWnd, 0);
            } else if(wParam==IDCANCEL) {
                ::EndDialog(hWnd, 0);
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

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load(xml);
        pugi::xpath_node_set tools = doc.select_nodes("/feed/entry");

        
        CStringA q="BEGIN;\n";
        for (pugi::xpath_node_set::const_iterator it = tools.begin(); it != tools.end(); ++it)
        {
            pugi::xpath_node node = *it;
                                
            CStringA email=sqlEscapeString(node.node().child("gd:email").attribute("address").value());
            CStringA title=sqlEscapeString(node.node().child_value("title")); if(title=="") title=email;
            CStringA key=sqlEscapeString(email);
            q+="INSERT OR REPLACE INTO contacts (key, display, email, bonus) VALUES(\""+key+"\", \""+title+"\", \""+email+"\", coalesce((SELECT bonus FROM contacts WHERE key=\""+key+"\"), 0));\n";
        }
        q+="END;\n";
        //OutputDebugStringA(q);

        sqlite3_exec(db, q, 0, 0, &zErrMsg);
    }
    void crawl() {
        WCHAR curDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, curDir);

        TCHAR accessToken[1024];
        GetPrivateProfileString(L"QSLLContacts", L"accessToken", L"", accessToken, sizeof(accessToken), CString(curDir)+"\\settings.ini");
        if(wcslen(accessToken)>0) {
            CStringA res;
            HttpGet(L"https://www.google.com/m8/feeds/contacts/default/full?max-results=999&access_token="+CString(accessToken), &res);
            parseGmailContacts(res);
        }
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
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
        g.MeasureString(sr->display, sr->display.GetLength(), &f, r1, &rOut);
        g.DrawString(sr->display, sr->display.GetLength(), &f, r1, &sfcenter, &SolidBrush(Color(0xFFFFFFFF)));

        Gdiplus::Font f2(L"Arial", 7.0f); 
        r1.Y+=rOut.Height;
        CString email=getString(sr->key+L"/email");
        g.DrawString(email, email.GetLength(), &f2, r1, &sfcenter, &SolidBrush(Color(0x88FFFFFF)));
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return Gdiplus::Bitmap::FromFile(L"icons\\contact.png");
    }
};
