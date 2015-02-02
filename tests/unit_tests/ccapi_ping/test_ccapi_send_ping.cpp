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

#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_send_ping_no_reply)
{
    static ccapi_send_error_t error;

    void setup()
    {
        Mock_create_all();

        th_start_ccapi();

        th_start_udp();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

TEST(test_ccapi_send_ping_no_reply, testSEND_ERROR_NONE)
{
    ccapi_ping_error_t error;

    connector_sm_send_ping_request_t header;

    header.transport = connector_transport_udp;
    header.response_required = connector_false;
    header.timeout_in_seconds = CCAPI_SEND_WAIT_FOREVER;
    header.request_id = NULL;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_ping_request, &header, connector_success);

    error = ccapi_send_ping(CCAPI_TRANSPORT_UDP);
    CHECK_EQUAL(CCAPI_PING_ERROR_NONE, error);
}

TEST(test_ccapi_send_ping_no_reply, testSEND_ERROR_RESPONSE_CANCEL)
{
    ccapi_ping_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_ping_info.response = connector_sm_ping_response_t::connector_sm_ping_status_cancel;
    }

    error = ccapi_send_ping(CCAPI_TRANSPORT_UDP);
    CHECK_EQUAL(CCAPI_PING_ERROR_RESPONSE_CANCEL, error);
}

TEST(test_ccapi_send_ping_no_reply, testSEND_ERROR_RESPONSE_TIMEOUT)
{
    ccapi_ping_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_ping_info.response = connector_sm_ping_response_t::connector_sm_ping_status_timeout;
    }

    error = ccapi_send_ping(CCAPI_TRANSPORT_UDP);
    CHECK_EQUAL(CCAPI_PING_ERROR_RESPONSE_TIMEOUT, error);
}

TEST(test_ccapi_send_ping_no_reply, testSEND_ERROR_RESPONSE_ERROR)
{
    ccapi_ping_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_ping_info.response = connector_sm_ping_response_t::connector_sm_ping_status_error;
    }

    error = ccapi_send_ping(CCAPI_TRANSPORT_UDP);
    CHECK_EQUAL(CCAPI_PING_ERROR_RESPONSE_ERROR, error);
}
