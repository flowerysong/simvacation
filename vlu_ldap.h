#ifndef VLU_LDAP_H
#define VLU_LDAP_H

/* FIXME */
#define LDAP_DEPRECATED         1

#include <lber.h>
#include <ldap.h>

#include "yasl.h"

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
