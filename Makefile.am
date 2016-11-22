ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = @LDFLAGS@ @LDAP_LDFLAGS@ @BDB_LDFLAGS@
AM_CPPFLAGS = @CPPFLAGS@ @LDAP_CPPFLAGS@ @BDB_CPPFLAGS@

bin_PROGRAMS = simvacation simunvacation

simvacation_SOURCES = yasl.c yasl.h argcargv.c argcargv.h vdb.h vlu.h simvacation.h simvacation.c
simvacation_LDADD =

simunvacation_SOURCES = yasl.c yasl.h argcargv.c argcargv.h simvacation.h vdb.h vlu.h simunvacation.c
simunvacation_LDADD =

if BACKEND_NULL
simvacation_SOURCES += vdb_null.c vdb_null.h vlu_null.c vlu_null.h
simunvacation_SOURCES += vdb_null.c vdb_null.h vlu_null.c vlu_null.h
else
simvacation_SOURCES += vlu_ldap.c vlu_ldap.h
simvacation_LDADD += -lldap -llber
simunvacation_SOURCES += vlu_ldap.c vlu_ldap.h
simunvacation_LDADD += -lldap -llber
endif

if BACKEND_LMDB
simvacation_SOURCES += rabin.c rabin.h vdb_lmdb.c vdb_lmdb.h
simvacation_LDADD += -llmdb
simunvacation_SOURCES += rabin.c rabin.h vdb_lmdb.c vdb_lmdb.h
simunvacation_LDADD += -llmdb
endif

if BACKEND_BDB
simvacation_SOURCES += vdb_berkeley.c vdb_berkeley.h
simvacation_LDADD += @BDB_LIBS@
simunvacation_SOURCES += vdb_berkeley.c vdb_berkeley.h
simunvacation_LDADD += @BDB_LIBS@
endif

if BACKEND_REDIS
simvacation_SOURCES += rabin.c rabin.h vdb_redis.c vdb_redis.h
simvacation_LDADD += $(URCL_LIBS)
simunvacation_SOURCES += rabin.c rabin.h vdb_redis.c vdb_redis.h
simunvacation_LDADD += $(URCL_LIBS)
endif

dist_man_MANS = simvacation.8 simunvacation.8

EXTRA_DIST = COPYING.yasl VERSION packaging/rpm/simvacation.spec

rpm: dist-xz
	rpmbuild -ta $(distdir).tar.xz
