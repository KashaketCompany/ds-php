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

/* $Id: inclued_zend.h 326127 2012-06-12 16:46:56Z felipe $ */

#ifndef PHP_INCLUED_ZEND_H
#define PHP_INCLUED_ZEND_H

#include "php.h"
#include "zend.h"
#include "zend_API.h"
#include "zend_compile.h"
#include "zend_hash.h"
#include "zend_extensions.h"

#if ZEND_MODULE_API_NO > 20060613 
#define ZEND_ENGINE_2_3
#endif
#if ZEND_MODULE_API_NO > 20050922
#define ZEND_ENGINE_2_2
#endif
#if ZEND_MODULE_API_NO > 20050921
#define ZEND_ENGINE_2_1
#endif
#ifdef ZEND_ENGINE_2_1
#include "zend_vm.h"
#endif

#ifndef ZEND_VM_KIND_CALL /* Not currently defined by any ZE version */
# define ZEND_VM_KIND_CALL	1
#endif

#ifndef ZEND_VM_KIND /* Indicates PHP < 5.1 */
# define ZEND_VM_KIND	ZEND_VM_KIND_CALL
#endif

#if !defined(ZEND_ENGINE_2_1)
#error "Zend Engine2 (php 5.1 or greater) is required to use this extension"
#endif

#if (ZEND_VM_KIND != ZEND_VM_KIND_CALL)
#error "Features only available to Call dispatch VM cores (should be default)"
#endif

#if PHP_VERSION_ID >= 50400
# define INCLUED_ZNODE znode_op
# define INCLUED_ZNODE_ELEM(_x, _y) (_x)->_y
# define CE_INFO(_x, _y) (_x)->info.user._y
#else
# define INCLUED_ZNODE znode
# define INCLUED_ZNODE_ELEM(_x, _y) (_x)->u._y
# define CE_INFO(_x, _y) (_x)->_y
#endif

void inclued_zend_init(TSRMLS_D);
void inclued_zend_shutdown(TSRMLS_D);

#endif /* PHP_INCLUED_ZEND_H */
