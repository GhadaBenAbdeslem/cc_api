/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_FIRMWARE_SERVICE_ENABLED)

static connector_callback_status_t ccapi_process_firmware_update_start(connector_firmware_download_start_t * const start_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;
    ccapi_firmware_update_error_t ccapi_firmware_update_error;

    ASSERT_MSG_GOTO(start_ptr->target_number < ccapi_data->service.firmware_update.target.count, done);

    connector_status = connector_callback_continue;
    start_ptr->status = connector_firmware_status_success;

    ccapi_logging_line("ccapi_process_firmware_update_start for target_number = '%d'", start_ptr->target_number);

    if (ccapi_data->service.firmware_update.service.update_started == CCAPI_TRUE)
    {
        start_ptr->status = connector_firmware_status_device_error;
        goto done;
    }

    if (start_ptr->code_size > ccapi_data->service.firmware_update.target.list[start_ptr->target_number].maximum_size)
    {
        start_ptr->status = connector_firmware_status_download_invalid_size;
        goto done;
    }

    if (ccapi_data->service.firmware_update.user_callbacks.request_cb != NULL)
    {
        ccapi_firmware_update_error = ccapi_data->service.firmware_update.user_callbacks.request_cb(start_ptr->target_number, start_ptr->filename, start_ptr->code_size);

        switch (ccapi_firmware_update_error)
        {
            case CCAPI_FIRMWARE_UPDATE_ERROR_NONE:
                break;    
            case CCAPI_FIRMWARE_UPDATE_ERROR_REFUSE_DOWNLOAD:
                start_ptr->status = connector_firmware_status_download_denied;
                break;    
        }
    } 

    /* TODO: alloc chunk_size ? */

    ccapi_data->service.firmware_update.service.update_started = CCAPI_TRUE;

done:
    return connector_status;
}


connector_callback_status_t ccapi_firmware_service_handler(connector_request_id_firmware_t const firmware_service_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;

    switch (firmware_service_request)
    {
        case connector_request_id_firmware_target_count:
        {
            connector_firmware_count_t * const count_ptr = data;

            ASSERT_MSG_GOTO(ccapi_data->config.firmware_supported == CCAPI_TRUE, done);

            count_ptr->count = ccapi_data->service.firmware_update.target.count;
           
            connector_status = connector_callback_continue;

            break;
        }

        case connector_request_id_firmware_info:
        {
            connector_firmware_info_t * const info_ptr = data;

            ASSERT_MSG_GOTO(ccapi_data->config.firmware_supported == CCAPI_TRUE, done);

            ASSERT_MSG_GOTO(info_ptr->target_number < ccapi_data->service.firmware_update.target.count, done);

            info_ptr->version.major = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.major;
            info_ptr->version.minor = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.minor;
            info_ptr->version.revision = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.revision;
            info_ptr->version.build = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.build;

            info_ptr->description = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].description;
            info_ptr->filespec = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].filespec;
           
            connector_status = connector_callback_continue;

            break;
        }

        case connector_request_id_firmware_download_start:
        {
            connector_firmware_download_start_t * const start_ptr = data;

            connector_status = ccapi_process_firmware_update_start(start_ptr, ccapi_data);

            break;
        }

        case connector_request_id_firmware_download_data:
        case connector_request_id_firmware_download_complete:
        case connector_request_id_firmware_download_abort:
        case connector_request_id_firmware_target_reset:
            ASSERT(0);
            break;  
    }

    ASSERT_MSG_GOTO(connector_status != connector_callback_unrecognized, done);

done:
    return connector_status;
}
#endif