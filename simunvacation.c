/*
 * simunvacation.c - clean up database files for people who are no longer
 * "on vacation".
 *
 * Copyright (c) 2004, 2013 Regents of The University of Michigan.
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation, and that the name of The University
 * of Michigan not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. This software is supplied as is without expressed or
 * implied warranties of any kind.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/param.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>

#include "simvacation.h"
#include "vdb_berkeley.h"
#include "vlu_ldap.h"

void usage();

int main( int argc, char **argv)
{
    int debug = 0;
    extern int optind, opterr;
    extern char *optarg;
    char ch;

    struct vlu *vlu;
    struct vdb *vdb;
    char *vlu_config = CONFFILE;

    struct name_list *uniqnames;
    struct name_list *u;

    while (( ch = getopt( argc, argv, "c:d" )) != EOF ) {
	switch( (char) ch ) {

	case 'c':
	    vlu_config = optarg;
	    break;

        case 'd':
            debug = 1;
            break;

	case '?':
	default:
	    usage();
	}
    }

    if ( debug ) {
        openlog( "simunvacation", LOG_NOWAIT | LOG_PERROR | LOG_PID, LOG_VACATION );
    }
    else {
        openlog( "simunvacation", LOG_PID, LOG_VACATION );
    }

    vdb = malloc( sizeof( struct vdb ));
    memset( vdb, 0, sizeof( struct vdb ));
    vdb_init( vdb, "simunvacation" );
    uniqnames = vdb_get_names( vdb );

    vlu = vlu_init( vlu_config );
    if ( vlu_connect( vlu ) != 0 ) {
        vdb_close( vdb );
        exit( 1 );
    }

    /*
     * For each candidate uniqname, check to see if user is
     * still on vacation.  If not, remove the .dir and .pag files.
     */
     for ( u = uniqnames; u; u = u->next ) {
        switch ( vlu_search( vlu, u->name )) {
        case VLU_RESULT_PERMFAIL:
            syslog( LOG_INFO, "cleaning up %s", u->name );
            vdb_clean( vdb, u->name );
            break;
        case VLU_RESULT_TEMPFAIL:
            syslog( LOG_ERR, "lookup error processing %s", u->name );
            break;
        default:
            syslog( LOG_DEBUG, "leaving %s alone", u->name );
            break;
        }
    }

    vdb_close( vdb );
    vlu_close( vlu );
    exit( 0 );
}

void usage()
{
    fprintf( stderr, "usage: simunvacation [-v vbdir] [-s searchbase]\n" );
    fprintf( stderr, "                [-h ldap_host] [-p ldap_port]\n" );
    exit( 1 );
}
