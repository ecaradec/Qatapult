#include "SimpleOptDialog.h"
#include "Layout.h"

#undef SelectFont
#undef SelectPen
#undef SelectBitmap
#undef SelectBrush

struct ImageBtn : CWindowImpl<ImageBtn, CButton> {
    BEGIN_MSG_MAP(ImageBtn)
        MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
    END_MSG_MAP()

    ImageBtn() {
        bmp=0;
    }
    LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        bHandled=TRUE;

        DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
        Graphics g(dis->hDC);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.DrawImage(bmp, Gdiplus::RectF(0,0,29,29));

        return 0;
    }
    void setImage(const CString &img) {
        filename = img;
        delete bmp;
        bmp = Gdiplus::Bitmap::FromFile( img );
    }
    void OnFinalMessage(HWND) {
        delete bmp;
    }
    
    CString          filename;
    Gdiplus::Bitmap *bmp;
};

struct ImageCtrl : CWindowImpl<ImageCtrl, CStatic> {
    BEGIN_MSG_MAP(ImageCtrl)
        MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
    END_MSG_MAP()

    ImageCtrl() {
        bmp=0;
    }
    LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        bHandled=TRUE;

        DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
        Graphics g(dis->hDC);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.DrawImage(bmp, Gdiplus::RectF(0,0,29,29));

        return 0;
    }
    void setImage(const CString &img) {
        delete bmp;
        bmp = Gdiplus::Bitmap::FromFile( img );
    }
    void OnFinalMessage(HWND) {
        delete bmp;
    }
    
    Gdiplus::Bitmap *bmp;
};

CString getTextArg(pugi::xml_node &n) {
    if(n.child("lbl"))
        return UTF8toUTF16(n.child_value("lbl"));
    return UTF8toUTF16(n.child_value());
}
CString getTextType(pugi::xml_node &n) {
    if(n.child("lbl"))
        return L"K";
    return L"T";
}

// arguments
struct ArgCtrl : CWindowImpl<ArgCtrl, CTabCtrl> {
    CContainedWindow  m_tabtype;    
	CContainedWindow  m_tabkeyword;    
    ImageBtn          m_kwIcon;
    CComboBox cb1;
    enum {
        ID_MAP_TYPE=1,
        ID_MAP_KEYWORD
    };
    
    enum {
        ID_ARG_ICON=1,
        ID_ARG_TEXT
    };

    BEGIN_MSG_MAP(ArgCtrl)
        REFLECTED_NOTIFY_CODE_HANDLER(TCN_SELCHANGE, OnSelChangingArg)
        REFLECT_NOTIFICATIONS()
	ALT_MSG_MAP(ID_MAP_TYPE)
        if (GetFocus() != cb1)
        REFLECT_NOTIFICATIONS()
    ALT_MSG_MAP(ID_MAP_KEYWORD)
        COMMAND_HANDLER(ID_ARG_ICON, BN_CLICKED, OnClick)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()
    
    void OnFinalMessage(HWND hwnd) {
        m_kwIcon.Detach();
    }
    void Create(HWND hwnd, CRect &r, int id) {
        CRect rc;
		CPoint pos;
        
        CWindowImpl<ArgCtrl, CTabCtrl>::Create(hwnd, r, L"", WS_CHILD|WS_VISIBLE,0,id);        

		AddItem(L"Selector");
		AddItem(L"Keyword");

		// content of arguments
        GetClientRect(&rc);
		AdjustRect(FALSE,&rc);

        // create type tab
        m_tabtype.Create(L"#32770", this, ID_MAP_TYPE, *this, rc, L"", WS_CHILD|WS_VISIBLE);
            Layout l(m_tabtype, 0);
            
            l.pos.y+=6;
            l.pos.x+=5;
            cb1.Create(m_tabtype, l.getSpace(l.r.Width()-10, 20), L"", WS_CHILD|WS_VISIBLE|WS_BORDER|CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, ID_ARG_TEXT);
            for(int i=0;i<m_pTypes->size(); i++)
                cb1.AddString((*m_pTypes)[i]);

            l.pos.y-=6;

        // create keyword tab
		m_tabkeyword.Create(L"#32770", this, ID_MAP_KEYWORD, *this, rc, L"", WS_CHILD);
            Layout l2(m_tabkeyword, 0);    
                            
            m_kwIcon.Create(m_tabkeyword, l2.getSpace(32, 32), L"Icon", WS_CHILD|WS_VISIBLE|BS_OWNERDRAW, 0, ID_ARG_ICON);
            CEdit ed2;
            l2.pos.y+=6;
            ed2.Create(m_tabkeyword, l2.getSpace(l2.r.Width()-32-10, 20), L"", WS_CHILD|WS_VISIBLE|ES_CENTER|ES_AUTOHSCROLL|WS_BORDER, 0, ID_ARG_TEXT);            
            l2.pos.y-=6;

            l2.clearRow();
    }

    void setType(const CString &text) {
        clear();
        SetCurSel(0);
        cb1.SelectString(0,text);

        //m_tabtype.SetDlgItemText(ID_ARG_TEXT, text);
        updateTabVisibility();
    }
    void setKeyword(const CString &text, const CString &ico) {
        clear();
        SetCurSel(1);
        m_tabkeyword.SetDlgItemText(ID_ARG_TEXT, text);        
        m_kwIcon.setImage( ico );
        updateTabVisibility();
    }
    void clear() {
        m_tabtype.SetDlgItemText(ID_ARG_TEXT, L"");
        m_tabkeyword.SetDlgItemText(ID_ARG_TEXT, L"");        
        delete m_kwIcon.bmp;
        m_kwIcon.bmp=0;
    }
	LRESULT OnClick(WORD hiWord, WORD loWord, HWND lParam, BOOL& bHandled) {
        CFileDialog dlg(true, L"", L"", OFN_NOCHANGEDIR, L"*.*", *this);
        dlg.DoModal();
        m_kwIcon.setImage( dlg.m_szFileName );
        m_kwIcon.Invalidate(TRUE);
        return S_OK;
	}
    LRESULT OnSelChangingArg(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled) {
        updateTabVisibility();
        return S_OK;
	}
    void updateTabVisibility() {
        m_tabtype.ShowWindow(GetCurSel()==0?SW_SHOW:SW_HIDE);
		m_tabkeyword.ShowWindow(GetCurSel()==1?SW_SHOW:SW_HIDE);        
    }

    std::vector<CString> *m_pTypes;
};

//class ActionsDlg : public SimpleOptDialog
struct ActionsDlg : CDialogImpl<ActionsDlg>/*,
               CDialogResize<MyDlg>*/ {    

    enum {
        ID_LISTVIEW=1,
        ID_ARG1,
        ID_ARG2,
        ID_ARG3,
        ID_TABACTION,
        ID_NEW,
        ID_DEL,
        ID_SAVE
    };
    enum {
        ID_MAP_ACTION=1,
        ID_MAP_SCRIPT
    };
    enum {
        ID_ACTION_CMD=1,
        ID_ACTION_ARGS,
        ID_ACTION_DIR
    };
    enum {
        ID_SCRIPT_TEXT=1
    };

	BEGIN_MSG_MAP(ActionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SAVESETTINGS, OnSaveSettings)
        NOTIFY_HANDLER(ID_TABACTION, TCN_SELCHANGE, OnSelChangingAction)
        NOTIFY_HANDLER(ID_LISTVIEW, LVN_ITEMCHANGING, OnRuleSelecting)
        NOTIFY_HANDLER(ID_LISTVIEW, LVN_ITEMCHANGED, OnRuleSelected)
        COMMAND_HANDLER(ID_NEW, BN_CLICKED, OnNew)
        COMMAND_HANDLER(ID_DEL, BN_CLICKED, OnDel)
        COMMAND_HANDLER(ID_SAVE, BN_CLICKED, OnSave)
		REFLECT_NOTIFICATIONS()
    ALT_MSG_MAP(ID_MAP_ACTION)
    ALT_MSG_MAP(ID_MAP_SCRIPT)		
	END_MSG_MAP()

    // list of actions
    CButton           m_newBtn;
    CButton           m_delBtn;
    CListViewCtrl     m_listview;    

    ArgCtrl           m_argsctrl[3];
    
    // actions
    CTabCtrl          m_actiontab;
    CContainedWindow  m_actionpage;     
    CContainedWindow  m_scriptpage;
    CFont             m_largefont;    
    pugi::xpath_node  m_rules;    

    void OnFinalMessage(HWND) {
        m_listview.Detach();
        m_actiontab.Detach();
        m_actionpage.Detach();
        m_scriptpage.Detach();
        for(int i=0;i<3;i++) {
            m_argsctrl[i].m_tabtype.Detach();
            m_argsctrl[i].m_tabkeyword.Detach();
            m_argsctrl[i].m_kwIcon.Detach();
        }
        if(m_largefont)
            m_largefont.DeleteObject();
    }

    LRESULT OnNew(SHORT id, SHORT code, HWND lParam, BOOL& bHandled) {
        saveEdition(); // save current

        pugi::xml_node rule = m_rules.node().append_child("rule");

        pugi::xpath_node_set ns=settings.select_nodes("/settings/rules/rule");
        int s=ns.size();

        int pos=insertRule( rule );
        m_listview.SelectItem( pos );
        m_listview.SetFocus();

        enableEdition(true);
        return S_OK;
    }

    LRESULT OnDel(SHORT id, SHORT code, HWND lParam, BOOL& bHandled) {
        pugi::xpath_node_set ns=settings.select_nodes("/settings/rules/rule");
        int sel=m_listview.GetSelectedIndex();
        m_listview.DeleteItem(sel);
        if(sel==-1)
            return S_OK;
        m_rules.node().remove_child(ns[sel].node());
        
        if(m_listview.GetItemCount()>0) {
            if(sel-1>=0)
                sel--;
            m_listview.SelectItem( sel );
            m_listview.SetFocus();
        } else {
            clearEdition();
            enableEdition(false);
        }

        return S_OK;
    }

    LRESULT OnSave(SHORT id, SHORT code, HWND lParam, BOOL& bHandled) {
        saveEdition();
        return S_OK;
    }
    
    void clearEdition() {
        // clear edition
        for(int i=0;i<3;i++) {
            m_argsctrl[i].clear();
        }

        // set command
        m_actionpage.SetDlgItemText(ID_ACTION_CMD, L"");
        m_actionpage.SetDlgItemText(ID_ACTION_ARGS, L"");
        m_actionpage.SetDlgItemText(ID_ACTION_DIR, L"");
        // set script
        m_scriptpage.SetDlgItemText(ID_SCRIPT_TEXT, L"");
        
        updateActionTab();        
    }

    class dbg_xml_writer : public pugi::xml_writer
    {
    public:
        virtual void write(const void* data, size_t size) {
            CStringA s; s.SetString((char*)data,size);
            OutputDebugStringA(s);
        }
    };

    void saveEdition() {
        int sel=m_listview.GetSelectedIndex();
        if(sel==-1)
            return;
        pugi::xpath_node_set ns=settings.select_nodes("/settings/rules/rule");                
        int s=ns.size();

        pugi::xml_node r=ns[sel].node();
        while(r.first_child()) {
            r.remove_child(r.first_child());
        }

        for(int i=0;i<3;i++) {
            if( m_argsctrl[i].GetCurSel() == 0) {
                CString type;
                m_argsctrl[i].m_tabtype.GetDlgItemText(ArgCtrl::ID_ARG_TEXT, type);
                if(type=="")
                    continue;
                r.append_child("arg").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(type));
            } else {
                CString lbl;
                m_argsctrl[i].m_tabkeyword.GetDlgItemText(ArgCtrl::ID_ARG_TEXT, lbl);
                if(lbl=="")
                    continue;
                pugi::xml_node arg=r.append_child("arg");
                arg.append_child("lbl").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(lbl));
                arg.append_child("ico").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(m_argsctrl[i].m_kwIcon.filename));
            }
        }

        if( m_actiontab.GetCurSel() == 0) {
            // set command        
            CString cmd, args, workdir;
            m_actionpage.GetDlgItemText(ID_ACTION_CMD, cmd);
            r.append_child("cmd").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(cmd));
            m_actionpage.GetDlgItemText(ID_ACTION_ARGS, args);
            r.append_child("args").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(args));
            m_actionpage.GetDlgItemText(ID_ACTION_DIR, workdir);
            r.append_child("workdir").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(workdir));
        } else {
            // set script
            CString script;
            m_scriptpage.GetDlgItemText(ID_SCRIPT_TEXT, script);
            r.append_child("script").append_child(pugi::node_pcdata).set_value(UTF16toUTF8(script));
        }
                
        dbg_xml_writer w;
        r.print(w);

        updateRule(m_listview.GetSelectedIndex(), r);
    }

    void enableEdition(bool b) {
        //GetDlgItem(ID_NEW).EnableWindow(b);
        m_delBtn.EnableWindow(b);

        for(int i=0;i<3;i++) {
            m_argsctrl[i].EnableWindow(b);
            m_argsctrl[i].m_tabkeyword.GetDlgItem(ArgCtrl::ID_ARG_TEXT).EnableWindow(b);
            m_argsctrl[i].m_tabkeyword.GetDlgItem(ArgCtrl::ID_ARG_ICON).EnableWindow(b);
            m_argsctrl[i].m_tabtype.GetDlgItem(ArgCtrl::ID_ARG_TEXT).EnableWindow(b);
        }

        m_actiontab.EnableWindow(b);

        m_actionpage.GetDlgItem(ID_ACTION_CMD).EnableWindow(b);
        m_actionpage.GetDlgItem(ID_ACTION_ARGS).EnableWindow(b);
        m_actionpage.GetDlgItem(ID_ACTION_DIR).EnableWindow(b);

        m_scriptpage.GetDlgItem(ID_SCRIPT_TEXT).EnableWindow(b);        
    }
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        maxHeight=0;
		        
        Layout l(*this);

        CStatic rulesLbl;
        rulesLbl.Create(*this, l.getSpace(80,25), L"Rules : ", WS_CHILD|WS_VISIBLE|SS_RIGHT);

        m_newBtn.Create(*this, l.getSpace(100,25), L"New", WS_CHILD|WS_VISIBLE, 0, ID_NEW);
        m_delBtn.Create(*this, l.getSpace(100,25), L"Delete", WS_CHILD|WS_VISIBLE, 0, ID_DEL);

        l.clearRow();
        l.pos.x+=85;
        
        // list of rules
        m_listview.Create(*this, l.getSpace(l.r.right-l.pos.x, 112), L"", WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,WS_EX_CLIENTEDGE,ID_LISTVIEW);        
        CRect lvRect;
        m_listview.GetClientRect(&lvRect);
        m_listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);        
        m_listview.AddColumn(L"Command",1);
        m_listview.SetColumnWidth(0, 0.2*lvRect.Width());
        m_listview.AddColumn(L"Action",1);
        m_listview.SetColumnWidth(1, 0.795*lvRect.Width());
        l.clearRow();
        
        CStatic argsLbl;
        argsLbl.Create(*this, l.getSpace(80,25), L"Arguments : ", WS_CHILD|WS_VISIBLE|SS_RIGHT);

        // arguments of the rule
        int argWidth=(l.r.right-l.pos.x-5-5)/3;
        for(int i=0;i<3;i++) {
            m_argsctrl[i].Create(*this, l.getSpace(argWidth, 61), ID_ARG1+i);		    
        }

        l.clearRow();

        CStatic actionLbl;
        actionLbl.Create(*this, l.getSpace(80,25), L"Action : ", WS_CHILD|WS_VISIBLE|SS_RIGHT);
        
        // action of the rule
        CRect rcActionTab=l.getSpace(l.r.right-l.pos.x, 127);
        m_actiontab.Create(*this, rcActionTab, L"", WS_CHILD|WS_VISIBLE,0,ID_TABACTION);
		m_actiontab.AddItem(L"Command");
		m_actiontab.AddItem(L"Script");

        //CButton helpBtn;
        //helpBtn.Create(*this, CRect(rcActionTab.right-60,rcActionTab.top-2, rcActionTab.right, rcActionTab.top+20), L"Help", WS_CHILD|WS_VISIBLE);

        l.clearRow();

        maxHeight=0;

        // shell action
		CRect rc;
        m_actiontab.GetClientRect(&rc);
		m_actiontab.AdjustRect(FALSE,&rc);	

        Layout lAction(m_actiontab);        
        m_actionpage.Create(L"#32770", this, ID_MAP_ACTION, m_actiontab, rc, L"", WS_CHILD|WS_VISIBLE);                
        
        CStatic cmd,    args,    dir;
        CEdit   cmdEd, argsEd, dirEd;
        cmd.Create(m_actionpage, lAction.getSpace(80,25), L"Command :", WS_CHILD|WS_VISIBLE);
        cmdEd.Create(m_actionpage, lAction.getSpace(lAction.r.right-lAction.pos.x-15,22), L"", WS_CHILD|WS_VISIBLE|WS_TABSTOP, WS_EX_CLIENTEDGE, ID_ACTION_CMD);
        lAction.clearRow();
        args.Create(m_actionpage, lAction.getSpace(80,25), L"Arguments :", WS_CHILD|WS_VISIBLE);
        argsEd.Create(m_actionpage, lAction.getSpace(lAction.r.right-lAction.pos.x-15,22), L"", WS_CHILD|WS_VISIBLE|WS_TABSTOP, WS_EX_CLIENTEDGE, ID_ACTION_ARGS);
        lAction.clearRow();
        dir.Create(m_actionpage, lAction.getSpace(80,25), L"Directory :", WS_CHILD|WS_VISIBLE);
        dirEd.Create(m_actionpage, lAction.getSpace(lAction.r.right-lAction.pos.x-15,22), L"", WS_CHILD|WS_VISIBLE|WS_TABSTOP, WS_EX_CLIENTEDGE, ID_ACTION_DIR);
        lAction.clearRow();
        
        // script action
        m_scriptpage.Create(L"#32770", this, ID_MAP_SCRIPT, m_actiontab, rc, L"", WS_CHILD);

        CRect rcscript;
		m_scriptpage.GetClientRect(&rcscript);
        rcscript.DeflateRect(5,5,5,5);

        CEdit m_script;
        m_script.Create(m_scriptpage, rcscript, L"", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN|WS_VSCROLL, WS_EX_CLIENTEDGE, ID_SCRIPT_TEXT);

        SetStdFontOfDescendants(*this);
        
        // fill everything with data
        m_rules=settings.select_single_node("/settings/rules");
        
        pugi::xpath_node_set ns=settings.select_nodes("/settings/rules/rule");
        
        for(pugi::xpath_node_set::const_iterator it=ns.begin(); it!=ns.end(); it++) {
            insertRule(it->node());
        }

        enableEdition(false);

		return S_OK;
	}

    LRESULT OnSaveSettings(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        saveEdition();
        return S_OK;
    }

    int insertRule(pugi::xml_node &rule) {
        LVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.pszText=L"";
        lvi.mask=LVFIF_TEXT|LVIF_NORECOMPUTE;
        lvi.iItem=m_listview.GetItemCount();
        int r=m_listview.InsertItem(&lvi);
        m_listview.SetCheckState(r,TRUE);

        updateRule(r, rule);
        return r;
    }

    void updateRule(int r, pugi::xml_node &rule) {
        pugi::xpath_node_set args=rule.select_nodes("arg");
            
        bool first=true;
        CString arguments;
        CString action;
        for(pugi::xpath_node_set::const_iterator arg=args.begin(); arg!=args.end(); arg++) {
            /*if(!first) {
                arguments+=L",";                    
            } else {
                first=false;
            }
            CString lbl=arg->node().child_value("lbl");
            if(lbl!=L"")
                arguments+=L"\""+lbl+L"\"";
            else
                arguments+=arg->node().child_value();*/
            // get the first keyword as command name
            if(!arg->node().child("lbl"))
                continue;
            arguments=arg->node().child_value("lbl");
            break;
        }
            
        CString script=rule.child_value("script");
        CString cmd=rule.child_value("cmd");
        std::vector<CString> q; 
        if(script!=L"") {
            action=script;
        } else if(cmd!=L"") {
            action=cmd+" "+rule.child_value("args");
        }                        

        m_listview.SetItemText(r, 0, arguments);
        m_listview.SetItemText(r, 1, action);
    }

	LRESULT OnSelChangingAction(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled) {
        updateActionTab();
        return S_OK;
	}

    void updateActionTab() {
        int csel=m_actiontab.GetCurSel();
		m_actionpage.ShowWindow(csel==0?SW_SHOW:SW_HIDE);
		m_scriptpage.ShowWindow(csel==1?SW_SHOW:SW_HIDE);		
    }

    LRESULT OnRuleSelecting(int wParam, LPNMHDR lParam, BOOL& bHandled) {
        //saveEdition();
        return S_OK;
    }

    LRESULT OnRuleSelected(int wParam, LPNMHDR lParam, BOOL& bHandled) {

        LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;

        if((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_SELECTED) != (pnmv->uOldState & LVIS_SELECTED))
        {
            if (pnmv->uNewState & LVIS_SELECTED) {                    
                pugi::xpath_node_set ns=settings.select_nodes("/settings/rules/rule");
                
                if(pnmv->iItem>=ns.size())
                    return S_OK;

                pugi::xpath_node_set args=ns[pnmv->iItem].node().select_nodes("arg");

                for(int i=0;i<3;i++) {
                    if(i>=args.size()) {
                        m_argsctrl[i].clear();
                        continue;
                    }                    

                    CString t;
                    CString icofilename;

                    t=getTextType(args[i].node());
                    if( t==L"T" )
                        m_argsctrl[i].setType(getTextArg(args[i].node()));
                    else if( t==L"K" )
                        m_argsctrl[i].setKeyword(getTextArg(args[i].node()), UTF8toUTF16(args[i].node().child_value("ico")));
                    else
                        m_argsctrl[i].clear();
                }

                // set command
                m_actionpage.SetDlgItemText(ID_ACTION_CMD, UTF8toUTF16(ns[pnmv->iItem].node().child_value("cmd")));
                m_actionpage.SetDlgItemText(ID_ACTION_ARGS, UTF8toUTF16(ns[pnmv->iItem].node().child_value("args")));
                m_actionpage.SetDlgItemText(ID_ACTION_DIR, UTF8toUTF16(ns[pnmv->iItem].node().child_value("value")));
                // set script
                m_scriptpage.SetDlgItemText(ID_SCRIPT_TEXT, UTF8toUTF16(ns[pnmv->iItem].node().child_value("script")));

                if( ns[pnmv->iItem].node().child("cmd") ) {
                    m_actiontab.SetCurSel(0);
                } else {
                    
                    m_actiontab.SetCurSel(1);
                }
                updateActionTab();

                enableEdition(true);
            } else {
                clearEdition();
                enableEdition(false);
            }
        }

        Invalidate(TRUE);
        return S_OK;
    }
    
    int maxHeight;
    CRect getNextRect(CPoint &pos, CSize size) {
        CRect cpos(pos, size);        
        pos.x+=cpos.Width()+2;        
        maxHeight=max(size.cy,maxHeight);
        return cpos;
    }
    void clearRow(CPoint &pos) {
        pos.x=0;
        pos.y+=(maxHeight+5);
        maxHeight=0;
    }

	enum { IDD = IDD_EMPTY };
};