dnl $Id: config.m4 251214 2008-01-23 22:56:00Z tony2001 $
dnl config.m4 for extension inclued

PHP_ARG_ENABLE(inclued, whether to enable inclued support,
[  --enable-inclued           Enable inclued support])

if test "$PHP_INCLUED" != "no"; then
	
  inclued_sources="inclued.c \
  					inclued_zend.c"

  PHP_NEW_EXTENSION(inclued, $inclued_sources, $ext_shared)
fi
