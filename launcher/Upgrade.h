#include <sqlite3/sqlite3.h>

inline void UpgradeTable(sqlite3 *db, const char *table) {
    char *zErrMsg = 0;

    CStringA t(table);
    CString s;
    sqlite3_exec(db, "PRAGMA user_version", getStringCB, &s,0);
    switch(_ttoi(s)) {
    case 0:
        sqlite3_exec(db, "ALTER TABLE "+t+" ADD uses INTEGER DEFAULT 0", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_exec(db, "UPDATE "+t+" SET uses=bonus/5", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_exec(db, "PRAGMA user_version=1", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);
    }
}