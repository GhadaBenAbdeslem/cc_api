#include "test_helper_functions.h"

#define TEST_TARGET "my_target"
#define RESPONSE  { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, \
                    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f }

static char const * ccapi_receive_status_expected_target = NULL;
static ccapi_transport_t ccapi_receive_status_expected_transport = CCAPI_TRANSPORT_TCP;
static ccapi_bool_t ccapi_receive_status_cb_called = CCAPI_FALSE;
static ccapi_buffer_info_t * ccapi_receive_status_expected_response = NULL;
static ccapi_receive_error_t ccapi_receive_status_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

void clean_ccapi_receive_status_data(void)
{
    ccapi_receive_status_expected_target = NULL;
    ccapi_receive_status_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_status_cb_called = CCAPI_FALSE;
    ccapi_receive_status_expected_response = NULL;
    ccapi_receive_status_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;
}

static void test_receive_data_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{
    (void)target;
    (void)transport;
    (void)request_buffer_info;
    (void)response_buffer_info;
    (void)receive_error;

    return;
}

static void test_receive_status_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{

    STRCMP_EQUAL(ccapi_receive_status_expected_target, target);
    CHECK_EQUAL(ccapi_receive_status_expected_transport, transport);

    if (ccapi_receive_status_expected_response != NULL)
    {
        CHECK_EQUAL(ccapi_receive_status_expected_response->buffer, response_buffer_info->buffer);
		CHECK_EQUAL(ccapi_receive_status_expected_response->length, response_buffer_info->length);
    }
    
    CHECK_EQUAL(ccapi_receive_status_expected_receive_error, receive_error);

    ccapi_receive_status_cb_called = CCAPI_TRUE;

    return;
}

TEST_GROUP(test_ccapi_receive_status_callback)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_receive_service_t receive_service = {NULL, test_receive_data_cb, test_receive_status_cb};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.receive = &receive_service;

        clean_ccapi_receive_status_data();

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
        CHECK_EQUAL(receive_service.data_cb, ccapi_data_single_instance->service.receive.user_callbacks.data_cb);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_receive_status_callback, testStatusOK_NoResponse)
{
    connector_request_id_t request;
    connector_data_service_status_t ccfsm_receive_status_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)malloc(sizeof *svc_receive);
    char * target = (char *)malloc(sizeof(TEST_TARGET));
    memcpy(target, TEST_TARGET, sizeof(TEST_TARGET));
    svc_receive->target = target;
    svc_receive->user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive->user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive->response_required = CCAPI_TRUE;  
    svc_receive->response_buffer_info.buffer = NULL;
    svc_receive->response_buffer_info.length = 0;
    svc_receive->receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccapi_receive_status_expected_target = TEST_TARGET;
    ccapi_receive_status_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_status_expected_response = NULL;
    ccapi_receive_status_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_status_data.transport = connector_transport_tcp;
    ccfsm_receive_status_data.user_context = svc_receive;
    ccfsm_receive_status_data.status = connector_data_service_status_t::connector_data_service_status_complete;

    request.data_service_request = connector_request_id_data_service_receive_status;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_status_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_status_data.user_context == svc_receive);
    {
        CHECK_EQUAL(svc_receive->receive_error, CCAPI_RECEIVE_ERROR_NONE);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_status_cb_called);
}

TEST(test_ccapi_receive_status_callback, testStatusOK_WithResponse)
{
    connector_request_id_t request;
    connector_data_service_status_t ccfsm_receive_status_data;
    connector_callback_status_t status;

    uint8_t const exp_response[] = RESPONSE;
    ccapi_buffer_info_t expected_response;

    ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)malloc(sizeof *svc_receive);
    char * target = (char *)malloc(sizeof(TEST_TARGET));
    memcpy(target, TEST_TARGET, sizeof(TEST_TARGET));
    svc_receive->target = target;
    svc_receive->user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive->user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive->response_required = CCAPI_TRUE;  
    svc_receive->response_buffer_info.buffer = (void *)exp_response;
    svc_receive->response_buffer_info.length = sizeof exp_response;
    svc_receive->receive_error = CCAPI_RECEIVE_ERROR_NONE;

    expected_response.buffer = (void *)exp_response;
    expected_response.length = sizeof exp_response;

    ccapi_receive_status_expected_target = TEST_TARGET;
    ccapi_receive_status_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_status_expected_response = &expected_response;
    ccapi_receive_status_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_status_data.transport = connector_transport_tcp;
    ccfsm_receive_status_data.user_context = svc_receive;
    ccfsm_receive_status_data.status = connector_data_service_status_t::connector_data_service_status_complete;

    request.data_service_request = connector_request_id_data_service_receive_status;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_status_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_status_data.user_context == svc_receive);
    {
        CHECK_EQUAL(svc_receive->receive_error, CCAPI_RECEIVE_ERROR_NONE);


        CHECK(svc_receive->response_buffer_info.buffer != NULL);
        CHECK(svc_receive->response_buffer_info.length == sizeof exp_response);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_status_cb_called);
}

TEST(test_ccapi_receive_status_callback, testERROR_STATUS_CANCEL)
{
    connector_request_id_t request;
    connector_data_service_status_t ccfsm_receive_status_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)malloc(sizeof *svc_receive);
    char * target = (char *)malloc(sizeof(TEST_TARGET));
    memcpy(target, TEST_TARGET, sizeof(TEST_TARGET));
    svc_receive->target = target;
    svc_receive->user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive->user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive->response_required = CCAPI_TRUE;  
    svc_receive->response_buffer_info.buffer = NULL;
    svc_receive->response_buffer_info.length = 0;
    svc_receive->receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccapi_receive_status_expected_target = TEST_TARGET;
    ccapi_receive_status_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_status_expected_response = NULL;
    ccapi_receive_status_expected_receive_error = CCAPI_RECEIVE_ERROR_STATUS_CANCEL;

    ccfsm_receive_status_data.transport = connector_transport_tcp;
    ccfsm_receive_status_data.user_context = svc_receive;
    ccfsm_receive_status_data.status = connector_data_service_status_t::connector_data_service_status_cancel;

    request.data_service_request = connector_request_id_data_service_receive_status;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_status_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_status_data.user_context == svc_receive);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_status_cb_called);
}

TEST(test_ccapi_receive_status_callback, testERROR_STATUS_TIMEOUT)
{
    connector_request_id_t request;
    connector_data_service_status_t ccfsm_receive_status_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)malloc(sizeof *svc_receive);
    char * target = (char *)malloc(sizeof(TEST_TARGET));
    memcpy(target, TEST_TARGET, sizeof(TEST_TARGET));
    svc_receive->target = target;
    svc_receive->user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive->user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive->response_required = CCAPI_TRUE;  
    svc_receive->response_buffer_info.buffer = NULL;
    svc_receive->response_buffer_info.length = 0;
    svc_receive->receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccapi_receive_status_expected_target = TEST_TARGET;
    ccapi_receive_status_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_status_expected_response = NULL;
    ccapi_receive_status_expected_receive_error = CCAPI_RECEIVE_ERROR_STATUS_TIMEOUT;

    ccfsm_receive_status_data.transport = connector_transport_tcp;
    ccfsm_receive_status_data.user_context = svc_receive;
    ccfsm_receive_status_data.status = connector_data_service_status_t::connector_data_service_status_timeout;

    request.data_service_request = connector_request_id_data_service_receive_status;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_status_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_status_data.user_context == svc_receive);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_status_cb_called);
}

TEST(test_ccapi_receive_status_callback, testERROR_STATUS_SESSION_ERROR)
{
    connector_request_id_t request;
    connector_data_service_status_t ccfsm_receive_status_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)malloc(sizeof *svc_receive);
    char * target = (char *)malloc(sizeof(TEST_TARGET));
    memcpy(target, TEST_TARGET, sizeof(TEST_TARGET));
    svc_receive->target = target;
    svc_receive->user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive->user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive->response_required = CCAPI_TRUE;  
    svc_receive->response_buffer_info.buffer = NULL;
    svc_receive->response_buffer_info.length = 0;
    svc_receive->receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccapi_receive_status_expected_target = TEST_TARGET;
    ccapi_receive_status_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_status_expected_response = NULL;
    ccapi_receive_status_expected_receive_error = CCAPI_RECEIVE_ERROR_STATUS_SESSION_ERROR;

    ccfsm_receive_status_data.transport = connector_transport_tcp;
    ccfsm_receive_status_data.user_context = svc_receive;
    ccfsm_receive_status_data.status = connector_data_service_status_t::connector_data_service_status_session_error;

    request.data_service_request = connector_request_id_data_service_receive_status;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_status_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_status_data.user_context == svc_receive);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_status_cb_called);
}