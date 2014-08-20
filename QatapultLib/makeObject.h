#include "SourceResult.h"

SourceResult getResultFromFilePath(const CString &path,Source *s);
SourceResult getResultFromXML(pugi::xml_node &xml, Source *src);