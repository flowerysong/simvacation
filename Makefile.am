ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = @LDFLAGS@ @LDAP_LDFLAGS@ @BDB_LDFLAGS@ 
AM_CPPFLAGS = @CPPFLAGS@ @LDAP_CPPFLAGS@ @BDB_CPPFLAGS@

bin_PROGRAMS = simvacation simunvacation simvacreport

simvacation_SOURCES = sds/sds.c sds/sds.h argcargv.c argcargv.h vdb_berkeley.c vdb_berkeley.h vlu_ldap.c vlu_ldap.h simvacation.h simvacation.c
simvacation_LDADD = -lldap -llber @BDB_LIBS@

simunvacation_SOURCES = sds/sds.c sds/sds.h argcargv.c argcargv.h vdb_berkeley.c vdb_berkeley.h vlu_ldap.c vlu_ldap.h simvacation.h simunvacation.c
simunvacation_LDADD = -lldap -llber @BDB_LIBS@

simvacreport_SOURCES = simvacreport.c
simvacreport_LDADD = @BDB_LIBS@

dist_man_MANS = simvacation.8 simunvacation.8 simvacreport.8

EXTRA_DIST = sds/LICENSE sds/README.md
