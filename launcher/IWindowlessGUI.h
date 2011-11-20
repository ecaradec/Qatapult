#pragma once
struct IWindowlessGUI {
    virtual ~IWindowlessGUI() {}
    virtual void Invalidate() = 0;
    virtual void OnQueryChange(const CString &text) = 0;    
};