#include "stdafx.h"
#include "WebsitePlugin.h"
#include "Record.h"

//DB websites("websites",
//            "key INTEGER PRIMARY KEY, text TEXT, href TEXT, searchHref TEXT, uses INTEGER DEFAULT 0");

WebsitesDB websites;

// upgrades should work that way ?
// addcolumn("text",TEXT)