#pragma once
#include "geticon.h"
#include "sqlite3/sqlite3.h"
#include "FileVerbSource.h"

struct FileSource : Source {
    FileSource() : Source(L"File",L"FILE") {
        type=L"FILE";

        int rc = sqlite3_open("databases\\files.db", &db);
        
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE files(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    ~FileSource() {
        sqlite3_close(db);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int flags) {
        CString q(query);

        if(q.GetLength()==0)
            return;

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
                                    "SELECT key, display, expand, 0, bonus FROM files WHERE display LIKE ?;",
                                    -1, &stmt, &unused);
            rc = sqlite3_bind_text16(stmt, 1, CString(L"%")+q.GetString()+L"%", -1, SQLITE_STATIC);
            int i=0;
            while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
                results.push_back(SourceResult(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),        // key
                                               UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),        // display
                                               UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),        // expand
                                               this,                         // source
                                               sqlite3_column_int(stmt,3),   // id
                                               0,                            // data
                                               sqlite3_column_int(stmt,4))); // bonus
            }

            const char *errmsg=sqlite3_errmsg(db) ;
            sqlite3_finalize(stmt);

            return;
        }

        CString Q(q); Q.MakeUpper();

        // network works for \\FREEBOX\Disque dur\ but not for \\FREEBOX\
        // need a way to enumerate servers and shares
        
        CString d=q.Left(q.ReverseFind(L'\\'));
        CString f=q.Mid(q.ReverseFind(L'\\')+1).MakeUpper();        

        // if we are matching exactly a directory without a ending slash add it
        if((q.Right(1)==L":" || q.Right(2)==L":\\") && q.GetLength()<=3) {
            SourceResult r;
            r.key=d+L"\\";
            r.display=d;
            r.expand=d+L"\\";
            r.source=this;
            r.bonus=100;
            results.push_back(r);
        }

        HANDLE h;
        WIN32_FIND_DATA w32fd;
        h=FindFirstFile(d+L"\\*", &w32fd);
        bool b=(h!=INVALID_HANDLE_VALUE);               
        while(b) {                    
            CString expand;
        
            if(CString(w32fd.cFileName)==L".") {
                CString noslash=q.Left(q.ReverseFind(L'\\'));
                CString foldername=noslash.Mid(noslash.ReverseFind(L'\\')+1);

                if(CString(foldername).MakeUpper().Find(f)!=-1 && f==L"") {
                    SourceResult r;
                    r.key=noslash+L"\\";
                    r.display=foldername;
                    r.expand=noslash+L"\\";
                    r.source=this;
                    r.bonus=1000;
                    results.push_back(r);
                }
            } else if(CString(w32fd.cFileName)==L"..") {
            } else {
                bool isdirectory=!!(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
                if(isdirectory /*&& f==CString(w32fd.cFileName).MakeUpper()*/)
                    expand = CString(d+L"\\"+w32fd.cFileName+L"\\");
                else
                    expand = CString(d+L"\\"+w32fd.cFileName);

                if(CString(w32fd.cFileName).MakeUpper().Find(f)!=-1) {
                    SourceResult r;
                    r.key=expand;
                    r.display=w32fd.cFileName;
                    r.expand=expand;
                    r.source=this;
                    r.rank=10;
                    results.push_back(r);
                }
            }
            b=!!FindNextFile(h, &w32fd);
        }
        FindClose(h);
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        if(sr->icon)
            g.DrawImage(sr->icon, RectF(r.X+10, r.Y+10, 128, 128));

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        CString q(m_pUI->getQuery());
        CString d=q.Left(q.ReverseFind(L'\\'));
        CString f=q.Mid(q.ReverseFind(L'\\')+1);

        drawEmphased(g, sr->display, f, RectF(r.X+10, r.Y+r.Height-17, r.Width-20, 20));

        m_pUI->setStatus(getString(*sr,L"path"));
    }
    void validate(SourceResult *r) {

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        CString path=r->source->getString(*r,L"path");

        CString filename(path);
        if(filename.Right(4)==L".lnk") {
            PathRemoveExtension(filename.GetBuffer()); filename.ReleaseBuffer();
        }
        filename=PathFindFileName(filename.GetBuffer()); filename.ReleaseBuffer();  

        CString key=md5(path);
            
        rc = sqlite3_prepare_v2(db,
                                "INSERT OR REPLACE INTO files(key,display,expand,path,bonus,mark) VALUES(?, ?, ?, ?, coalesce((SELECT bonus FROM files WHERE key=?), 0), ?);\n",
                                -1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, key.GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(stmt, 2, filename.TrimRight('\\').GetString(), -1, SQLITE_STATIC);
        rc = sqlite3_bind_text16(stmt, 3, path.TrimRight('\\').GetString(), -1, SQLITE_STATIC);
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
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        CString path=r->source->getString(*r,L"path");
        return ::getIcon(path,flags);
    }
    CString getItemString(const TCHAR *key, const TCHAR *val) {
        // if that key exists get the path from the base
        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        char buff[4096];
        sprintf(buff, "SELECT %s FROM files WHERE key = ?;\n", CStringA(val));
        rc = sqlite3_prepare_v2(db, buff, -1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, key, -1, SQLITE_STATIC);            
        rc=sqlite3_step(stmt);

        CString str=UTF8toUTF16((char*)sqlite3_column_text(stmt,0));            
        sqlite3_finalize(stmt);
        return str;
    }
    CString getString(SourceResult &sr, const TCHAR *val) {        
        if(CString(val)==L"directory") {
            CString fp(getString(sr,L"path"));
            return fp.Left(fp.ReverseFind(L'\\'));
        } else if(CString(val)==L"filename") {
            CString fp(getString(sr,L"path"));
            fp.TrimRight(L"\\");
            return fp.Mid(fp.ReverseFind(L'\\')+1);
        }

        CString str=getItemString(sr.key,val);
        if(str!="")
            return str;

        //  if value is path but the db query didn't return anything (this could ultimatly be stored in the extra data member )
        if(val==L"path") {
            return sr.key;
        }
        return L"";
    }

    sqlite3 *db;
};