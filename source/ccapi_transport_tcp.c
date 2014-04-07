#include "ccapi_definitions.h"
#include "ccapi/ccapi_transport_tcp.h"

static ccapi_bool_t valid_keepalives(ccapi_tcp_info_t const * const tcp_start, ccapi_tcp_start_error_t * const error)
{
    ccapi_bool_t success = CCAPI_TRUE;

    if (tcp_start->keepalives.rx != 0)
    {
        if (tcp_start->keepalives.rx > CCAPI_KEEPALIVES_RX_MAX || tcp_start->keepalives.rx < CCAPI_KEEPALIVES_RX_MIN)
        {
            success = CCAPI_FALSE;
            goto done;
        }
    }

    if (tcp_start->keepalives.tx != 0)
    {
        if (tcp_start->keepalives.tx > CCAPI_KEEPALIVES_TX_MAX || tcp_start->keepalives.tx < CCAPI_KEEPALIVES_TX_MIN)
        {
            success = CCAPI_FALSE;
            goto done;
        }
    }

    if (tcp_start->keepalives.wait_count != 0)
    {
        if (tcp_start->keepalives.wait_count > CCAPI_KEEPALIVES_WCNT_MAX || tcp_start->keepalives.wait_count < CCAPI_KEEPALIVES_WCNT_MIN)
        {
            success = CCAPI_FALSE;
            goto done;
        }
    }

done:
    switch(success)
    {
        case CCAPI_FALSE:
        {
            ccapi_logging_line("ccxapi_start_transport_tcp: invalid keepalive configuration");
            *error = CCAPI_TCP_START_ERROR_KEEPALIVES;
            break;
        }
        case CCAPI_TRUE:
        {
            break;
        }
    }

    return success;
}

static ccapi_bool_t valid_connection(ccapi_tcp_info_t const * const tcp_start, ccapi_tcp_start_error_t * const error)
{
    ccapi_bool_t success = CCAPI_TRUE;

    switch(tcp_start->connection.ip.type)
    {
        case CCAPI_IPV4:
        {
            uint8_t const invalid_ipv4[] = {0x00, 0x00, 0x00, 0x00};

            if (memcmp(tcp_start->connection.ip.address.ipv4, invalid_ipv4, sizeof tcp_start->connection.ip.address.ipv4) == 0)
            {
                ccapi_logging_line("ccxapi_start_transport_tcp: invalid IPv4");
                *error = CCAPI_TCP_START_ERROR_IP;
                success = CCAPI_FALSE;
                goto done;
            }
            break;
        }
        case CCAPI_IPV6:
        {
            uint8_t const invalid_ipv6[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            if (memcmp(tcp_start->connection.ip.address.ipv6, invalid_ipv6, sizeof tcp_start->connection.ip.address.ipv6) == 0)
            {
                ccapi_logging_line("ccxapi_start_transport_tcp: invalid IPv6");
                *error = CCAPI_TCP_START_ERROR_IP;
                success = CCAPI_FALSE;
                goto done;
            }
            break;
        }
    }

    switch (tcp_start->connection.type)
    {
        case CCAPI_CONNECTION_LAN:
        {
            uint8_t const invalid_mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            if (memcmp(invalid_mac, tcp_start->connection.info.lan.mac_address, sizeof invalid_mac) == 0)
            {
                success = CCAPI_FALSE;
                *error = CCAPI_TCP_START_ERROR_INVALID_MAC;
                goto done;
            }
            break;
        }
        case CCAPI_CONNECTION_WAN:
        {
            if (tcp_start->connection.info.wan.phone_number == NULL)
            {
                ccapi_logging_line("ccxapi_start_transport_tcp: invalid Phone number");
                *error = CCAPI_TCP_START_ERROR_PHONE;
                success = CCAPI_FALSE;
                goto done;
            }
        }
            break;
    }
done:
    return success;
}

static ccapi_bool_t valid_malloc(void * ptr, ccapi_tcp_start_error_t * const error)
{
    if (ptr == NULL)
    {
        *error = CCAPI_TCP_START_ERROR_INSUFFICIENT_MEMORY;
        return CCAPI_FALSE;
    }
    else
    {
        return CCAPI_TRUE;
    }
}

static ccapi_bool_t copy_lan_info(ccapi_tcp_info_t * const dest, ccapi_tcp_info_t const * const source, ccapi_tcp_start_error_t * const error)
{
    ccapi_bool_t success = CCAPI_TRUE;

    UNUSED_ARGUMENT(error);
    switch(source->connection.ip.type)
    {
        case CCAPI_IPV4:
        {
            memcpy(dest->connection.ip.address.ipv4, dest->connection.ip.address.ipv4, sizeof dest->connection.ip.address.ipv4);
            break;
        }
        case CCAPI_IPV6:
        {
            memcpy(dest->connection.ip.address.ipv6, dest->connection.ip.address.ipv6, sizeof dest->connection.ip.address.ipv6);
            break;
        }
    }

    return success;
}

static ccapi_bool_t copy_wan_info(ccapi_tcp_info_t * const dest, ccapi_tcp_info_t const * const source, ccapi_tcp_start_error_t * const error)
{
    ccapi_bool_t success = CCAPI_TRUE;

    if (source->connection.info.wan.phone_number != NULL)
    {
        dest->connection.info.wan.phone_number = ccapi_malloc(strlen(source->connection.info.wan.phone_number) + 1);
        if (!valid_malloc(dest->connection.info.wan.phone_number, error))
        {
            success = CCAPI_FALSE;
            goto done;
        }
        strcpy(dest->connection.info.wan.phone_number, source->connection.info.wan.phone_number);
    }

done:
    return success;
}

static ccapi_bool_t copy_ccapi_tcp_info_t_structure(ccapi_tcp_info_t * const dest, ccapi_tcp_info_t const * const source, ccapi_tcp_start_error_t * const error)
{
    ccapi_bool_t success = CCAPI_TRUE;
    *dest = *source; /* Strings and pointers to buffer need to be copied manually to allocated spaces. */

    if (dest->keepalives.rx == 0)
    {
        dest->keepalives.rx = CCAPI_KEEPALIVES_RX_DEFAULT;
    }
    if (dest->keepalives.tx == 0)
    {
        dest->keepalives.tx = CCAPI_KEEPALIVES_TX_DEFAULT;
    }
    if (dest->keepalives.wait_count == 0)
    {
        dest->keepalives.wait_count = CCAPI_KEEPALIVES_WCNT_DEFAULT;
    }

    if (source->connection.password != NULL)
    {
        dest->connection.password = ccapi_malloc(strlen(source->connection.password) + 1);
        if (!valid_malloc(dest->connection.password, error))
        {
            goto done;
        }
        strcpy(dest->connection.password, source->connection.password);
    }

    switch (source->connection.type)
    {
        case CCAPI_CONNECTION_LAN:
        {
            success = copy_lan_info(dest, source, error);
            break;
        }
        case CCAPI_CONNECTION_WAN:
        {
            success = copy_wan_info(dest, source, error);
            break;
        }
    }
done:
    return success;
}

ccapi_tcp_start_error_t ccxapi_start_transport_tcp(ccapi_data_t * const ccapi_data, ccapi_tcp_info_t const * const tcp_start)
{
    ccapi_tcp_start_error_t error = CCAPI_TCP_START_ERROR_NONE;

    if (ccapi_data == NULL || ccapi_data->thread.connector_run->status == CCAPI_THREAD_NOT_STARTED)
    {
        ccapi_logging_line("ccxapi_start_transport_tcp: CCAPI not started");

        error = CCAPI_TCP_START_ERROR_CCAPI_STOPPED;
        goto done;
    }

    if (tcp_start == NULL)
    {
        ccapi_logging_line("ccxapi_start_transport_tcp: invalid argument %p", (void *)tcp_start);
        error = CCAPI_TCP_START_ERROR_NULL_POINTER;
        goto done;
    }

    ccapi_data->transport_tcp.info = ccapi_malloc(sizeof *ccapi_data->transport_tcp.info);
    if (!valid_malloc(ccapi_data->transport_tcp.info, &error))
    {
        goto done;
    }

    if (!valid_keepalives(tcp_start, &error))
    {
        goto done;
    }

    if (!valid_connection(tcp_start, &error))
    {
        goto done;
    }

    if (copy_ccapi_tcp_info_t_structure(ccapi_data->transport_tcp.info, tcp_start, &error) != CCAPI_TRUE)
    {
        goto done;
    }

    ccapi_data->transport_tcp.connected = CCAPI_FALSE;

    {
        connector_transport_t transport = connector_transport_tcp;
        connector_status_t const connector_status = connector_initiate_action(ccapi_data->connector_handle, connector_initiate_transport_start, &transport);
        switch (connector_status)
        {
            case connector_success:
                break;
            case connector_init_error:
            case connector_invalid_data:
            case connector_service_busy:
                error = CCAPI_TCP_START_ERROR_INIT;
                break;
            case connector_invalid_data_size:
            case connector_invalid_data_range:
            case connector_keepalive_error:
            case connector_bad_version:
            case connector_device_terminated:
            case connector_invalid_response:
            case connector_no_resource:
            case connector_unavailable:
            case connector_idle:
            case connector_working:
            case connector_pending:
            case connector_active:
            case connector_abort:
            case connector_device_error:
            case connector_exceed_timeout:
            case connector_invalid_payload_packet:
            case connector_open_error:
                error = CCAPI_TCP_START_ERROR_INIT;
                ASSERT_MSG_GOTO(0, done);
                break;
        }
    }

    {
        ccapi_bool_t const wait_forever = CCAPI_BOOL(tcp_start->connection.timeout == 0);

        if (wait_forever)
        {
            do {
                ccimp_os_yield();
            } while (!ccapi_data->transport_tcp.connected);
        }
        else
        {
            ccapi_bool_t timeout = CCAPI_FALSE;
            ccimp_os_system_up_time_t time_start;
            ccimp_os_system_up_time_t end_time;

            ccimp_os_get_system_time(&time_start);
            end_time.sys_uptime = time_start.sys_uptime + tcp_start->connection.timeout + 1;
            do {
                ccimp_os_system_up_time_t system_uptime;

                ccimp_os_yield();
                ccimp_os_get_system_time(&system_uptime);
                if (system_uptime.sys_uptime > end_time.sys_uptime)
                {
                    timeout = CCAPI_TRUE;
                }
            } while (!ccapi_data->transport_tcp.connected && !timeout);

            if (timeout)
            {
                error = CCAPI_TCP_START_ERROR_TIMEOUT;
                goto done;
            }
        }
    }
done:
    return error;
}

ccapi_tcp_stop_error_t ccxapi_stop_transport_tcp(ccapi_data_t * const ccapi_data, ccapi_tcp_stop_t const * const tcp_stop)
{
    ccapi_tcp_stop_error_t error = CCAPI_TCP_STOP_ERROR_NONE;

    UNUSED_ARGUMENT(tcp_stop);
    if (ccapi_data == NULL || !ccapi_data->transport_tcp.connected)
    {
        error = CCAPI_TCP_STOP_ERROR_NOT_STARTED;
        goto done;
    }

    {
        connector_status_t connector_status;
        connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};

        connector_status = connector_initiate_action(ccapi_data->connector_handle, connector_initiate_transport_stop, &stop_data);

        switch(connector_status)
        {
            case connector_success:
                break;
            case connector_init_error:
            case connector_invalid_data:
            case connector_service_busy:
            case connector_invalid_data_size:
            case connector_invalid_data_range:
            case connector_keepalive_error:
            case connector_bad_version:
            case connector_device_terminated:
            case connector_invalid_response:
            case connector_no_resource:
            case connector_unavailable:
            case connector_idle:
            case connector_working:
            case connector_pending:
            case connector_active:
            case connector_abort:
            case connector_device_error:
            case connector_exceed_timeout:
            case connector_invalid_payload_packet:
            case connector_open_error:
                error = CCAPI_TCP_STOP_ERROR_CCFSM;
                ASSERT_MSG_GOTO(0, done);
                break;
        }
    }

    {
            ccapi_bool_t const wait_forever = CCAPI_BOOL(tcp_stop->timeout == 0);

            if (wait_forever)
            {
                do {
                    ccimp_os_yield();
                } while (ccapi_data->transport_tcp.connected);
            }
            else
            {
                ccapi_bool_t timeout = CCAPI_FALSE;
                ccimp_os_system_up_time_t time_start;
                ccimp_os_system_up_time_t end_time;

                ccimp_os_get_system_time(&time_start);
                end_time.sys_uptime = time_start.sys_uptime + tcp_stop->timeout + 1;
                do {
                    ccimp_os_system_up_time_t system_uptime;

                    ccimp_os_yield();
                    ccimp_os_get_system_time(&system_uptime);
                    if (system_uptime.sys_uptime > end_time.sys_uptime)
                    {
                        timeout = CCAPI_TRUE;
                    }
                } while (ccapi_data->transport_tcp.connected && !timeout);

                if (timeout)
                {
                    error = CCAPI_TCP_STOP_ERROR_TIMEOUT;
                    goto done;
                }
            }
        }

done:
    return error;
}


ccapi_tcp_start_error_t ccapi_start_transport_tcp(ccapi_tcp_info_t const * const tcp_start)
{
    return ccxapi_start_transport_tcp(ccapi_data_single_instance, tcp_start);
}

ccapi_tcp_stop_error_t ccapi_stop_transport_tcp(ccapi_tcp_stop_t const * const tcp_stop)
{
    return ccxapi_stop_transport_tcp(ccapi_data_single_instance, tcp_stop);
}
