/*
 * Copyright (c) 2004-2016 Regents of The University of Michigan
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

#include "simvacation.h"
#include "vlu_ldap.h"

    struct vlu *
vlu_init( const ucl_object_t *config ) {
    struct vlu          *vlu;
    struct vlu          *ret = NULL;
    const ucl_object_t  *config_key;

    if (( vlu = malloc( sizeof( struct vlu ))) == NULL ) {
        syslog( LOG_ERR, "vlu_init: malloc error: %m" );
        return NULL;
    }
    memset( vlu, 0, sizeof( struct vlu ));

    vlu->attr_vacation = ATTR_ONVAC;
    vlu->attr_vacation_msg = ATTR_VACMSG;
    vlu->attr_cn = ATTR_CN;
    vlu->attr_name = ATTR_NAME;
    vlu->ldap_timeout.tv_sec = 30;

    if (( config = ucl_object_lookup( config, "ldap" )) == NULL ) {
        syslog( LOG_DEBUG, "vlu_init: no config information, using defaults" );
        goto error;
    } else {
        if (( config_key = ucl_object_lookup( config, "host" )) != NULL ) {
            if ( !ucl_object_tostring_safe( config_key, &(vlu->ldap_host))) {
                syslog( LOG_ERR, "vlu_init: ucl_object_tostring_safe failed" );
                goto error;
            }
        }
        if (( config_key = ucl_object_lookup( config, "port" )) != NULL ) {
            if ( !ucl_object_toint_safe( config_key, &(vlu->ldap_port ))) {
                syslog( LOG_ERR, "vlu_init: ucl_object_toint_safe failed" );
                goto error;
            }
        }
        if (( config_key = ucl_object_lookup( config,
                "vacationattr" )) != NULL ) {
            if ( !ucl_object_tostring_safe( config_key,
                    &(vlu->attr_vacation))) {
                syslog( LOG_ERR, "vlu_init: ucl_object_tostring_safe failed" );
                goto error;
            }
        }
    }

    ret = vlu;

error:
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
    const char *attrs[] = {
        vlu->attr_cn,
        vlu->attr_vacation,
        vlu->attr_vacation_msg,
        vlu->attr_name,
        NULL
    };

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

    rawmsg = ldap_get_values( vlu->ld, vlu->result, vlu->attr_vacation_msg );

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
    char **res;

    res = ldap_get_values( vlu->ld, vlu->result, vlu->attr_name );
    if ( res == NULL ) {
        res = ldap_explode_dn( ldap_get_dn( vlu->ld, vlu->result ), 1 );
    }
    return res[ 0 ];
}

    void
vlu_close ( struct vlu *vlu ) {

}
