#ifndef BACKEND_VDB_H
#define BACKEND_VDB_H

struct vdb          *vdb_init( char * );
void                vdb_close( struct vdb * );
int                 vdb_recent( struct vdb *, char * );
int                 vdb_store_interval( struct vdb *, time_t );
int                 vdb_store_reply( struct vdb *, char * );
struct name_list    *vdb_get_names( struct vdb * );
void                vdb_clean( struct vdb *, char * );
void                vdb_gc( struct vdb * );

#endif /* BACKEND_VDB_H */
