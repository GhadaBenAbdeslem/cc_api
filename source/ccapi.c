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
    ccimp_malloc_t malloc_info;
    ccimp_status_t status;

    malloc_info.size = size;
    status = ccimp_malloc(&malloc_info);

    switch (status)
    {
        case CCIMP_STATUS_OK:
            return malloc_info.ptr;
        case CCIMP_STATUS_ABORT:
        case CCIMP_STATUS_BUSY:
            break;
    }
    return NULL;
}

ccimp_status_t ccapi_free(void * ptr)
{
    ccimp_free_t free_info;

    free_info.ptr = ptr;

    return ccimp_free(&free_info);
}

ccimp_status_t ccapi_syncr_release(void * syncr_object)
{
    ccimp_os_syncr_release_t release_data;

    release_data.syncr_object = syncr_object;
        
    return ccimp_os_syncr_release(&release_data);
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

static connector_callback_status_t connector_callback_status_from_ccimp_status(ccimp_status_t const ccimp_status)
{
    connector_callback_status_t callback_status = connector_callback_abort;

    switch(ccimp_status)
    {
        case CCIMP_STATUS_ABORT:
            callback_status = connector_callback_abort;
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
                firmware_supported->supported = ccapi_data->config.firmware_supported;
            }
            break;
        case connector_request_id_config_file_system:
            {
                connector_config_supported_t * filesystem_supported = data;
                filesystem_supported->supported = ccapi_data->config.filesystem_supported;
            }
            break;
        case connector_request_id_config_remote_configuration:
            {
                connector_config_supported_t * rci_supported = data;
                rci_supported->supported = ccapi_data->config.rci_supported;
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
                connection_type->type = ccapi_data->transport_tcp.info->connection.type;
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
    ccimp_status_t ccimp_status = CCIMP_STATUS_ABORT;

    UNUSED_ARGUMENT(ccapi_data);
    switch (os_request) {
        case connector_request_id_os_malloc:
        {
            ccimp_malloc_t * malloc_data = data;

            ccimp_status = ccimp_malloc(malloc_data);
            break;
        }

        case connector_request_id_os_free:
        {
            ccimp_free_t * free_data = data;

            ccimp_status = ccimp_free(free_data);
            break;
        }

        case connector_request_id_os_yield:
        {
            ccimp_status = ccimp_os_yield();
            break;
        }

        case connector_request_id_os_system_up_time:
        {
            ccimp_os_system_up_time_t * system_uptime = data;

            ccimp_status = ccimp_os_get_system_time(system_uptime);
            break;
        }

        case connector_request_id_os_reboot:
        {
            ccimp_status = ccimp_hal_reset();
            break;
        }

        case connector_request_id_os_realloc:
        {
            ccimp_realloc_t * realloc_info = data;
            ccimp_status = ccimp_realloc(realloc_info);
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
    ccimp_status_t ccimp_status = CCIMP_STATUS_ABORT;

    UNUSED_ARGUMENT(ccapi_data);
    switch (network_request)
    {
        case connector_request_id_network_open:
        {
            ccimp_network_open_t * open_data = data;

            ccimp_status = ccimp_network_tcp_open(open_data);
            break;
        }

        case connector_request_id_network_send:
        {
            ccimp_network_send_t * send_data = data;

            ccimp_status = ccimp_network_tcp_send(send_data);
            break;
        }

        case connector_request_id_network_receive:
        {
            ccimp_network_receive_t * receive_data = data;

            ccimp_status = ccimp_network_tcp_receive(receive_data);
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
                case CCIMP_STATUS_ABORT:
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
                    connector_close_data->reconnect = ask_user_if_reconnect(close_status, ccapi_data);
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
            ccimp_status = CCIMP_STATUS_ABORT;
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
            }
            break;
        }
    }

    return connector_status;
}

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
        default:
            status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }

done:
    return status;
}
