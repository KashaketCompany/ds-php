The PHP Interpreter
===================

This is the github mirror of the official PHP repository located at
http://git.php.net.

[![Build Status](https://secure.travis-ci.org/php/php-src.png?branch=master)](http://travis-ci.org/php/php-src)


  PHP Building Process
=======================
1. Download latest version of php-sdk <a href="http://windows.php.net/downloads/php-sdk/">here</a>
2. Open in the php-sdk dir command promt, type 'bin\phpsdk_buildtree.bat phpdev' in it (you can replace "phpdev" with whatever do you like)
3. Copy (phpdev)/vc9 to (phpdev)/vc11
4. Extract this repo *.zip - ped files to the php development dir (phpdev) / vc11 / x86
5. Open vs2012 x86 native tools command promt
6. Type 'cd c:\php-sdk', run it
7. Type 'bin\phpsdk_setvars', run it
8. Type 'cd (phpdev)/vc11/x86/php-5.6.40-src'
9. Type 'buildconf.bat'
10. Type 'configure --help'
11. Configure php via configure command
12. Type 'nmake snap' for the snapshot build, or just 'nmake' for the normal build
https://wiki.php.net/internals/windows/stepbystepbuild
Pull Requests
=============
PHP accepts pull requests via github. Discussions are done on github, but
depending on the topic can also be relayed to the official PHP developer
mailinglist internals@lists.php.net.

New features require an RFC and must be accepted by the developers.
See https://wiki.php.net/rfc and https://wiki.php.net/rfc/voting for more
information on the process.

Bug fixes **do not** require an RFC, but require a bugtracker ticket. Always
open a ticket at https://bugs.php.net and reference the bug id using #NNNNNN.

    Fix #55371: get_magic_quotes_gpc() throws deprecation warning

    After removing magic quotes, the get_magic_quotes_gpc function caused
    a deprecate warning. get_magic_quotes_gpc can be used to detected
    the magic_quotes behavior and therefore should not raise a warning at any
    time. The patch removes this warning

We do not merge pull requests directly on github. All PRs will be
pulled and pushed through http://git.php.net.


Guidelines for contributors
===========================
- [CODING_STANDARDS](/CODING_STANDARDS)
- [README.GIT-RULES](/README.GIT-RULES)
- [README.MAILINGLIST_RULES](/README.MAILINGLIST_RULES)
- [README.RELEASE_PROCESS](/README.RELEASE_PROCESS)

