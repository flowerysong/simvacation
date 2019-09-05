#ifndef VLU_H
#define VLU_H

#include <ucl.h>

#ifdef HAVE_LDAP
#include <lber.h>
#include <ldap.h>
#endif /* HAVE_LDAP */

typedef enum {
    VLU_RESULT_OK,
    VLU_RESULT_TEMPFAIL,
    VLU_RESULT_PERMFAIL,
} vlu_result;

#ifdef HAVE_LDAP
struct vlu_ldap {
    LDAP *         ld;
    LDAPMessage *  result;
    struct timeval timeout;
    const char *   attr_vacation;
    const char *   attr_vacation_msg;
    const char *   attr_name;
    const char *   search_base;
    char **        attrs;
};
#endif /* HAVE_LDAP */

typedef union vlu {
    int null;
#ifdef HAVE_LDAP
    struct vlu_ldap *ldap;
#endif /* HAVE_LDAP */
} VLU;

struct vlu_backend {
    VLU *(*init)(const ucl_object_t *);
    vlu_result (*search)(VLU *, char *);
    yastr (*message)(VLU *, char *);
    struct name_list *(*aliases)(VLU *, char *);
    yastr (*name)(VLU *, char *);
    yastr (*display_name)(VLU *, char *);
    void (*close)(VLU *);
};

struct vlu_backend *vlu_backend(const char *);
VLU *               vlu_init(const ucl_object_t *);
vlu_result          vlu_search(VLU *, char *);
yastr               vlu_message(VLU *, char *);
struct name_list *  vlu_aliases(VLU *, char *);
yastr               vlu_name(VLU *, char *);
yastr               vlu_display_name(VLU *, char *);
void                vlu_close(VLU *);

#ifdef HAVE_LDAP
VLU *             ldap_vlu_init(const ucl_object_t *);
vlu_result        ldap_vlu_search(VLU *, char *);
yastr             ldap_vlu_message(VLU *, char *);
struct name_list *ldap_vlu_aliases(VLU *, char *);
yastr             ldap_vlu_name(VLU *, char *);
yastr             ldap_vlu_display_name(VLU *, char *);
void              ldap_vlu_close(VLU *);
#endif /* HAVE_LDAP */

#endif /* VLU_H */
