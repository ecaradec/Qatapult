// 
// HttpSubmit(L"https://accounts.google.com/o/oauth2/token/code=%s&client_id=152444811162-mhjnj3csgt4km2icp0uni71d3n3assln.apps.googleusercontent.com&client_secret=0yZWReVI_5zq9lPvuUH5TO2h&redirect_uri=urn:ietf:wg:oauth:2.0:oob&grant_type=authorization_code")

void HttpSubmit(const CString &href, CHAR *data, CStringA *res) {
    TCHAR hostname[4096];
    DWORD hostnameLen=sizeof(hostname);
    UrlGetPart(href, hostname, &hostnameLen, URL_PART_HOSTNAME, 0);

    TCHAR query[4096];    
    DWORD queryLen=sizeof(query);
    UrlGetPart(href, query, &queryLen, URL_PART_QUERY, 0);

    int port=INTERNET_DEFAULT_HTTP_PORT;
    TCHAR portstr[4096];    
    DWORD portstrLen=sizeof(portstr);
    if(UrlGetPart(href, portstr, &portstrLen, URL_PART_PORT, 0) == S_OK) {
        port=_ttoi(portstr);
    } else {
        TCHAR scheme[4096];    
        DWORD schemeLen=sizeof(scheme);
        UrlGetPart(href, scheme, &schemeLen, URL_PART_SCHEME, 0);
        if(wcscmp(scheme, L"http")==0)
            port=INTERNET_DEFAULT_HTTP_PORT;
        else if(wcscmp(scheme, L"https")==0)
            port=INTERNET_DEFAULT_HTTPS_PORT;
    }

    CString location=href;
    location=location.Mid(location.Find(L"/", 8)); // 8 skip the scheme and ://


    BOOL  bResults = FALSE;
    HINTERNET hSession = NULL,
                hConnect = NULL,
                hRequest = NULL;

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(  L"Qatapult/1.0", 
                                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                WINHTTP_NO_PROXY_NAME, 
                                WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect( hSession, hostname,
                                    port, 0);

    // Create an HTTP Request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest( hConnect, L"POST", 
                                        location, 
                                        NULL, WINHTTP_NO_REFERER, 
                                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                                        WINHTTP_FLAG_SECURE);

    // Add a request header.    
    if( hRequest ) {
        bResults = WinHttpAddRequestHeaders(hRequest, 
                                            L"Content-Type: application/x-www-form-urlencoded",
                                            (ULONG)-1L,
                                            WINHTTP_ADDREQ_FLAG_ADD );        
    }

    if (hRequest) 
        bResults = WinHttpSendRequest( hRequest, 
                                        WINHTTP_NO_ADDITIONAL_HEADERS,
                                        0,
                                        data, 
                                        strlen(data), 
                                        strlen(data), 0);

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse( hRequest, NULL);

    // Keep checking for data until there is nothing left.
    if (bResults)
    {
        DWORD dwSize;
        do 
        {
            // Check for available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable( hRequest, &dwSize)) 
            {
                wchar_t buff[1024];
                swprintf(buff, L"Error %u in WinHttpQueryDataAvailable.\n", GetLastError() );
                OutputDebugString(buff);
                break;
            }
            
            // No more available data.
            if (!dwSize)
                break;

            // Allocate space for the buffer.
            char *pszOutBuffer = new char[dwSize+1];
            if (!pszOutBuffer)
            {
                OutputDebugString(L"Out of memory\n");
                break;
            }
            
            // Read the Data.
            ZeroMemory(pszOutBuffer, dwSize+1);

            DWORD dwDownloaded;
            if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, 
                                    dwSize, &dwDownloaded))
            {                                  
                wchar_t buff[1024];
                wsprintf(buff, L"Error %u in WinHttpReadData.\n", GetLastError() );
                OutputDebugString(buff);
            }
            else
            {
                *res+=pszOutBuffer;
                OutputDebugStringA(pszOutBuffer);               
            }
        
            // Free the memory allocated to the buffer.
            delete [] pszOutBuffer;

            // This condition should never be reached since WinHttpQueryDataAvailable
            // reported that there are bits to read.
            if (!dwDownloaded)
                break;
                
        } while (dwSize > 0);
    }
    else
    {
        // Report any errors.
        wchar_t buff[1024];
        wsprintf(buff, L"Error %d has occurred.\n", GetLastError() );
        OutputDebugString(buff);
    }

    // PLACE ADDITIONAL CODE HERE.

    // Report any errors.
    if (!bResults) {
        wchar_t buff[1024];
        wsprintf(buff, L"Error %d has occurred.\n", GetLastError() );
        OutputDebugString(buff);
    }

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}

int HttpGet(const CString &href, CStringA *res) {
    TCHAR hostname[4096];
    DWORD hostnameLen=sizeof(hostname);
    UrlGetPart(href, hostname, &hostnameLen, URL_PART_HOSTNAME, 0);

    TCHAR query[4096];    
    DWORD queryLen=sizeof(query);
    UrlGetPart(href, query, &queryLen, URL_PART_QUERY, 0);

    int port=INTERNET_DEFAULT_HTTP_PORT;
    TCHAR portstr[4096];    
    DWORD portstrLen=sizeof(portstr);
    if(UrlGetPart(href, portstr, &portstrLen, URL_PART_PORT, 0) == S_OK) {
        port=_ttoi(portstr);
    } else {
        TCHAR scheme[4096];    
        DWORD schemeLen=sizeof(scheme);
        UrlGetPart(href, scheme, &schemeLen, URL_PART_SCHEME, 0);
        if(wcscmp(scheme, L"http")==0)
            port=INTERNET_DEFAULT_HTTP_PORT;
        else if(wcscmp(scheme, L"https")==0)
            port=INTERNET_DEFAULT_HTTPS_PORT;
    }

    CString location=href;
    location=location.Mid(location.Find(L"/", 8));

    BOOL  bResults = FALSE;
    HINTERNET hSession = NULL,
                hConnect = NULL,
                hRequest = NULL;

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(  L"Qatapult/1.0", 
                                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                WINHTTP_NO_PROXY_NAME, 
                                WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect( hSession, hostname, port, 0);

    // Create an HTTP Request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest( hConnect, L"GET", 
                                        location, 
                                        NULL, WINHTTP_NO_REFERER, 
                                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                                        WINHTTP_FLAG_SECURE);

    // Add a request header.
    char buff[4096]={0};

    if (hRequest) 
        bResults = WinHttpSendRequest( hRequest, 
                                        WINHTTP_NO_ADDITIONAL_HEADERS,
                                        0,
                                        buff, 
                                        strlen(buff), 
                                        strlen(buff), 0);

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse( hRequest, NULL);

    DWORD status=0;
    DWORD statusLen=sizeof(status);
    DWORD index=0;
    if(bResults)
        bResults=WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusLen, &index);

    // Keep checking for data until there is nothing left.
    if (bResults)
    {
        DWORD dwSize;
        do 
        {
            // Check for available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable( hRequest, &dwSize)) 
            {
                wchar_t buff[1024];
                swprintf(buff, L"Error %u in WinHttpQueryDataAvailable.\n", GetLastError() );
                OutputDebugString(buff);
                break;
            }
            
            // No more available data.
            if (!dwSize)
                break;

            // Allocate space for the buffer.
            char *pszOutBuffer = new char[dwSize+1];
            if (!pszOutBuffer)
            {
                OutputDebugString(L"Out of memory\n");
                break;
            }
            
            // Read the Data.
            ZeroMemory(pszOutBuffer, dwSize+1);

            DWORD dwDownloaded;
            if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, 
                                    dwSize, &dwDownloaded))
            {                                  
                wchar_t buff[1024];
                wsprintf(buff, L"Error %u in WinHttpReadData.\n", GetLastError() );
                OutputDebugString(buff);
            }
            else
            {
                *res+=pszOutBuffer;
            }
        
            // Free the memory allocated to the buffer.
            delete [] pszOutBuffer;

            // This condition should never be reached since WinHttpQueryDataAvailable
            // reported that there are bits to read.
            if (!dwDownloaded)
                break;
                
        } while (dwSize > 0);
    }
    else
    {
        // Report any errors.
        wchar_t buff[1024];
        wsprintf(buff, L"Error %d has occurred.\n", GetLastError() );
        OutputDebugString(buff);
    }

    // PLACE ADDITIONAL CODE HERE.

    // Report any errors.
    if (!bResults) {
        wchar_t buff[1024];
        wsprintf(buff, L"Error %d has occurred.\n", GetLastError() );
        OutputDebugString(buff);
    }

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}