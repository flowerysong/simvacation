/*
 * Copyright (c) 2004, 2013, 2014 Regents of The University of Michigan.
 * Copyright (c) 1983, 1987 Regents of the University of California.
 * Copyright (c) 1983 Eric P. Allman.
 * All Rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <sys/param.h>

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>

#include "simvacation.h"
#include "vdb_berkeley.h"
#include "vdb.h"

void    bdb_err_log ( const DB_ENV *, const char *, const char * );
char   *bdb_path( char *, char * );

    struct vdb *
vdb_init( char *rcpt )
{
    int         rc;
    char        *path;
    struct vdb  *vdb;

    if (( vdb = calloc( 1, sizeof( struct vdb ))) == NULL ) {
        return( NULL );
    }

    path = bdb_path( VDBDIR, rcpt );

    if (( rc = db_create( &vdb->dbp, NULL, 0 )) != 0) {
        syslog( LOG_ALERT,  "bdb: db_create: %s", db_strerror(rc));
        goto error;
    }   

    vdb->dbp->set_errpfx( vdb->dbp, "DB creation");
    vdb->dbp->set_errcall( vdb->dbp, bdb_err_log);
    if ((rc = vdb->dbp->set_pagesize( vdb->dbp, 1024)) != 0) {
	vdb->dbp->err(vdb->dbp, rc, "set_pagesize");
        goto error;
    } 
    if ((rc = vdb->dbp->set_cachesize( vdb->dbp, 0, 32 * 1024, 0)) != 0) {
	vdb->dbp->err( vdb->dbp, rc, "set_cachesize");
        goto error;
    }
	
    if ((rc = vdb->dbp->open(
            vdb->dbp,       /* DB handle */
	    NULL,           /* transaction handle */
	    path,           /* db file name */
	    NULL,           /* database name */
	    DB_BTREE,       /* DB type */
	    DB_CREATE,      /* Create db if it doesn't exist */
	    0664)) != 0) {
	vdb->dbp->err(vdb->dbp, rc, "%s: open", path);
	syslog( LOG_ALERT, "bdb: %s: %s\n", path, strerror( errno ));
        goto error;
    }
    vdb->dbp->set_errpfx( vdb->dbp, "");

    return( vdb );

error:
    free( vdb );
    return( NULL );
}

    void
vdb_close( struct vdb *vdb )
{   
    if ( vdb ) {
        if ( vdb->dbp ) {
            (void)vdb->dbp->close( vdb->dbp, 0);
        }
        free( vdb );
    }
}

    int
vdb_recent( struct vdb *vdb, char *from )
{
    DBT		key;
    DBT		data;
    time_t	then;
    time_t	next;
    int		rc;

    /* get interval time */
    memset (&key, 0, sizeof( key ));
    key.data = VIT;
    key.size = strlen(VIT) - 1;

    memset (&data, 0, sizeof( data ));

    data.data = &next;
    data.size = sizeof( next );

    next = 0;   /* for debugging */
    vdb->dbp->set_errpfx(vdb->dbp, "Getting VIT");
    if ((rc = vdb->dbp->get(vdb->dbp, NULL, &key, &data, 0)) == 0) {
	memcpy (&next, data.data, sizeof( next ));
    } else {
	next = SECSPERDAY * DAYSPERWEEK;
    }

    /* get record for this email address */
    memset (&key, 0, sizeof( key ));
    key.data = from;
    key.size = strlen( from );

    memset (&data, 0, sizeof( data ));
    then = 0;   /* for debugging */
    vdb->dbp->set_errpfx(vdb->dbp, "Getting recent time");
    if ((rc = vdb->dbp->get(vdb->dbp, NULL, &key, &data, 0)) == 0) {
	memcpy (&then, data.data, sizeof( then ));

	if ( next == LONG_MAX || then + next > time(( time_t *) NULL ))
	    return( 1 );
    }
    return( 0 );
}

   int
vdb_store_interval( struct vdb *vdb, time_t interval )
{
    DBT  key;
    DBT  data;
    int  rc;

    memset (&key, 0, sizeof ( key ));
    key.data = VIT;
    key.size = strlen( VIT ) - 1;
    memset (&data, 0, sizeof ( data ));
    data.data = (char *) &interval;
    data.size = sizeof( interval );
    rc = vdb->dbp->put(vdb->dbp, NULL, &key, &data, (u_int32_t) 0); /* allow overwrites */
    if (rc != 0) {
	syslog( LOG_ALERT, "bdb: error while putting interval: %d, %s", 
		rc, db_strerror(rc) );
    }
    return (rc);
}

    int
vdb_store_reply( struct vdb *vdb, char *from )
{
    DBT  key;
    DBT  data;
    int  rc;

    time_t now;

    memset (&key, 0, sizeof ( key ));
    key.data = from;
    key.size = strlen( from );
    (void) time( &now );
    memset (&data, 0, sizeof ( data ));
    data.data = (char *) &now;
    data.size = sizeof( now );
    rc = vdb->dbp->put(vdb->dbp, NULL, &key, &data, 0);  /* allow overwrites */
    if (rc != 0) {
	syslog( LOG_ALERT, "bdb: error while putting reply time: %d, %s", 
		rc, db_strerror(rc) );
    }

    return (rc);
}

    struct name_list *
vdb_get_names( struct vdb *vdb )
{
    char                *cur_path;
    char                cur_char;
    char                *vdbdir = VDBDIR;
    char                buf[ MAXPATHLEN ];
    DIR                 *dirp;
    struct dirent       *cur_entry;
    int                 len;
    struct name_list    *names;
    struct name_list    *cur_name;

    if ( access( vdbdir, F_OK )) {
        syslog( LOG_ALERT, "vdb_get_names: cannot open %s", vdbdir );
        return NULL;
    }

    names = malloc( sizeof( struct name_list ));
    memset( names, 0, sizeof( struct name_list ));
    cur_name = names;

    for ( cur_char = 'a'; cur_char <= 'z'; cur_char++ ) {
        sprintf( cur_path, "%s/%c", vdbdir, cur_char );
        if (( dirp = opendir( cur_path )) == NULL ) {
            syslog( LOG_ALERT, "vdb_get_names: cannot open %s", cur_path );
            continue;
        }

        while (( cur_entry = readdir( dirp )) != NULL ) {
            len = strlen( cur_entry->d_name );
            if ( len < 5 ||
                    strncmp( &cur_entry->d_name[ len - 4 ], ".ddb", 4 ) != 0 ) {
                continue;
            }

            if ( cur_name->name ) {
                cur_name->next = malloc( sizeof( struct name_list ));
                cur_name = cur_name->next;
                cur_name->next = NULL;
            }
            strncpy( buf, cur_entry->d_name, len - 4 );
            buf[ len - 4 ] = '\0';
            cur_name->name = strdup( buf );
        }
    }
    
    if ( cur_name->name ) {
        return names;
    }

    return NULL;
}

    void
vdb_clean( struct vdb *vdb, char *user )
{
    char *fname = bdb_path( VDBDIR, user );
    if ( unlink( fname ) < 0 ) {
        syslog( LOG_ALERT, "vdb_clean: cannot remove %s: %m", fname );
    }
}

    void
vdb_gc( struct vdb *vdb )
{
    return;
}

    char *
bdb_path( char *dir, char *user )
{
    char	buf[ MAXPATHLEN ];

    if (( user == NULL ) || ( dir == NULL )) {
	return( NULL );
    }

    if ( (strlen(dir) + strlen (user) + 4) > sizeof (buf) ) {
	return( NULL );
    }

    sprintf( buf, "%s/%c/%s.ddb", dir, user[0], user );
    return( strdup( buf ));
}

    void 
bdb_err_log ( const DB_ENV * dbenv, const char *errpfx, const char *msg )
{
    syslog( LOG_ALERT, "bdb: %s: %s", errpfx, msg);
    return;
}
