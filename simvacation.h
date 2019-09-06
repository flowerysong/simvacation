#ifndef SIMVACATION_H
#define SIMVACATION_H

#include <stdbool.h>

#include <ucl.h>

#include "config.h"
#include "yasl.h"

#define HEADER_UNKNOWN 0
#define HEADER_RECIPIENT 1
#define HEADER_APPEND 2
#define HEADER_NOREPLY 3

/* FIXME: move to config file */
/* FIXME: Add Auto: to Subject? */
#define SUBJECTPREFIX "Out of email contact"

typedef enum {
    VAC_RESULT_OK,
    VAC_RESULT_TEMPFAIL,
    VAC_RESULT_PERMFAIL,
} vac_result;

struct headers {
    yastr subject;
    yastr messageid;
    yastr references;
    yastr inreplyto;
    bool  rcpt_match;
};

extern ucl_object_t *vac_config;
#endif
