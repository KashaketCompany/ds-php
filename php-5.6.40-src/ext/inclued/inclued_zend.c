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

/* $Id: inclued_zend.c 326127 2012-06-12 16:46:56Z felipe $ */

#include "php_inclued.h"
#include "inclued_zend.h"

#define INCLUED_EX_T(offset)					(*(temp_variable *)((char*)execute_data->Ts + offset))

static zval *inclued_get_zval_ptr(INCLUED_ZNODE *node, int op_type, zval **freeval, zend_execute_data *execute_data TSRMLS_DC) /* {{{ */
{
	*freeval = NULL;

	switch (op_type) {
		case IS_CONST:
#if PHP_VERSION_ID >= 50400
			return INCLUED_ZNODE_ELEM(node, zv);
#else
			return &(INCLUED_ZNODE_ELEM(node, constant));
#endif
		case IS_VAR:
			return INCLUED_EX_T(INCLUED_ZNODE_ELEM(node, var)).var.ptr;
		case IS_TMP_VAR:
			return (*freeval = &INCLUED_EX_T(INCLUED_ZNODE_ELEM(node, var)).tmp_var);
#ifdef ZEND_ENGINE_2_1
		case IS_CV:
		{
			zval ***ret = &execute_data->CVs[INCLUED_ZNODE_ELEM(node, var)];

			if (!*ret) {
				zend_compiled_variable *cv = &EG(active_op_array)->vars[INCLUED_ZNODE_ELEM(node, var)];

				if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void**)ret)==FAILURE) {
					zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
					return &EG(uninitialized_zval);
				}
			}
			return **ret;
		}
#endif
		case IS_UNUSED:
		default:
			return NULL;
	}
}
/* }}} */

static void add_include(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */
{
	zend_op *opline = execute_data->opline;
	zval * includes_hash = INCLUED_G(includes_hash);
	char * operation;

	zval * include_entry;
	zval * autoload_entry;

	php_stream_wrapper *wrapper;
	zval *inc_filename = NULL, tmp_inc_filename;
	zval *freeop1 = NULL;
	char *path_for_open;
	zend_file_handle file_handle;
#if PHP_VERSION_ID >= 50400
	int op_type = opline->extended_value;
#else
	int op_type = opline->op2.u.constant.value.lval;
#endif
	zend_execute_data *real_execute_data;


	assert(includes_hash != NULL);

	switch(op_type)
	{
		case ZEND_INCLUDE:
		{
			operation = "include";
		}
		break;
		case ZEND_REQUIRE:
		{
			operation = "require";
		}
		break;
		case ZEND_REQUIRE_ONCE:
		{
			operation = "require_once";
		}
		break;
		case ZEND_INCLUDE_ONCE:
		{
			operation = "include_once";
		}
		break;
		case ZEND_EVAL:
		{
			operation = "eval";
		}
		break;
	}
	
	if(op_type == ZEND_EVAL)
	{
		MAKE_STD_ZVAL(include_entry);
		
		array_init(include_entry);
		add_assoc_string_ex(include_entry, "operation", sizeof("operation"), operation, 1);
		add_assoc_long_ex(include_entry, "op_type", sizeof("op_type"), op_type);
		add_assoc_string_ex(include_entry, "fromfile", sizeof("fromfile"), execute_data->op_array->filename, 1);
		add_assoc_long_ex(include_entry, "fromline", sizeof("fromline"), opline->lineno);

		if(execute_data->op_array->function_name) 
		{
			add_assoc_string_ex(include_entry, "function", sizeof("function"), (char*)execute_data->op_array->function_name, 1);
		}

		add_next_index_zval(includes_hash, include_entry);

		return;
	}

#if PHP_VERSION_ID >= 50400
	inc_filename = inclued_get_zval_ptr(&opline->op1, opline->op1_type, &freeop1, execute_data TSRMLS_CC);
#else
	inc_filename = inclued_get_zval_ptr(&opline->op1, opline->op1.op_type, &freeop1, execute_data TSRMLS_CC);
#endif

	if(Z_TYPE_P(inc_filename) != IS_STRING) 
	{
		tmp_inc_filename = *inc_filename;
		zval_copy_ctor(&tmp_inc_filename);
		INIT_PZVAL(&tmp_inc_filename);
		convert_to_string(&tmp_inc_filename);
		inc_filename = &tmp_inc_filename;
	}

	wrapper = php_stream_locate_url_wrapper(Z_STRVAL_P(inc_filename), &path_for_open, 0 TSRMLS_CC);


	if(SUCCESS == zend_stream_open(inc_filename->value.str.val, &file_handle TSRMLS_CC))
	{
		/* sledgehammer ! */
		if (!file_handle.opened_path)
		{
			file_handle.opened_path = estrndup(inc_filename->value.str.val, inc_filename->value.str.len);
		}

		/*
		fprintf(stderr, "%s%s('%s') from %s:%d\n", 
			(op_type & (ZEND_INCLUDE_ONCE | ZEND_INCLUDE)) ? "include" : "require",
			(op_type & (ZEND_INCLUDE_ONCE | ZEND_REQUIRE_ONCE)) ? "_once" : "",
			file_handle.opened_path,
			execute_data->op_array->filename,
			opline->lineno);
		*/

		MAKE_STD_ZVAL(include_entry);
		
		array_init(include_entry);

		add_assoc_string_ex(include_entry, "operation", sizeof("operation"), operation, 1);
		add_assoc_long_ex(include_entry, "op_type", sizeof("op_type"), op_type);
		add_assoc_string_ex(include_entry, "filename", sizeof("filename"), Z_STRVAL_P(inc_filename), 1);
		add_assoc_string_ex(include_entry, "opened_path", sizeof("opened_path"), file_handle.opened_path, 1);

		if(zend_hash_exists(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path)+1))
		{
			add_assoc_bool_ex(include_entry, "duplicate", sizeof("duplicate"), 1);
		}
		
		add_assoc_string_ex(include_entry, "fromfile", sizeof("fromfile"), execute_data->op_array->filename, 1);
		add_assoc_long_ex(include_entry, "fromline", sizeof("fromline"), opline->lineno);

		if(execute_data->op_array->function_name) 
		{
			add_assoc_string_ex(include_entry, "function", sizeof("function"), (char*)execute_data->op_array->function_name, 1);
		}

		if(execute_data->op_array->function_name && (strcmp("__autoload", execute_data->op_array->function_name) == 0))
		{
			real_execute_data = execute_data->prev_execute_data;
			if(real_execute_data->opline == NULL &&
					real_execute_data->prev_execute_data != NULL &&
					real_execute_data->prev_execute_data->opline != NULL)
			{
				MAKE_STD_ZVAL(autoload_entry);
				array_init(autoload_entry);
				
				real_execute_data = real_execute_data->prev_execute_data;
				
				add_assoc_string_ex(autoload_entry, "fromfile", sizeof("fromfile"), real_execute_data->op_array->filename, 1);
				add_assoc_long_ex(autoload_entry, "fromline", sizeof("fromline"), real_execute_data->opline->lineno);

				add_assoc_zval(include_entry, "autoload", autoload_entry);
			}
		}

#if PHP_VERSION_ID >= 50400
		if(opline->op1_type != IS_CONST)
#else
		if(opline->op1.op_type != IS_CONST)
#endif
		{
			add_assoc_bool_ex(include_entry, "variable_include", sizeof("variable_include"), 1);
		}
		
		if(wrapper != &php_plain_files_wrapper)
		{
			add_assoc_string_ex(include_entry, "streamwrapper", sizeof("streamwrapper"), (char*)wrapper->wops->label, 1);
		}

		add_next_index_zval(includes_hash, include_entry);

#if (PHP_MAJOR_VERSION >= 5) && (PHP_MINOR_VERSION >= 3)
		zend_file_handle_dtor(&file_handle TSRMLS_CC);
#else
		zend_file_handle_dtor(&file_handle);
#endif
	}

	if(inc_filename==&tmp_inc_filename)
	{
		zval_dtor(&tmp_inc_filename);
	}
}
/* }}} */

static int inclued_op_ZEND_INCLUDE_OR_EVAL(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */
{
	if(INCLUED_G(sampled))
	{
		add_include(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
	}

	return ZEND_USER_OPCODE_DISPATCH; 
}
/* }}} */

#define INCLUED_ZCE(entry, ce) do { \
		MAKE_STD_ZVAL(entry);	\
		array_init(entry);		\
		add_assoc_stringl(entry, "name", ce->name, ce->name_length, 1); \
		if(ce->type == ZEND_INTERNAL_CLASS) { \
			add_assoc_bool(entry,  "internal", 1); \
		} else { \
			add_assoc_string(entry,  "filename", CE_INFO(ce, filename), 1); \
			add_assoc_long(entry,    "line", CE_INFO(ce, line_start)); \
		} \
	}while(0)

static void add_class(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */
{
	zend_op *opline = execute_data->opline;
	zval *inh_hash = INCLUED_G(inh_hash);
	zval *class_entry = NULL;
	zend_uchar opcode = opline->opcode;
	zend_class_entry* parent_ce = NULL;

	assert(inh_hash != NULL);	

	MAKE_STD_ZVAL(class_entry);
	array_init(class_entry);
	
	assert(opcode == ZEND_DECLARE_CLASS || opcode == ZEND_DECLARE_INHERITED_CLASS);

	/* TODO: class flags - public, private, abstract etc ... */

	add_assoc_string(class_entry, "operation", (opcode == ZEND_DECLARE_CLASS) ? "declare_class" : "declare_inherited_class", 1);
	add_assoc_string(class_entry, "filename",	execute_data->op_array->filename, 1);
	add_assoc_long(class_entry,   "line", opline->lineno);
#if PHP_VERSION_ID >= 50400	
	add_assoc_stringl(class_entry,"name", Z_STRVAL_P(opline->op2.zv), Z_STRLEN_P(opline->op2.zv), 1);
	add_assoc_stringl(class_entry,"mangled", Z_STRVAL_P(opline->op1.zv), Z_STRLEN_P(opline->op1.zv), 1);
#else
	add_assoc_stringl(class_entry,"name", Z_STRVAL_P(&opline->op2.u.constant), Z_STRLEN_P(&opline->op2.u.constant), 1);
	add_assoc_stringl(class_entry,"mangled", Z_STRVAL_P(&opline->op1.u.constant), Z_STRLEN_P(&opline->op1.u.constant), 1);
#endif

	if(opcode == ZEND_DECLARE_INHERITED_CLASS && (parent_ce = INCLUED_EX_T(opline->extended_value).class_entry))
	{
		zval *parent_entry = NULL;

		INCLUED_ZCE(parent_entry, parent_ce);

		add_assoc_zval(class_entry, "parent", parent_entry);
	}

	if(execute_data->op_array->function_name) 
	{
		add_assoc_string(class_entry, "function", (char*)execute_data->op_array->function_name, 1);
	}

	add_next_index_zval(inh_hash, class_entry);
}
/* }}} */

static int inclued_op_ZEND_DECLARE_CLASS(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */
{
	if(INCLUED_G(sampled))
	{
		add_class(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
	}

	return ZEND_USER_OPCODE_DISPATCH; 
}
/* }}} */

#define inclued_op_ZEND_DECLARE_INHERITED_CLASS inclued_op_ZEND_DECLARE_CLASS

static void add_interface(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */
{
	zend_op *opline = execute_data->opline;
	zval *inh_hash = INCLUED_G(inh_hash);
	zend_class_entry *ce = INCLUED_EX_T(INCLUED_ZNODE_ELEM(&opline->op1, var)).class_entry;
	zend_class_entry *ice = NULL;

	zval *iface_entry, *iface;
	assert(inh_hash != NULL);	

	MAKE_STD_ZVAL(iface_entry);
	array_init(iface_entry);

	add_assoc_string(iface_entry, "operation", "add_interface", 1);
	add_assoc_string(iface_entry, "filename",	execute_data->op_array->filename, 1);
	add_assoc_long(iface_entry,   "line", opline->lineno);
	add_assoc_stringl(iface_entry, "class_name", ce->name, ce->name_length, 1);

	if(execute_data->op_array->function_name) 
	{
		add_assoc_string(iface_entry, "function", (char*)execute_data->op_array->function_name, 1);
	}
	
#ifdef ZEND_ENGINE_2_3
# if PHP_VERSION_ID >= 50400
	ice = zend_fetch_class(Z_STRVAL_P(opline->op2.zv), 
									Z_STRLEN_P(opline->op2.zv), 
									opline->extended_value TSRMLS_CC);
# else 
	ice = zend_fetch_class(Z_STRVAL(opline->op2.u.constant), 
									Z_STRLEN(opline->op2.u.constant), 
									opline->extended_value TSRMLS_CC);
# endif
#else
	ice = INCLUED_EX_T(opline->op2.u.var).class_entry;
#endif

	INCLUED_ZCE(iface, ice);

	add_assoc_zval(iface_entry, "interface", iface);

	add_next_index_zval(inh_hash, iface_entry);
}
/* }}} */

static int inclued_op_ZEND_ADD_INTERFACE(ZEND_OPCODE_HANDLER_ARGS) /* {{{ */
{
	if(INCLUED_G(sampled))
	{
		add_interface(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
	}

	return ZEND_USER_OPCODE_DISPATCH;
}
/* }}} */

void inclued_zend_init(TSRMLS_D) /* {{{ */
{
	if(INCLUED_G(enabled))
	{
		if(zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, 
			inclued_op_ZEND_INCLUDE_OR_EVAL) == FAILURE) 
		{
			zend_error(E_NOTICE, "cannot install inclued opcode overrides");
			INCLUED_G(enabled) = 0;
		}
		
		if((zend_set_user_opcode_handler(ZEND_DECLARE_CLASS, 
				inclued_op_ZEND_DECLARE_CLASS) == FAILURE) 
			||	(zend_set_user_opcode_handler(ZEND_DECLARE_INHERITED_CLASS, 
				inclued_op_ZEND_DECLARE_INHERITED_CLASS) == FAILURE))
		{
			zend_error(E_NOTICE, "cannot install inclued class inheritance overrides");
		}

		if(zend_set_user_opcode_handler(ZEND_ADD_INTERFACE, inclued_op_ZEND_ADD_INTERFACE) == FAILURE) 
		{
			zend_error(E_NOTICE, "cannot install inclued interface overrides");
		}
	}
}
/* }}} */

void inclued_zend_shutdown(TSRMLS_D) /* {{{ */
{
	/* nothing */
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
