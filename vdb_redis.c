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

static char *redis_vdb_key(char *, char *);

VDB *
redis_vdb_init(const ucl_object_t *config, const char *rcpt) {
    VDB *       vdb;
    VDB *       res = NULL;
    const char *host = "127.0.0.1";
    int64_t     port = 6379;

    if (!ucl_object_tostring_safe(
                ucl_object_lookup_path(config, "redis.host"), &host)) {
        syslog(LOG_ERR, "vdb_init: ucl_object_tostring_safe failed");
        goto error;
    }
    if (!ucl_object_toint_safe(
                ucl_object_lookup_path(config, "redis.port"), &port)) {
        syslog(LOG_ERR, "vdb_init: ucl_object_toint_safe failed");
        goto error;
    }

    if ((vdb = calloc(1, sizeof(VDB))) == NULL) {
        goto error;
    }

    if ((vdb->redis = urcl_connect(host, port)) == NULL) {
        syslog(LOG_ALERT, "redis vdb_init urcl_connect: failed");
        goto error;
    }

    vdb->interval = SECSPERDAY * DAYSPERWEEK;
    vdb->rcpt = strdup(rcpt);

    res = vdb;

error:
    if (vdb && !res) {
        redis_vdb_close(vdb);
    }

    return (res);
}

void
redis_vdb_close(VDB *vdb) {
    if (vdb) {
        if (vdb->redis) {
            urcl_free(vdb->redis);
        }
        free(vdb);
    }
}

int
redis_vdb_recent(VDB *vdb, char *from) {
    int         retval = 0;
    time_t      last, now;
    char *      key;
    redisReply *res = NULL;

    if ((key = redis_vdb_key(vdb->rcpt, from)) == NULL) {
        return (0);
    }

    if (((res = urcl_command(vdb->redis, key, "GET %s", key)) == NULL) ||
            (res->type != REDIS_REPLY_STRING)) {
        goto cleanup;
    }

    last = (time_t)strtoll(res->str, NULL, 10);

    if ((now = time(NULL)) < 0) {
        syslog(LOG_ALERT, "redis vdb_recent time: %m");
    } else if (now < (last + vdb->interval)) {
        retval = 1;
    } else {
        /* This shouldn't happen, so let's clean it up */
        urcl_free_result(urcl_command(vdb->redis, key, "DEL %s", key));
    }

cleanup:
    urcl_free_result(res);
    free(key);
    return (retval);
}

int
redis_vdb_store_reply(VDB *vdb, char *from) {
    time_t now;
    char * key;
    char   value[ 16 ];
    char   expire[ 16 ];

    if ((now = time(NULL)) < 0) {
        syslog(LOG_ALERT, "redis vdb_store_reply time: %m");
        return (1);
    }

    key = redis_vdb_key(vdb->rcpt, from);
    snprintf(value, 16, "%lld", (long long)now);
    snprintf(expire, 16, "%lld", (long long)vdb->interval);

    urcl_free_result(urcl_command(vdb->redis, key, "SET %s %s", key, value));
    urcl_free_result(
            urcl_command(vdb->redis, key, "EXPIRE %s %s", key, expire));

    return (0);
}

static char *
redis_vdb_key(char *rcpt, char *from) {
    char key[ 128 ];

    snprintf(key, 128, "simvacation:user:%s:%" PRIx64, rcpt,
            rabin_fingerprint(from, strlen(from)));
    syslog(LOG_DEBUG, "redis_vdb_key: %s", key);

    return (strdup(key));
}
