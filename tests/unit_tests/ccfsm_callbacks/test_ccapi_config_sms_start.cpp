#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_config_sms_start)
{
    void setup()
    {

        Mock_create_all();

        th_start_ccapi();
        th_start_sms();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);
        Mock_destroy_all();
    }
};

TEST(test_ccapi_config_sms_start,testMaxSessions)
{
    connector_request_id_t request;
    connector_config_sm_max_sessions_t max_s = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_sm_sms_max_sessions;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &max_s, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(max_s.max_sessions,ccapi_data_single_instance->transport_sms.info->limit.max_sessions);

}

TEST(test_ccapi_config_sms_start,testRxTimeout)
{
    connector_request_id_t request;
    connector_config_sm_rx_timeout_t rx_t = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_sm_sms_rx_timeout;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &rx_t, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(rx_t.rx_timeout,ccapi_data_single_instance->transport_sms.info->limit.rx_timeout);

}

TEST(test_ccapi_config_sms_start,testPhoneNumber)
{
    connector_request_id_t request;
    connector_config_pointer_string_t phone = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_get_device_cloud_phone;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &phone, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(strlen(ccapi_data_single_instance->transport_sms.info->cloud_config.phone_number), phone.length);
    CHECK_EQUAL(ccapi_data_single_instance->transport_sms.info->cloud_config.phone_number, phone.string);
    STRCMP_EQUAL(ccapi_data_single_instance->transport_sms.info->cloud_config.phone_number, phone.string);

}

TEST(test_ccapi_config_sms_start,testServiceID)
{
    connector_request_id_t request;
    connector_config_pointer_string_t id = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_device_cloud_service_id;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &id, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(strlen(ccapi_data_single_instance->transport_sms.info->cloud_config.service_id), id.length);
    CHECK_EQUAL(ccapi_data_single_instance->transport_sms.info->cloud_config.service_id, id.string);
    STRCMP_EQUAL(ccapi_data_single_instance->transport_sms.info->cloud_config.service_id, id.string);

}