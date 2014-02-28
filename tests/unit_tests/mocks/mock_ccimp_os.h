/*
 * mock_ccimp_os.h
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#ifndef _MOCK_CCIMP_OS_H_
#define _MOCK_CCIMP_OS_H_

#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

#define MOCK_MALLOC_ENABLED 1	/* Mock disabled. Do malloc normally */

typedef enum {
    MOCK_THREAD_DISABLED,    /* Mock disabled. Create thread normally */
    MOCK_THREAD_ENABLED1,    /* Don't create thread, return FALSE */
    MOCK_THREAD_ENABLED2     /* Create thread but corrupting argument */
} mock_thread_bahavior_t;

void Mock_ccimp_malloc_create(void);
void Mock_ccimp_malloc_destroy(void);
void Mock_ccimp_malloc_expectAndReturn(size_t expect, void * retval);

void Mock_ccimp_create_thread_create(void);
void Mock_ccimp_create_thread_destroy(void);
void Mock_ccimp_create_thread_expectAndReturn(ccimp_create_thread_info_t * const create_thread_info, mock_thread_bahavior_t behavior, ccapi_bool_t retval);

#endif
