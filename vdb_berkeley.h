#ifndef BACKEND_BDB_H
#define BACKEND_BDB_H

#define DB_DBM_HSEARCH    1
#include <db.h>

#define VIT     "__VACATION__INTERVAL__TIMER__"

#ifndef VDBDIR
#define VDBDIR  "/var/lib/simvacation"   /* dir for vacation databases */
#endif

struct vdb {
    DB  *dbp;
    char *path;
};

#include "vdb.h"

#endif /* BACKEND_BDB_H */
