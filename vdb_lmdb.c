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
#include "vdb_lmdb.h"

char *lmdb_vdb_key( char *, char * );
void lmdb_vdb_assert( MDB_env *, const char * );

    struct vdb *
vdb_init( char *rcpt )
{
    int         rc;
    struct vdb  *vdb;

    if (( vdb = calloc( 1, sizeof( struct vdb ))) == NULL ) {
        return( NULL );
    }

    if (( rc = mdb_env_create( &vdb->dbenv )) != 0 ) {
        syslog( LOG_ALERT,
                "lmdb vdb_init mdb_env_create: %s", mdb_strerror( rc ));
        goto error;
    }

    if (( rc = mdb_env_set_assert( vdb->dbenv, &lmdb_vdb_assert )) != 0 ) {
        syslog( LOG_ALERT,
                "lmdb vdb_init mdb_env_set_assert: %s", mdb_strerror( rc ));
        goto error;
    }

    if (( rc = mdb_env_set_mapsize( vdb->dbenv, 1073741824 )) != 0 ) {
        syslog( LOG_ALERT,
                "lmdb vdb_init mdb_env_mapsize: %s", mdb_strerror( rc ));
        goto error;
    }

    if (( rc = mdb_env_open( vdb->dbenv, VDBDIR, 0, 0664 )) != 0 ) {
        syslog( LOG_ALERT,
                "lmdb vdb_init mdb_env_open: %s", mdb_strerror( rc ));
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
lmdb_vdb_assert( MDB_env *dbenv, const char *msg ) {
    syslog( LOG_ALERT, "lmdb assert: %s", msg );
    /* FIXME: is this the correct action? */
    exit( EX_TEMPFAIL );
}

    void
vdb_close( struct vdb *vdb )
{
    if ( vdb ) {
        if ( vdb->dbenv ) {
            mdb_env_close( vdb->dbenv );
        }
        free( vdb );
    }
}

    int
vdb_recent( struct vdb *vdb, char *from )
{
    int         rc, retval = 0;
    time_t      last, now;
    MDB_txn     *txn;
    MDB_dbi     dbi;
    MDB_val     key, data;
    char       *keyval = NULL;

    if (( keyval = lmdb_vdb_key( vdb->rcpt, from )) == NULL ) {
        return( 0 );
    }

    if (( rc = mdb_txn_begin( vdb->dbenv, NULL, MDB_RDONLY, &txn )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_recent mdb_txn_begin: %s", mdb_strerror( rc ));
        free( keyval );
        return( 0 );
    }

    if (( rc = mdb_dbi_open( txn, NULL, MDB_CREATE, &dbi )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_recent mdb_dbi_open: %s", mdb_strerror( rc ));
        goto cleanup;
    }

    key.mv_size = strlen( keyval );
    key.mv_data = keyval;

    if (( rc = mdb_get( txn, dbi, &key, &data )) != 0 ) {
        if ( rc != MDB_NOTFOUND ) {
            syslog( LOG_ALERT, "lmdb vdb_recent mdb_get: %s", mdb_strerror( rc ));
        }
        goto cleanup;
    }

    if ( data.mv_size != sizeof( last )) {
        syslog( LOG_ALERT, "lmdb vdb_recent: retrieved bad data" );
        goto cleanup;
    }

    last = *(time_t *)data.mv_data;

    if (( now = time( NULL )) < 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_recent time: %m" );
    } else if (now < ( last + vdb->interval )) {
        retval = 1;
    }

cleanup:
    free( keyval );
    mdb_txn_abort( txn );
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
    int         rc;
    time_t      now;
    MDB_txn     *txn;
    MDB_dbi     dbi;
    MDB_val     key, data;
    char        *keyval;

    if (( now = time( NULL )) < 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_store_reply time: %m" );
        return( 1 );
    }

    if (( rc = mdb_txn_begin( vdb->dbenv, NULL, 0, &txn )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_store_reply mdb_txn_begin: %s", mdb_strerror( rc ));
        return( 1 );
    }

    if (( rc = mdb_dbi_open( txn, NULL, MDB_CREATE, &dbi )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_store_reply mdb_dbi_open: %s", mdb_strerror( rc ));
        mdb_txn_abort( txn );
        return( 1 );
    }

    keyval = lmdb_vdb_key( vdb->rcpt, from );
    key.mv_size = strlen( keyval );
    key.mv_data = keyval;

    data.mv_size = sizeof( now );
    data.mv_data = &now;

    if (( rc = mdb_put( txn, dbi, &key, &data, 0 )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_store_reply mdb_put: %s", mdb_strerror( rc ));
        mdb_txn_abort( txn );
        return( 1 );
    }

    if (( rc = mdb_txn_commit( txn )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_store_reply mdb_txn_commit: %s", mdb_strerror( rc ));
        return( 1 );
    }

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
    /* Nothing; the LMDB backend only does vdb_gc */
    return;
}

    void
vdb_gc( struct vdb *vdb )
{
    int         rc;
    MDB_txn     *txn;
    MDB_dbi     dbi;
    MDB_cursor  *cursor;
    MDB_val     key, data;
    time_t      expire;

    if (( expire = time( NULL )) < 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_gc time: %m" );
        return;
    }

    expire = expire - vdb->interval;

    if (( rc = mdb_txn_begin( vdb->dbenv, NULL, 0, &txn )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_gc mdb_txn_begin: %s", mdb_strerror( rc ));
        return;
    }

    if (( rc = mdb_dbi_open( txn, NULL, MDB_CREATE, &dbi )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_gc mdb_dbi_open: %s", mdb_strerror( rc ));
        mdb_txn_abort( txn );
        return;
    }

    if (( rc = mdb_cursor_open( txn, dbi, &cursor )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_gc mdb_cursor_open: %s", mdb_strerror( rc ));
        mdb_txn_abort( txn );
        return;
    }

    while (( rc = mdb_cursor_get( cursor, &key, &data, MDB_NEXT )) == 0 ) {
        if (( strncmp( key.mv_data, "user:", 5 ) == 0 ) &&
                data.mv_size == sizeof( expire ) &&
                (time_t)data.mv_data < expire ) {
            if (( rc = mdb_del( txn, dbi, &key, NULL )) != 0 ) {
                syslog( LOG_ALERT, "lmdb vdb_gc mdb_del: %s", mdb_strerror( rc ));
            }
        }
    }
    if ( rc != MDB_NOTFOUND ) {
        syslog( LOG_ALERT, "lmdb vdb_gc mdb_cursor_get: %s", mdb_strerror( rc ));
    }

    if (( rc = mdb_txn_commit( txn )) != 0 ) {
        syslog( LOG_ALERT, "lmdb vdb_gc mdb_txn_commit: %s", mdb_strerror( rc ));
    }
}

    char *
lmdb_vdb_key( char *rcpt, char *from )
{
    /* MDB_MAXKEYSIZE. 511 is the (old?) default and should be safe. */
    char            key[ 511 ];
    
    snprintf( key, 511, "user:%s:%llx", rcpt,
            rabin_fingerprint( from, strlen( from )));
    syslog( LOG_DEBUG, "lmdb_vdb_key: %s", key );

    return( strdup( key ));
}
