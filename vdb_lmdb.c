/*
 * Copyright (c) Regents of The University of Michigan
 * See COPYING.
 */


#include <sys/param.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "rabin.h"
#include "simvacation.h"
#include "vdb.h"

char *lmdb_vdb_key(char *, char *);
void  lmdb_vdb_assert(MDB_env *, const char *);

VDB *
lmdb_vdb_init(const ucl_object_t *config, const char *rcpt) {
    int  rc;
    VDB *vdb;

    if ((vdb = calloc(1, sizeof(VDB))) == NULL) {
        return (NULL);
    }

    if ((rc = mdb_env_create(&vdb->lmdb)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_init mdb_env_create: %s", mdb_strerror(rc));
        goto error;
    }

    if ((rc = mdb_env_set_assert(vdb->lmdb, &lmdb_vdb_assert)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_init mdb_env_set_assert: %s",
                mdb_strerror(rc));
        goto error;
    }

    if ((rc = mdb_env_set_mapsize(vdb->lmdb, 1073741824)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_init mdb_env_mapsize: %s",
                mdb_strerror(rc));
        goto error;
    }

    /* FIXME: location should be configurable */
    if ((rc = mdb_env_open(vdb->lmdb, "/var/lib/simvacation", 0, 0664)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_init mdb_env_open: %s", mdb_strerror(rc));
        goto error;
    }

    vdb->interval = SECSPERDAY * DAYSPERWEEK;
    vdb->rcpt = strdup(rcpt);

    return (vdb);

error:
    vdb_close(vdb);
    return (NULL);
}

void
lmdb_vdb_assert(MDB_env *dbenv, const char *msg) {
    syslog(LOG_ALERT, "lmdb assert: %s", msg);
    /* FIXME: is this the correct action? */
    exit(EX_TEMPFAIL);
}

void
lmdb_vdb_close(VDB *vdb) {
    if (vdb) {
        if (vdb->lmdb) {
            mdb_env_close(vdb->lmdb);
        }
        free(vdb);
    }
}

int
lmdb_vdb_recent(VDB *vdb, char *from) {
    int      rc, retval = 0;
    time_t   last, now;
    MDB_txn *txn;
    MDB_dbi  dbi;
    MDB_val  key, data;
    char *   keyval = NULL;

    if ((keyval = lmdb_vdb_key(vdb->rcpt, from)) == NULL) {
        return (0);
    }

    if ((rc = mdb_txn_begin(vdb->lmdb, NULL, MDB_RDONLY, &txn)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_recent mdb_txn_begin: %s",
                mdb_strerror(rc));
        free(keyval);
        return (0);
    }

    if ((rc = mdb_dbi_open(txn, NULL, MDB_CREATE, &dbi)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_recent mdb_dbi_open: %s", mdb_strerror(rc));
        goto cleanup;
    }

    key.mv_size = strlen(keyval);
    key.mv_data = keyval;

    if ((rc = mdb_get(txn, dbi, &key, &data)) != 0) {
        if (rc != MDB_NOTFOUND) {
            syslog(LOG_ALERT, "lmdb vdb_recent mdb_get: %s", mdb_strerror(rc));
        }
        goto cleanup;
    }

    if (data.mv_size != sizeof(last)) {
        syslog(LOG_ALERT, "lmdb vdb_recent: retrieved bad data");
        goto cleanup;
    }

    last = *(time_t *)data.mv_data;

    if ((now = time(NULL)) < 0) {
        syslog(LOG_ALERT, "lmdb vdb_recent time: %m");
    } else if (now < (last + vdb->interval)) {
        retval = 1;
    }

cleanup:
    free(keyval);
    mdb_txn_abort(txn);
    return (retval);
}

int
lmdb_vdb_store_reply(VDB *vdb, char *from) {
    int      rc;
    time_t   now;
    MDB_txn *txn;
    MDB_dbi  dbi;
    MDB_val  key, data;
    char *   keyval;

    if ((now = time(NULL)) < 0) {
        syslog(LOG_ALERT, "lmdb vdb_store_reply time: %m");
        return (1);
    }

    if ((rc = mdb_txn_begin(vdb->lmdb, NULL, 0, &txn)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_store_reply mdb_txn_begin: %s",
                mdb_strerror(rc));
        return (1);
    }

    if ((rc = mdb_dbi_open(txn, NULL, MDB_CREATE, &dbi)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_store_reply mdb_dbi_open: %s",
                mdb_strerror(rc));
        mdb_txn_abort(txn);
        return (1);
    }

    keyval = lmdb_vdb_key(vdb->rcpt, from);
    key.mv_size = strlen(keyval);
    key.mv_data = keyval;

    data.mv_size = sizeof(now);
    data.mv_data = &now;

    if ((rc = mdb_put(txn, dbi, &key, &data, 0)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_store_reply mdb_put: %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return (1);
    }

    if ((rc = mdb_txn_commit(txn)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_store_reply mdb_txn_commit: %s",
                mdb_strerror(rc));
        return (1);
    }

    return (0);
}

void
lmdb_vdb_gc(VDB *vdb) {
    int         rc;
    MDB_txn *   txn;
    MDB_dbi     dbi;
    MDB_cursor *cursor;
    MDB_val     key, data;
    time_t      expire;

    if ((expire = time(NULL)) < 0) {
        syslog(LOG_ALERT, "lmdb vdb_gc time: %m");
        return;
    }

    expire = expire - vdb->interval;

    if ((rc = mdb_txn_begin(vdb->lmdb, NULL, 0, &txn)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_gc mdb_txn_begin: %s", mdb_strerror(rc));
        return;
    }

    if ((rc = mdb_dbi_open(txn, NULL, MDB_CREATE, &dbi)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_gc mdb_dbi_open: %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return;
    }

    if ((rc = mdb_cursor_open(txn, dbi, &cursor)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_gc mdb_cursor_open: %s", mdb_strerror(rc));
        mdb_txn_abort(txn);
        return;
    }

    while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
        if ((strncmp(key.mv_data, "user:", 5) == 0) &&
                data.mv_size == sizeof(expire) &&
                (time_t)data.mv_data < expire) {
            if ((rc = mdb_del(txn, dbi, &key, NULL)) != 0) {
                syslog(LOG_ALERT, "lmdb vdb_gc mdb_del: %s", mdb_strerror(rc));
            }
        }
    }
    if (rc != MDB_NOTFOUND) {
        syslog(LOG_ALERT, "lmdb vdb_gc mdb_cursor_get: %s", mdb_strerror(rc));
    }

    if ((rc = mdb_txn_commit(txn)) != 0) {
        syslog(LOG_ALERT, "lmdb vdb_gc mdb_txn_commit: %s", mdb_strerror(rc));
    }
}

char *
lmdb_vdb_key(char *rcpt, char *from) {
    /* MDB_MAXKEYSIZE. 511 is the (old?) default and should be safe. */
    char key[ 511 ];

    snprintf(key, 511, "user:%s:%" PRIx64, rcpt,
            rabin_fingerprint(from, strlen(from)));
    syslog(LOG_DEBUG, "lmdb_vdb_key: %s", key);

    return (strdup(key));
}
