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

/* $Id: inclued.c 326127 2012-06-12 16:46:56Z felipe $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
# include <io.h>
# include <fcntl.h>
#endif

#include "php_inclued.h"
#include "inclued_zend.h"

/* {{{ stupid stringifcation */
#if DEFAULT_SLASH == '/'
#	define DEFAULT_SLASH_STRING "/"
#elif DEFAULT_SLASH == '\\'
#	define DEFAULT_SLASH_STRING "\\"
#else
#	error "Unknown value for DEFAULT_SLASH"
#endif
/* }}} */

/* {{{ ZEND_DECLARE_MODULE_GLOBALS(inclued) */
ZEND_DECLARE_MODULE_GLOBALS(inclued)
/* }}} */

/* {{{ php_inclued_init_globals */
static void php_inclued_init_globals(zend_inclued_globals* inclued_globals TSRMLS_DC)
{
	inclued_globals->enabled = 0;
	inclued_globals->sampled = 1;
	inclued_globals->random_sampling = 0;
	inclued_globals->counter = 0;
	inclued_globals->dumpdir = 0;
	inclued_globals->includes_hash = NULL;
	inclued_globals->inh_hash = NULL;
}
/* }}} */

/* {{{ php_inclued_shutdown_globals */
static void php_inclued_shutdown_globals(zend_inclued_globals* inclued_globals TSRMLS_DC)
{
	/* nothing ? */
}
/* }}} */

/* {{{ ini entries */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("inclued.enabled", "0", PHP_INI_SYSTEM, OnUpdateBool, enabled, zend_inclued_globals, inclued_globals)
	STD_PHP_INI_BOOLEAN("inclued.random_sampling", "0", PHP_INI_SYSTEM, OnUpdateLong, random_sampling, zend_inclued_globals, inclued_globals)
	STD_PHP_INI_ENTRY("inclued.dumpdir", NULL, PHP_INI_SYSTEM, OnUpdateString, dumpdir, zend_inclued_globals, inclued_globals)
PHP_INI_END()
/* }}} */

#ifdef COMPILE_DL_INCLUED
ZEND_GET_MODULE(inclued)
#endif

/* {{{ copy_class_table */
static zval* copy_class_table(HashTable *ht)
{
	HashPosition pos;
	zval *class_table = NULL;
	zend_class_entry **pce = NULL;
	zend_class_entry *ce = NULL;
	char *key = NULL;
	uint keylen = 0;
	zval *class_entry, *parent_entry;

	MAKE_STD_ZVAL(class_table);
	array_init(class_table);

	zend_hash_internal_pointer_reset_ex(ht, &pos);

	while(zend_hash_get_current_data_ex(ht, (void **) &pce, &pos) == SUCCESS) 
	{
		zend_hash_get_current_key_ex(ht, &key, &keylen, NULL, 0, &pos);

		ce = pce[0];

		if(ce->type == ZEND_INTERNAL_CLASS) 
		{ 
			goto next;
		}

		/* TODO: class flags */

		MAKE_STD_ZVAL(class_entry);
		array_init(class_entry);

		add_assoc_stringl_ex(class_entry, "name", sizeof("name"), ce->name, ce->name_length, 1);
		if(ce->name[0] != key[0])
		{
			add_assoc_stringl_ex(class_entry, "mangled_name", sizeof("mangled_name"), key, keylen-1, 1);
		}
		add_assoc_string_ex(class_entry, "filename", sizeof("filename"), CE_INFO(ce, filename), 1);
		add_assoc_long_ex(class_entry, "line", sizeof("line"), CE_INFO(ce, line_start));

		if(ce->parent)
		{
			MAKE_STD_ZVAL(parent_entry);
			array_init(parent_entry);
			add_assoc_stringl_ex(parent_entry, "name", sizeof("name"), ce->parent->name, ce->parent->name_length, 1);
			if(ce->parent->type == ZEND_INTERNAL_CLASS) 
			{ 
				add_assoc_bool_ex(parent_entry, "internal", sizeof("internal"), 1);
			} 
			else
			{
				add_assoc_string_ex(parent_entry, "filename", sizeof("filename"), CE_INFO(ce, filename), 1);
				add_assoc_long_ex(parent_entry, "line", sizeof("line"), CE_INFO(ce, line_start));
			}

			add_assoc_zval_ex(class_entry, "parent", sizeof("parent"), parent_entry);
		}

		add_next_index_zval(class_table, class_entry);
next:
		zend_hash_move_forward_ex(ht, &pos);
	}

	return class_table;
}
/* }}} */

/* {{{ pack_output */
static void pack_output(zval* return_value TSRMLS_DC)
{

	zval **data;
	zval *includes_hash;
	zval *inh_hash;
	zval *request_hash;
	zval *class_table;

#define COPY_INCLUED_G(h) do { \
	MAKE_STD_ZVAL(h); \
	*h = *INCLUED_G(h);\
	zval_copy_ctor(h); \
	INIT_PZVAL(h); \
	}\
	while(0)
	
	COPY_INCLUED_G(includes_hash);
	COPY_INCLUED_G(inh_hash);

	MAKE_STD_ZVAL(request_hash);
	
	array_init(request_hash);
	
#define PULL_ZVAL(symtab, name)  do { \
		if (zend_hash_find((symtab), #name , sizeof(#name), (void **) &data) != FAILURE) \
		{ \
			zval *copy_zval;\
			MAKE_STD_ZVAL(copy_zval);\
			*copy_zval = *data[0];\
			zval_copy_ctor(copy_zval);\
			INIT_PZVAL(copy_zval); \
			add_assoc_zval_ex(request_hash, #name, sizeof(#name), copy_zval);\
		}\
	} while(0);

	PULL_ZVAL(&EG(symbol_table), PHP_SELF);
	PULL_ZVAL(&EG(symbol_table), _REQUEST);
	PULL_ZVAL(&EG(symbol_table), _COOKIE);

	if (zend_hash_find((&EG(symbol_table)), "_SERVER", sizeof("_SERVER"), (void **) &data) != FAILURE) 
	{
		HashTable *_server = Z_ARRVAL_PP(data);
		PULL_ZVAL(_server, SCRIPT_FILENAME);
		PULL_ZVAL(_server, REQUEST_URI);
		PULL_ZVAL(_server, REQUEST_TIME);
	}

	class_table = copy_class_table(EG(class_table));

	array_init(return_value);
	
	add_assoc_zval_ex(return_value, "request", sizeof("request"), request_hash);
	add_assoc_zval_ex(return_value, "includes", sizeof("includes"), includes_hash);
	add_assoc_zval_ex(return_value, "inheritance", sizeof("inheritance"), inh_hash);
	add_assoc_zval_ex(return_value, "classes", sizeof("classes"), class_table);
	
	return;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(inclued)
{
	ZEND_INIT_MODULE_GLOBALS(inclued, php_inclued_init_globals, php_inclued_shutdown_globals);

	REGISTER_INI_ENTRIES();
	
	if(INCLUED_G(enabled)) 
	{
		inclued_zend_init(TSRMLS_C);
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 *  */
PHP_MSHUTDOWN_FUNCTION(inclued)
{
	if(INCLUED_G(enabled)) 
	{
		inclued_zend_shutdown(TSRMLS_C);
	}
	
#ifdef ZTS
	ts_free_id(inclued_globals_id);
#else
	php_inclued_shutdown_globals(&inclued_globals);
#endif
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION(inclued) */
PHP_RINIT_FUNCTION(inclued)
{
	int sample = 0;
	if(INCLUED_G(enabled)) {

		INCLUED_G(counter)++;

		if(INCLUED_G(random_sampling)) {
			sample = INCLUED_G(counter) % (INCLUED_G(random_sampling));
		}

		INCLUED_G(sampled) = (sample == 0);

		if(INCLUED_G(sampled)) {
			/* allocate new info table */
			ALLOC_INIT_ZVAL(INCLUED_G(includes_hash));
			array_init(INCLUED_G(includes_hash));

			ALLOC_INIT_ZVAL(INCLUED_G(inh_hash));
			array_init(INCLUED_G(inh_hash));
		}
    }
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(inclued) */
PHP_RSHUTDOWN_FUNCTION(inclued)
{
	char path[MAXPATHLEN];
	FILE *dumpfile;
#ifdef ZTS
	int dumpfd;
#endif
	smart_str buf = {0};
	php_serialize_data_t var_hash;
	zval retval;
	zval *pretval = &retval;
	
	if(INCLUED_G(enabled) && INCLUED_G(sampled) &&
		INCLUED_G(dumpdir) && INCLUED_G(dumpdir)[0] != '\0') {

#ifndef ZTS
		snprintf(path, MAXPATHLEN, "%s" DEFAULT_SLASH_STRING "inclued.%05d.%d", 
					INCLUED_G(dumpdir), 
					getpid(), INCLUED_G(counter));

		if(!(dumpfile = fopen(path, "w"))) {
			zend_error(E_WARNING, "cannot write to %s", path);
			return SUCCESS;
		}
#else
		snprintf(path, MAXPATHLEN, "%s" DEFAULT_SLASH_STRING "inclued.XXXXXX", 
					INCLUED_G(dumpdir));

#ifndef HAVE_MKSTEMP
	{
		char *ptr = mktemp(path);
		dumpfd = open(ptr, O_RDWR|O_TRUNC|O_EXCL|O_CREAT, 0600);
		if (dumpfd == -1)  {
			zend_error(E_WARNING, "cannot write to %s", path);
			return SUCCESS;
		}
	}
#else
		if((dumpfd = mkstemp(path)) == -1) {
			zend_error(E_WARNING, "cannot write to %s", path);
			return SUCCESS;
		}
#endif

		dumpfile = fdopen(dumpfd, "w"); 
		close(dumpfd);
#endif

		pack_output(pretval TSRMLS_CC);

		PHP_VAR_SERIALIZE_INIT(var_hash);
		php_var_serialize(&buf, (zval**)&(pretval), &var_hash TSRMLS_CC);
		PHP_VAR_SERIALIZE_DESTROY(var_hash);
		
		fwrite(buf.c, buf.len, 1, dumpfile);
		fclose(dumpfile);
		smart_str_free(&buf);
		zval_dtor(pretval);
	}

	if(INCLUED_G(enabled) && INCLUED_G(sampled)) {
		/* free allocated stuff */
		zval_ptr_dtor(&INCLUED_G(includes_hash));
		INCLUED_G(includes_hash) = NULL;
		zval_ptr_dtor(&INCLUED_G(inh_hash));
		INCLUED_G(inh_hash) = NULL;
		INCLUED_G(sampled) = 0;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(inclued)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "inclued support", "enabled");
	php_info_print_table_row(2, "extension version", PHP_INCLUED_VERSION);
	php_info_print_table_end();
	
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto mixed inclued_get_data(void)
 */
PHP_FUNCTION(inclued_get_data)
{
	if(INCLUED_G(enabled) && INCLUED_G(sampled)) 
	{
		pack_output(return_value TSRMLS_CC);
		return;
	}
	else
	{
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ inclued_functions[]
 */
zend_function_entry inclued_functions[] = {
	PHP_FE(inclued_get_data,		NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ inclued_module_entry
 */
zend_module_entry inclued_module_entry = {
	STANDARD_MODULE_HEADER,
	"inclued",
	inclued_functions,
	PHP_MINIT(inclued),
	PHP_MSHUTDOWN(inclued),
	PHP_RINIT(inclued),
	PHP_RSHUTDOWN(inclued),
	PHP_MINFO(inclued),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_INCLUED_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
