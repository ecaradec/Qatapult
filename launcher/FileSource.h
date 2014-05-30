#pragma once
#include "geticon.h"
#include "sqlite3/sqlite3.h"
#include "FileVerbSource.h"
#include "ShellLink.h"
#include "TextObject.h"
#include "KVPack.h"

struct FileSource : Source {
    FileSource() : Source(L"FILE",L"Filesystem (Catalog )") {
        int rc = sqlite3_open("databases\\files.db", &db);
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE files(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        UpgradeTable(db,"files");
    }
    ~FileSource() {
        sqlite3_close(db);
    }
    void collect(const TCHAR *query, KVPack &pack, int flags, std::map<CString,bool> &activetypes) {
        if(activetypes.size()>0 && 
           activetypes.find(L"FILE")==activetypes.end())
            return;
        
        bool fileExpected=activetypes.find(L"FILE")!=activetypes.end();
        //bool newFileExpected=activetypes.find(L"FILE/NEW")!=activetypes.end();
        
        CString q(query);

        /*if(q.GetLength()==0) {
            if(newFileExpected) {
                FileObject *f=new FileObject(L"...",this,L"...",L"...",L"...");
                f->m_iconname=L"icons\\emptyfile.png";
                results.push_back(f);
            }
            return;
        }

        // not a root name ? search from history
        if(q.Find(L":")!=1 && q.Find(L"\\\\")==-1) {
            if(newFileExpected) {
                FileObject *f=new FileObject(L"...",this,L"...",L"...",L"...");
                f->m_iconname=L"icons\\emptyfile.png";
                results.push_back(f);
            }
            return;
        }*/

        CString Q(q); Q.MakeUpper();

        // network works for \\FREEBOX\Disque dur\ but not for \\FREEBOX\
        // need a way to enumerate servers and shares
        if(q.Find(L":")==1 && q.GetLength()==2)
            q+=L"\\";

        CString d=q.Left(q.ReverseFind(L'\\'));
        CString f=q.Mid(q.ReverseFind(L'\\')+1).MakeUpper();        

        // if we are matching exactly a directory without a ending slash add it
        if((q.Right(1)==L":" || q.Right(2)==L":\\") && q.GetLength()<=3) {
            uint8 *pobj=pack.beginBlock();
            pack.pack(L"type",L"FILE");
            pack.pack(L"source",(uint32)this);
            pack.pack(L"path",d+L"\\");
            pack.pack(L"expand",d+L"\\");
            pack.pack(L"filename",d);
            pack.pack(L"text",d);
            pack.pack(L"bonus",100);
            pack.pack(L"uses",(uint32)0);
            pack.endBlock(pobj);
            //results.push_back(new FileObject(d+L"\\",this,d,d+L"\\",d+L"\\"));
            //results.back().bonus()=100;
        }

        bool fileFound=false;
        HANDLE h;
        WIN32_FIND_DATA w32fd;
        h=FindFirstFile(d+L"\\*", &w32fd);
        bool b=(h!=INVALID_HANDLE_VALUE);               
        while(b) {                    
            CString expand;
        
            if(CString(w32fd.cFileName)==L".") {
                CString noslash=q.Left(q.ReverseFind(L'\\'));
                CString foldername=noslash.Mid(noslash.ReverseFind(L'\\')+1);

                if(FuzzyMatch(foldername,f) && f==L"") {
                    fileFound=true;

                    uint8 *pobj=pack.beginBlock();
                    pack.pack(L"type",L"FILE");
                    pack.pack(L"source",(uint32)this);
                    pack.pack(L"path",noslash+L"\\");
                    pack.pack(L"expand",noslash+L"\\");
                    pack.pack(L"filename",foldername);
                    pack.pack(L"text",foldername);
                    pack.pack(L"bonus",100);
                    pack.pack(L"uses",(uint32)0);
                    pack.endBlock(pobj);

                    //results.push_back(new FileObject(noslash+L"\\",this,foldername,noslash+L"\\",noslash+L"\\"));
                    //results.back().bonus()=100;
                }
            } else if(CString(w32fd.cFileName)==L"..") {
            } else {
                bool isdirectory=!!(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

                if(isdirectory /*&& f==CString(w32fd.cFileName).MakeUpper()*/)
                    expand = CString(d+L"\\"+w32fd.cFileName+L"\\");
                else
                    expand = CString(d+L"\\"+w32fd.cFileName);

                if(FuzzyMatch(w32fd.cFileName,f)) {
                    fileFound=true;
                    
                    uint8 *pobj=pack.beginBlock();
                    pack.pack(L"type",L"FILE");
                    pack.pack(L"source",(uint32)this);
                    pack.pack(L"path",expand);
                    pack.pack(L"expand",expand);
                    pack.pack(L"filename",w32fd.cFileName);
                    pack.pack(L"text",w32fd.cFileName);
                    pack.pack(L"status",expand);
                    pack.pack(L"bonus",(uint32)0);
                    pack.pack(L"uses",(uint32)0);
                    pack.endBlock(pobj);                    
                    
                    //results.push_back(new FileObject(expand,this,w32fd.cFileName,expand,expand));
                    //results.back().rank()=10;
                }
            }
            b=!!FindNextFile(h, &w32fd);
        }
        FindClose(h);

        /*if(newFileExpected && !fileFound) {
            CString q(query);
            CString filename=q.Mid(q.ReverseFind(L'\\')+1);
            FileObject *f=new FileObject(query,this,filename,query,query);
            f->m_iconname=L"icons\\emptyfile.png";
            results.push_back(f);
        }*/
    }
    void validate(SourceResult *r) {

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        CString path=r->object()->getString(L"path");
        path.TrimRight(L"\\");
        if((GetFileAttributes(path)&FILE_ATTRIBUTE_DIRECTORY)!=0)
            path+="\\";

        CString filename(path);
        if(filename.Right(4)==L".lnk") {
            PathRemoveExtension(filename.GetBuffer()); filename.ReleaseBuffer();
        }
        filename=PathFindFileName(filename.GetBuffer()); filename.ReleaseBuffer();

        CString key=md5(path);
            
        rc = sqlite3_prepare_v2(db,
                                "INSERT OR REPLACE INTO files(key,display,expand,path,uses,mark) VALUES(?, ?, ?, ?, coalesce((SELECT uses FROM files WHERE key=?)+1, 0),?);\n",
                                -1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, key.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(stmt, 2, filename.TrimRight('\\').GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(stmt, 3, path.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(stmt, 4, path.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(stmt, 5, key.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_int(stmt, 6, 0);
        rc = sqlite3_step(stmt);
        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(stmt);
    }
    void crawl() {
        // should scan the history and remove non available items
    }
    void rate(const CString &q, Object *r) {
        CString Q(q);

        Q=Q.Mid(Q.ReverseFind(L'\\')+1);

        r->m_rank=0;
        if(m_prefix!=0 && r->getString(L"text")[0]==m_prefix)
            r->m_rank+=100;

        CString P(r->getString(L"path"));

        P.TrimRight(L"\\");
        P=P.Mid(P.ReverseFind(L'\\')+1);        
        r->m_rank=min(100,r->m_uses*5) + r->m_bonus + r->m_rank+100*evalMatch(P,Q);
    }
    sqlite3 *db;
};

struct FileHistorySource : Source {
    FileHistorySource() : Source(L"FILE",L"File History (Catalog )") {
        int rc = sqlite3_open("databases\\files.db", &db);
        
        // each migration is unique
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE files(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        UpgradeTable(db,"files");
    }
    ~FileHistorySource() {
        sqlite3_close(db);
    }
    void collect(const TCHAR *query, KVPack &pack, int flags, std::map<CString,bool> &activetypes) {        
        if(activetypes.size()>0 && activetypes.find(L"FILE")==activetypes.end()) {
            return;
        }

        CString q(query);

        //if(q.GetLength()==0)
        //    return;

        // not a root name ? search from history
        if(q.Find(L":\\")!=1 && q.Find(L"\\\\")==-1) {
            CString q(query);
            q.Replace(L"_",L"\\_");
            q.Replace(L"%",L"\\%");
            q.Replace(L"'",L"\\'");
            q.Replace(L"\"",L"\\\"");

            char *zErrMsg = 0;
            
            sqlite3_stmt *stmt=0;
            const char *unused=0;
            int rc;

            rc = sqlite3_prepare_v2(db,
                                    "SELECT key, display, expand, path, uses FROM files WHERE display LIKE ?;",
                                    -1, &stmt, &unused);
            rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q).GetString(), -1, SQLITE_STATIC);
            int i=0;
            while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
                //
                // TODO : check if the file exists before returning and delete it if it doesn't
                //
                CString key=UTF8toUTF16((char*)sqlite3_column_text(stmt,0));
                CString path=UTF8toUTF16((char*)sqlite3_column_text(stmt,3));
                if(path.Mid(1,2)==":\\" && !FileExists(path)) {
                    sqlite3_stmt *del_stmt=0;
                    rc = sqlite3_prepare_v2(db, "DELETE FROM files WHERE key=?;", -1, &del_stmt, &unused);
                    rc = sqlite3_bind_text16(del_stmt, 1, key, -1, SQLITE_STATIC);
                    rc=sqlite3_step(del_stmt);
                    sqlite3_finalize(del_stmt);
                    continue;
                }

                uint8 *pobj=pack.beginBlock();
                pack.pack(L"type",L"FILE");
                pack.pack(L"source",(uint32)this);
                pack.pack(L"key",key);
                pack.pack(L"path",path);
                pack.pack(L"expand",UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));                
                pack.pack(L"text",UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));
                pack.pack(L"status",UTF8toUTF16((char*)sqlite3_column_text(stmt,1)));
                pack.pack(L"bonus",(uint32)0);
                pack.pack(L"uses",(uint32)sqlite3_column_int(stmt,4));
                pack.endBlock(pobj);  
            }

            const char *errmsg=sqlite3_errmsg(db) ;
            sqlite3_finalize(stmt);

            return;
        }
    }
    void validate(SourceResult *r) {
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        CString path=r->object()->getString(L"path");
        path.TrimRight(L"\\");
        
        CString key=md5(path);            
        rc = sqlite3_prepare_v2(db,
                                "UPDATE files SET uses=uses+1 WHERE key=?",
                                -1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, key.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(stmt);
    }
    
    void crawl() {
        // should scan the history and remove non available items
    }

    sqlite3 *db;
};