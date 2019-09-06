#ifndef VUTIL_H
#define VUTIL_H

#include "simvacation.h"

vac_result read_vacation_config(const char *);
vac_result pexecv(yastr *);
yastr      canon_from(const yastr);
yastr      check_from(const yastr);
bool       is_substring(const char *, const char *, bool);

#endif /* VUTIL_H */
