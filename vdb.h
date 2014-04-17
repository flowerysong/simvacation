#ifndef BACKEND_VDB_H
#define BACKEND_VDB_H

/*
 * backend_vdb.h
 *
 * Copyright (c) 2014 Regents of The University of Michigan.
 * All Rights Reserved.
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

int                 vdb_init( struct vdb *, char * );
void                vdb_close( struct vdb * );
int                 vdb_recent( struct vdb *, char * );
int                 vdb_store_interval( struct vdb *, time_t );
int                 vdb_store_reply( struct vdb *, char * );
struct name_list    *vdb_get_names( struct vdb * );
void                vdb_clean( struct vdb *, char * );
#endif /* BACKEND_VDB_H */
