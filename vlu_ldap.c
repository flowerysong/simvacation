/*
 * Copyright (c) 2004-2015 Regents of The University of Michigan
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


#include <limits.h>
#include <sysexits.h>
#include <time.h>
#include <syslog.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "argcargv.h"
#include "simvacation.h"
#include "vlu_ldap.h"

    struct vlu  *
vlu_init( char *config ) {
    struct vlu *vlu;
    struct vlu *ret = NULL;
    FILE *fp;
    ACAV *acav = NULL;
    char **av;
    int ac;
    char *line = NULL;
    size_t len = 0;

    if (( vlu = malloc( sizeof( struct vlu ))) == NULL ) {
        syslog( LOG_ERR, "vlu_init: malloc error: %m" );
        return NULL;
    }
    memset( vlu, 0, sizeof( struct vlu ));

    vlu->attr_vacation = ATTR_ONVAC;
    vlu->attr_vacation_msg = ATTR_VACMSG;
    vlu->attr_cn = ATTR_CN;
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
            if ((vlu->attr_vacation = strdup( av[ 1 ] )) == NULL ) {
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
    static char *attrs[4];

    attrs[ 0 ] = vlu->attr_cn;
    attrs[ 1 ] = vlu->attr_vacation;
    attrs[ 2 ] = vlu->attr_vacation_msg;
    attrs[ 3 ] = NULL;

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

    vacstatus = ldap_get_values( vlu->ld, vlu->result, vlu->attr_vacation );

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

    struct name_list *
vlu_aliases( struct vlu *vlu, char *rcpt ) {
    char **cnames;
    int i;
    struct name_list *result;
    struct name_list *cur;

    cnames = ldap_get_values( vlu->ld, vlu->result, ATTR_CN );

    if (( cur = malloc( sizeof( struct name_list ))) == NULL ) {
	syslog( LOG_ALERT, "vlu_aliases: malloc for name_list failed");
        return NULL;
    }

    cur->name = rcpt;
    cur->next = NULL;
    result = cur;

    for ( i = 0; cnames && cnames[i] != NULL; i++ ) {
	if (( cur->next = malloc( sizeof( struct name_list ))) == NULL ) {
	    syslog( LOG_ALERT, "vlu_aliases: malloc for name_list failed");
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
    char    **rawmsg;
    int     i;
    yastr   vacmsg;
    char    *p;

    rawmsg = ldap_get_values( vlu->ld, vlu->result, ATTR_VACMSG );

    if ( rawmsg == NULL ) {
        return NULL;
    }

    vacmsg = yaslempty();

    for ( i = 0; rawmsg[i] != NULL; i++ ) {
        vacmsg = yaslcat( vacmsg, rawmsg[i] );
    }

    for ( p = vacmsg; *p ; p++ ) {
        if ( *p == '$' ) {
            *p = '\n';
        }
    }

    return vacmsg;
}

    char *
vlu_name( struct vlu *vlu, char *rcpt ) {
    return ldap_get_dn( vlu->ld, vlu->result );
}

    char *
vlu_display_name( struct vlu *vlu, char *rcpt ) {
    char **xdn = ldap_explode_dn( ldap_get_dn( vlu->ld, vlu->result ), 1 );
    return xdn[0];
}

    void
vlu_close ( struct vlu *vlu ) {

}
