
struct WindowlessInput {
    WindowlessInput(IWindowlessGUI *p):m_pParent(p) {
        m_text=L"";
    }
    void Draw(HDC hdc, const RectF &r) {
        Graphics g(hdc);
        Gdiplus::Font f(L"Arial", 10.0f);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        g.DrawString(m_text, m_text.GetLength(), &f, r, &sf, &SolidBrush(Color(0xFFFFFFFF)));
        // g.DrawDriverString((UINT16*)CStringW(txt).GetBuffer(),txt.GetLength(),&m_gdipFont,&b,pos,DriverStringOptionsCmapLookup,&m);
    }
    void OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam) {
        if(msg==WM_CHAR) {            
            if(wparam==8)
                m_text=m_text.Left(m_text.GetLength() -1 );
            else if(wparam<VK_SPACE)
                return;
            else
                m_text.AppendChar(wparam);

            m_pParent->OnQueryChange(m_text);
        }

        m_pParent->Invalidate();
    }
    void SetText(const CString &txt) {
        m_text=txt;
        m_pParent->OnQueryChange(m_text);
        m_pParent->Invalidate();
    }
    IWindowlessGUI  *m_pParent;
    CString          m_text;
};
