#ifndef BACKEND_LMDB_H
#define BACKEND_LMDB_H

/*
 * backend_lmdb.h
 *
 * Copyright (c) 2015 Regents of The University of Michigan.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation, and that the name of The University
 * of Michigan not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. This software is supplied as is without expressed or
 * implied warranties of any kind.
 *
 */

#include <lmdb.h>

#ifndef VDBDIR
#define VDBDIR  "/var/lib/vacation"   /* dir for vacation database */
#endif

struct vdb {
    MDB_env *dbenv;
    char    *rcpt;
    time_t  interval;
};

#include "vdb.h"

#endif /* BACKEND_LMDB_H */
