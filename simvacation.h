#ifndef SIMVACATION_H
#define SIMVACATION_H

/*
 * simvacation.h
 *
 * Copyright (c) 2007, 2013 Regents of The University of Michigan.
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

#define HEADER_UNKNOWN      0
#define HEADER_RECIPIENT    1
#define HEADER_SUBJECT      2
#define HEADER_NOREPLY      3

#define VIT     "__VACATION__INTERVAL__TIMER__"

#ifndef VDBDIR
#define VDBDIR  "/var/vacationdb"   /* dir for vacation databases */
#endif

#ifndef _PATH_SENDMAIL
#define _PATH_SENDMAIL  "/usr/sbin/sendmail"
#endif

#define LDAP_HOST       "ldap.itd.umich.edu"
#define BIND_DN         NULL
#define BIND_METHOD     NULL
#define SEARCHBASE      "ou=People,dc=umich,dc=edu"
#define ATTR_ONVAC      "onVacation"
#define ATTR_VACMSG     "vacationMessage"
#define ATTR_CN         "cn"
#define DOMAIN          "umich.edu"
/* FIXME: Add Auto: to Subject? */
#define SUBJECTPREFIX     "Out of email contact"

#endif
