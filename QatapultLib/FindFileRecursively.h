#pragma once

inline void FindFilesRecursively(LPCTSTR lpFolder, LPCTSTR lpFilePattern, std::vector<CString> &files, int maxdepth, int depth=0)
{
    depth++;
    if(depth>maxdepth)
        return;

    TCHAR szFullPattern[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    // first we are going to process any subdirectories
    PathCombine(szFullPattern, lpFolder, _T("*"));
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && CString(FindFileData.cFileName)!=L"." && CString(FindFileData.cFileName)!=L"..")
            {
                // found a subdirectory; recurse into it
                PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
                FindFilesRecursively(szFullPattern, lpFilePattern, files, maxdepth, depth);
            }
        } while(FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
    // now we are going to look for the matching files
    PathCombine(szFullPattern, lpFolder, lpFilePattern);
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // found a file; do something with it
                PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
                files.push_back(szFullPattern);
                //_tprintf_s(_T("%s\n"), szFullPattern);
            }
        } while(FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
}