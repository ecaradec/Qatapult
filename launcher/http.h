// 
// HttpSubmit(L"https://accounts.google.com/o/oauth2/token/code=%s&client_id=152444811162-mhjnj3csgt4km2icp0uni71d3n3assln.apps.googleusercontent.com&client_secret=0yZWReVI_5zq9lPvuUH5TO2h&redirect_uri=urn:ietf:wg:oauth:2.0:oob&grant_type=authorization_code")

void HttpSubmit(const CString &href, CHAR *data, CStringA *res);

int HttpGet(const CString &href, CStringA *res);

int HttpDownload(const CString &href, CStringA path);

int HttpGetBuffer(const CString &href, void **data, int *totalsize);