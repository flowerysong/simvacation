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
vdb_init(const yastr rcpt) {
    return calloc(1, 1);
}

void
vdb_close(VDB *vdb) {
    free(vdb);
    return;
}

vdb_status
vdb_recent(VDB *vdb, const yastr from) {
    return VDB_STATUS_OK;
}

time_t
vdb_interval() {
    return (time_t)ucl_object_todouble(
            ucl_object_lookup_path(vac_config, "core.interval"));
}

vac_result
vdb_store_reply(VDB *vdb, const yastr from) {
    return VAC_RESULT_OK;
}

ucl_object_t *
vdb_get_names(VDB *vdb) {
    return ucl_object_typed_new(UCL_ARRAY);
}

void
vdb_clean(VDB *vdb, const yastr user) {
    return;
}

void
vdb_gc(VDB *vdb) {
    return;
}
