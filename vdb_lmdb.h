#ifndef BACKEND_LMDB_H
#define BACKEND_LMDB_H

#include <lmdb.h>

#ifndef VDBDIR
#define VDBDIR  "/var/lib/vacation"   /* dir for vacation database */
#endif

struct vdb {
    MDB_env *dbenv;
    char    *rcpt;
    time_t  interval;
};

#include "vdb.h"

#endif /* BACKEND_LMDB_H */
