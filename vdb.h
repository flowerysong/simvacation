#ifndef BACKEND_VDB_H
#define BACKEND_VDB_H

#include <ucl.h>

#ifdef HAVE_LMDB
#include <lmdb.h>
#endif

#ifdef HAVE_URCL
#include <urcl.h>
#endif /* HAVE_URCL */


typedef struct vdb {
    union {
        int null;
#ifdef HAVE_URCL
        urclHandle *redis;
#endif /* HAVE_URCL */
#ifdef HAVE_LMDB
        MDB_env *lmdb;
#endif /* HAVE_LMDB */
    };
    char * rcpt;
    time_t interval;
} VDB;

struct vdb_backend {
    VDB *(*init)(const ucl_object_t *, const char *);
    void (*close)(VDB *);
    int (*recent)(VDB *, char *);
    int (*store_interval)(VDB *, time_t);
    int (*store_reply)(VDB *, char *);
    struct name_list *(*get_names)(VDB *);
    void (*clean)(VDB *, char *);
    void (*gc)(VDB *);
};

struct vdb_backend *vdb_backend(const char *);
VDB *               vdb_init(const ucl_object_t *, const char *);
void                vdb_close(VDB *);
int                 vdb_recent(VDB *, char *);
int                 vdb_store_interval(VDB *, time_t);
int                 vdb_store_reply(VDB *, char *);
struct name_list *  vdb_get_names(VDB *);
void                vdb_clean(VDB *, char *);
void                vdb_gc(VDB *);

#ifdef HAVE_LMDB
VDB *lmdb_vdb_init(const ucl_object_t *, const char *);
void lmdb_vdb_close(VDB *);
int  lmdb_vdb_recent(VDB *, char *);
int  lmdb_vdb_store_reply(VDB *, char *);
void lmdb_vdb_gc(VDB *);
#endif /* HAVE_LMDB */

#ifdef HAVE_URCL
VDB *redis_vdb_init(const ucl_object_t *, const char *);
void redis_vdb_close(VDB *);
int  redis_vdb_recent(VDB *, char *);
int  redis_vdb_store_reply(VDB *, char *);
#endif /* HAVE_URCL */

#endif /* BACKEND_VDB_H */
