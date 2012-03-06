#pragma once

struct Command {
    CString display;
    CString verb;
    int     id;
};

bool getContextMenu(const CString &d, const CString &f, IContextMenu **ppCM);

HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, UINT idCmdOffset);

void getItemVerbs(const CString &d, const CString &f, std::vector<Command> &commands);