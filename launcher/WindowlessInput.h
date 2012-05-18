#pragma once
struct WindowlessInput {
    WindowlessInput(IWindowlessGUI *p):m_pParent(p) {
        m_text=L"";
        m_caretpos=0;
    }
    void draw(Graphics &g, const RectF &r, StringFormat &sf, TCHAR prefix, float fontsize, Color c=0xFFFFFFFF) {
        Gdiplus::Font f(g_fontfamily, fontsize);
        
        int caretpos=m_caretpos;
        CString text(m_text);
        if(text[0]==prefix) {
            text=text.Mid(1);
            caretpos--;
            caretpos=max(caretpos,0);
        }        

        g.DrawString(text, -1, &f, r, &sf, &SolidBrush(c));        

        Gdiplus::RectF r1;
        if(caretpos!=0) {
            // get position of the caret
            Gdiplus::StringFormat sfmt;  
            sfmt.SetFormatFlags(StringFormatFlagsNoClip|StringFormatFlagsMeasureTrailingSpaces);
        
            int             charRangeCount=1;
            Gdiplus::CharacterRange charRange[1] = { CharacterRange(caretpos-1, 1) };
            Gdiplus::Region         regions[1];

            sfmt.SetMeasurableCharacterRanges(charRangeCount, charRange);        
            g.MeasureCharacterRanges(text, -1, &f, r, &sfmt, charRangeCount, regions);
            
            regions[0].GetBounds(&r1, &g);

            r1.X+=r1.Width-2;
            r1.Width=2;
        } else {
            r1.X=r.X;
            r1.Y=r.Y;
            r1.Width=2;
            r1.Height=15;
        }
        
        g.FillRectangle(&SolidBrush(Color(c)), r1);
    }
    void setText(const CString &txt) {
        m_text=txt;
        m_caretpos=m_text.GetLength();
    }
    void appendText(const CString &text) {
        m_text.Append(text);
        m_caretpos=m_text.GetLength();
    }
    void back(bool ctrl) {
        int oldpos=m_caretpos;
        if(ctrl) {
            int i;
            for(i=m_caretpos-1; i>0;i--)
                if(m_text[i]!=L' ' && m_text[i]!=L'\\')
                    break;

            for(;i>0;i--)
                if(m_text[i]==L' ' || m_text[i]==L'\\') {
                    i+=1;
                    break;
                }

            m_caretpos=i ;
        } else 
            m_caretpos--;

        m_caretpos=max(m_caretpos, 0);
        m_caretpos=min(m_caretpos, m_text.GetLength());
        m_text=m_text.Left(m_caretpos)+m_text.Mid(oldpos);
        m_pParent->invalidate();
    }
    void del(bool ctrl) {
        int nextword=m_text.GetLength();
        if(ctrl) {
            int i;
            for(i=m_caretpos; i<m_text.GetLength();i++)
                if(m_text[i]!=L' ' && m_text[i]!=L'\\')
                    break;

            for(;i<m_text.GetLength();i++)
                if(m_text[i]==L' ' || m_text[i]==L'\\') {
                    i+=1;
                    break;
                }

            nextword=i;
        } else {
            nextword=m_caretpos+1;
        }

        m_text=m_text.Left(m_caretpos)+m_text.Mid(nextword);
        m_pParent->invalidate();
    }
    void moveCaretRight(bool ctrl) {
        if(ctrl) {
            int i;
            for(i=m_caretpos; i<m_text.GetLength();i++)
                if(m_text[i]!=L' ' && m_text[i]!=L'\\')
                    break;

            for(;i<m_text.GetLength();i++)
                if(m_text[i]==L' ' || m_text[i]==L'\\') {
                    i+=1;
                    break;
                }

            m_caretpos=i;
        } else 
            m_caretpos++;

        m_caretpos=min(m_caretpos, m_text.GetLength());
        m_pParent->invalidate();
    }
    void moveCaretLeft(bool ctrl) {
        if(ctrl) {
            int i;
            for(i=m_caretpos-1; i>=0;i--)
                if(m_text[i]!=L' ' && m_text[i]!=L'\\')
                    break;

            for(;i>=0;i--)
                if(m_text[i]==L' ' || m_text[i]==L'\\') {
                    i+=1;
                    break;
                }

            m_caretpos=i;
        } else 
            m_caretpos--;
        m_caretpos=max(m_caretpos,0);
        m_pParent->invalidate();
    }
    void appendAtCaret(const CString &s) {
        m_text=m_text.Left(m_caretpos)+s+m_text.Mid(m_caretpos);
        m_caretpos=m_text.Left(m_caretpos).GetLength()+s.GetLength();
        m_pParent->invalidate();
    }
    void home() {
        m_caretpos=0;
        m_pParent->invalidate();
    }
    void end() {
        m_caretpos=m_text.GetLength();
        m_pParent->invalidate();
    }
    IWindowlessGUI  *m_pParent;
    CString          m_text;
    int              m_caretpos;
};
