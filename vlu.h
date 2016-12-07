#ifndef VLU_H
#define VLU_H

#include <ucl.h>

#define VLU_RESULT_OK       0
#define VLU_RESULT_TEMPFAIL 1
#define VLU_RESULT_PERMFAIL 2

struct vlu          *vlu_init( const ucl_object_t * );
int                 vlu_connect( struct vlu * );
int                 vlu_search( struct vlu *, char * );
char                *vlu_message( struct vlu *, char * );
struct name_list    *vlu_aliases( struct vlu *, char * );
char                *vlu_name( struct vlu *, char * );
char                *vlu_display_name( struct vlu *, char * );
void                vlu_close( struct vlu * );

#endif /* VLU_H */
