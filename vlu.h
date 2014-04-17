#ifndef VLU_H
#define VLU_H

/*
 * vlu.h
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

#define VLU_RESULT_OK       0
#define VLU_RESULT_TEMPFAIL 1
#define VLU_RESULT_PERMFAIL 2

struct vlu          *vlu_init( char * );
int                 vlu_connect( struct vlu * );
int                 vlu_search( struct vlu *, char * );
char                *vlu_message( struct vlu *, char * );
struct name_list    *vlu_aliases( struct vlu *, char * );
char                *vlu_name( struct vlu *, char * );
char                *vlu_display_name( struct vlu *, char * );
void                vlu_close( struct vlu * );

#endif /* VLU_H */
