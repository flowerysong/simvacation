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
