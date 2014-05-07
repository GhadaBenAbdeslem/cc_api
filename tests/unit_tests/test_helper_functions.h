/*
 * test_helper_functions.h
 *
 *  Created on: Mar 18, 2014
 *      Author: spastor
 */

#ifndef _TEST_HELPER_FUNCTIONS_H_
#define _TEST_HELPER_FUNCTIONS_H_

#define CCAPI_CONST_PROTECTION_UNLOCK
#define CONNECTOR_CONST_PROTECTION

#define TH_DEVICE_TYPE_STRING      "Device type"
#define TH_DEVICE_CLOUD_URL_STRING "login.etherios.com"

#include "CppUTest/CommandLineTestRunner.h"
#include "mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"

typedef struct {
    ccimp_fs_handle_t ccimp_handle;
    char * file_path;
    ccapi_fs_request_t request;
} ccapi_fs_file_handle_t;
}

typedef int my_filesystem_context_t;
typedef int my_filesystem_dir_handle_t;

extern my_filesystem_context_t my_fs_context; /* Defined in mock_ccimp_filesystem.cpp */
extern my_filesystem_dir_handle_t dir_handle; /* Defined in mock_ccimp_filesystem.cpp */
extern ccapi_bool_t ccapi_tcp_keepalives_cb_called;
extern ccapi_keepalive_status_t ccapi_tcp_keepalives_cb_argument;
extern ccapi_bool_t ccapi_tcp_close_cb_called;
extern ccapi_tcp_close_cause_t ccapi_tcp_close_cb_argument;

void th_fill_start_structure_with_good_parameters(ccapi_start_t * start);
void th_start_ccapi(void);
void th_fill_tcp_wan_ipv4_callbacks_info(ccapi_tcp_info_t * tcp_start);
void th_fill_tcp_lan_ipv4(ccapi_tcp_info_t * tcp_start);

void th_start_tcp_wan_ipv4_with_callbacks(void);
void th_start_tcp_lan_ipv4(void);
void th_start_tcp_lan_ipv6_password_keepalives(void);
void th_stop_ccapi(ccapi_data_t * const ccapi_data);
pthread_t th_aux_ccapi_start(void * argument);
int th_stop_aux_thread(pthread_t pthread);
ccapi_fs_file_handle_t * th_filesystem_openfile(char const * const path, connector_file_system_open_t * const ccfsm_open_data, int flags);
void th_filesystem_prepare_ccimp_dir_entry_status_call(ccimp_fs_dir_entry_status_t * const ccimp_fs_dir_entry_status_data, char const * const path);
void th_filesystem_prepare_ccimp_dir_open_data_call(ccimp_fs_dir_open_t * const ccimp_dir_open_data, char const * const path);
void th_filesystem_prepare_ccimp_dir_close_call(ccimp_fs_dir_close_t * const ccimp_dir_close_data);
void th_call_ccimp_fs_error_desc_and_check_error(void * ccfsm_errnum, connector_file_system_error_t ccfsm_fs_error);
void create_test_file(char const * const path, void const * const data, size_t bytes);
void destroy_test_file(char const * const path);

void * th_expect_malloc(size_t size, th_malloc_behavior_t behavior, bool expect_free);

#endif
