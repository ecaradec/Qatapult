//
// UserSource is a source of custom object added by the user in json format
//
#include "json/json.h"
#include "Source.h"
#include "Utility.h"

struct UserSource : Source {
    /*sqlite3 *db;
    sqlite3_stmt *getusesstmt;
    sqlite3_stmt *validatestmt;*/
    
    UserSource(const TCHAR *path=0) : Source(L"User") {
        if(path)
            loadJSONFile(path);
        /*FILE *f=fopen("databases\\user.json","r");
        char line[4096];
        while(fgets(line,4096,f)) {
                        
            char *errorPos = 0;
	        char *errorDesc = 0;
	        int errorLine = 0;
	        block_allocator allocator(1 << 10);
	
            json_value *root = json_parse(line, &errorPos, &errorDesc, &errorLine, &allocator);
        }
        fclose(f);*/
        /*type=name;
        int rc = sqlite3_open("databases\\user.db", &db);

        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE main(key TEXT PRIMARY KEY ASC, uses INTEGER DEFAULT 0)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        const char *unused=0;
        rc = sqlite3_prepare_v2(db,"SELECT uses FROM main WHERE key = ?;",-1, &getusesstmt, &unused);
        rc = sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO main (key, uses) VALUES(?, coalesce((SELECT uses FROM main WHERE key=?), 0)+1);",-1, &validatestmt, &unused);*/
    }
    ~UserSource() {
        /*sqlite3_finalize(getusesstmt);
        sqlite3_finalize(validatestmt);        
        sqlite3_close(db);*/
    }
    virtual void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes) {
        if(activetypes.size()==0)
            return;

        for(KVObject o=m_pack.root().first(); o!=m_pack.root().end(); o=o.next()) {
            if(activetypes.find(o.getString(L"type"))==activetypes.end())
                continue;
            if(!FuzzyMatch(o.getString(L"text"),query))
                continue;
            pack.writeObject(o);
        }

        /*if(activetypes.size()>0 && activetypes.find(type)==activetypes.end())
            return;

        CString q(query); q.MakeUpper();
        for(std::vector<std::pair<CString,CString> >::iterator it=m_index.begin(); it!=m_index.end();it++) {
            if(!FuzzyMatch(it->first,q)) continue;

            pack.begin(KV_Map);
                pack.writePairString(L"type",type);
                pack.writePairString(L"key",it->first);
                pack.writePairUint32(L"source",(uint32)this);
                pack.writePairString(L"text",it->first);
                pack.writePairString(L"icon",it->second);                
                pack.writePairUint32(L"bonus",20);
                int rc = sqlite3_bind_text16(getusesstmt, 1, it->first, -1, SQLITE_STATIC);                       
                if(sqlite3_step(getusesstmt)==SQLITE_ROW) {
                    pack.writePairUint32(L"uses",sqlite3_column_int(getusesstmt,0));
                } else {
                    pack.writePairUint32(L"uses",(uint32)0);
                }
                sqlite3_reset(getusesstmt);
            pack.end();
        }*/
    }
    void validate(Object *o) {
        /*int rc = sqlite3_bind_text16(validatestmt, 1, o->key, -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(validatestmt, 2, o->key, -1, SQLITE_STATIC);
        sqlite3_step(validatestmt);
        sqlite3_reset(validatestmt);
        const char *err=sqlite3_errmsg(db);*/
    }
    void loadJSON(const char *json) {
        char *errorPos = 0;
	    char *errorDesc = 0;
	    int errorLine = 0;
	    block_allocator allocator(1 << 10);
	            
        json_value *root = json_parse((char*)json, &errorPos, &errorDesc, &errorLine, &allocator);
        if(!root) return;
        if( root->type != JSON_ARRAY ) return;
        m_pack.begin(KV_Array);
        for (json_value *it = root->first_child; it; it = it->next_sibling) {
            if(it->type!=JSON_OBJECT) continue;
            m_pack.begin(KV_Map);
            m_pack.writePairUint32(L"source",(uint32)this);
            m_pack.writePairUint32(L"bonus",10);
            m_pack.writePairUint32(L"uses",10);
            for (json_value *it2 = it->first_child; it2; it2 = it2->next_sibling) {
                if(it2->type!=JSON_STRING) continue;                
                m_pack.writePairString(UTF8toUTF16(it2->name),UTF8toUTF16(it2->string_value));
            }
            m_pack.end();
        }
        m_pack.end();
    }
    void loadJSONFile(const TCHAR *path) {
        loadJSON(getFileContent(path));
    }
    KVPack  m_pack;
    CString m_type;
};
