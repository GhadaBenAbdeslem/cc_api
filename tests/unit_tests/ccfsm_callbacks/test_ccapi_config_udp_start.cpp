#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_config_udp_start)
{
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

TEST(test_ccapi_config_udp_start,testMaxSessions)
{
    connector_request_id_t request;
    connector_config_sm_max_sessions_t max_s = {44};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_sm_udp_max_sessions;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &max_s, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(max_s.max_sessions,ccapi_data_single_instance->transport_udp.info->limit.max_sessions);

}

TEST(test_ccapi_config_udp_start,testRxTimeout)
{
    connector_request_id_t request;
    connector_config_sm_rx_timeout_t rx_t = {99};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_sm_udp_rx_timeout;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &rx_t, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(rx_t.rx_timeout,ccapi_data_single_instance->transport_udp.info->limit.rx_timeout);

}

