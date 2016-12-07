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
#define ATTR_NAME       "displayName"

struct vlu {
    LDAP            *ld;
    LDAPMessage     *result;
    const char      *ldap_host;
    int64_t         ldap_port;
    struct timeval  ldap_timeout;
    const char      *attr_vacation;
    const char      *attr_vacation_msg;
    const char      *attr_cn;
    const char      *attr_name;
};

#include "vlu.h"

#endif /* VLU_LDAP_H */
