ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = @LDFLAGS@ @LDAP_LDFLAGS@ @BDB_LDFLAGS@ 
AM_CPPFLAGS = @CPPFLAGS@ @LDAP_CPPFLAGS@ @BDB_CPPFLAGS@

bin_PROGRAMS = simvacation simunvacation

simvacation_SOURCES = yasl/src/yasl.c yasl/src/yasl.h argcargv.c argcargv.h simvacation.h simvacation.c
simvacation_LDADD =

simunvacation_SOURCES = yasl/src/yasl.c yasl/src/yasl.h argcargv.c argcargv.h simvacation.h simunvacation.c
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
## simvacreport doesn't use the abstraction layer and isn't used by us
## so I might chuck it.
simvacreport_SOURCES = simvacreport.c
simvacreport_LDADD = @BDB_LIBS@
bin_PROGRAMS += simvacreport
endif

dist_man_MANS = simvacation.8 simunvacation.8 simvacreport.8

EXTRA_DIST = yasl/LICENSE yasl/README.rst
