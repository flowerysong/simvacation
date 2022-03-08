# simvacation

## Background

simvacation is a heavily modified version of the BSD vacation tool.
Traces of the original code (which was designed to be manually added
to a user's `.forward` file and store vacation state in a per-user
Berkeley DB) can still be seen, but close integration with our LDAP
directory and mail routing system has significantly changed its
internal architecture.

## Dependencies

simvacation is developed and used mainly on Linux systems, but tries
to avoid gratuitous incompatibility with other Unix-like systems.

In order to build simvacation, you will need the following:

* A C compiler. Compilation has been tested with [GCC](https://gcc.gnu.org/)
  and [clang](https://clang.llvm.org/), and other modern compilers should
  also work.
* make
* pkg-config or a compatible replacement.
* [libucl](https://github.com/vstakhov/libucl)

Optional dependencies:

* [URCL](https://github.com/simta/urcl) for Redis VDB support
* [LMDB](https://symas.com/lightning-memory-mapped-database/) for LMDB VDB support
* [OpenLDAP](https://www.openldap.org/) for LDAP VLU support

## Testing

Tests can be run with `make check`. simvacation's test suite requires
[pytest](https://pytest.org) >= 3.9 and Python >= 3.7; you may also
want to install [cmocka](https://cmocka.org/) and pass `--with-cmocka`
to the `configure` script to enable additional unit tests.

Some tests require spawning an ephemeral redis instance. If
`redis-server` is not found these tests will be skipped.

Some tests require an LDAP server with [a specific set of
data](test/ldap/README.md). If the `LDAP_SERVER` environment variable
is not set these tests will be skipped. If you haven't set up LDAP
correctly, these tests will fail.


```
./configure --with-cmocka CFLAGS='-g -O0 --coverage'
make check
gcov *.c
```

## Contact Us

<simta@umich.edu>
