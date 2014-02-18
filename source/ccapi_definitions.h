/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCAPI_DEFINITIONS_H_
#define _CCAPI_DEFINITIONS_H_

#include "ccimp/ccimp_types.h"
#include "connector_api.h"

#define UNUSED_ARGUMENT(a)  (void)(a)

typedef struct {
    uint32_t vendor_id;
    uint8_t device_id[DEVICE_ID_LENGTH];
    char * device_type;
    char * device_cloud_url;
    ccapi_bool_t cli_supported;
    ccapi_bool_t receive_supported;
    ccapi_bool_t firmware_supported;
    ccapi_bool_t rci_supported;
    ccapi_bool_t filesystem_supported;
} ccapi_config_t;

typedef struct {
    void * connector_handle;
    ccapi_config_t config;
    ccapi_bool_t connector_thread_is_running;
} ccapi_data_t;

extern ccapi_data_t * ccapi_data;
extern void ccapi_connector_run_thread(void * const argument);
extern connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data);

#endif
