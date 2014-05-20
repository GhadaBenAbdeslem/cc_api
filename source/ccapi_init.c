#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

static ccapi_start_error_t check_params(ccapi_start_t const * const start)
{
    ccapi_start_error_t error = CCAPI_START_ERROR_NONE;
    uint8_t const invalid_device_id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (start->vendor_id == 0x00)
    {
        error = CCAPI_START_ERROR_INVALID_VENDORID;
        goto done;
    }

    if (memcmp(start->device_id, invalid_device_id, sizeof invalid_device_id) == 0)
    {
        error = CCAPI_START_ERROR_INVALID_DEVICEID;
        goto done;
    }

    if (start->device_cloud_url == NULL || start->device_cloud_url[0] == '\0')
    {
        error = CCAPI_START_ERROR_INVALID_URL;
        goto done;
    }

    if (start->device_type == NULL || start->device_type[0] == '\0')
    {
        error = CCAPI_START_ERROR_INVALID_DEVICETYPE;
        goto done;
    }

done:
    return error;
}

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
static void free_filesystem_dir_entry_list(ccapi_data_t * const ccapi_data)
{
    ccapi_fs_virtual_dir_t * dir_entry = ccapi_data->service.file_system.virtual_dir_list;

    do {
        ccapi_fs_virtual_dir_t * const next_dir_entry = dir_entry->next;
        ccapi_free(dir_entry->local_dir);
        ccapi_free(dir_entry->virtual_dir);
        ccapi_free(dir_entry);
        dir_entry = next_dir_entry;
    } while (dir_entry != NULL);
}
#endif

static void free_ccapi_data_internal_resources(ccapi_data_t * const ccapi_data)
{
    ASSERT_MSG_GOTO(ccapi_data != NULL, done);

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
    if (ccapi_data->file_system_syncr != NULL)
    {
        ASSERT_MSG(ccapi_syncr_destroy(ccapi_data->file_system_syncr) == CCIMP_STATUS_OK);
    }

    if (ccapi_data->config.filesystem_supported)
    {
        if (ccapi_data->service.file_system.virtual_dir_list != NULL)
        {
            free_filesystem_dir_entry_list(ccapi_data);
        }
    }
#endif

    reset_heap_ptr(&ccapi_data->config.device_type);
    reset_heap_ptr(&ccapi_data->config.device_cloud_url);
    reset_heap_ptr(&ccapi_data->thread.connector_run);

    if (ccapi_data->initiate_action_syncr != NULL)
    {   
        ASSERT_MSG(ccapi_syncr_destroy(ccapi_data->initiate_action_syncr) == CCIMP_STATUS_OK);
    }

done:
    return; 
}

static ccapi_start_error_t check_malloc(void const * const p)
{
    if (p == NULL)
        return CCAPI_START_ERROR_INSUFFICIENT_MEMORY;
    else
        return CCAPI_START_ERROR_NONE;
}

/* This function allocates ccapi_data_t so other ccXapi_* functions can use it as a handler */
ccapi_start_error_t ccxapi_start(ccapi_data_t * * const ccapi_handle, ccapi_start_t const * const start)
{
    ccapi_start_error_t error = CCAPI_START_ERROR_NONE;
    ccapi_data_t * ccapi_data = NULL;

    if (ccapi_handle == NULL)
    {
        error = CCAPI_START_ERROR_NULL_PARAMETER;
        goto done;
    }

    if (*ccapi_handle != NULL)
    {
        error = CCAPI_START_ERROR_ALREADY_STARTED;
        goto done;
    }

    ccapi_data = ccapi_malloc(sizeof *ccapi_data);
    *ccapi_handle = ccapi_data;

    error = check_malloc(ccapi_data);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    ccapi_data->initiate_action_syncr = NULL;
    ccapi_data->service.file_system.virtual_dir_list = NULL;
    ccapi_data->file_system_syncr = NULL;
    /* Initialize one single time for all connector instances the logging syncr object */
    if (logging_syncr == NULL)
    {
        ccimp_os_syncr_create_t create_data;
    
        if (ccimp_os_syncr_create(&create_data) != CCIMP_STATUS_OK || ccapi_syncr_release(create_data.syncr_object) != CCIMP_STATUS_OK)
        {
            error = CCAPI_START_ERROR_SYNCR_FAILED;
            goto done;
        }

        logging_syncr = create_data.syncr_object;
    }

    ccapi_data->config.device_type = NULL;
    ccapi_data->config.device_cloud_url = NULL;
    ccapi_data->thread.connector_run = NULL;
    ccapi_data->initiate_action_syncr = NULL;

    if (start == NULL)
    {
        error = CCAPI_START_ERROR_NULL_PARAMETER;
        goto done;
    }

    error = check_params(start);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    ccapi_data->config.vendor_id = start->vendor_id;
    memcpy(ccapi_data->config.device_id, start->device_id, sizeof ccapi_data->config.device_id);

    ccapi_data->config.device_type = ccapi_malloc(strlen(start->device_type) + 1);
    error = check_malloc(ccapi_data->config.device_type);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_type, start->device_type);

    ccapi_data->config.device_cloud_url = ccapi_malloc(strlen(start->device_cloud_url) + 1);
    error = check_malloc(ccapi_data->config.device_cloud_url);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_cloud_url, start->device_cloud_url);

    ccapi_data->config.cli_supported = start->service.cli == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.receive_supported = start->service.receive == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.firmware_supported = start->service.firmware == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.rci_supported = start->service.rci == NULL ? CCAPI_FALSE : CCAPI_TRUE;

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
    if (start->service.file_system != NULL)
    {
        ccapi_data->config.filesystem_supported = CCAPI_TRUE;
        ccapi_data->service.file_system.user_callbacks.access_cb = start->service.file_system->access_cb;
        ccapi_data->service.file_system.user_callbacks.changed_cb = start->service.file_system->changed_cb;
        ccapi_data->service.file_system.imp_context = NULL;
        ccapi_data->service.file_system.virtual_dir_list = NULL;
    }
    else
#endif
    {
        ccapi_data->config.filesystem_supported = CCAPI_FALSE;
    }

    ccapi_data->connector_handle = connector_init(ccapi_connector_callback, ccapi_data);
    error = check_malloc(ccapi_data->connector_handle);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    ccapi_data->transport_tcp.connected = CCAPI_FALSE;
#if (defined CCIMP_UDP_TRANSPORT_ENABLED)
    ccapi_data->transport_udp.started = CCAPI_FALSE;
#endif
#if (defined CCIMP_SMS_TRANSPORT_ENABLED)
    ccapi_data->transport_sms.started = CCAPI_FALSE;
#endif
    {
        ccapi_data->thread.connector_run = ccapi_malloc(sizeof *ccapi_data->thread.connector_run);
        error = check_malloc(ccapi_data->thread.connector_run);
        if (error != CCAPI_START_ERROR_NONE)
            goto done;

        ccapi_data->thread.connector_run->status = CCAPI_THREAD_REQUEST_START;
        ccapi_data->thread.connector_run->ccimp_info.argument = ccapi_data;
        ccapi_data->thread.connector_run->ccimp_info.start = ccapi_connector_run_thread;
        ccapi_data->thread.connector_run->ccimp_info.type = CCIMP_THREAD_CONNECTOR_RUN;

        if (ccimp_os_create_thread(&ccapi_data->thread.connector_run->ccimp_info) != CCIMP_STATUS_OK)
        {
            error = CCAPI_START_ERROR_THREAD_FAILED;
            goto done;
        }

        do
        {
            ccimp_os_yield();
        } while (ccapi_data->thread.connector_run->status == CCAPI_THREAD_REQUEST_START);
    }

    ccapi_data->initiate_action_syncr = ccapi_syncr_create_and_release();
    if (ccapi_data->initiate_action_syncr == NULL)
    {
        error = CCAPI_START_ERROR_SYNCR_FAILED;
        goto done;
    }

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
    ccapi_data->file_system_syncr = ccapi_syncr_create_and_release();
    if (ccapi_data->file_system_syncr == NULL)
    {
        error = CCAPI_START_ERROR_SYNCR_FAILED;
        goto done;
    }
#endif

done:
    switch (error)
    {
        case CCAPI_START_ERROR_NONE:
            break;
        case CCAPI_START_ERROR_NULL_PARAMETER:
        case CCAPI_START_ERROR_INVALID_VENDORID:
        case CCAPI_START_ERROR_INVALID_DEVICEID:
        case CCAPI_START_ERROR_INVALID_URL:
        case CCAPI_START_ERROR_INVALID_DEVICETYPE:
        case CCAPI_START_ERROR_INSUFFICIENT_MEMORY:
        case CCAPI_START_ERROR_THREAD_FAILED:
        case CCAPI_START_ERROR_SYNCR_FAILED:
        case CCAPI_START_ERROR_ALREADY_STARTED:
        case CCAPI_START_ERROR_COUNT:
            if (ccapi_data != NULL)
            {
                free_ccapi_data_internal_resources(ccapi_data);
                ccapi_free(ccapi_data);
            }
            break;
    }

    ccapi_logging_line("ccapi_start ret %d", error);

    return error;
}

ccapi_stop_error_t ccxapi_stop(ccapi_data_t * const ccapi_data, ccapi_stop_t const behavior)
{
    ccapi_stop_error_t error = CCAPI_STOP_ERROR_NOT_STARTED;

    UNUSED_ARGUMENT(behavior);
    if (ccapi_data == NULL || ccapi_data->thread.connector_run->status == CCAPI_THREAD_NOT_STARTED)
        goto done;
    {
        connector_status_t connector_status = connector_initiate_action_secure(ccapi_data, connector_initiate_terminate, NULL);
        switch(connector_status)
        {
        case connector_success:
            error = CCAPI_STOP_ERROR_NONE;
            break;
        case connector_init_error:
            break;
        case connector_invalid_data_size:
            break;
        case connector_invalid_data_range:
            break;
        case connector_invalid_data:
            break;
        case connector_keepalive_error:
            break;
        case connector_bad_version:
            break;
        case connector_device_terminated:
            break;
        case connector_service_busy:
            break;
        case connector_invalid_response:
            break;
        case connector_no_resource:
            break;
        case connector_unavailable:
            break;
        case connector_idle:
            break;
        case connector_working:
            break;
        case connector_pending:
            break;
        case connector_active:
            break;
        case connector_abort:
            break;
        case connector_device_error:
            break;
        case connector_exceed_timeout:
            break;
        case connector_invalid_payload_packet:
            break;
        case connector_open_error:
            break;
        }
    }

    do {
        ccimp_os_yield();
    } while (ccapi_data->thread.connector_run->status != CCAPI_THREAD_NOT_STARTED);

done:
    switch (error)
    {
        case CCAPI_STOP_ERROR_NONE:
            free_ccapi_data_internal_resources(ccapi_data);
            ccapi_free(ccapi_data);
            break;
        case CCAPI_STOP_ERROR_NOT_STARTED:
            break;
    }

    return error;
}

ccapi_start_error_t ccapi_start(ccapi_start_t const * const start)
{
	ccapi_start_error_t error;

    error = ccxapi_start(&ccapi_data_single_instance, start);

    switch (error)
    {
        case CCAPI_START_ERROR_NONE:
            break;
        case CCAPI_START_ERROR_NULL_PARAMETER:
        case CCAPI_START_ERROR_INVALID_VENDORID:
        case CCAPI_START_ERROR_INVALID_DEVICEID:
        case CCAPI_START_ERROR_INVALID_URL:
        case CCAPI_START_ERROR_INVALID_DEVICETYPE:
        case CCAPI_START_ERROR_INSUFFICIENT_MEMORY:
        case CCAPI_START_ERROR_THREAD_FAILED:
        case CCAPI_START_ERROR_SYNCR_FAILED:
        case CCAPI_START_ERROR_ALREADY_STARTED:
        case CCAPI_START_ERROR_COUNT:
            ccapi_data_single_instance = NULL;
            break;
    }

	return error;
}

ccapi_stop_error_t ccapi_stop(ccapi_stop_t const behavior)
{
    ccapi_stop_error_t error;

    error = ccxapi_stop(ccapi_data_single_instance, behavior);
    switch (error)
    {
        case CCAPI_STOP_ERROR_NONE:
            ccapi_data_single_instance = NULL;
            break;
        case CCAPI_STOP_ERROR_NOT_STARTED:
            break;
    }

    return error;
}
