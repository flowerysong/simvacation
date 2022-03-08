/*
 * Copyright (c) Regents of The University of Michigan
 * See COPYING.
 */

#include <config.h>

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
    functable->group_search = vlu_group_search;
    functable->message = vlu_message;
    functable->subject_prefix = vlu_subject_prefix;
    functable->aliases = vlu_aliases;
    functable->name = vlu_name;
    functable->display_name = vlu_display_name;
    functable->close = vlu_close;

    if (strcasecmp(provider, "ldap") == 0) {
#ifdef HAVE_LDAP
        functable->init = ldap_vlu_init;
        functable->search = ldap_vlu_search;
        functable->group_search = ldap_vlu_group_search;
        functable->message = ldap_vlu_message;
        functable->subject_prefix = ldap_vlu_subject_prefix;
        functable->aliases = ldap_vlu_aliases;
        functable->name = ldap_vlu_name;
        functable->display_name = ldap_vlu_display_name;
        return functable;
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
vlu_init() {
    VLU *vlu;

    if ((vlu = calloc(1, sizeof(VLU))) == NULL) {
        syslog(LOG_ERR, "vlu_init: calloc error: %m");
        return NULL;
    }

    return vlu;
}

vac_result
vlu_search(VLU *vlu, const yastr rcpt) {
    return VAC_RESULT_OK;
}

vac_result
vlu_group_search(VLU *vlu, const yastr rcpt) {
    return VAC_RESULT_OK;
}

ucl_object_t *
vlu_aliases(VLU *vlu, const yastr rcpt) {
    return ucl_object_fromstring(rcpt);
}

yastr
vlu_message(VLU *vlu, const yastr rcpt) {
    return NULL;
}

yastr
vlu_subject_prefix(VLU *vlu, const yastr rcpt) {
    return yaslauto(ucl_object_tostring(
            ucl_object_lookup_path(vac_config, "core.subject_prefix")));
}

yastr
vlu_name(VLU *vlu, const yastr rcpt) {
    return yaslauto(rcpt);
}

yastr
vlu_display_name(VLU *vlu, const yastr rcpt) {
    return yaslauto(rcpt);
}

void
vlu_close(VLU *vlu) {}
