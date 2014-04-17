#ifndef VLU_LDAP_H
#define VLU_LDAP_H

/*
 * vlu.h
 *
 * Copyright (c) 2014 Regents of The University of Michigan.
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

/* FIXME */
#define LDAP_DEPRECATED         1

#include <lber.h>
#include <ldap.h>

#include "sds/sds.h"

#define LDAP_HOST       "ldap.itd.umich.edu"
#define BIND_DN         NULL
#define BIND_METHOD     NULL
#define SEARCHBASE      "ou=People,dc=umich,dc=edu"
#define ATTR_ONVAC      "onVacation"
#define ATTR_VACMSG     "vacationMessage"
#define ATTR_CN         "cn"

struct vlu {
    LDAP            *ld;
    LDAPMessage     *result;
    char            *ldap_host;
    int             ldap_port;
    struct timeval  ldap_timeout;
    char            *attr_vacation;
    char            *attr_vacation_msg;
    char            *attr_cn;
};

#include "vlu.h"

#endif /* VLU_LDAP_H */
