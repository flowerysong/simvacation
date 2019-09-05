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

struct vlu_backend *
vlu_backend(const char *provider) {
    struct vlu_backend *functable;

    functable = malloc(sizeof(struct vlu_backend));

    functable->init = vlu_init;
    functable->search = vlu_search;
    functable->message = vlu_message;
    functable->aliases = vlu_aliases;
    functable->name = vlu_name;
    functable->display_name = vlu_display_name;
    functable->close = vlu_close;

    if (strcasecmp(provider, "ldap") == 0) {
#ifdef HAVE_LDAP
        functable->init = ldap_vlu_init;
        functable->search = ldap_vlu_search;
        functable->message = ldap_vlu_message;
        functable->aliases = ldap_vlu_aliases;
        functable->name = ldap_vlu_name;
        functable->display_name = ldap_vlu_display_name;
#else  /* HAVE_LDAP */
        syslog(LOG_ERR, "vlu_backend: LDAP was disabled during compilation");
        return NULL;
#endif /* HAVE_LDAP */
    }

    if (strcasecmp(provider, "null") == 0) {
        return functable;
    }

    syslog(LOG_ERR, "vlu_backend: unknown backend %s", provider);
    return NULL;
}

VLU *
vlu_init(const ucl_object_t *config) {
    VLU *vlu;

    if ((vlu = calloc(1, sizeof(VLU))) == NULL) {
        syslog(LOG_ERR, "vlu_init: calloc error: %m");
        return NULL;
    }

    return vlu;
}

vlu_result
vlu_search(VLU *vlu, char *rcpt) {
    return VLU_RESULT_OK;
}

struct name_list *
vlu_aliases(VLU *vlu, char *rcpt) {
    struct name_list *result;

    if ((result = malloc(sizeof(struct name_list))) == NULL) {
        syslog(LOG_ALERT, "vlu_aliases: malloc for name_list failed");
        return NULL;
    }

    result->name = yaslauto(rcpt);
    result->next = NULL;

    return result;
}

yastr
vlu_message(VLU *vlu, char *rcpt) {
    return NULL;
}

yastr
vlu_name(VLU *vlu, char *rcpt) {
    return yaslauto(rcpt);
}

yastr
vlu_display_name(VLU *vlu, char *rcpt) {
    return yaslauto(rcpt);
}

void
vlu_close(VLU *vlu) {}
