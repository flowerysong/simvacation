/*
 * Copyright (c) 2004-2015 Regents of The University of Michigan.
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

/*
 * Return parsed argc/argv.
 */

#include "config.h"

#include <stdlib.h>
#include <strings.h>

#include "argcargv.h"

#define ACV_ARGC		10
#define ACV_WHITE		0
#define ACV_WORD		1
#define ACV_BRACKET		2
#define ACV_DQUOTE		3

static ACAV *acavg = NULL;

    ACAV*
acav_alloc( void )
{
    ACAV *acav;

    if (( acav = (ACAV*)malloc( sizeof( ACAV ))) == NULL ) {
	return( NULL );
    }
    if (( acav->acv_argv =
	    (char **)malloc( sizeof(char *) * ( ACV_ARGC ))) == NULL ) {
	return( NULL );
    }
    acav->acv_argc = ACV_ARGC;

    return( acav );
}

/*
 * acav->acv_argv = **argv[] if passed an ACAV
 */

    int
acav_parse( ACAV *acav, char *line, char **argv[] )
{
    int		ac = 0;
    int		state = ACV_WHITE;

    if ( acav == NULL ) {
	if ( acavg == NULL ) {
	    if (( acavg = acav_alloc() ) == NULL ) {
                return ( -1 );
            };
	}
	acav = acavg;
    }

    for ( ; *line != '\0'; line++ ) {
	switch ( *line ) {
	case ' ' :
	case '\t' :
	case '\n' :
	    if ( state == ACV_WORD ) {
		*line = '\0';
		state = ACV_WHITE;
	    }
	    break;
	default :
	    if ( state == ACV_WHITE ) {
		acav->acv_argv[ ac++ ] = line;
		if ( ac >= acav->acv_argc ) {
		    /* realloc */
		    if (( acav->acv_argv = (char **)realloc( acav->acv_argv,
			    sizeof( char * ) * ( acav->acv_argc + ACV_ARGC )))
			    == NULL ) {
			return( -1 );
		    }
		    acav->acv_argc += ACV_ARGC;
		}
		state = ACV_WORD;
	    }
	}
    }

    acav->acv_argv[ ac ] = NULL; 
    *argv = acav->acv_argv;
    return( ac );
}

    int
acav_free( ACAV *acav )
{
    if ( acav ) {
	free( acav->acv_argv );
	free( acav );
    }

    return( 0 );
}
