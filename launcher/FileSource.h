#pragma once
#include "geticon.h"
#include "sqlite3/sqlite3.h"
#include "FileVerbSource.h"
#include "ShellLink.h"

// history should be a source as well
/*
struct FileType {
    virtual CString getString(SourceResult &sr, const TCHAR *val) {
        if(CString(val)==L"rdirectory") {
            CString fp(getString(sr,L"rpath"));
            return fp.Left(fp.ReverseFind(L'\\'));
        } else if(CString(val)==L"directory") {
            CString fp(getString(sr,L"path"));
            return fp.Left(fp.ReverseFind(L'\\'));
        } else if(CString(val)==L"rfilename") {
            CString fp(getString(sr,L"rpath"));
            fp.TrimRight(L"\\");            
            return fp.Mid(fp.ReverseFind(L'\\')+1);
        } else if(CString(val)==L"filename") {
            CString fp(getString(sr,L"path"));            
            fp.TrimRight(L"\\");            
            return fp.Mid(fp.ReverseFind(L'\\')+1);
        } else if(CString(val)==L"rpath") {
            CString path(getItemString(sr.key,L"path"));
            if(path.Right(4)==L".lnk")
                return getShortcutPath(path);
            return path;
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
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        Gdiplus::Font fn(GetSettingsString(L"general",L"font",L"Arial"),8.0f);
        float textheight=fn.GetHeight(&g);
        bool h=(float(r.Width)/r.Height)>2;

        StringAlignment alignment=getStdAlignment(h);
        RectF ricon=getStdIconPos(r,h);
        RectF rtext=getStdTextPos(r,h,textheight);
        
        if(sr->icon)
            g.DrawImage(sr->icon, ricon);        
        
        drawEmphased(g, sr->display, m_pUI->getQuery(), rtext, DE_UNDERLINE, alignment);
    }
    virtual void drawListItem(Graphics &g, DRAWITEMSTRUCT *dis, RectF &r) {        
        if(dis->itemData==0)
            return;

        //if(dis->itemState&ODS_SELECTED)
        //    g.FillRectangle(&SolidBrush(Color(0xFFDDDDFF)), r);
        //else
        g.FillRectangle(&SolidBrush(Color(0xFFFFFFFF)), r);

        SourceResult *sr=(SourceResult*)dis->itemData;
        if(!sr->smallicon)
            sr->smallicon=getIcon(sr,ICON_SIZE_SMALL);
        
        if(sr->smallicon)
            g.DrawImage(sr->smallicon, RectF(r.X+10, r.Y, r.Height, r.Height)); // height not a bug, think a minute
        
        REAL x=r.X+r.Height+5+10;
        
        CString str(sr->display);
        if(str[0]==m_prefix)
            str=str.Mid(1);

        g.DrawString(str, -1, &itemlistFont, RectF(x, r.Y+5.0f, r.Width-x, 14.0f), &sfitemlist, &SolidBrush(Color(0xFF000000)));
        
        StringFormat sfscore;
        sfscore.SetAlignment(StringAlignmentNear);
        g.DrawString(ItoS(sr->rank), -1, &itemscoreFont, RectF(r.X+r.Height+5+10, r.Y+25, r.Width, r.Height), &sfscore, &SolidBrush(Color(0xFF000000)));

        Font pathfont(GetSettingsString(L"general",L"font",L"Arial"), 8.0f);
        StringFormat sfpath;
        sfpath.SetTrimming(StringTrimmingEllipsisPath);
        CString path(sr->source->getString(*sr,L"path"));
        path.TrimRight(L'\\');
        g.DrawString(path.Left(path.ReverseFind(L'\\')), -1, &pathfont, RectF(r.X+r.Height+5+40, r.Y+25, r.Width-(r.X+r.Height+5+40), 14), &sfpath, &SolidBrush(Color(0xBB000000)));
    }
    virtual void validate(SourceResult *r)  {} // ail je ne sais pas de quelle base il s'agit ?
    virtual void crawl() {}
    // copy makes a deep copy
    virtual void copy(const SourceResult &r, SourceResult *out) {
        *out=r;
        if(r.icon)
            out->icon=r.icon->Clone(0,0,r.icon->GetWidth(),r.icon->GetHeight(),r.icon->GetPixelFormat());
        if(r.smallicon)
            out->smallicon=r.smallicon->Clone(0,0,r.smallicon->GetWidth(),r.smallicon->GetHeight(),r.smallicon->GetPixelFormat());
    }
    virtual void clear(SourceResult &r) {
        delete r.icon; r.icon=0;
        delete r.smallicon; r.smallicon=0;
    }
};
*/


struct FileSource : Source {
    FileSource() : Source(L"FILE",L"Filesystem (Catalog )") {
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

                if(FuzzyMatch(foldername,f)!=-1 && f==L"") {
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

                if(FuzzyMatch(w32fd.cFileName,f)) {
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
        Gdiplus::Font fn(GetSettingsString(L"general",L"font",L"Arial"),8.0f);
        float textheight=fn.GetHeight(&g);
        bool h=(float(r.Width)/r.Height)>2;

        StringAlignment alignment=getStdAlignment(h);
        RectF ricon=getStdIconPos(r,h);
        RectF rtext=getStdTextPos(r,h,textheight);

        if(sr->icon)
            g.DrawImage(sr->icon, ricon);

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        CString q(m_pUI->getQuery());
        CString d=q.Left(q.ReverseFind(L'\\'));
        CString f=q.Mid(q.ReverseFind(L'\\')+1);

        drawEmphased(g, sr->display, f, rtext, DE_UNDERLINE, alignment);

        m_pUI->setStatus(getString(*sr,L"path"));
    }
    void validate(SourceResult *r) {

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        CString path=r->source->getString(*r,L"path");
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
                                "INSERT OR REPLACE INTO files(key,display,expand,path,bonus,mark) VALUES(?, ?, ?, ?, coalesce((SELECT bonus FROM files WHERE key=?), 0), ?);\n",
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
        if(CString(val)==L"rdirectory") {
            CString fp(getString(sr,L"rpath"));
            return fp.Left(fp.ReverseFind(L'\\'));
        } else if(CString(val)==L"directory") {
            CString fp(getString(sr,L"path"));
            return fp.Left(fp.ReverseFind(L'\\'));
        } else if(CString(val)==L"rfilename") {
            CString fp(getString(sr,L"rpath"));
            fp.TrimRight(L"\\");            
            return fp.Mid(fp.ReverseFind(L'\\')+1);
        } else if(CString(val)==L"filename") {
            CString fp(getString(sr,L"path"));            
            fp.TrimRight(L"\\");            
            return fp.Mid(fp.ReverseFind(L'\\')+1);
        } else if(CString(val)==L"rpath") {
            CString path(getItemString(sr.key,L"path"));
            if(path.Right(4)==L".lnk")
                return getShortcutPath(path);
            return path;
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

struct FileHistorySource : Source {
    FileHistorySource() : Source(L"FILE",L"File History (Catalog )") {
        int rc = sqlite3_open("databases\\files.db", &db);
        
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE files(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    ~FileHistorySource() {
        sqlite3_close(db);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int flags) {
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
    }
    virtual void drawItem(Graphics &g, SourceResult *sr, RectF &r) {
        Gdiplus::Font fn(GetSettingsString(L"general",L"font",L"Arial"),8.0f);
        float textheight=fn.GetHeight(&g);
        bool h=(float(r.Width)/r.Height)>2;

        StringAlignment alignment=getStdAlignment(h);
        RectF ricon=getStdIconPos(r,h);
        RectF rtext=getStdTextPos(r,h,textheight);

        if(sr->icon)
            g.DrawImage(sr->icon, ricon);

        StringFormat sfcenter;
        sfcenter.SetAlignment(StringAlignmentCenter);    
        sfcenter.SetTrimming(StringTrimmingEllipsisCharacter);

        CString q(m_pUI->getQuery());
        CString d=q.Left(q.ReverseFind(L'\\'));
        CString f=q.Mid(q.ReverseFind(L'\\')+1);

        drawEmphased(g, sr->display, f, rtext);

        m_pUI->setStatus(getString(*sr,L"path"));
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
        } else if(CString(val)==L"path") {
            CString path(getItemString(sr.key,L"path"));
            if(path.Right(4)==L".lnk")
                return getShortcutPath(path);
            return path;
        }
        return getItemString(sr.key,val);
    }

    sqlite3 *db;
};