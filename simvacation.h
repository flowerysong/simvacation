#ifndef SIMVACATION_H
#define SIMVACATION_H

#include "yasl.h"
#include <config.h>

#define HEADER_UNKNOWN 0
#define HEADER_RECIPIENT 1
#define HEADER_APPEND 2
#define HEADER_NOREPLY 3

#ifndef CONFFILE
#define CONFFILE "/etc/mail/simvacation.conf"
#endif

#ifndef _PATH_SENDMAIL
#define _PATH_SENDMAIL "/usr/sbin/sendmail"
#endif

#define DOMAIN "umich.edu"

/* FIXME: Add Auto: to Subject? */
#define SUBJECTPREFIX "Out of email contact"

/* From tzfile.h */
#define SECSPERMIN 60
#define MINSPERHOUR 60
#define HOURSPERDAY 24
#define DAYSPERWEEK 7
#define DAYSPERNYEAR 365
#define DAYSPERLYEAR 366
#define SECSPERHOUR (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY ((time_t)SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR 12

struct name_list {
    struct name_list *next;
    char *            name;
};

struct headers {
    yastr subject;
    yastr messageid;
    yastr references;
    yastr inreplyto;
};

#endif
