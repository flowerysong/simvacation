if BUILD_CMOCKA
AM_LDFLAGS = \
	@LDFLAGS@ \
	$(LIBUCL_LIBS) \
	@CMOCKA_LIBS@

AM_CPPFLAGS = \
	@CPPFLAGS@ \
	@CMOCKA_CFLAGS@ \
	-I$(top_srcdir)

check_PROGRAMS = cmocka_vutil
cmocka_vutil_SOURCES = unit_vutil.c ../vutil.c ../vutil.h ../yasl.c ../yasl.h
endif

check:
	pytest -x --verbose
