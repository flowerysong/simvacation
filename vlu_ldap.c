/*
 * Copyright (c) 2004, 2013 Regents of The University of Michigan.
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


#include <limits.h>
#include <sysexits.h>
#include <time.h>
#include <syslog.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define LDAP_DEPRECATED		1

#include <lber.h>
#include <ldap.h>

#include "argcargv.h"
#include "simvacation.h"
#include "vlu_ldap.h"
#include "vlu.h"

    struct vlu  *
vlu_init( char *config ) {
    struct vlu *vlu;
    struct vlu *ret = NULL;
    struct timeval timeout;
    FILE *fp;
    ACAV *acav;
    char **av;
    int ac;
    char *line = NULL;
    size_t len = 0;

    if (( vlu = malloc( sizeof( struct vlu ))) == NULL ) {
        syslog( LOG_ERR, "vlu_init: malloc error: %m" );
        return NULL;
    }
    memset( vlu, 0, sizeof( struct vlu ));

    vlu->ldap_timeout.tv_sec = 30;

    if (( fp = fopen( config, "r" )) == NULL ) {
        syslog( LOG_ERR, "vlu_init: open %s: %m", config );
        goto error;
    }

    if (( acav = acav_alloc( )) == NULL ) {
        syslog( LOG_ERR, "vlu_init: acav_alloc error" );
        goto error;
    }

    while ((getline( &line, &len, fp)) != -1 ) {
        if (( line[0] == '#' ) || ( line[0] == '\0' )) {
            continue;
        }

        if (( ac = acav_parse( acav, line, &av )) < 0 ) {
            syslog( LOG_ERR, "vlu_init: acav_parse error" );
            goto error;
        }

        if ( strcasecmp( av[ 0 ], "host" ) == 0 ) {
            if ( ac != 2 ) {
                syslog( LOG_ERR, "vlu_init: usage: host <ldap host>" );
                goto error;
            }
            if ((vlu->ldap_host = strdup( av[ 1 ] )) == NULL ) {
                syslog( LOG_ERR, "vlu_init: strdup: %m" );
                goto error;
            }
        }
        else if ( strcasecmp( av[ 0 ], "port" ) == 0 ) {
            if ( ac != 2 ) {
                syslog( LOG_ERR, "vlu_init: usage: port <ldap port>" );
                goto error;
            }
            vlu->ldap_port = atoi( av[ 1 ] );
        }
        else if ( strcasecmp( av[ 0 ], "timeout" ) == 0 ) {
            if ( ac != 2 ) {
                syslog( LOG_ERR, "vlu_init: usage: timeout <ldap timeout>" );
                goto error;
            }
            vlu->ldap_timeout.tv_sec = atoi( av[ 1 ]);
        }
        else if ( strcasecmp( av[ 0 ], "vacationattr" ) == 0 ) {
            if ( ac != 2 ) {
                syslog( LOG_ERR, "vlu_init: usage: vacationattr <attribute>" );
                goto error;
            }
            if ((vlu->vacationattr = strdup( av[ 1 ] )) == NULL ) {
                syslog( LOG_ERR, "vlu_init: strdup: %m" );
                goto error;
            }
        }
    }

    ret = vlu;

    error:

    free( line );

    if ( acav ) {
        acav_free( acav );  
    }

    if ( ret == NULL ) {
        free( vlu );
    }
    
    return ret;
}

    int
vlu_connect( struct vlu *vlu ) {
    char *errmsg;

    if (( vlu->ld = ldap_open( vlu->ldap_host, vlu->ldap_port )) == NULL ) {
	syslog( LOG_INFO, "ldap: ldap_open failed" );
	return( 1 );
    }

    if (( ldap_simple_bind_s( vlu->ld, BIND_DN, BIND_METHOD ))
            != LDAP_SUCCESS ) {
	ldap_get_option ( vlu->ld, LDAP_OPT_ERROR_STRING, &errmsg) ;
	syslog( LOG_ALERT, "vlu_connect: ldap_simple_bind failed: %s", errmsg);
	free (errmsg);
	return( 1 );
    }

    return( 0 );
}

    int
vlu_search( struct vlu *vlu, char *rcpt ) {
    char filter[64];
    int rc, retval, matches;
    LDAPMessage *result;
    char **vacstatus;
    char *searchbase = SEARCHBASE;
    static char *attrs[] = { ATTR_VACMSG, ATTR_CN, NULL, NULL };

    attrs[ 2 ] = vlu->vacationattr;

    sprintf( filter, "uid=%s", rcpt );
    rc = ldap_search_st( vlu->ld, searchbase, LDAP_SCOPE_SUBTREE,
	    filter, attrs, 0, &vlu->ldap_timeout, &result );

    switch ( rc ) {
    case LDAP_SUCCESS:
	break;
    case LDAP_SERVER_DOWN:
    case LDAP_TIMEOUT:
    case LDAP_BUSY:
    case LDAP_UNAVAILABLE:
    case LDAP_OTHER:
    case LDAP_LOCAL_ERROR:
	retval = VLU_RESULT_TEMPFAIL;
	break;
    default:
	retval = VLU_RESULT_PERMFAIL;
    }
    if ( rc != LDAP_SUCCESS ) {
        syslog( LOG_ALERT, "vlu_search: ldap_search failed: %s",
                ldap_err2string( rc ));
        return( retval );
    }		

    matches = ldap_count_entries( vlu->ld, result );

    if ( matches > 1 ) {
	syslog( LOG_ALERT, "vlu_search: multiple matches for %s", rcpt );
	return( VLU_RESULT_PERMFAIL );
    } else if ( matches == 0 ) {
	syslog( LOG_ALERT, "vlu_search: no match for %s", rcpt );
	return( VLU_RESULT_PERMFAIL );
    }
    
    vlu->result = ldap_first_entry( vlu->ld, result );

    vacstatus = ldap_get_values( vlu->ld, vlu->result, vlu->vacationattr );

    if ( vacstatus && ( strcasecmp( *vacstatus, "TRUE" ) == 0 )) {
        syslog( LOG_DEBUG, "vlu_search: user %s on vacation", rcpt );
    }
    else {
        /* User is no longer on vacation */
        syslog( LOG_INFO, "vlu_search: user %s is not on vacation", rcpt );
        return( VLU_RESULT_PERMFAIL );
    }

    return( VLU_RESULT_OK );
}

    struct alias *
vlu_aliases( struct vlu *vlu, char *rcpt ) {
    char **cnames;
    int i;
    struct alias *result;
    struct alias *cur;

    cnames = ldap_get_values( vlu->ld, vlu->result, ATTR_CN );

    if (( cur = malloc( sizeof( struct alias ))) == NULL ) {
	syslog( LOG_ALERT, "vlu_aliases: malloc for alias failed");
        return NULL;
    }

    cur->name = rcpt;
    cur->next = NULL;
    result = cur;

    for ( i = 0; cnames && cnames[i] != NULL; i++ ) {
	if (( cur->next = malloc( sizeof( struct alias ))) == NULL ) {
	    syslog( LOG_ALERT, "vlu_aliases: malloc for alias failed");
            return NULL;
	}
	cur = cur->next;
	cur->name = cnames[i];
	cur->next = NULL;
    }

    return result;
}

    char *
vlu_message( struct vlu *vlu, char *rcpt ) {
    char **rawmsg;
    int i, pos;
    size_t len;
    char *vacmsg;
    char *p;

    rawmsg = ldap_get_values( vlu->ld, vlu->result, ATTR_VACMSG );

    if ( rawmsg == NULL ) {
        return NULL;
    }
    
    pos = 0;
    len = 50;
    vacmsg = malloc( len );

    for ( i = 0; rawmsg[i] != NULL; i++ ) {
        for ( p = rawmsg[i]; *p ; p++ ) {
            if ( *p == '$' ) {
                vacmsg[ pos ] = '\n';
            }
            else {
                vacmsg[ pos ] = *p;
            }

            pos++;
            if ( pos >= len ) {
                len += 50;
                vacmsg = realloc( vacmsg, len );
            }
        }
    }

    return vacmsg;
}

    void
vlu_close ( struct vlu *vlu ) {
    
}
