/*
 * test_ccapi_stop.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: spastor
 */

#include "CppUTest/CommandLineTestRunner.h"

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_tcp_start_test)
{
    void setup()
    {
        ccapi_start_error_t start_error;
        ccapi_start_t start;

        Mock_create_all();
        fill_start_structure_with_good_parameters(&start);
        start_error = ccapi_start(&start);
        CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_terminate, NULL, connector_success);
        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);
        Mock_destroy_all();
    }
};

ccapi_bool_t ccapi_tcp_close_cb(void)
{
    return CCAPI_TRUE;
}

void ccapi_tcp_keepalives_cb(ccapi_keepalive_status_t status)
{
    UNUSED_ARGUMENT(status);
    return;
}

TEST(ccapi_tcp_start_test, testConnectorInitiateActionOK)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;

    tcp_start.callback.close = ccapi_tcp_close_cb;
    tcp_start.callback.keepalive = ccapi_tcp_keepalives_cb;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
}

TEST(ccapi_tcp_start_test, testConnectorInitiateActionInitError)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint32_t ipv4 = 0xC0A80101; /* 192.168.1.1 */
    uint8_t mac[MAC_ADDR_LENGTH] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
    tcp_start.connection.info.lan.ip.address.ipv4 = ipv4;
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof mac);

    tcp_start.callback.close = ccapi_tcp_close_cb;
    tcp_start.callback.keepalive = ccapi_tcp_keepalives_cb;

    connector_transport_t connector_transport = connector_transport_tcp;

    Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_init_error);
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INIT, error);

    Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_invalid_data);
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INIT, error);

    Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_service_busy);
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INIT, error);
}

TEST(ccapi_tcp_start_test, testConnectorInitiateActionUnknownError)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint32_t ipv4 = 0xC0A80101; /* 192.168.1.1 */
    uint8_t mac[MAC_ADDR_LENGTH] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
    tcp_start.connection.info.lan.ip.address.ipv4 = ipv4;
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof mac);

    tcp_start.callback.close = ccapi_tcp_close_cb;
    tcp_start.callback.keepalive = ccapi_tcp_keepalives_cb;

    connector_transport_t connector_transport = connector_transport_tcp;

    Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_abort);
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INIT, error);
}
