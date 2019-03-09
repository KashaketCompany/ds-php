/*
  +----------------------------------------------------------------------+
  | Inclued                                                              |
  +----------------------------------------------------------------------+
  | Copyright (c) 2007 The PHP Group                                     |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Gopal Vijayaraghavan <gopalv@php.net>                       |
  +----------------------------------------------------------------------+
*/

/* $Id: php_inclued.h 326129 2012-06-12 18:43:47Z felipe $ */

#ifndef PHP_INCLUED_H
#define PHP_INCLUED_H

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_globals.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"

extern zend_module_entry inclued_module_entry;
#define phpext_inclued_ptr &inclued_module_entry

#define PHP_INCLUED_VERSION "0.1.3"

#ifdef PHP_WIN32
#define PHP_INCLUED_API __declspec(dllexport)
#else
#define PHP_INCLUED_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(inclued);
PHP_MSHUTDOWN_FUNCTION(inclued);
PHP_RINIT_FUNCTION(inclued);
PHP_RSHUTDOWN_FUNCTION(inclued);
PHP_MINFO_FUNCTION(inclued);


/* {{{ zend_inclued_globals */
ZEND_BEGIN_MODULE_GLOBALS(inclued)
	/* configuration parameters */
	zend_bool enabled;		/* defaults to false */
	zend_bool sampled;		/* turns on if sample is being dumped */
	int random_sampling;	/* random sampling of requests */
	int counter;			/* increment per request */
	char* dumpdir;			/* dump dir */
	zval* includes_hash; 	/* hash storing the includes */
	zval* inh_hash; 		/* hash storing inheritance operations */
ZEND_END_MODULE_GLOBALS(inclued)
/* }}} */

/* {{{ extern inclued_globals */
ZEND_EXTERN_MODULE_GLOBALS(inclued)
/* }}} */

#ifdef ZTS
#define INCLUED_G(v) TSRMG(inclued_globals_id, zend_inclued_globals *, v)
#else
#define INCLUED_G(v) (inclued_globals.v)
#endif

#endif	/* PHP_INCLUED_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
