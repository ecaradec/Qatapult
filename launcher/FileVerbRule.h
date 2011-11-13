
HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, UINT idCmdOffset)
{
   CMINVOKECOMMANDINFO ici;
   ZeroMemory(&ici, sizeof(ici));
   ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
   ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCmdOffset);
   ici.nShow = SW_SHOWNORMAL;

   return pCM->InvokeCommand(&ici);
}

struct FileVerbRule : Rule {
    FileVerbRule(const CString &filepath, ClauncherDlg *pLB):m_pUI(pLB) {        
        CString fp(filepath); fp.TrimRight(L"\\");
        pLB->SetWindowText(fp);

        CString d=fp.Left(fp.ReverseFind(L'\\'));
        CString filename=fp.Mid(fp.ReverseFind(L'\\')+1);
        
        CComPtr<IShellFolder> psfDesktop, psfWinFiles;
        PIDLIST_RELATIVE pidlFolder, pidlItem;
        DWORD eaten;
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
            m_pContextMenu->QueryContextMenu(m.GetSafeHmenu(), 0, 0, 0xFFFF, CMF_EXTENDEDVERBS);
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
                    s+=" [MENU]";

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
    void collect(const TCHAR *query, std::vector<RuleResult> &results) {
        for(std::list<Command>::iterator it=m_commands.begin();it!=m_commands.end();it++) {
            if(CString(it->display).MakeUpper().Find(CString(query).MakeUpper())!=-1) {
                RuleResult r;
                r.display=it->display;
                r.expandStr=it->display;
                r.id=it->id;
                r.rule=this;
                results.push_back(r);
            }
        }
    }
    Rule *validate() {
        RuleResult *r=m_pUI->GetSelectedItem();
        HRESULT hr=ProcessCMCommand(m_pContextMenu, r->id);
        return 0;
    }
    Gdiplus::Bitmap *getIcon(RuleResult *r) {
        WCHAR buff[256];
        m_pContextMenu->GetCommandString(r->id, GCS_VERBW, 0, (LPSTR)buff, sizeof(buff));
        Gdiplus::Bitmap *bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\"+CString(buff)+L".png");
        if(bmp->GetLastStatus()!=Gdiplus::Ok) {
            delete bmp;
            bmp=Gdiplus::Bitmap::FromFile(L"..\\icons\\defaultverb.png");
        }
        return bmp;
    }

    IContextMenu        *m_pContextMenu;
    ClauncherDlg            *m_pUI;
    struct Command {
        CString expandStr;
        CString display;
        int     id;
    };
    std::list<Command> m_commands;
};
