/*
 * Copyright (c) Regents of The University of Michigan
 * See COPYING.
 */

#include <stdlib.h>
#include <syslog.h>
#include <time.h>

#include "simvacation.h"
#include "vdb.h"

struct vdb_backend *
vdb_backend(const char *provider) {
    struct vdb_backend *functable;

    functable = malloc(sizeof(struct vdb_backend));

    /* Default all functions to the null implementation */
    functable->init = vdb_init;
    functable->close = vdb_close;
    functable->recent = vdb_recent;
    functable->store_interval = vdb_store_interval;
    functable->store_reply = vdb_store_reply;
    functable->get_names = vdb_get_names;
    functable->clean = vdb_clean;
    functable->gc = vdb_gc;

    if (strcasecmp(provider, "redis") == 0) {
#ifdef HAVE_URCL
        functable->init = redis_vdb_init;
        functable->close = redis_vdb_close;
        functable->recent = redis_vdb_recent;
        functable->store_reply = redis_vdb_store_reply;
        return functable;
#else  /* HAVE_URCL */
        syslog(LOG_ERR, "vdb_backend: redis was disabled during compilation");
        return NULL;
#endif /* HAVE_URCL */
    }

    if (strcasecmp(provider, "lmdb") == 0) {
#ifdef HAVE_LMDB
        functable->init = lmdb_vdb_init;
        functable->close = lmdb_vdb_close;
        functable->recent = lmdb_vdb_recent;
        functable->store_reply = lmdb_vdb_store_reply;
        functable->gc = lmdb_vdb_gc;
        return functable;
#else  /* HAVE_LMDB */
        syslog(LOG_ERR, "vdb_backend: LMDB was disabled during compilation");
        return NULL;
#endif /* HAVE_LMDB */
    }

    if (strcasecmp(provider, "null") == 0) {
        return functable;
    }

    syslog(LOG_ERR, "vdb_backend: unknown backend %s", provider);
    free(functable);
    return NULL;
}

VDB *
vdb_init(const ucl_object_t *config, const char *rcpt) {
    return (calloc(1, 1));
}

void
vdb_close(VDB *vdb) {
    free(vdb);
    return;
}

int
vdb_recent(VDB *vdb, char *from) {
    return (0);
}

int
vdb_store_interval(VDB *vdb, time_t interval) {
    vdb->interval = interval;
    return (0);
}

int
vdb_store_reply(VDB *vdb, char *from) {
    return (0);
}

struct name_list *
vdb_get_names(VDB *vdb) {
    return (NULL);
}

void
vdb_clean(VDB *vdb, char *user) {
    return;
}

void
vdb_gc(VDB *vdb) {
    return;
}
