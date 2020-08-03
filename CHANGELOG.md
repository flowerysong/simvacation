# Changelog
All notable changes to this project will be documented in this file.

## [1.0.1] - 2020-08-03

### Fixed
- Added `simvacation.conf` to the distributed tarball to fix the build.


## [1.0.0] - 2020-08-03

### Fixed
- Replies to simta group errors addresses are explicitly suppressed, since the
  heuristics that normally catch these have edge cases.
- The build system was not correctly adding CFLAGS for library dependencies,
  resulting in broken builds under certain circumstances.

### Changed
- simvacation can now be compiled with support for multiple VDB and VLU
  backends with run-time configuration, instead of relying on compile-time
  selection of a single backend of each type.
- The deprecated Berkeley DB backend has been removed.
- When a display name is not available the From header will leave it out
  entirely, instead of using the uniqname as a placeholder.

### Added
- Basic unit and integration tests using pytest and CMocka.


## [0.12.0] - 2018-09-14

### Fixed
- Replies are correctly sent to the original sending address, not the
  canonicalized one (e.g. with BATV tags removed) we use to check the
  vacation database for recent replies.

### Added
- Support for canonicalizing the special Barracuda implementation of BATV.


## [0.11.0] - 2017-03-01

### Fixed
- Better SRS canonicalization.

### Changed
- LDAP no longer relies on deprecated library interfaces.
- MIME headers are added to the generated message.
- A `Message-ID` header is included in the generated message.
- Still more build system updates.
- Replaced individual nonstandard config files with a central, structured
  config using libucl.
- LDAP: When possible, return a pretty displayName for use in the From field.

### Added
- Redis database backend
- Support for canonicalizing BATV addresses


## [0.10.0] - 2015-08-11

### Changed
- More code refactoring and build process changes.
- Removed `simvacreport`

### Added
- LMDB database backend.
- Support for reversing SRS rewriting.


## [0.9.0] - 2014-02-27

### Fixed
- `Auto-Submitted: no` will no longer suppress responses.
- Detection of headers that should suppress responses is more robust and correct.

### Changed
- Responses to messages with `List-*` headers are suppressed.
- The Microsoft `X-Auto-Response-Suppress` header is parsed and respected.
- Reworked build system using Automake and better library detection macros.

### Added
- Improved RFC 3834 compliance by adding `In-Reply-To` and `References` to
  generated messages, when possible.


## [0.8.1] - 2013-11-05

### Fixed
- Correct logging of unknown From addresses.

### Changed
- General code sanitization and cleanup.


## [0.8.0] - 2013-02-05

Initial tagged release
