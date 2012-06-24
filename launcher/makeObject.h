SourceResult getResultFromIDispatch(const CString &type, const CString &key, IDispatch *args,Source *src);
SourceResult getResultFromFilePath(const CString &path,Source *s);
SourceResult getResultFromXML(pugi::xml_node &xml, Source *src);