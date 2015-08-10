/*
 * Copyright (c) 2004, 2013 Regents of The University of Michigan.
 * Copyright (c) 1983, 1987 Regents of the University of California.
 * Copyright (c) 1983 Eric P. Allman.
 * All Rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * * Neither the name of the University nor the names of its contributors
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
#include <unistd.h>

#include "simvacation.h"
#include "vdb.h"
#include "vlu.h"

/*
 *  VACATION -- return a message to the sender when on vacation.
 *
 *	This program is invoked as a message receiver.  It returns a
 *	message specified by the user to whomever sent the mail, taking
 *	care not to return a message too often to prevent "I am on
 *	vacation" loops.
 */

int     check_from();
void    myexit( int );
int     nsearch( char *, char * );
int     pexecv( char *path, char ** );
void    readheaders();
yastr   append_header( yastr, char *, int );
int     check_header( char *, const char * );
int     sendmessage( char *, char * );
void    usage( char * );


/* RFC 2822 2.1.1
 *  There are two limits that this standard places on the number of
 *  characters in a line. Each line of characters MUST be no more than
 *  998 characters, and SHOULD be no more than 78 characters, excluding
 *  the CRLF.
 */
#define	MAXLINE	1000
#define PRETTYLINE 80

struct name_list  *names;
struct headers  *h;
static char    from[MAXLINE];

char            *rcpt;
struct vdb      *vdb;
struct vlu      *vlu;

extern int optind, opterr;
extern char *optarg;

int debug;


    int
main( int argc, char **argv )
{
    debug = 0;
    time_t interval;
    int ch, rc;
    char *vacmsg;
    char *progname;
    char *vlu_config = CONFFILE;

    if ( (progname = strrchr( argv[0], '/' )) == NULL )
	progname = strdup( argv[0] );
    else
	progname = strdup( progname + 1 );

    openlog( progname, LOG_PID, LOG_VACATION );
    opterr = 0;
    interval = -1;

    h = malloc( sizeof( struct headers ));
    memset( h, 0, sizeof( struct headers ));

    from[0] = '\0';

    while (( ch = getopt( argc, argv, "c:df:r:" )) != EOF ) {
	switch( (char) ch ) {
        case 'c':
            vlu_config = optarg;
            break;
        case 'd':
            debug = 1;
            break;
	case 'f':
	    strncpy(from, optarg, MAXLINE - 1);
	    from[MAXLINE - 1] = '\0';
	    break;
	case 'r':
	    if ( isdigit( *optarg )) {
		interval = atol( optarg ) * SECSPERDAY;
		if ( interval < 0 )
		    usage( progname );
	    }
	    else
		interval = LONG_MAX;
	    break;

	    case '?':
	    default:
		usage( progname );
	}
    }
    argc -= optind;
    argv += optind;

    if ( !*argv ) {
	usage( progname );
	myexit( EX_USAGE );
    }

    if ( debug ) {
        openlog( progname, LOG_NOWAIT | LOG_PERROR | LOG_PID, LOG_VACATION );
    }

    rcpt = *argv;

    if (( vlu = vlu_init( vlu_config )) == NULL ) {
        myexit( EX_TEMPFAIL );
    }

    if ( vlu_connect( vlu ) != 0 ) {
        myexit( EX_TEMPFAIL );
    }

    if (( rc = vlu_search( vlu, rcpt )) != VLU_RESULT_OK ) {
        switch ( rc ) {
        case VLU_RESULT_PERMFAIL:
            /* We're done processing this message. */
            myexit( EX_OK );

        case VLU_RESULT_TEMPFAIL:
        default:
            myexit( EX_TEMPFAIL );
        }
        myexit( rc );
    }

    if (( vdb = vdb_init( rcpt )) == NULL ) {
        myexit( EX_OK );
    }

    if (interval != -1) {
	vdb_store_interval( vdb, interval );
    }

    names = vlu_aliases( vlu, rcpt );

    readheaders();

    if ( !vdb_recent( vdb, from )) {
	char rcptstr[MAXLINE];
        if (( vacmsg = vlu_message( vlu, rcpt )) == NULL ) {
            vacmsg = "I am currently out of email contact.\n"
                    "Your mail will be read when I return.";
        }

	vdb_store_reply( vdb, from );
	sprintf(rcptstr, "%s@%s", rcpt, DOMAIN);
	sendmessage( rcptstr, vacmsg );
	syslog( LOG_DEBUG, "mail: sent message for %s to %s from %s",
			rcpt, from, rcptstr );
    } else {
	syslog( LOG_DEBUG, "mail: suppressed message for %s to %s",
		rcpt, from );
    }

    myexit( EX_OK );
}

/*
 * readheaders --
 *	read mail headers
 */
    void
readheaders( )
{
    struct name_list *cur;
    char *p;
    int tome, state, stripfield = 0;
    char buf[MAXLINE];
    yastr *current_hdr = NULL;

    state = HEADER_UNKNOWN;
    tome = 0;
    while ( fgets( buf, sizeof( buf ), stdin ) && *buf != '\n' ) {
        if ( strncasecmp( buf, "From ", 5 ) == 0 ) {
            state = HEADER_UNKNOWN;
            /* FIXME: What is this looking at? */
	    for ( p = buf + 5; *p && *p != ' '; ++p );
            *p = '\0';
            strncpy( from, buf + 5, MAXLINE - 1 );
            from[MAXLINE - 1] = '\0';
            if (( p = index( from, '\n' )))
                *p = '\0';
        }
        if ( check_header( buf, "Message-ID:" ) == 0 ) {
            state = HEADER_APPEND;
            stripfield = 1;
            current_hdr = &h->messageid;
        }
        else if ( check_header( buf, "References:" ) == 0 ) {
            state = HEADER_APPEND;
            stripfield = 1;
            current_hdr = &h->references;
        }
        else if ( check_header( buf, "In-Reply-To:" ) == 0 ) {
            state = HEADER_APPEND;
            stripfield = 1;
            current_hdr = &h->inreplyto;
        }
        /* RFC 3834 2
         *  Automatic responses SHOULD NOT be issued in response to any
         *  message which contains an Auto-Submitted header field (see below),
         *  where that field has any value other than "no"
         */
        else if ( check_header( buf, "Auto-Submitted:" ) == 0 ) {
            state = HEADER_NOREPLY;
            p = buf + 15;
            while ( *++p && isspace( *p ));
            if (!*p) {
                break;
            }
            if ( strncasecmp( p, "no", 2 ) != 0 ) {
                syslog( LOG_DEBUG, "readheaders: suppressing message: %s", buf );
                myexit( EX_OK );
            }
        }
        /* RFC 3834 2
         *  A responder MAY refuse to send a response to a subject message
         *  which contains any header or content which makes it appear to the
         *  responder that a response would not be appropriate.
         */
        else if ( check_header( buf, "List-" ) == 0 ) {
            /* RFC 3834 2
             *  For similar reasons, a responder MAY ignore any subject message
             *  with a List-* field [RFC2369].
             */
            state = HEADER_NOREPLY;
            syslog( LOG_DEBUG, "readheaders: suppressing message: %s", buf );
            myexit( EX_OK );
        }
        else if ( check_header( buf, "Precedence" ) == 0 ) {
            /* RFC 3834 2
             *  For instance, if the subject message contained a
             *  Precedence header field [RFC2076] with a value of
             *  "list" the responder might guess that the traffic had
             *  arrived from a mailing list, and would not respond if
             *  the response were only intended for personal messages.
             *  [...]
             *  (Because Precedence is not a standard header field, and
             *  its use and interpretation vary widely in the wild,
             *  no particular responder behavior in the presence of
             *  Precedence is recommended by this specification.
             */
            state = HEADER_NOREPLY;
	    if (( buf[10] == ':' || buf[10] == ' ' || buf[10] == '\t') &&
                    ( p = index( buf, ':' ))) {
                while ( *++p && isspace( *p ));
                if ( !*p ) {
                    break;
                }
                if ( strncasecmp( p, "junk", 4 ) == 0 ||
                     strncasecmp( p, "bulk", 4 ) == 0 ||
                     strncasecmp( p, "list", 4 ) == 0 ) {
                    syslog( LOG_DEBUG, "readheaders: suppressing message: precedence %s", p );
                    myexit( EX_OK );
                }
            }
        }
        else if ( check_header( buf, "X-Auto-Response-Suppress:" ) == 0 ) {
            /* MS-OXCMAIL 2.1.3.2.20
             *
             * X-Auto-Response-Suppress value   | Meaning
             * ----------------------------------------------------------------
             * None                             |
             * ----------------------------------------------------------------
             * All                              |
             * ----------------------------------------------------------------
             * DR                               | Suppress delivery reports
             *                                  | from transport.
             * ----------------------------------------------------------------
             * NDR                              | Suppress non-delivery reports
             *                                  | from transport.
             * ----------------------------------------------------------------
             * RN                               | Suppress read notifications
             *                                  | from receiving client.
             * ----------------------------------------------------------------
             * NRN                              | Suppress non-read
             *                                  | notifications from receiving
             *                                  | client.
             * ----------------------------------------------------------------
             * OOF                              | Suppress Out of Office (OOF)
             *                                  | notifications.
             * ----------------------------------------------------------------
             * AutoReply                        | Suppress auto-reply messages
             *                                  | other than OOF notifications.
             * ----------------------------------------------------------------
             *
             * The order of these values in the header is not important.
             */
            state = HEADER_NOREPLY;
            if ( nsearch( "OOF", buf ) || nsearch( "All", buf )) {
                syslog( LOG_DEBUG, "readheaders: suppressing message: %s", buf );
                myexit( EX_OK );
            }
        }
        /* RFC 3834 2
         *  Personal and Group responses whose purpose is to notify the sender
         *  of a message of a temporary absence of the recipient (e.g.,
         *  "vacation" and "out of the office" notices) SHOULD NOT be issued
         *  unless a valid address for the recipient is explicitly included in
         *  a recipient (e.g., To, Cc, Bcc, Resent-To, Resent-Cc, or Resent-
         *  Bcc) field of the subject message.
         */
        else if ( check_header( buf, "Cc:" ) == 0 ) {
            state = HEADER_RECIPIENT;
        }
        else if ( check_header( buf, "To:" ) == 0 ) {
            state = HEADER_RECIPIENT;
        }
        else if ( check_header( buf, "Subject:" ) == 0 ) {
            state = HEADER_APPEND;
            stripfield = 1;
            current_hdr = &h->subject;
	}
        else if ( !isspace( *buf )) {
            /* Not a continuation of the previous header line, reset flag
             *
             * RFC 2822 2.2.3
             *  Unfolding is accomplished by simply removing any CRLF
             *  that is immediately followed by WSP.
             */
            state = HEADER_UNKNOWN;
        }

        switch ( state ) {
        case HEADER_RECIPIENT:
            if ( tome ) {
                break;
            }
            for ( cur = names; !tome && cur; cur = cur->next ) {
		tome += nsearch( cur->name, buf );
	    }
            break;
        case HEADER_APPEND:
            *current_hdr = append_header( *current_hdr, buf, stripfield );
            stripfield = 0;

            break;
	}
    }

    if ( !*from ) {
        syslog( LOG_NOTICE, "readheaders: skipping message: unknown sender" );
        myexit( EX_OK );
    }

    syslog( LOG_DEBUG, "readheaders: mail from %s", from );

    if ( !tome ) {
	syslog( LOG_NOTICE, "readheaders: suppressing message: mail does not appear to be to user %s", vlu_name( vlu, rcpt ));
	myexit( EX_OK );
    }

    if ( check_from()) {
        syslog( LOG_NOTICE, "readheaders: suppressing message: bad sender %s", from );
        myexit( EX_OK );
    }

}

    int
check_header( char *line, const char *field ) {
    return strncasecmp( field, line, strlen( field ));
}

    yastr
append_header( yastr str, char *value, int stripfield )
{
    if ( stripfield ) {
        value = index( value, ':' );
        while ( value && *++value && isspace( *value ));
    }

    if ( str == NULL ) {
        str = yaslempty();
    }

    yastr s = yaslcat( str, value );
    yasltrim( s, "\n" );
    return s;
}

/*
 * nsearch --
 *	do a nice, slow, search of a string for a substring.
 *	(umich) - change any of {'.', '_'} to ' '.
 */
    int
nsearch( char *name, char *str )
{
    int len;
    char *c;

    /*
     * convert any periods or underscores to spaces
     */
    for ( c = str; *c; c++ ) {
	if (( *c == '.' ) || ( *c == '_' )) {
	    *c = ' ';
	}
    }

    for ( len = strlen( name ); *str; ++str ) {
	if ( !strncasecmp( name, str, len )) {
	    return( 1 );
	}
    }
    return( 0 );
}

/* check_from
 *
 * RFC 3834 2
 *  Responders MUST NOT generate any response for which the
 *  destination of that response would be a null address (e.g., an
 *  address for which SMTP MAIL FROM or Return-Path is <>), since the
 *  response would not be delivered to a useful destination.
 *  Responders MAY refuse to generate responses for addresses commonly
 *  used as return addresses by responders - e.g., those with local-
 *  parts matching "owner-*", "*-request", "MAILER-DAEMON", etc.
 */
    int
check_from()
{
    static struct ignore {
		char	*name;
		int	len;
    } ignore[] = {
		{ "-request", 8 },
                { "postmaster", 10 },
                { "uucp", 4 },
		{ "mailer-daemon", 13 },
                { "mailer", 6 },
                { "-relay", 6 },
		{ "<>", 2 },
		{ NULL, 0 },
    };
    struct ignore *cur;
    int len;
    char *p;
    char *at;
    char *sep;
    char buf[ MAXLINE ];

    /* Canonicalize SRS addresses. We don't need to verify hashes and timestamps
     * because this doesn't increase our risk of replying to a forged address.
     */
    if (( strncasecmp( from, "SRS", 3 ) == 0 ) && ( strlen( from ) > 13 ) &&
        (( from[ 3 ] == '0' ) || ( from[ 3 ] == '1' )) &&
        (( from[ 4 ] == '=' ) || ( from[ 4 ] == '-' ) ||
        ( from[ 4 ] == '+' ))) {
        if ( from[ 3 ] == '1' ) {
            p = strstr( from, "==" ) + 2;
        } else {
            p = from + 5;
        }

        /* Skip the hash and timestamp */
        if (( p != NULL ) &&
                (( p = strchr( p, '=' )) != NULL ) &&
                (( p = strchr( p + 1, '=' )) != NULL )) {
            p++;
        }

        /* p is now domain.tld=localpart@forwarder.tld or NULL
         * or the address is borked */
        if (( p != NULL ) && (( sep = strchr( p, '=' )) != NULL )) {
            if (( at = strrchr( sep, '@' )) != NULL ) {
                *at = '\0';
            }
            *sep = '\0';
            sprintf( buf, "%s@%s", sep + 1, p );
            syslog( LOG_NOTICE, "check_from: corrected for SRS: %s", buf );
            strncpy( from, buf, MAXLINE - 1 );
        }
    }

    if (( p = strrchr( from, '@' )) == NULL ) {
        for ( p = from ; *p; ++p );
    }

    len = p - from;
    for ( cur = ignore; cur->name; ++cur ) {
	if ( len >= cur->len &&
		strncasecmp( cur->name, p - cur->len, cur->len == 0 )) {
	    return( 1 );
        }
    }

    return( 0 );
}

/*
 * sendmessage --
 *	exec sendmail to send the vacation file to sender
 */
    int
sendmessage( char *myname, char *vmsg )
{
    char	*nargv[5];

#ifdef HAVE_SENDMAIL
    nargv[0] = "sendmail";
    nargv[1] = "-f<>";
    nargv[2] = from;
    nargv[3] = NULL;
#else
    nargv[0] = "simsendmail";   /* really simsendmail */
    nargv[1] = "-f";
    nargv[2] = "";
    nargv[3] = from;
    nargv[4] = NULL;
#endif

    if (( pexecv( _PATH_SENDMAIL, nargv )) == -1 ) {
	syslog( LOG_ERR, "mail: pexecv of %s failed", _PATH_SENDMAIL );
	return( EX_TEMPFAIL );
    }
    /*
     * Our stdout should now be hooked up to sendmail's stdin.
     * Generate headers and print the vacation message.
     */

    /* RFC 3834 3.1.1
     *  For responses sent by Personal Responders, the From field SHOULD
     *  contain the name of the recipient of the subject message (i.e.,
     *  the user on whose behalf the response is being sent) and an
     *  address chosen by the recipient of the subject message to be
     *  recognizable to correspondents.  Often this will be the same
     *  address that was used to send the subject message to that
     *  recipient.
     */
    printf( "From: %s <%s>\n", vlu_display_name( vlu, rcpt ), myname );

    /* RFC 3834 3.1.3
     *  The To header field SHOULD indicate the recipient of the response.
     *  In general there SHOULD only be one recipient of any automatic
     *  response.  This minimizes the potential for sorcerer's apprentice
     *  mode and denial-of-service attacks.
     */
    printf( "To: %s\n", from );

    /* RFC 3834 3.1.5
     *  The Subject field SHOULD contain a brief indication that the message
     *  is an automatic response, followed by contents of the Subject field
     *  (or a portion thereof) from the subject message.  The prefix "Auto:"
     *  MAY be used as such an indication.  If used, this prefix SHOULD be
     *  followed by an ASCII SPACE character (0x20).
     */
    printf( "Subject: %s", SUBJECTPREFIX );
    if ( yasllen( h->subject ) > 0 ) {
	if ( check_header( h->subject, "Re:" ) != 0 ) {
	    printf( " (Re: %s)", h->subject );
	} else {
	    printf( " (%s)", h->subject );
	}
    }
    printf( "\n" );

    /* RFC 3834 3.1.6
     *  The In-Reply-To and References fields SHOULD be provided in the
     *  header of a response message if there was a Message-ID field in the
     *  subject message, according to the rules in [RFC2822] section
     *  3.6.4.
     */
    if ( yasllen( h->messageid ) > 0 ) {
        /* RFC 2822 3.6.4
         *  The "In-Reply-To:" field will contain the contents of the
         *  "Message-ID:" field of the message to which this one is a reply
         *  (the "parent message"). [...] If there is no "Message-ID:" field
         *  in any of the parent messages, then the new message will have no
         *  "In-Reply-To:" field.
         */
        printf( "In-Reply-To: %s\n", h->messageid );
    }

    /* RFC 2822 3.6.4
     *  The "References:" field will contain the contents of the 
     *  parent's "References:" field (if any) followed by the contents
     *  of the parent's "Message-ID:" field (if any). If the parent
     *  message does not contain a "References:" field but does have an
     *  "In-Reply-To:" field containing a single message identifier, then
     *  the "References:" field will contain the contents of the parent's
     *  "In-Reply-To:" field followed by the contents of the parent's
     *  "Message-ID:" field (if any).
     */
    if ( yasllen( h->references ) > 0 ) {
        printf( "References: %s %s\n", h->references, h->messageid );
    }
    else if ( yasllen( h->inreplyto ) > 0 ) {
        printf( "References: %s %s\n", h->inreplyto, h->messageid );
    }
    else if ( yasllen( h->messageid ) > 0 ) {
        printf( "References: %s\n", h->messageid );
    }

    /* RFC 3834 3.1.7
     *  The Auto-Submitted field, with a value of "auto-replied", SHOULD be
     *  included in the message header of any automatic response.
     */
    printf( "Auto-Submitted: auto-replied\n" );

    /* End of headers */
    printf( "\n" );

    printf( "%s\n", vmsg );

    return( 0 );
}

    void
usage( char * progname)
{
    /* FIXME: wth? */
    syslog(LOG_NOTICE, "uid %u: usage: %s login\n", getuid(), progname);
    myexit( 0 );
}


    void
myexit( int retval )
{
    vdb_close( vdb );
    vlu_close( vlu );

    exit( retval );
}


/*
 * Interface to pipe and exec, for starting children in pipelines.
 *
 * Manipulates file descriptors 0, 1, and 2, such that the new child
 * is reading from the parent's output.
 */
    int
pexecv( char *path, char **argv )
{
    int		fd[ 2 ], c;

    if ( pipe( fd ) < 0 ) {
	return( -1 );
    }

    switch ( c = vfork()) {
    case -1 :
	return( -1 );
	/* NOTREACHED */

    case 0 :
	if ( close( fd[ 1 ] ) < 0 ) {
	    return( -1 );
	}
	if ( dup2( fd[ 0 ], 0 ) < 0 ) {
	    return( -1 );
	}
	if ( close( fd[ 0 ] ) < 0 ) {
	    return( -1 );
	}
	execv( path, argv );
	return( -1 );
	/* NOTREACHED */

    default :
	if ( close( fd[ 0 ] ) < 0 ) {
	    return( -1 );
	}
	if ( dup2( fd[ 1 ], 1 ) < 0 ) {
	    return( -1 );
	}
	if ( close( fd[ 1 ] ) < 0 ) {
	    return( -1 );
	}
	return( c );
    }
}
