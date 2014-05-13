/*
 * Copyright (c) 2014 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#include <stdio.h>
#include "ccapi/ccapi.h"

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
	
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
    start->vendor_id = 0x030000DB; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    memcpy(start->device_id, device_id, sizeof start->device_id);
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = NULL;
    start->service.rci = NULL;
}

static ccapi_bool_t ccapi_tcp_close_cb(ccapi_tcp_close_cause_t cause)
{
    ccapi_bool_t reconnect;
    switch (cause)
    {
        case CCAPI_TCP_CLOSE_DISCONNECTED:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_DISCONNECTED\n");
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_REDIRECTED:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_REDIRECTED\n");
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_NO_KEEPALIVE:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_NO_KEEPALIVE\n");
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_DATA_ERROR:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_DATA_ERROR\n");
            reconnect = CCAPI_TRUE;
            break;
    }
    return reconnect;
}

int main (void)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t start_error = CCAPI_START_ERROR_NONE;
    ccapi_tcp_start_error_t tcp_start_error;
    ccapi_tcp_info_t tcp_info = {{0}};
    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    ccapi_dp_b_error_t dp_b_error;
    char hint_string[100] = "";
    ccapi_string_info_t hint_string_info;

    fill_start_structure_with_good_parameters(&start);

    start_error = ccapi_start(&start);

    if (start_error == CCAPI_START_ERROR_NONE)
    {
        printf("ccapi_start success\n");
    }
    else
    {
        printf("ccapi_start error %d\n", start_error);
        goto done;
    }

    tcp_info.connection.type = CCAPI_CONNECTION_LAN;
    memcpy(tcp_info.connection.ip.address.ipv4, ipv4, sizeof tcp_info.connection.ip.address.ipv4);
    memcpy(tcp_info.connection.info.lan.mac_address, mac, sizeof tcp_info.connection.info.lan.mac_address);

    tcp_info.callback.close = ccapi_tcp_close_cb;
    tcp_info.callback.keepalive = NULL;

    tcp_start_error = ccapi_start_transport_tcp(&tcp_info);
    if (tcp_start_error == CCAPI_TCP_START_ERROR_NONE)
    {
        printf("ccapi_start_transport_tcp success\n");
    }
    else
    {
        printf("ccapi_start_transport_tcp failed with error %d\n", tcp_start_error);
        goto done;
    }

    {
        #define SEND_DP_BINARY_SIZE_OK 100

        uint8_t * data = malloc(SEND_DP_BINARY_SIZE_OK);
        uint32_t i;

        for (i=0 ; i < SEND_DP_BINARY_SIZE_OK ; i++)
        {
			data[i] = 'a';
        }

        dp_b_error = ccapi_dp_binary_send_data(CCAPI_TRANSPORT_TCP, "ccapi_send_dp_binary", data, SEND_DP_BINARY_SIZE_OK);
        if (dp_b_error == CCAPI_DP_B_ERROR_NONE)
        {
            printf("ccapi_dp_binary_send_data success\n");
        }
        else
        {
            printf("ccapi_dp_binary_send_data failed with error %d\n", dp_b_error);
        }

        for (i=0 ; i < SEND_DP_BINARY_SIZE_OK ; i++)
        {
			data[i] = 'b';
        }

        hint_string_info.string = hint_string;
        hint_string_info.length = sizeof(hint_string);

        dp_b_error = ccapi_dp_binary_send_data_with_reply(CCAPI_TRANSPORT_TCP, "ccapi_send_dp_binary_with_reply", data, SEND_DP_BINARY_SIZE_OK, SEND_WAIT_FOREVER, &hint_string_info);
        if (dp_b_error == CCAPI_DP_B_ERROR_NONE)
        {
            printf("ccapi_dp_binary_send_data_with_reply success\n");
        }
        else
        {
            printf("ccapi_dp_binary_send_data_with_reply failed with error %d\n", dp_b_error);

            if (strlen(hint_string_info.string) != 0)
                printf("ccapi_dp_binary_send_data_with_reply hint %s\n", hint_string_info.string);
        }
    }

    {
        #define SEND_DP_BINARY_SIZE_ERROR 100000

        uint8_t * data = malloc(SEND_DP_BINARY_SIZE_ERROR);
        uint32_t i;

        for (i=0 ; i < SEND_DP_BINARY_SIZE_ERROR ; i++)
        {
			data[i] = 'c';
        }

        hint_string_info.string = hint_string;
        hint_string_info.length = sizeof(hint_string);

        dp_b_error = ccapi_dp_binary_send_data_with_reply(CCAPI_TRANSPORT_TCP, "ccapi_send_dp_binary_with_reply_error", data, SEND_DP_BINARY_SIZE_ERROR, SEND_WAIT_FOREVER, &hint_string_info);
        if (dp_b_error == CCAPI_DP_B_ERROR_NONE)
        {
            printf("ccapi_dp_binary_send_data_with_reply success\n");
        }
        else
        {
            printf("ccapi_dp_binary_send_data_with_reply failed with error %d\n", dp_b_error);

            if (strlen(hint_string_info.string) != 0)
                printf("ccapi_dp_binary_send_data_with_reply hint %s\n", hint_string_info.string);
        }
    }
done:
    return 0;
}
