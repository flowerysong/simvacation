/*
 * Copyright (c) 2015 Regents of The University of Michigan.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <sys/param.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include "rabin.h"
#include "simvacation.h"
#include "vdb_redis.h"

static char *redis_vdb_key( char *, char * );

    struct vdb *
vdb_init( char *rcpt )
{
    struct vdb  *vdb;

    if (( vdb = calloc( 1, sizeof( struct vdb ))) == NULL ) {
        return( NULL );
    }

    if (( vdb->u = urcl_connect( VDBDIR, 6379 )) == NULL ) {
        syslog( LOG_ALERT, "redis vdb_init urcl_connect: failed" );
        goto error;
    }

    vdb->interval = SECSPERDAY * DAYSPERWEEK;
    vdb->rcpt = strdup( rcpt );

    return( vdb );

error:
    vdb_close( vdb );
    return( NULL );
}

    void
vdb_close( struct vdb *vdb )
{
    if ( vdb ) {
        if ( vdb->u ) {
            urcl_free( vdb->u );
        }
        free( vdb );
    }
}

    int
vdb_recent( struct vdb *vdb, char *from )
{
    int         retval = 0;
    time_t      last, now;
    char        *key;
    redisReply  *res = NULL;

    if (( key = redis_vdb_key( vdb->rcpt, from )) == NULL ) {
        return( 0 );
    }

    if ((( res = urcl_command( vdb->u, key, "GET %s", key )) == NULL ) ||
            ( res->type != REDIS_REPLY_STRING )) {
        goto cleanup;
    }

    last = (time_t)strtoll( res->str, NULL, 10 );

    if (( now = time( NULL )) < 0 ) {
        syslog( LOG_ALERT, "redis vdb_recent time: %m" );
    } else if ( now < ( last + vdb->interval )) {
        retval = 1;
    } else {
        /* This shouldn't happen, so let's clean it up */
        urcl_free_result( urcl_command( vdb->u, key, "DEL %s", key ));
    }

cleanup:
    urcl_free_result( res );
    free( key );
    return( retval );
}

   int
vdb_store_interval( struct vdb *vdb, time_t interval )
{
    vdb->interval = interval;
    return( 0 );
}

    int
vdb_store_reply( struct vdb *vdb, char *from )
{
    time_t      now;
    char        *key;
    char        value[ 16 ];
    char        expire[ 16 ];

    if (( now = time( NULL )) < 0 ) {
        syslog( LOG_ALERT, "redis vdb_store_reply time: %m" );
        return( 1 );
    }

    key = redis_vdb_key( vdb->rcpt, from );
    snprintf( value, 16, "%lld", (long long)now );
    snprintf( expire, 16, "%lld", vdb->interval );

    urcl_free_result( urcl_command( vdb->u, key, "SET %s %s", key, value ));
    urcl_free_result( urcl_command( vdb->u, key, "EXPIRE %s %s", key, expire ));

    return( 0 );
}

    struct name_list *
vdb_get_names( struct vdb *vdb )
{
    return( NULL );
}

    void
vdb_clean( struct vdb *vdb, char *user )
{
    /* Nothing; redis automatically expires stale keys */
    return;
}

    void
vdb_gc( struct vdb *vdb )
{
    /* Nothing; redis automatically expires stale keys */
    return;
}

    static char *
redis_vdb_key( char *rcpt, char *from )
{
    char            key[ 128 ];

    snprintf( key, 128, "simvacation:user:%s:%llx", rcpt,
            rabin_fingerprint( from, strlen( from )));
    syslog( LOG_DEBUG, "redis_vdb_key: %s", key );

    return( strdup( key ));
}
