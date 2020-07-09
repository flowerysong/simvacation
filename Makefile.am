ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . test

AM_LDFLAGS = \
	@LDFLAGS@ \
	@LDAP_LDFLAGS@ \
	$(LIBUCL_LIBS) \
	$(URCL_LIBS) \
	@EXTRALIBS@

AM_CPPFLAGS = \
	@CPPFLAGS@ \
	$(LIBUCL_CFLAGS) \
	$(URCL_CFLAGS) \
	@CMOCKA_CFLAGS@ \
	@LDAP_CPPFLAGS@

bin_PROGRAMS = simvacation simunvacation
noinst_PROGRAMS = genimbed

COMMON_FILES = \
	rabin.h rabin.c \
	yasl.h yasl.c \
	vdb.h vdb.c \
	vlu.h vlu.c \
	vutil.h vutil.c \
	simvacation.h

if BUILD_CMOCKA
check_PROGRAMS = test/cmocka_vutil
test_cmocka_vutil_LDADD = @CMOCKA_LIBS@
test_cmocka_vutil_SOURCES = test/unit_vutil.c vutil.c vutil.h yasl.c yasl.h
endif

if BUILD_LDAP
COMMON_FILES += vlu_ldap.c
endif

if BUILD_LMDB
COMMON_FILES += vdb_lmdb.c
endif

if BUILD_REDIS
COMMON_FILES += vdb_redis.c
endif

BUILT_SOURCES = embedded_config.h

genimbed_SOURCES = genimbed.c

simvacation_SOURCES = simvacation.c $(COMMON_FILES)

simunvacation_SOURCES = simunvacation.c $(COMMON_FILES)

EXTRA_DIST = COPYING.yasl VERSION packaging/rpm/simvacation.spec

embedded_config.h: genimbed$(EXEEXT) simvacation.conf Makefile
	./genimbed$(EXEEXT) simvacation.conf CONFIG_BASE > embedded_config.h

rpm: dist-xz
	rpmbuild -ta $(distdir).tar.xz
