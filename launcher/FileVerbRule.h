#pragma once

struct FileVerbSource : Source {
    FileVerbSource(): m_def(-1), m_pContextMenu(0) {        
        type=L"FILEVERB";
    }
    ~FileVerbSource() {
        if(m_pContextMenu)
            m_pContextMenu->Release();
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &args, std::vector<SourceResult> &results, int def) {
        CString fp(args[0].expandStr); fp.TrimRight(L"\\");
        //pLB->SetWindowText(fp);

        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString filename=fp.Mid(fp.ReverseFind(L'\\')+1);
        
        if(m_curItem!=fp || m_def!=def) {
            m_commands.clear();

            m_curItem=fp;
            CComPtr<IShellFolder> psfDesktop, psfWinFiles;
            PIDLIST_RELATIVE pidlFolder, pidlItem;
            DWORD eaten;
            if(m_pContextMenu!=0)
                m_pContextMenu->Release();
            m_pContextMenu=0;
        
            HRESULT hr;
            if(FAILED(hr = SHGetDesktopFolder(&psfDesktop)))
                return;

            if(FAILED(hr=psfDesktop->ParseDisplayName(0, 0, (LPOLESTR)CStringW(d).GetBuffer(), 0, &pidlFolder, NULL)))
                return;

            if(FAILED(hr = psfDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder, (LPVOID *) &psfWinFiles)))
                return;
        
            // sometimes the given filename is not parsable under the given IShellFolder but is parsable under the desktop folder
            if(SUCCEEDED(hr = psfWinFiles->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(filename).GetBuffer(), &eaten, &pidlItem, 0)))
            {
                if(FAILED(hr=psfWinFiles->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)&m_pContextMenu)))
                    return;
            }
            else if(SUCCEEDED(hr= psfDesktop->ParseDisplayName(0, 0,  (LPOLESTR)CStringW(filename).GetBuffer(), &eaten, &pidlItem, 0)))
            {
                if(FAILED(hr=psfDesktop->GetUIObjectOf(::GetDesktopWindow(), 1, (LPCITEMIDLIST*)&pidlItem, IID_IContextMenu, 0, (void**)&m_pContextMenu)))
                    return;
            }
            if(m_pContextMenu) {
                CMenu m;
                m.CreatePopupMenu();
                if(def==0)
                    m_pContextMenu->QueryContextMenu(m.GetSafeHmenu(), 0, 0, 0xFFFF, CMF_NORMAL);
                else
                    m_pContextMenu->QueryContextMenu(m.GetSafeHmenu(), 0, 0, 0xFFFF, CMF_DEFAULTONLY);

                int i=0;
                CString s;
                for(int i=0;i<m.GetMenuItemCount();i++) {
            
                    MENUITEMINFO mii;
                    memset(&mii, 0, sizeof(mii));
                    mii.cbSize=sizeof(MENUITEMINFO);
                    WCHAR buff[1024]={0};
                    mii.dwTypeData=(LPWSTR)&buff;
                    mii.cch=sizeof(buff);
                    mii.fMask=MIIM_STRING|MIIM_SUBMENU|MIIM_ID;
                    bool b=m.GetMenuItemInfo(i, &mii,TRUE);

                    int c=m.GetMenuString(i, s, MF_BYPOSITION);
                    s.Replace(L"&&",L"__EAMP__");
                    s.Replace(L"&",L"");
                    s.Replace(L"__EAMP__",L"&");

                    if(mii.hSubMenu)
                        continue;
                        //s+=" [MENU]";

                    if(s.GetLength()>0) {
                        Command c;
                        c.expandStr=s;
                        c.display=s;
                        c.id=mii.wID;
                        m_commands.push_back(c);
                    }
                }
                //m.TrackPopupMenu(0, 0, 0, this, 0);
            }
             
            // add some homemade rules
            /*sources.push_back(L"Open");
            sources.push_back(L"Edit");
            sources.push_back(L"Properties");
            sources.push_back(L"Sendto");*/
        }


        for(std::list<Command>::iterator it=m_commands.begin();it!=m_commands.end();it++) {
            if(CString(it->display).MakeUpper().Find(CString(query).MakeUpper())!=-1) {
                SourceResult r;
                r.display=it->display;
                r.expandStr=it->expandStr;
                r.id=it->id;
                r.source=this;
                r.data=m_pContextMenu;
                results.push_back(r);
            }
        }
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r) {
        WCHAR buff[256];
        m_pContextMenu->GetCommandString(r->id, GCS_VERBW, 0, (LPSTR)buff, sizeof(buff));
        Gdiplus::Bitmap *bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\"+CString(buff)+L".png");
        if(bmp->GetLastStatus()!=Gdiplus::Ok) {
            delete bmp;
            bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\defaultverb.png");
        }
        return bmp;
    }

    int                  m_def;
    CString              m_curItem;
    IContextMenu        *m_pContextMenu;
    ClauncherDlg        *m_pUI;
    struct Command {
        CString expandStr;
        CString display;
        int     id;
    };
    std::list<Command> m_commands;
};
