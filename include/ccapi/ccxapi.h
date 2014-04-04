/*
 * ccxapi.h
 *
 *  Created on: Mar 31, 2014
 *      Author: hbujanda
 */

#ifndef _CCXAPI_H_
#define _CCXAPI_H_

/* 
 * Cloud Connector Multi-instance API
 *
 * Don't distribute this file. It's only for internal usage.
 *
 * This header is the multi-instance variant of ccapi.h
 * include "ccapi/ccxapi.h" instead of "ccapi/ccapi.h" from your applications if you pretend to start several 
 * instances of Cloud Connector.
 * Then use the ccxapi_* functions declared below instead of the ccapi_* functions declared in "ccapi/ccapi.h"
 */

#include "ccapi/ccapi.h"

typedef struct ccapi_handle * ccapi_handle_t;

ccapi_start_error_t ccxapi_start(ccapi_handle_t * const ccapi_handle, ccapi_start_t const * const start);
ccapi_stop_error_t ccxapi_stop(ccapi_handle_t const ccapi_handle, ccapi_stop_t const behavior);

ccapi_tcp_start_error_t ccxapi_start_transport_tcp(ccapi_handle_t const ccapi_handle, ccapi_tcp_info_t const * const tcp_start);

#endif
