#include "stdafx.h"
#include "StartMenuSource.h"
#include "pugixml.hpp"

 CString GetSpecialFolder(int csidl) {
     CString tmp;
     SHGetFolderPath(0, csidl, 0, SHGFP_TYPE_CURRENT, tmp.GetBufferSetLength(MAX_PATH)); tmp.ReleaseBuffer();
     return tmp;
 }
 
int uselev;