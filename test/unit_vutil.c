#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "vutil.h"
#include "yasl.h"

static void
test_canon_from_noop(void **state) {
    assert_string_equal(
            canon_from(yaslauto("foo@example.com")), "foo@example.com");
    assert_string_equal(canon_from(yaslauto("\"foo bar\"@example.com")),
            "\"foo bar\"@example.com");
}

static void
test_canon_from_srs(void **state) {
    assert_string_equal(
            canon_from(yaslauto(
                    "SRS0=gfkgj=fp=subdomain.example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");

    /* Alternate initial separators */
    assert_string_equal(
            canon_from(yaslauto(
                    "SRS0+gfkgj=fp=subdomain.example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");
    assert_string_equal(
            canon_from(yaslauto(
                    "SRS0-gfkgj=fp=subdomain.example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");

    /* Quoted localpart */
    assert_string_equal(canon_from(yaslauto("\"SRS0=gfkgj=fp=subdomain.example."
                                            "edu=foo bar\"@example.edu")),
            "\"foo bar\"@subdomain.example.edu");

    /* SRS can also use different schemes, in which case we can't reverse it. */
    assert_string_equal(
            canon_from(yaslauto("SRS0=thisisreallyanopaquestring@example.edu")),
            "SRS0=thisisreallyanopaquestring@example.edu");
    assert_string_equal(
            canon_from(yaslauto("\"SRS0=this is really an opaque string\"@example.edu")),
            "\"SRS0=this is really an opaque string\"@example.edu");

    /* Reforwarded */
    assert_string_equal(
            canon_from(yaslauto("SRS1=re2xz=example.com==gfkgj=fp=subdomain."
                                "example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");
    assert_string_equal(
            canon_from(yaslauto("SRS1+re2xz=example.com==gfkgj=fp=subdomain."
                                "example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");
    assert_string_equal(
            canon_from(yaslauto("SRS1-re2xz=example.com==gfkgj=fp=subdomain."
                                "example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");
    assert_string_equal(
            canon_from(yaslauto("SRS1=re2xz=example.com=+gfkgj=fp=subdomain."
                                "example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");
    assert_string_equal(
            canon_from(yaslauto("SRS1=re2xz=example.com=-gfkgj=fp=subdomain."
                                "example.edu=foo@example.edu")),
            "foo@subdomain.example.edu");
}

static void
test_canon_from_batv(void **state) {
    assert_string_equal(canon_from(yaslauto("prvs=somehash=foo@example.com")),
            "foo@example.com");
    assert_string_equal(canon_from(yaslauto("prvs=notreally@example.com")),
            "prvs=notreally@example.com");
}

static void
test_canon_from_barracuda(void **state) {
    assert_string_equal(canon_from(yaslauto("btv1==somehash==foo@example.com")),
            "foo@example.com");
    assert_string_equal(canon_from(yaslauto("btv1==notreally@example.com")),
            "btv1==notreally@example.com");
}

static void
test_check_from_noop(void **state) {
    assert_string_equal(
            check_from(yaslauto("foo@example.com")), "foo@example.com");
}

static void
test_check_from_simta_group(void **state) {
    assert_null(check_from(yaslauto("foo-errors@example.com")));
    assert_null(check_from(yaslauto("\"foo foo-errors\"@example.com")));
    assert_non_null(check_from(yaslauto("errors@example.com")));
}

static void
test_check_from_mailsystem(void **state) {
    assert_null(check_from(yaslauto("postmaster@example.com")));
    assert_null(check_from(yaslauto("mailer-daemon@example.com")));
    assert_null(check_from(yaslauto("foo-request@example.com")));
    assert_null(check_from(yaslauto("mailer@example.com")));
    assert_null(check_from(yaslauto("foo-relay@example.com")));
}

int
main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_canon_from_noop),
            cmocka_unit_test(test_canon_from_srs),
            cmocka_unit_test(test_canon_from_batv),
            cmocka_unit_test(test_canon_from_barracuda),
            cmocka_unit_test(test_check_from_noop),
            cmocka_unit_test(test_check_from_simta_group),
            cmocka_unit_test(test_check_from_mailsystem),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
