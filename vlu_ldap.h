#ifndef VLU_LDAP_H
#define VLU_LDAP_H

#include <lber.h>
#include <ldap.h>

#include "yasl.h"

#define LDAP_HOST "ldap.itd.umich.edu"
#define BIND_DN NULL
#define BIND_METHOD NULL
#define SEARCHBASE "ou=People,dc=umich,dc=edu"
#define ATTR_ONVAC "onVacation"
#define ATTR_VACMSG "vacationMessage"
#define ATTR_CN "cn"
#define ATTR_NAME "displayName"

struct vlu {
    LDAP *         ld;
    LDAPMessage *  result;
    const char *   ldap_uri;
    struct timeval ldap_timeout;
    char *         attr_vacation;
    char *         attr_vacation_msg;
    char *         attr_cn;
    char *         attr_name;
};

#include "vlu.h"

#endif /* VLU_LDAP_H */
