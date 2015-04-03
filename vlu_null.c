/*
 * Copyright (c) 2013-2015 Regents of The University of Michigan
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
#include "vlu_null.h"

    struct vlu  *
vlu_init( char *config ) {
    struct vlu *vlu;

    if (( vlu = malloc( sizeof( struct vlu ))) == NULL ) {
        syslog( LOG_ERR, "vlu_init: malloc error: %m" );
        return NULL;
    }
    memset( vlu, 0, sizeof( struct vlu ));

    return vlu;
}

    int
vlu_connect( struct vlu *vlu ) {
    return( 0 );
}

    int
vlu_search( struct vlu *vlu, char *rcpt ) {
    return( VLU_RESULT_OK );
}

    struct name_list *
vlu_aliases( struct vlu *vlu, char *rcpt ) {
    struct name_list *result;

    if (( result = malloc( sizeof( struct name_list ))) == NULL ) {
        syslog( LOG_ALERT, "vlu_aliases: malloc for name_list failed");
        return NULL;
    }

    result->name = rcpt;
    result->next = NULL;

    return result;
}

    char *
vlu_message( struct vlu *vlu, char *rcpt ) {
    return NULL;
}

    char *
vlu_name( struct vlu *vlu, char *rcpt ) {
    return "ezekielh";
}

    char *
vlu_display_name( struct vlu *vlu, char *rcpt ) {
    return "Ezekiel Hendrickson";
}

    void
vlu_close ( struct vlu *vlu ) {
    
}
