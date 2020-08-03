# Changelog
All notable changes to this project will be documented in this file.

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
