#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_status_handler)
{
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
        th_start_tcp_wan_ipv4_with_callbacks();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

TEST(test_ccapi_status_handler, testStatusTCPStarted)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_communication_started};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(test_ccapi_status_handler, testStatusTCPKeepaliveMissed)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_keepalive_missed};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_keepalives_cb_called);
    CHECK_EQUAL(CCAPI_KEEPALIVE_MISSED, ccapi_tcp_keepalives_cb_argument);
}

TEST(test_ccapi_status_handler, testStatusTCPKeepaliveRestored)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_keepalive_restored};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->transport_tcp.connected);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_keepalives_cb_called);
    CHECK_EQUAL(CCAPI_KEEPALIVE_RESTORED, ccapi_tcp_keepalives_cb_argument);
}

TEST(test_ccapi_status_handler, testStatusStop)
{
    connector_request_id_t request;
    connector_initiate_stop_request_t stop_status = {connector_transport_tcp, connector_stop_immediately, NULL};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_stop_completed;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &stop_status, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, connector_status);
}
