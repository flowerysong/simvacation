/*
 * Copyright (c) Regents of The University of Michigan
 * See COPYING.
 */


#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sysexits.h>
#include <syslog.h>
#include <time.h>

#include "simvacation.h"
#include "vlu.h"

VLU *
ldap_vlu_init(const ucl_object_t *config) {
    VLU *         vlu = NULL;
    const char *  uri = NULL;
    struct berval credentials = {0};
    int           rc;

    if ((vlu = malloc(sizeof(VLU))) == NULL) {
        syslog(LOG_ERR, "vlu_init: malloc error: %m");
        return NULL;
    }
    memset(vlu, 0, sizeof(VLU));

    if (!ucl_object_tostring_safe(
                ucl_object_lookup_path(config, "ldap.search_base"),
                &(vlu->ldap->search_base))) {
        syslog(LOG_ERR, "vlu_init: ucl_object_tostring_safe failed");
        return NULL;
    }


    if (!ucl_object_tostring_safe(
                ucl_object_lookup_path(config, "ldap.attributes.vacation"),
                &(vlu->ldap->attr_vacation))) {
        syslog(LOG_ERR, "vlu_init: ucl_object_tostring_safe failed");
        return NULL;
    }

    if (!ucl_object_tostring_safe(ucl_object_lookup_path(config,
                                          "ldap.attributes.vacation_message"),
                &(vlu->ldap->attr_vacation_msg))) {
        syslog(LOG_ERR, "vlu_init: ucl_object_tostring_safe failed");
        return NULL;
    }

    if (!ucl_object_tostring_safe(
                ucl_object_lookup_path(config, "ldap.attributes.name"),
                &(vlu->ldap->attr_name))) {
        syslog(LOG_ERR, "vlu_init: ucl_object_tostring_safe failed");
        return NULL;
    }

    vlu->ldap->attrs = calloc(4, sizeof(char *));
    vlu->ldap->attrs[ 0 ] = strdup("cn");
    vlu->ldap->attrs[ 1 ] = strdup(vlu->ldap->attr_vacation);
    vlu->ldap->attrs[ 2 ] = strdup(vlu->ldap->attr_vacation_msg);
    vlu->ldap->attrs[ 3 ] = strdup(vlu->ldap->attr_name);

    vlu->ldap->timeout.tv_sec = 30;

    if (!ucl_object_tostring_safe(
                ucl_object_lookup_path(config, "ldap.uri"), &uri)) {
        syslog(LOG_ERR, "vlu_init: ucl_object_tostring_safe failed");
        return NULL;
    }

    if (ldap_initialize(&(vlu->ldap->ld), uri) != LDAP_SUCCESS) {
        syslog(LOG_INFO, "ldap: ldap_initialize failed");
        return NULL;
    }

    if ((rc = ldap_sasl_bind_s(vlu->ldap->ld, NULL, LDAP_SASL_SIMPLE,
                 &credentials, NULL, NULL, NULL)) != LDAP_SUCCESS) {
        syslog(LOG_ALERT, "vlu_connect: ldap_sasl_bind_s failed: %s",
                ldap_err2string(rc));
        return NULL;
    }

    return vlu;
}

vlu_result
ldap_vlu_search(VLU *vlu, char *rcpt) {
    char            filter[ 64 ];
    int             rc, retval, matches;
    LDAPMessage *   result;
    struct berval **vacstatus;

    sprintf(filter, "uid=%s", rcpt);
    rc = ldap_search_ext_s(vlu->ldap->ld, vlu->ldap->search_base,
            LDAP_SCOPE_SUBTREE, filter, vlu->ldap->attrs, 0, NULL, NULL,
            &vlu->ldap->timeout, 0, &result);

    switch (rc) {
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
    if (rc != LDAP_SUCCESS) {
        syslog(LOG_ALERT, "vlu_search: ldap_search failed: %s",
                ldap_err2string(rc));
        return retval;
    }

    matches = ldap_count_entries(vlu->ldap->ld, result);

    if (matches > 1) {
        syslog(LOG_ALERT, "vlu_search: multiple matches for %s", rcpt);
        return VLU_RESULT_PERMFAIL;
    } else if (matches == 0) {
        syslog(LOG_ALERT, "vlu_search: no match for %s", rcpt);
        return VLU_RESULT_PERMFAIL;
    }

    vlu->ldap->result = ldap_first_entry(vlu->ldap->ld, result);

    vacstatus = ldap_get_values_len(
            vlu->ldap->ld, vlu->ldap->result, vlu->ldap->attr_vacation);

    if (vacstatus && (vacstatus[ 0 ]->bv_len == 4) &&
            (strncasecmp(vacstatus[ 0 ]->bv_val, "TRUE", 4) == 0)) {
        syslog(LOG_DEBUG, "vlu_search: user %s on vacation", rcpt);
    } else {
        /* User is no longer on vacation */
        syslog(LOG_INFO, "vlu_search: user %s is not on vacation", rcpt);
        return VLU_RESULT_PERMFAIL;
    }

    return VLU_RESULT_OK;
}

struct name_list *
ldap_vlu_aliases(VLU *vlu, char *rcpt) {
    struct berval **  cnames;
    int               i;
    struct name_list *result;
    struct name_list *cur;

    cnames = ldap_get_values_len(vlu->ldap->ld, vlu->ldap->result, "cn");

    if ((cur = malloc(sizeof(struct name_list))) == NULL) {
        syslog(LOG_ALERT, "vlu_aliases: malloc for name_list failed");
        return NULL;
    }

    cur->name = rcpt;
    cur->next = NULL;
    result = cur;

    for (i = 0; cnames && cnames[ i ] != NULL; i++) {
        if ((cur->next = malloc(sizeof(struct name_list))) == NULL) {
            syslog(LOG_ALERT, "vlu_aliases: malloc for name_list failed");
            return NULL;
        }
        cur = cur->next;
        cur->name = strndup(cnames[ i ]->bv_val, cnames[ i ]->bv_len);
        cur->next = NULL;
    }

    return result;
}

yastr
ldap_vlu_message(VLU *vlu, char *rcpt) {
    struct berval **rawmsg;
    int             i;
    yastr           vacmsg;

    if ((rawmsg = ldap_get_values_len(vlu->ldap->ld, vlu->ldap->result,
                 vlu->ldap->attr_vacation_msg)) == NULL) {
        return NULL;
    }

    vacmsg = yaslempty();

    for (i = 0; rawmsg[ i ] != NULL; i++) {
        vacmsg = yaslcatlen(vacmsg, rawmsg[ i ]->bv_val, rawmsg[ i ]->bv_len);
    }

    yaslmapchars(vacmsg, "$", "\n", 1);

    return vacmsg;
}

yastr
ldap_vlu_name(VLU *vlu, char *rcpt) {
    return yaslauto(ldap_get_dn(vlu->ldap->ld, vlu->ldap->result));
}

yastr
ldap_vlu_display_name(VLU *vlu, char *rcpt) {
    struct berval **name;
    LDAPDN          dn = NULL;

    if ((name = ldap_get_values_len(vlu->ldap->ld, vlu->ldap->result,
                 vlu->ldap->attr_name)) == NULL) {
        ldap_str2dn(ldap_get_dn(vlu->ldap->ld, vlu->ldap->result), &dn,
                LDAP_DN_FORMAT_LDAPV3);
        return (yaslnew(
                dn[ 0 ][ 0 ]->la_value.bv_val, dn[ 0 ][ 0 ]->la_value.bv_len));
    }
    return yaslnew(name[ 0 ]->bv_val, name[ 0 ]->bv_len);
}
