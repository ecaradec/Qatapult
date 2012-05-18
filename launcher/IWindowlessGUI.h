#pragma once
struct IWindowlessGUI {
    virtual ~IWindowlessGUI() {}
    virtual void invalidate() = 0;
    virtual void onQueryChange(const CString &text, bool select=true) = 0;
};