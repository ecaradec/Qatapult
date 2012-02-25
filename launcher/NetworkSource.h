BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr, std::vector<CString> &lnks);

struct NetworkSource : Source {
    NetworkSource() : Source(L"FILE",L"Network (Catalog )") {
        m_icon=L"icons\\networklocal.png";
        m_ignoreemptyquery=true;
        int rc = sqlite3_open("databases\\network.db", &db);
        
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE files(key TEXT PRIMARY KEY ASC, display TEXT, expand TEXT, path TEXT, verb TEXT, bonus INTEGER, mark INTEGER)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    ~NetworkSource() {
        sqlite3_close(db);
    }
    void collect(const TCHAR *query, std::vector<SourceResult> &results, int flags) {
        // could probably be done in subclass as well as the callback since sourceresult will not change 
        CString q(query);

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;

        rc = sqlite3_prepare_v2(db,"SELECT key, display, expand, path, 0, bonus FROM files WHERE display LIKE ?;",-1, &stmt, &unused);
        rc = sqlite3_bind_text16(stmt, 1, fuzzyfyArg(q), -1, SQLITE_STATIC);
        int i=0;
        while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
            results.push_back(SourceResult(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),        // key
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),        // display
                                            UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),        // expand
                                            this,                         // source
                                            sqlite3_column_int(stmt,3),   // id
                                            0,                            // data
                                            sqlite3_column_int(stmt,4))); // bonus
            
            results.back().object=new FileObject(UTF8toUTF16((char*)sqlite3_column_text(stmt,0)),
                                                    this,
                                                    UTF8toUTF16((char*)sqlite3_column_text(stmt,1)),
                                                    UTF8toUTF16((char*)sqlite3_column_text(stmt,2)),
                                                    UTF8toUTF16((char*)sqlite3_column_text(stmt,3)));
        }

        const char *errmsg=sqlite3_errmsg(db) ;
        sqlite3_finalize(stmt);
    }
    void crawl() {
        std::vector<CString> lnks;
        EnumerateFunc(0, lnks);

        sqlite3_stmt *stmt=0;
        const char *unused=0;
        int rc;
        char *zErrMsg=0;

        // there can only exists one single mark
        int mark;
        sqlite3_exec(db, "SELECT mark FROM files LIMIT 1;", getIntCB, &mark, &zErrMsg);
        sqlite3_free(zErrMsg);

        rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        for(uint i=0;i<lnks.size();i++) {
            CString str(lnks[i]);

            CString startmenu_key=md5(lnks[i]);
            
            rc = sqlite3_prepare_v2(db,
                                    "INSERT OR REPLACE INTO files(key,display,expand,path,bonus,mark) VALUES(?, ?, ?, ?, coalesce((SELECT bonus FROM files WHERE key=?), 0), ?);\n",
                                    -1, &stmt, &unused);
            rc = sqlite3_bind_text16(stmt, 1, startmenu_key.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 2, str.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 3, str+L"\\", -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 4, str.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_text16(stmt, 5, startmenu_key.GetString(), -1, SQLITE_STATIC);
            rc = sqlite3_bind_int(stmt, 6, mark+1);
            rc = sqlite3_step(stmt);
            const char *errmsg=sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
        }

        rc = sqlite3_exec(db, "END;", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
    Gdiplus::Bitmap *getIcon(SourceResult *r, long flags) {
        return r->object->getIcon(flags);
    }
    CString getString(SourceResult &sr,const TCHAR *val_) {
        return sr.object->getString(val_);
    }
    Source *getSource(SourceResult &r, CString &query) {
        query=r.display+L"\\";
        return (Source*)-1;
    }
    sqlite3 *db;
};

#pragma comment(lib, "mpr.lib")

#include <winnetwk.h>

void DisplayStruct(int i, LPNETRESOURCE lpnrLocal);

BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr, std::vector<CString> &lnks)
{
	//if(lpnr && (lpnr->dwUsage&0x80000000)!=0)
	//	return 0;

	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;     // 16K is a good size
	DWORD cEntries = -1;        // enumerate all possible entries
	LPNETRESOURCE lpnrLocal;    // pointer to enumerated structures
	DWORD i;
	//
	// Call the WNetOpenEnum function to begin the enumeration.
	//
	dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, // all network resources
		RESOURCETYPE_ANY,   // all resources
		0,  // enumerate all resources
		lpnr,       // NULL first time the function is called
		&hEnum);    // handle to the resource

	if (dwResult != NO_ERROR) {
		//printf("WnetOpenEnum failed with error %d\n", dwResult);
		return FALSE;
	}
	//
	// Call the GlobalAlloc function to allocate resources.
	//
	lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);
	if (lpnrLocal == NULL) {
		//printf("WnetOpenEnum failed with error %d\n", dwResult);
		//      NetErrorHandler(hwnd, dwResult, (LPSTR)"WNetOpenEnum");
		return FALSE;
	}

	do {
		//
		// Initialize the buffer.
		//
		ZeroMemory(lpnrLocal, cbBuffer);
		//
		// Call the WNetEnumResource function to continue
		//  the enumeration.
		//
		dwResultEnum = WNetEnumResource(hEnum,  // resource handle
			&cEntries,      // defined locally as -1
			lpnrLocal,      // LPNETRESOURCE
			&cbBuffer);     // buffer size
		//
		// If the call succeeds, loop through the structures.
		//
		if (dwResultEnum == NO_ERROR) {
			for (i = 0; i < cEntries; i++) {
				// Call an application-defined function to
				//  display the contents of the NETRESOURCE structures.
				//
				//DisplayStruct(i, &lpnrLocal[i]);
                lnks.push_back(lpnrLocal[i].lpRemoteName);

				// If the NETRESOURCE structure represents a container resource, 
				//  call the EnumerateFunc function recursively.

				if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage
					& RESOURCEUSAGE_CONTAINER))
					//          if(!EnumerateFunc(hwnd, hdc, &lpnrLocal[i]))
					if (!EnumerateFunc(&lpnrLocal[i], lnks))
						;//printf("EnumerateFunc returned FALSE\n");
				//            TextOut(hdc, 10, 10, "EnumerateFunc returned FALSE.", 29);
			}
		}
		// Process errors.
		//
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS) {
			//printf("WNetEnumResource failed with error %d\n", dwResultEnum);

			//      NetErrorHandler(hwnd, dwResultEnum, (LPSTR)"WNetEnumResource");
			break;
		}
	}
	//
	// End do.
	//
	while (dwResultEnum != ERROR_NO_MORE_ITEMS);
	//
	// Call the GlobalFree function to free the memory.
	//
	GlobalFree((HGLOBAL) lpnrLocal);
	//
	// Call WNetCloseEnum to end the enumeration.
	//
	dwResult = WNetCloseEnum(hEnum);

	if (dwResult != NO_ERROR) {
		//
		// Process errors.
		//
		//printf("WNetCloseEnum failed with error %d\n", dwResult);
		//    NetErrorHandler(hwnd, dwResult, (LPSTR)"WNetCloseEnum");
		return FALSE;
	}

	return TRUE;
}

/*void DisplayStruct(int i, LPNETRESOURCE lpnrLocal)
{
	printf("NETRESOURCE[%d] Scope: ", i);
	switch (lpnrLocal->dwScope) {
	case (RESOURCE_CONNECTED):
		printf("connected\n");
		break;
	case (RESOURCE_GLOBALNET):
		printf("all resources\n");
		break;
	case (RESOURCE_REMEMBERED):
		printf("remembered\n");
		break;
	default:
		printf("unknown scope %d\n", lpnrLocal->dwScope);
		break;
	}

	printf("NETRESOURCE[%d] Type: ", i);
	switch (lpnrLocal->dwType) {
	case (RESOURCETYPE_ANY):
		printf("any\n");
		break;
	case (RESOURCETYPE_DISK):
		printf("disk\n");
		break;
	case (RESOURCETYPE_PRINT):
		printf("print\n");
		break;
	default:
		printf("unknown type %d\n", lpnrLocal->dwType);
		break;
	}

	printf("NETRESOURCE[%d] DisplayType: ", i);
	switch (lpnrLocal->dwDisplayType) {
	case (RESOURCEDISPLAYTYPE_GENERIC):
		printf("generic\n");
		break;
	case (RESOURCEDISPLAYTYPE_DOMAIN):
		printf("domain\n");
		break;
	case (RESOURCEDISPLAYTYPE_SERVER):
		printf("server\n");
		break;
	case (RESOURCEDISPLAYTYPE_SHARE):
		printf("share\n");
		break;
	case (RESOURCEDISPLAYTYPE_FILE):
		printf("file\n");
		break;
	case (RESOURCEDISPLAYTYPE_GROUP):
		printf("group\n");
		break;
	case (RESOURCEDISPLAYTYPE_NETWORK):
		printf("network\n");
		break;
	default:
		printf("unknown display type %d\n", lpnrLocal->dwDisplayType);
		break;
	}

	printf("NETRESOURCE[%d] Usage: 0x%x = ", i, lpnrLocal->dwUsage);
	if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONNECTABLE)
		printf("connectable ");
	if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONTAINER)
		printf("container ");
	printf("\n");

	printf("NETRESOURCE[%d] Localname: %S\n", i, lpnrLocal->lpLocalName);
	printf("NETRESOURCE[%d] Remotename: %S\n", i, lpnrLocal->lpRemoteName);
	printf("NETRESOURCE[%d] Comment: %S\n", i, lpnrLocal->lpComment);
	printf("NETRESOURCE[%d] Provider: %S\n", i, lpnrLocal->lpProvider);
	printf("\n");
}
*/
