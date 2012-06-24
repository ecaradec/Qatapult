#include "makeObject.h"

struct History {        
    History() {
        int rc = sqlite3_open("databases\\commands.db", &db);
        
        char *zErrMsg = 0;
        sqlite3_exec(db, "CREATE TABLE commands(key TEXT PRIMARY KEY ASC, xml TEXT)", 0, 0, &zErrMsg);
        sqlite3_free(zErrMsg);

        const char *unused=0;
        rc = sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO commands(key,xml) VALUES(?, ?);\n", -1, &stmtSave, &unused);

        rc = sqlite3_prepare_v2(db, "SELECT * FROM commands;\n", -1, &stmtLoad, &unused);
    }
    ~History() {
        sqlite3_finalize(stmtSave);
        sqlite3_finalize(stmtLoad);
    }
    void saveCommand(CommandObject *c) {
        int rc;
        rc = sqlite3_bind_text16(stmtSave, 1, c->key.GetString(), -1, SQLITE_STATIC);
        CString xml = c->toXML();            
        rc = sqlite3_bind_text16(stmtSave, 2, xml.GetString(), -1, SQLITE_STATIC);            
        rc = sqlite3_step(stmtSave);
        const char *errmsg=sqlite3_errmsg(db);
        sqlite3_reset(stmtSave);
    }
    void loadCommands(std::vector<std::shared_ptr<CommandObject> > &history) {
        int rc;
        while((rc=sqlite3_step(stmtLoad))==SQLITE_ROW) {

            std::vector<RuleArg> ra;
            ra.push_back(RuleArg());

            char *buff=(char*)sqlite3_column_text(stmtLoad,1);
            pugi::xml_document d; d.load( buff, pugi::parse_default );

            pugi::xpath_node_set command=d.select_nodes("command/*");
            int c=0;
            for(pugi::xpath_node_set::const_iterator it=command.begin(); it!=command.end(); it++) {
                ra[c].m_results.push_back( getResultFromXML(it->node(), m_inputsource) );
                c++;
            }

                
            history.push_back(std::shared_ptr<CommandObject>(new CommandObject(ra, m_inputsource)));
        }
        const char *errmsg=sqlite3_errmsg(db);
        sqlite3_reset(stmtLoad);
    }

    Source *m_inputsource;
    sqlite3_stmt *stmtSave;
    sqlite3_stmt *stmtLoad;
    sqlite3 *db;
};
