/*
 * ccapi.c
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

ccapi_data_t * ccapi_data_single_instance = NULL;

void * ccapi_malloc(size_t size)
{
    ccimp_os_malloc_t malloc_info;
    ccimp_status_t status;

    malloc_info.size = size;
    status = ccimp_os_malloc(&malloc_info);

    switch (status)
    {
        case CCIMP_STATUS_OK:
            return malloc_info.ptr;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            break;
    }
    return NULL;
}

ccimp_status_t ccapi_free(void * ptr)
{
    ccimp_os_free_t free_info;

    free_info.ptr = ptr;

    return ccimp_os_free(&free_info);
}


void * ccapi_syncr_create(void)
{
    ccimp_os_syncr_create_t create_data;
    ccimp_status_t const ccimp_status = ccimp_os_syncr_create(&create_data);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            ccapi_logging_line("ccapi_syncr_create() failed!");
            create_data.syncr_object = NULL;
            break;
    }

    return create_data.syncr_object;
}

void * ccapi_syncr_create_and_release(void)
{
    ccimp_status_t ccimp_status;
    void * syncr_object;
    syncr_object = ccapi_syncr_create();
    if (syncr_object == NULL)
    {
        goto done;
    }

    ccimp_status = ccapi_syncr_release(syncr_object);
    switch(ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            syncr_object = NULL;
            goto done;
    }

done:
    return syncr_object;
}

ccimp_status_t ccapi_syncr_acquire(void * syncr_object)
{
    ccimp_os_syncr_acquire_t acquire_data;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

    ASSERT_MSG_GOTO(syncr_object != NULL, done);
    acquire_data.syncr_object = syncr_object;
    acquire_data.timeout_ms = OS_SYNCR_ACQUIRE_INFINITE;

    ccimp_status = ccimp_os_syncr_acquire(&acquire_data);
    if (ccimp_status == CCIMP_STATUS_OK && acquire_data.acquired != CCAPI_TRUE)
    {
        ccimp_status = CCIMP_STATUS_ERROR;
    }

done:
    return ccimp_status;
}

ccimp_status_t ccapi_syncr_release(void * syncr_object)
{
    ccimp_os_syncr_release_t release_data;

    release_data.syncr_object = syncr_object;
        
    return ccimp_os_syncr_release(&release_data);
}

ccimp_status_t ccapi_syncr_destroy(void * syncr_object)
{
    ccimp_os_syncr_destroy_t destroy_data;

    destroy_data.syncr_object = syncr_object;

    return ccimp_os_syncr_destroy(&destroy_data);
}

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)

ccimp_status_t ccapi_open_file(ccapi_data_t * const ccapi_data, char const * const local_path, int const flags, ccimp_fs_handle_t * file_handler)
{
    ccimp_fs_file_open_t ccimp_fs_file_open_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->file_system_syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            goto done;
    }

    ccimp_fs_file_open_data.path = local_path;
    ccimp_fs_file_open_data.flags = flags;
    ccimp_fs_file_open_data.errnum.pointer = NULL;
    ccimp_fs_file_open_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_file_open(&ccimp_fs_file_open_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_file_open_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    ASSERT_MSG(ccapi_syncr_release(ccapi_data->file_system_syncr) == CCIMP_STATUS_OK);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            goto done;
    }

    *file_handler = ccimp_fs_file_open_data.handle;

done:
    return ccimp_status;
}

ccimp_status_t ccapi_read_file(ccapi_data_t * const ccapi_data, ccimp_fs_handle_t const file_handler, void * const data, size_t const bytes_available, size_t * const bytes_used)
{
    ccimp_fs_file_read_t ccimp_fs_file_read_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    *bytes_used = 0;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->file_system_syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            goto done;
    }

    ccimp_fs_file_read_data.handle = file_handler;
    ccimp_fs_file_read_data.buffer = data;
    ccimp_fs_file_read_data.bytes_available = bytes_available;
    ccimp_fs_file_read_data.errnum.pointer = NULL;
    ccimp_fs_file_read_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_file_read(&ccimp_fs_file_read_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_file_read_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    ASSERT_MSG(ccapi_syncr_release(ccapi_data->file_system_syncr) == CCIMP_STATUS_OK);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            goto done;
            break;
    }

    *bytes_used = ccimp_fs_file_read_data.bytes_used;

done:
    return ccimp_status;
}

ccimp_status_t ccapi_close_file(ccapi_data_t * const ccapi_data, ccimp_fs_handle_t const file_handler)
{
    ccimp_fs_file_close_t ccimp_fs_file_close_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->file_system_syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            goto done;
    }

    ccimp_fs_file_close_data.handle = file_handler;
    ccimp_fs_file_close_data.errnum.pointer = NULL;
    ccimp_fs_file_close_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_file_close(&ccimp_fs_file_close_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_file_close_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    ASSERT_MSG(ccapi_syncr_release(ccapi_data->file_system_syncr) == CCIMP_STATUS_OK);

done:
    return ccimp_status;
}

ccimp_status_t ccapi_get_dir_entry_status(ccapi_data_t * const ccapi_data, char const * const local_path, ccimp_fs_stat_t * const fs_status)
{
    ccimp_fs_dir_entry_status_t ccimp_fs_dir_entry_status_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->file_system_syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            goto done;
    }

    ccimp_fs_dir_entry_status_data.path = local_path;
    ccimp_fs_dir_entry_status_data.errnum.pointer = NULL;
    ccimp_fs_dir_entry_status_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_dir_entry_status(&ccimp_fs_dir_entry_status_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_dir_entry_status_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    ASSERT_MSG(ccapi_syncr_release(ccapi_data->file_system_syncr) == CCIMP_STATUS_OK);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            goto done;
    }

    memcpy(fs_status, &ccimp_fs_dir_entry_status_data.status, sizeof *fs_status);

done:
    return ccimp_status;
}
#endif

connector_status_t connector_initiate_action_secure(ccapi_data_t * const ccapi_data, connector_initiate_request_t const request, void const * const request_data)
{
    connector_status_t status;

    ASSERT_MSG(ccapi_syncr_acquire(ccapi_data->initiate_action_syncr) == CCIMP_STATUS_OK);

    status = connector_initiate_action(ccapi_data->connector_handle, request, request_data);

    ASSERT_MSG(ccapi_syncr_release(ccapi_data->initiate_action_syncr) == CCIMP_STATUS_OK);

    return status;
}

char * ccapi_strdup(char const * const string)
{
    size_t const string_size = strlen(string) + 1;
    char * dup_string = ccapi_malloc(string_size);

    if (dup_string != NULL)
    {
        memcpy(dup_string, string, string_size);
    }

    return dup_string;
}

void ccapi_connector_run_thread(void * const argument)
{
    ccapi_data_t * ccapi_data = argument;

    /* ccapi_data is corrupted, it's likely the implementer made it wrong passing argument to the new thread */
    ASSERT_MSG_GOTO(ccapi_data != NULL, done);

    ccapi_data->thread.connector_run->status = CCAPI_THREAD_RUNNING;
    while (ccapi_data->thread.connector_run->status == CCAPI_THREAD_RUNNING)
    {
        connector_status_t const status = connector_run(ccapi_data->connector_handle);

        ASSERT_MSG_GOTO(status != connector_init_error, done);

        switch(status)
        {
            case connector_device_terminated:
                ccapi_data->thread.connector_run->status = CCAPI_THREAD_REQUEST_STOP;
                break;
            default:
                break;
        }
    }
    ASSERT_MSG_GOTO(ccapi_data->thread.connector_run->status == CCAPI_THREAD_REQUEST_STOP, done);

    ccapi_data->thread.connector_run->status = CCAPI_THREAD_NOT_STARTED;
done:
    return;
}

connector_callback_status_t connector_callback_status_from_ccimp_status(ccimp_status_t const ccimp_status)
{
    connector_callback_status_t callback_status = connector_callback_abort;

    switch(ccimp_status)
    {
        case CCIMP_STATUS_ERROR:
            callback_status = connector_callback_error;
            break;
        case CCIMP_STATUS_OK:
            callback_status = connector_callback_continue;
            break;
        case CCIMP_STATUS_BUSY:
            callback_status = connector_callback_busy;
            break;
    }

    return callback_status;
}

connector_callback_status_t ccapi_config_handler(connector_request_id_config_t config_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t status = connector_callback_continue;

    ccapi_logging_line(TMP_INFO_PREFIX "ccapi_config_handler: config_request %d", config_request);

    switch (config_request)
    {
        case connector_request_id_config_device_id:
            {
                connector_config_pointer_data_t * device_id = data;
                device_id->data = ccapi_data->config.device_id;
            }
            break;
        case connector_request_id_config_device_cloud_url:
            {
                connector_config_pointer_string_t * device_cloud = data;
                device_cloud->string = ccapi_data->config.device_cloud_url;
                device_cloud->length = strlen(ccapi_data->config.device_cloud_url);
            }
            break;
        case connector_request_id_config_vendor_id:
            {
                connector_config_vendor_id_t * vendor_id = data;
                vendor_id->id = ccapi_data->config.vendor_id;
            }
            break;
        case connector_request_id_config_device_type:
            {
                connector_config_pointer_string_t * device_type = data;
                device_type->string = ccapi_data->config.device_type;
                device_type->length = strlen(ccapi_data->config.device_type);
            }
            break;
        case connector_request_id_config_firmware_facility:
            {
                connector_config_supported_t * firmware_supported = data;
                firmware_supported->supported = CCAPI_BOOL_TO_CONNECTOR_BOOL(ccapi_data->config.firmware_supported);
            }
            break;
#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
        case connector_request_id_config_file_system:
            {
                connector_config_supported_t * filesystem_supported = data;
                filesystem_supported->supported = CCAPI_BOOL_TO_CONNECTOR_BOOL(ccapi_data->config.filesystem_supported);
            }
            break;
#endif
        case connector_request_id_config_remote_configuration:
            {
                connector_config_supported_t * rci_supported = data;
                rci_supported->supported = CCAPI_BOOL_TO_CONNECTOR_BOOL(ccapi_data->config.rci_supported);
            }
            break;
        case connector_request_id_config_data_service:
            {
                connector_config_supported_t * data_service_supported = data;
                data_service_supported->supported = connector_true;
            }
            break;
        case connector_request_id_config_connection_type:
            {
                connector_config_connection_type_t * connection_type = data;
                switch (ccapi_data->transport_tcp.info->connection.type)
                {
                    case CCAPI_CONNECTION_LAN:
                        connection_type->type = connector_connection_type_lan;
                        break;
                    case CCAPI_CONNECTION_WAN:
                        connection_type->type = connector_connection_type_wan;
                        break;
                }
            }
            break;
        case connector_request_id_config_mac_addr:
            {
                connector_config_pointer_data_t * mac_addr = data;
                mac_addr->data = ccapi_data->transport_tcp.info->connection.info.lan.mac_address;
            }
            break;
        case connector_request_id_config_ip_addr:
            {
                connector_config_ip_address_t * ip_addr = data;
                switch(ccapi_data->transport_tcp.info->connection.ip.type)
                {
                    case CCAPI_IPV4:
                    {
                        ip_addr->ip_address_type = connector_ip_address_ipv4;
                        ip_addr->address = ccapi_data->transport_tcp.info->connection.ip.address.ipv4;
                        break;
                    }
                    case CCAPI_IPV6:
                    {
                        ip_addr->ip_address_type = connector_ip_address_ipv6;
                        ip_addr->address = ccapi_data->transport_tcp.info->connection.ip.address.ipv6;
                        break;
                    }
                }
            }
            break;
        case connector_request_id_config_identity_verification:
            {
                connector_config_identity_verification_t * id_verification = data;
                int const has_password = ccapi_data->transport_tcp.info->connection.password != NULL;

                id_verification->type = has_password ? connector_identity_verification_password : connector_identity_verification_simple;
            }
            break;
        case connector_request_id_config_password:
            {
                connector_config_pointer_string_t * password = data;
                password->string = ccapi_data->transport_tcp.info->connection.password;
                password->length = strlen(password->string);
            }
            break;
        case connector_request_id_config_max_transaction:
            {
                connector_config_max_transaction_t * max_transaction = data;
                max_transaction->count = ccapi_data->transport_tcp.info->connection.max_transactions;
            }
            break;
        case connector_request_id_config_rx_keepalive:
            {
                connector_config_keepalive_t * rx_keepalives = data;
                rx_keepalives->interval_in_seconds = ccapi_data->transport_tcp.info->keepalives.rx;
            }
            break;
        case connector_request_id_config_tx_keepalive:
            {
                connector_config_keepalive_t * tx_keepalives = data;
                tx_keepalives->interval_in_seconds = ccapi_data->transport_tcp.info->keepalives.tx;
            }
            break;
        case connector_request_id_config_wait_count:
            {
                connector_config_wait_count_t * wc_keepalives = data;
                wc_keepalives->count = ccapi_data->transport_tcp.info->keepalives.wait_count;
            }
            break;
        case connector_request_id_config_link_speed:
            {
                connector_config_link_speed_t * link_speed = data;
                link_speed->speed = ccapi_data->transport_tcp.info->connection.info.wan.link_speed;
            }
            break;
        case connector_request_id_config_phone_number:
            {
                connector_config_pointer_string_t * phone_number = data;
                phone_number->string = ccapi_data->transport_tcp.info->connection.info.wan.phone_number;
                phone_number->length = strlen(ccapi_data->transport_tcp.info->connection.info.wan.phone_number);
            }
            break;
        default:
            status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }
done:
    return status;
}

connector_callback_status_t ccapi_os_handler(connector_request_id_os_t os_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

    UNUSED_ARGUMENT(ccapi_data);
    switch (os_request) {
        case connector_request_id_os_malloc:
        {
            connector_os_malloc_t * connector_malloc_data = data;
            ccimp_os_malloc_t ccimp_malloc_data;

            ccimp_malloc_data.size = connector_malloc_data->size;
            ccimp_malloc_data.ptr = connector_malloc_data->ptr;

            ccimp_status = ccimp_os_malloc(&ccimp_malloc_data);

            connector_malloc_data->ptr = ccimp_malloc_data.ptr;
            break;
        }

        case connector_request_id_os_free:
        {
            connector_os_free_t * connector_free_data = data;
            ccimp_os_free_t ccimp_free_data;

            ccimp_free_data.ptr = connector_free_data->ptr;
            ccimp_status = ccimp_os_free(&ccimp_free_data);
            break;
        }

        case connector_request_id_os_yield:
        {
            ccimp_status = ccimp_os_yield();
            break;
        }

        case connector_request_id_os_system_up_time:
        {
            connector_os_system_up_time_t * connector_system_uptime = data;
            ccimp_os_system_up_time_t ccimp_system_uptime;

            ccimp_status = ccimp_os_get_system_time(&ccimp_system_uptime);

            connector_system_uptime->sys_uptime = ccimp_system_uptime.sys_uptime;
            break;
        }

        case connector_request_id_os_reboot:
        {
            ccimp_status = ccimp_hal_reset();
            break;
        }

        case connector_request_id_os_realloc:
        {
            connector_os_realloc_t * connector_realloc_data = data;
            ccimp_os_realloc_t ccimp_realloc_data;

            ccimp_realloc_data.new_size = connector_realloc_data->new_size;
            ccimp_realloc_data.old_size = connector_realloc_data->old_size;
            ccimp_realloc_data.ptr = connector_realloc_data->ptr;
            ccimp_status = ccimp_os_realloc(&ccimp_realloc_data);

            connector_realloc_data->ptr = ccimp_realloc_data.ptr;
            break;
        }
    }

    connector_status = connector_callback_status_from_ccimp_status(ccimp_status);

    return connector_status;
}

static ccapi_bool_t ask_user_if_reconnect(connector_close_status_t const close_status, ccapi_data_t const * const ccapi_data)
{
    ccapi_tcp_close_cb_t const close_cb = ccapi_data->transport_tcp.info->callback.close;
    ccapi_tcp_close_cause_t ccapi_close_cause;
    ccapi_bool_t reconnect = CCAPI_FALSE;

    if (close_cb != NULL)
    {
        switch (close_status)
        {
            case connector_close_status_cloud_disconnected:
                ccapi_close_cause = CCAPI_TCP_CLOSE_DISCONNECTED;
                break;
            case connector_close_status_cloud_redirected:
                ccapi_close_cause = CCAPI_TCP_CLOSE_REDIRECTED;
                break;
            case connector_close_status_device_error:
                ccapi_close_cause = CCAPI_TCP_CLOSE_DATA_ERROR;
                break;
            case connector_close_status_no_keepalive:
                ccapi_close_cause = CCAPI_TCP_CLOSE_NO_KEEPALIVE;
                break;
            case connector_close_status_device_stopped:
            case connector_close_status_device_terminated:
            case connector_close_status_abort:
                ASSERT_MSG_GOTO(0, done);
                break;
        }
        reconnect = close_cb(ccapi_close_cause);
    }
done:
    return reconnect;
}

connector_callback_status_t ccapi_network_handler(connector_request_id_network_t network_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

    UNUSED_ARGUMENT(ccapi_data);
    switch (network_request)
    {
        case connector_request_id_network_open:
        {
            connector_network_open_t * connector_open_data = data;
            ccimp_network_open_t ccimp_open_data;

            ccimp_open_data.device_cloud.url = connector_open_data->device_cloud.url;
            ccimp_open_data.handle = connector_open_data->handle;

            ccimp_status = ccimp_network_tcp_open(&ccimp_open_data);

            connector_open_data->handle = ccimp_open_data.handle;
            break;
        }

        case connector_request_id_network_send:
        {
            connector_network_send_t * connector_send_data = data;
            ccimp_network_send_t ccimp_send_data;

            ccimp_send_data.buffer = connector_send_data->buffer;
            ccimp_send_data.bytes_available = connector_send_data->bytes_available;
            ccimp_send_data.handle = connector_send_data->handle;
            ccimp_send_data.bytes_used = 0;

            ccimp_status = ccimp_network_tcp_send(&ccimp_send_data);

            connector_send_data->bytes_used = ccimp_send_data.bytes_used;
            break;
        }

        case connector_request_id_network_receive:
        {
            connector_network_receive_t * connector_receive_data = data;
            ccimp_network_receive_t ccimp_receive_data;

            ccimp_receive_data.buffer = connector_receive_data->buffer;
            ccimp_receive_data.bytes_available = connector_receive_data->bytes_available;
            ccimp_receive_data.handle = connector_receive_data->handle;
            ccimp_receive_data.bytes_used = 0;

            ccimp_status = ccimp_network_tcp_receive(&ccimp_receive_data);

            connector_receive_data->bytes_used = ccimp_receive_data.bytes_used;
            break;
        }

        case connector_request_id_network_close:
        {
            connector_network_close_t * connector_close_data = data;
            ccimp_network_close_t close_data;
            connector_close_status_t const close_status = connector_close_data->status;

            close_data.handle = connector_close_data->handle;
            ccimp_status = ccimp_network_tcp_close(&close_data);

            switch(ccimp_status)
            {
                case CCIMP_STATUS_OK:
                    ccapi_data->transport_tcp.connected = CCAPI_FALSE;
                    break;
                case CCIMP_STATUS_ERROR:
                case CCIMP_STATUS_BUSY:
                    goto done;
                    break;
            }

            switch (close_status)
            {
                case connector_close_status_cloud_disconnected:
                case connector_close_status_cloud_redirected:
                case connector_close_status_device_error:
                case connector_close_status_no_keepalive:
                {
                    connector_close_data->reconnect = CCAPI_BOOL_TO_CONNECTOR_BOOL(ask_user_if_reconnect(close_status, ccapi_data));
                    break;
                }
                case connector_close_status_device_stopped:
                case connector_close_status_device_terminated:
                case connector_close_status_abort:
                {
                    connector_close_data->reconnect = connector_false;
                    break;
                }
            }
            break;
        }

        default:
        {
            ccimp_status = CCIMP_STATUS_ERROR;
            break;
        }
    }

done:

    connector_status = connector_callback_status_from_ccimp_status(ccimp_status);

    return connector_status;
}

connector_callback_status_t ccapi_status_handler(connector_request_id_status_t status_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_continue;

    switch (status_request)
    {
        case connector_request_id_status_tcp:
        {
            connector_status_tcp_event_t * tcp_event = data;

            switch(tcp_event->status)
            {
                case connector_tcp_communication_started:
                {
                    ccapi_data->transport_tcp.connected = CCAPI_TRUE;
                    break;
                }
                case connector_tcp_keepalive_missed:
                {
                    ccapi_keepalive_status_t const keepalive_status = CCAPI_KEEPALIVE_MISSED;

                    if (ccapi_data->transport_tcp.info->callback.keepalive != NULL)
                    {
                        ccapi_data->transport_tcp.info->callback.keepalive(keepalive_status);
                    }
                    break;
                }
                case connector_tcp_keepalive_restored:
                {
                    ccapi_keepalive_status_t const keepalive_status = CCAPI_KEEPALIVE_RESTORED;

                    if (ccapi_data->transport_tcp.info->callback.keepalive != NULL)
                    {
                        ccapi_data->transport_tcp.info->callback.keepalive(keepalive_status);
                    }
                    break;
                }
            }
            break;
        }
        case connector_request_id_status_stop_completed:
        {
            connector_initiate_stop_request_t * stop_request = data;

            switch (stop_request->transport)
            {
                case connector_transport_tcp:
                case connector_transport_all:
                {
                    ccapi_data->transport_tcp.connected = CCAPI_FALSE;
                    break;
                }
#if (defined CONNECTOR_TRANSPORT_UDP)
                case connector_transport_udp:
                    /* TODO */
                    break;
#endif
#if (defined CONNECTOR_TRANSPORT_SMS)
                case connector_transport_sms:
                    /* TODO */
                    break;
#endif
            }
            break;
        }
    }

    return connector_status;
}

#if (defined CCIMP_DATA_SERVICE_ENABLED)
static connector_callback_status_t ccapi_process_send_data_request(connector_data_service_send_data_t *send_ptr)
{
    connector_callback_status_t status = connector_callback_error;
	
    if (send_ptr != NULL)
    {
        ccapi_svc_send_data_t * const svc_send = (ccapi_svc_send_data_t *)send_ptr->user_context;
        size_t bytes_expected_to_read;

        ASSERT_MSG_GOTO(svc_send != NULL, done);
        bytes_expected_to_read = (send_ptr->bytes_available > svc_send->bytes_remaining) ? svc_send->bytes_remaining : send_ptr->bytes_available;

        if (svc_send->sending_file == CCAPI_FALSE)
        {
            memcpy(send_ptr->buffer, svc_send->next_data, bytes_expected_to_read);
            svc_send->next_data = ((char *)svc_send->next_data) + bytes_expected_to_read;
        }
#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
        else
        {
            size_t bytes_read;

            ccimp_status_t ccimp_status = ccapi_read_file(svc_send->ccapi_data, svc_send->file_handler, send_ptr->buffer, bytes_expected_to_read, &bytes_read);
            if (ccimp_status != CCIMP_STATUS_OK)
            {
                svc_send->request_error = CCAPI_SEND_ERROR_ACCESSING_FILE;
                goto done;
            }

            if (bytes_expected_to_read != bytes_read)
            {
                bytes_expected_to_read = bytes_read;
                ASSERT_MSG(0); /* TODO: Remove later */
            }
        }
#endif

        send_ptr->bytes_used = bytes_expected_to_read;
        svc_send->bytes_remaining -= send_ptr->bytes_used;
        send_ptr->more_data = (svc_send->bytes_remaining > 0) ? connector_true : connector_false;

        status = connector_callback_continue;
    }
    else
    {
        ccapi_logging_line("process_send_data_request: no app data set to send\n");
    }

done:
    return status;
}

static connector_callback_status_t ccapi_process_send_data_response(connector_data_service_send_response_t const * const resp_ptr)
{

    ccapi_svc_send_data_t * const svc_send = (ccapi_svc_send_data_t *)resp_ptr->user_context;

    /* TODO: we could have a flag in svc_send where to check if user wants a response or not to skip this callback */

    ccapi_logging_line("Received %s response from Device Cloud\n", (resp_ptr->response == connector_data_service_send_response_success) ? "success" : "error");

    switch (resp_ptr->response)
    {
        case connector_data_service_send_response_success:
            svc_send->response_error = CCAPI_SEND_ERROR_NONE;
            break;
        case connector_data_service_send_response_bad_request:
            svc_send->response_error = CCAPI_SEND_ERROR_RESPONSE_BAD_REQUEST;
            break;
        case connector_data_service_send_response_unavailable:
            svc_send->response_error = CCAPI_SEND_ERROR_RESPONSE_UNAVAILABLE;
            break;
        case connector_data_service_send_response_cloud_error:
            svc_send->response_error = CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR;
            break;
        case connector_data_service_send_response_COUNT:
            ASSERT_MSG_GOTO(0, done);
            break;
    }

    if (resp_ptr->hint != NULL)
    {
        ccapi_logging_line("Device Cloud response hint %s\n", resp_ptr->hint);
    }

    if (svc_send->hint != NULL && resp_ptr->hint != NULL)
    {
        strncpy(svc_send->hint->string, resp_ptr->hint, svc_send->hint->length - 1);
        svc_send->hint->string[svc_send->hint->length - 1] = '\0';
    }

done:
    return connector_callback_continue;
}

static connector_callback_status_t ccapi_process_send_data_status(connector_data_service_status_t const * const status_ptr)
{
    ccapi_svc_send_data_t * const svc_send = (ccapi_svc_send_data_t *)status_ptr->user_context;
    connector_callback_status_t connector_status = connector_callback_error;

    ccapi_logging_line("Data service status: %d\n", status_ptr->status);
   
    switch (status_ptr->status)
    {
        case connector_data_service_status_complete:
            svc_send->status_error = CCAPI_SEND_ERROR_NONE;
            break;
        case connector_data_service_status_cancel:
            svc_send->status_error = CCAPI_SEND_ERROR_STATUS_CANCEL;
            break;
        case connector_data_service_status_timeout:
            svc_send->status_error = CCAPI_SEND_ERROR_STATUS_TIMEOUT;
            break;
        case connector_data_service_status_session_error:
            svc_send->status_error = CCAPI_SEND_ERROR_STATUS_SESSION_ERROR;
            ccapi_logging_line("Data service status: session_error=%d\n", status_ptr->session_error);
            break;
        case connector_data_service_status_COUNT:
            ASSERT_MSG_GOTO(0, done);
            break;
    }

    ASSERT_MSG_GOTO(svc_send->send_syncr != NULL, done);
    
    connector_status = (ccapi_syncr_release(svc_send->send_syncr) == CCIMP_STATUS_OK) ? connector_callback_continue : connector_callback_error;

done:
    return connector_status;
}

connector_callback_status_t ccapi_data_service_handler(connector_request_id_data_service_t const data_service_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;

    UNUSED_ARGUMENT(ccapi_data);

    switch (data_service_request)
    {
        case connector_request_id_data_service_send_data:
        {
            connector_data_service_send_data_t * send_ptr = data;

            connector_status = ccapi_process_send_data_request(send_ptr);

            break;
        }
        case connector_request_id_data_service_send_response:
        {
            connector_data_service_send_response_t * const resp_ptr = data;

            connector_status = ccapi_process_send_data_response(resp_ptr);

            break;
        }
        case connector_request_id_data_service_send_status:
        {
            connector_data_service_status_t * const status_ptr = data;
            
            connector_status = ccapi_process_send_data_status(status_ptr);

            break;
        }
        default:
            connector_status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }

done:
    return connector_status;
}
#endif

connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data, void * const context)
{
    connector_callback_status_t status = connector_callback_error;
    ccapi_data_t * ccapi_data = context;

    switch (class_id)
    {
        case connector_class_id_config:
            status = ccapi_config_handler(request_id.config_request, data, ccapi_data);
            break;
        case connector_class_id_operating_system:
            status = ccapi_os_handler(request_id.os_request, data, ccapi_data);
            break;
        case connector_class_id_network_tcp:
            status = ccapi_network_handler(request_id.network_request, data, ccapi_data);
            break;
        case connector_class_id_status:
            status = ccapi_status_handler(request_id.status_request, data, ccapi_data);
            break;
#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
        case connector_class_id_file_system:
            status = ccapi_filesystem_handler(request_id.file_system_request, data, ccapi_data);
            break;
#endif
#if (defined CCIMP_DATA_SERVICE_ENABLED)
        case connector_class_id_data_service:
            status = ccapi_data_service_handler(request_id.data_service_request, data, ccapi_data);
            break;
#endif
        default:
            status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }

done:
    return status;
}
