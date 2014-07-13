#pragma once
#include "http.h"
#include <time.h>
#include "Object.h"
#include "DBSource.h"

// those kind of sources could have a simplified load and save ?
struct ContactSource : DBSource {
    ContactSource();
    void parseGmailContacts(const char *xml) ;
    void crawl();    
    void collect(const TCHAR *query, KVPack &pack, int def, std::map<CString,bool> &activetypes);
};
