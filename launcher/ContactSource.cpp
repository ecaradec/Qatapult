#include "stdafx.h"
#include "ContactSource.h"
#include "resource.h"
#include "json/json.h"
#include "KVPack.h"

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


                char *errorPos = 0;
	            char *errorDesc = 0;
	            int errorLine = 0;
	            block_allocator allocator(1 << 10);
	
                json_value *root = json_parse((char*)res.GetString(), &errorPos, &errorDesc, &errorLine, &allocator);
                for (json_value *it = root->first_child; it; it = it->next_sibling) {
                    if(it->type==JSON_STRING && strcmp(it->name,"access_token")==0) {
                        SetSettingsStringA("gmailContacts", "accessToken", it->string_value);
                    } else if(it->type==JSON_STRING && strcmp(it->name,"refresh_token")==0) {
                        SetSettingsStringA("gmailContacts", "refreshToken", it->string_value);
                    }
                }
                
                g_pUI->invalidateIndex();
            }
        return TRUE;
    }
    return FALSE;
}

ContactObject::ContactObject(const CString &k, Source *s, const CString &text, const CString &email):Object(k,L"CONTACT",s,text) {
    values[L"expand"]=text;
    values[L"email"]=email;
}

void ContactObject::drawIcon(Graphics &g, RectF &r) {    
    if(!m_icon && GetFileAttributes("photos\\"+key+".jpg")!=INVALID_FILE_ATTRIBUTES)
        m_icon.reset(Gdiplus::Bitmap::FromFile(L"photos\\"+key+".jpg"));

    if(!m_icon)
        m_icon.reset(Gdiplus::Bitmap::FromFile(L"icons\\contact.png"));
    
    if(m_icon)
        g.DrawImage(m_icon.get(), r);
}
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
                                "INSERT OR REPLACE INTO contacts (key, display, email, uses) VALUES(?, ?, ?, coalesce((SELECT uses FROM contacts WHERE key=?), 0));\n",
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

            char *errorPos = 0;
	        char *errorDesc = 0;
	        int errorLine = 0;
	        block_allocator allocator(1 << 10);
	
            json_value *root = json_parse((char*)res.GetString(), &errorPos, &errorDesc, &errorLine, &allocator);

            for (json_value *it = root->first_child; it; it = it->next_sibling) {
                if(it->type==JSON_STRING && strcmp(it->name,"access_token")==0) {
                    SetSettingsStringA("gmailContacts", "accessToken", it->string_value);
                }
            }

            retry++;
            if(retry>1)
                break;
        }
            
        parseGmailContacts(res);
    }
}
void ContactSource::collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
    if(activetypes.size()>0 && activetypes.find(L"CONTACT")==activetypes.end())
        return;

    CString q(query);
    sqlite3_stmt *stmt=0;
    const char *unused=0;
    int rc;

    rc = sqlite3_prepare_v2(db,"SELECT key, display, email, uses FROM contacts WHERE display LIKE ?;",-1, &stmt, &unused);
    rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
    int i=0;
    while((rc=sqlite3_step(stmt))==SQLITE_ROW) {

        uint8 *pobj=pack.beginBlock();
        pack.pack(L"type",L"CONTACT");
        pack.pack(L"source",(uint32)this);
        pack.pack(L"key",UTF8toUTF16((char*)sqlite3_column_text(stmt,0)));
        pack.pack(L"text",UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));
        pack.pack(L"status",UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));
        pack.pack(L"expand",UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));
        pack.pack(L"email",UTF8toUTF16((char*)sqlite3_column_text(stmt,2)));
        pack.pack(L"bonus",(uint32)0);
        pack.pack(L"uses",(uint32)sqlite3_column_int(stmt,3));
        pack.endBlock(pobj);                  
    }

    const char *errmsg=sqlite3_errmsg(db) ;
    sqlite3_finalize(stmt);         
}
/*Gdiplus::Bitmap *ContactSource::getIcon(SourceResult *r, long flags) {
    return r->object()->getIcon(flags);
}*/


//key TEXT PRIMARY KEY ASC, display TEXT, email TEXT, bonus INTEGER
/*struct ContactsDB : DB {
    ContactsDB() : DB("contacts",
                      Array(std::make_pair("key",       TEXT),
                            std::make_pair("href",       TEXT),
                            std::make_pair("searchHref", TEXT),
                            std::make_pair("uses",       INTEGER)),1) {
    }
};

//DB contacts;

// how to handle tab ?
class ContactDlg : public SimpleOptDialog
{
public:
    enum { IDD = IDD_EMPTY };
 
    BEGIN_MSG_MAP(WebsiteSearchDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)        
        CHAIN_MSG_MAP(SimpleOptDialog)
    END_MSG_MAP()    

    WebsiteSearchDlg() {
    }
    void save(Record &r) {
        contacts.save(r);
    }
    bool del(Record &r) { 
        return contacts.del(r);
    }
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    { 
        SimpleOptDialog::OnInitDialog(uMsg,wParam,lParam,bHandled);

        columns=Array(CString(L"text"),CString(L"href"),CString(L"searchHref"));
        
        CRect r;
        RECT rcClient;                       // The parent window's client area.
        GetClientRect(&rcClient);

        int y=0;
        int x=0;

        r=rcClient;
        r.bottom=20;
        
        addLabel(L"Title", 0, y, 50, 20); addEdit(L"text", 55, y, 200, 20);                 y+=25;
        addLabel(L"Url",   0, y, 50, 20); addEdit(L"href",   55, y, rcClient.right-55, 20);   y+=25;
        addLabel(L"Query", 0, y, 50, 20); addEdit(L"searchHref", 55, y, rcClient.right-55, 20);   y+=25;

        x=rcClient.right-320;        
        addButton(L"New",   0, x, y, 100, 25); x+=110;
        addButton(L"Delete",1, x, y, 100, 25); x+=110;
        addButton(L"Save",  2, x, y, 100, 25); x+=110;        

        y+=30;
        hListView=addListView(rcClient.left, y, rcClient.right - rcClient.left, rcClient.bottom - y);

        ::GetClientRect(hListView, &r);

        // display, href, icon, key, searchhref
        addColumn(L"Title",0, 100);
        addColumn(L"Href", 1, 125);
        addColumn(L"Query",2, rcClient.right-225);

        records.clear();
        websites.query(records);

        int i=0;
        for(std::vector<Record>::iterator it=records.begin(); it!=records.end(); it++) {
            addItem(*it, columns );
        }

        if(records.size()==0) {
            enableEdition(false);
        } else {
            selectLVItem(0);
        }

        return S_OK;
    }
};
*/