#ifndef BACKEND_LMDB_H
#define BACKEND_LMDB_H

#include <urcl.h>

#ifndef VDBDIR
#define VDBDIR "127.0.0.1"
#endif

struct vdb {
    urclHandle *u;
    char *      rcpt;
    time_t      interval;
};

#include "vdb.h"

#endif /* BACKEND_LMDB_H */
