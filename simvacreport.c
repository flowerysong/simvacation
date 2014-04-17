/*
 * simvacreport.c
 *
 * Print a report of who's on vacation, and the list of recipients they've
 * sent vacation messages to.
 *
 * Copyright (c) 2005 Regents of The University of Michigan.
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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>

#include "simvacation.h"
#include "vdb_berkeley.h"

    void
usage( char *progname )
{                         
    fprintf( stderr, "usage: %s [-v vacdb]\n", progname);
    exit( 1 );
}

/* FIXME: vdb abstraction */

    int
main( int argc,  char **argv)
{

    char		c;
    char		vdbdir[MAXPATHLEN];
    char		vdbfile[MAXPATHLEN];
    char		buf[MAXPATHLEN];
    char		*vdbpath = VDBDIR;
    struct dirent	*dire;
    DIR			*dirp;

    DBT 		key, data;      /* NULL key/data for seq. access */
    DB			*dbp = NULL;	/* Berkeley DB instance */
    DBC			*dbcp;		/* Database Cursor for seq. access */

    char		*keybuf;	/* Char mapping across blob */
    int			rc;		/* universal return code */

    char 		ch;
    extern int 		optind, opterr;
    extern char 	*optarg;

    char		*progname;

    if ( (progname = strrchr( argv[0], '/' )) == NULL )
	progname = strdup( argv[0] );
    else
	progname = strdup( progname + 1 );

    while (( ch = getopt( argc, argv, "v:" )) != EOF ) {
        switch( (char) ch ) {
        case 'v':               /* dir for vacation db files */
            vdbpath = optarg;
            break;
            
        case '?':
        default:
            usage( progname );
        }   
    }


    for ( c = 'a'; c <= 'z'; c++ ) {
	sprintf( vdbdir, "%s/%c", vdbpath, c );
	if ((dirp = opendir( vdbdir )) == NULL) {
	    fprintf( stderr, "%s: can't open directory %s: %s\n",
		     progname, vdbdir, strerror(errno) );
	    continue;
	}

	while (( dire = readdir( dirp )) != NULL ) {
	    int		len;

	    /* is this a dbm file? */
	    len = strlen( dire->d_name );
	    if ( len > 4 ) {
		if ( !strncmp( &dire->d_name[len - 4],
			       ".ddb", 4 )) {
		    strncpy( buf, dire->d_name, len);
		    buf[len] = '\0';
		    sprintf( vdbfile, "%s/%s", vdbdir, buf );

		    /* Create the database handle */
		    if ((rc = db_create( &dbp, NULL, 0)) != 0) {
			fprintf ( stderr, 
			    "Failed creating database handle: db_create: %s\n",
        		    db_strerror(rc));
        		continue;  /* XXX Or do we want to quit */
		    }
		    if ((rc = dbp->open(dbp,  NULL, vdbfile, 
				NULL, DB_UNKNOWN, DB_RDONLY, 0664)) != 0) {

			fprintf (stderr, "Failed opening database: %s Error:%s\n",
				vdbdir, db_strerror(rc));
			(void)dbp->close(dbp, 0);
			continue;
		    }

		    printf( "user: %s\n", buf );
		    /* enumerate entries in database */

		    /* Acquire a cursor for the database. */
		    if ((rc = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
			fprintf (stderr, "Failed opening cursor: %s Error: %s\n",
			vdbdir, db_strerror(rc));
			(void)dbp->close(dbp, 0);
			continue;
		    }

    		    /* Initialize the key/data pair so the flags aren't set. */
		    memset(&key, 0, sizeof(key));
		    memset(&data, 0, sizeof(data));

		    while ((rc = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0){

			/*
			** DO NOT NULL TERMINATE key.data.
			** This Berkeley allocated buffer might not
			** be big enough.
			*/
			keybuf = (char *) key.data;

			if (strncmp(keybuf, VIT, strlen( VIT ) - 1) != 0) {
			    if ( keybuf  ) {
				printf ("\t%.*s\n", key.size, keybuf );
			    }
			}
		    }
		    if (rc != DB_NOTFOUND) {
			fprintf (stderr, "Failed reading db: %s Error: %s\n",
				vdbdir, db_strerror(rc));
		    }
		    if ((rc = dbcp->c_close(dbcp)) != 0) {
			fprintf (stderr, "Failed closing cursor: %s Error: %s\n",
				vdbdir, db_strerror(rc));
		    }
		    if ((rc = dbp->close(dbp, 0)) != 0) {
			fprintf (stderr, "Failed closing db: %s Error: %s\n",
				vdbdir, db_strerror(rc));
		    }
		}
	    }
	}
	closedir(dirp);
    }

    return( 0 );
}

