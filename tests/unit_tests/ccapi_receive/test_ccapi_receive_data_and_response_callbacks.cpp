#include "test_helper_functions.h"

#define TEST_TARGET "my_target"
#define DATA  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, \
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f }

#define DATA2 { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, \
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, \
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, \
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f }

#define RESPONSE  { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, \
                    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f }

static char const * ccapi_receive_data_expected_target = NULL;
static ccapi_transport_t ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
static ccapi_bool_t ccapi_receive_data_cb_called = CCAPI_FALSE;

static ccapi_buffer_info_t * ccapi_receive_data_expected_request = NULL;
static ccapi_buffer_info_t * ccapi_receive_data_expected_response = NULL;
static ccapi_receive_error_t ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;
static ccapi_buffer_info_t * ccapi_receive_data_desired_response = NULL;

void clean_ccapi_receive_data(void)
{
    ccapi_receive_data_expected_target = NULL;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_data_expected_request = NULL;
    ccapi_receive_data_expected_response = NULL;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;
    ccapi_receive_data_cb_called = CCAPI_FALSE;
    ccapi_receive_data_desired_response = NULL;
}

static void test_receive_data_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{
    STRCMP_EQUAL(ccapi_receive_data_expected_target, target);
    CHECK_EQUAL(ccapi_receive_data_expected_transport, transport);
    if (ccapi_receive_data_expected_request != NULL)
    {
        CHECK_EQUAL(ccapi_receive_data_expected_request->length, request_buffer_info->length);
        CHECK(memcmp(ccapi_receive_data_expected_request->buffer, request_buffer_info->buffer, request_buffer_info->length) == 0);
    }
    CHECK_EQUAL(ccapi_receive_data_expected_response, response_buffer_info);
    CHECK_EQUAL(ccapi_receive_data_expected_receive_error, receive_error);

    if (ccapi_receive_data_desired_response != NULL)
    {
        response_buffer_info->buffer = malloc(ccapi_receive_data_desired_response->length);
        memcpy(response_buffer_info->buffer, ccapi_receive_data_desired_response->buffer, ccapi_receive_data_desired_response->length);
        response_buffer_info->length = ccapi_receive_data_desired_response->length;
    }

    ccapi_receive_data_cb_called = CCAPI_TRUE;

    return;
}

TEST_GROUP(test_ccapi_receive_data_callback_NoReceiveSupport)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);

        clean_ccapi_receive_data();

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_receive_data_callback_NoReceiveSupport, testNoReceiveSupport)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_data_service_receive_reply_data_t ccfsm_receive_reply_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;

    #define MAX_RESPONSE_SIZE 100
    uint8_t response[MAX_RESPONSE_SIZE];

    svc_receive.target = (char*)target;
    svc_receive.response_required = CCAPI_TRUE;
    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = NULL;
    ccfsm_receive_data_data.bytes_used = 0;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);

    ccfsm_receive_reply_data.transport = connector_transport_tcp;
    ccfsm_receive_reply_data.user_context = &svc_receive;
    ccfsm_receive_reply_data.buffer = response;
    ccfsm_receive_reply_data.bytes_available = MAX_RESPONSE_SIZE;
    ccfsm_receive_reply_data.bytes_used = 0;
    ccfsm_receive_reply_data.more_data = connector_true;
    request.data_service_request = connector_request_id_data_service_receive_reply_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_reply_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context != NULL);
    CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NO_RECEIVE_SUPPORT);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_receive_data_cb_called);
}

TEST_GROUP(test_ccapi_receive_data_callback_MissingDataCallback)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_receive_service_t receive_service = {NULL, NULL, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.receive = &receive_service;

        clean_ccapi_receive_data();

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_INVALID_RECEIVE_DATA_CB);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_receive_data_callback_MissingDataCallback, testWeNotAllowNullDataCallback)
{
}

TEST_GROUP(test_ccapi_receive_data_callback)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_receive_service_t receive_service = {NULL, test_receive_data_cb, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.receive = &receive_service;

        clean_ccapi_receive_data();

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
        CHECK_EQUAL(receive_service.data_cb, ccapi_data_single_instance->service.receive.user_callbacks.data_cb);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_receive_data_callback, testUserContextNull)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = NULL;
    ccfsm_receive_data_data.buffer = NULL;
    ccfsm_receive_data_data.bytes_used = 0;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);

    CHECK(ccfsm_receive_data_data.user_context == NULL);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_BufferNULL)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive.user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive.response_required = CCAPI_TRUE;
    
    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_data_expected_request = &svc_receive.request_buffer_info;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = NULL;
    ccfsm_receive_data_data.bytes_used = 0;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)ccfsm_receive_data_data.user_context;
        CHECK_EQUAL(svc_receive->receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive->request_buffer_info.buffer != NULL); /* Should be? */
        CHECK(svc_receive->request_buffer_info.length == 0);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_OneDataCallOneResponseCall)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_data_service_receive_reply_data_t ccfsm_receive_reply_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;
    uint8_t const exp_response[] = RESPONSE;
    ccapi_buffer_info_t expected_request;
    ccapi_buffer_info_t expected_response;

    #define MAX_RESPONSE_SIZE 100
    uint8_t response[MAX_RESPONSE_SIZE];

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive.user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive.max_request_size = CCAPI_RECEIVE_NO_LIMIT;
    svc_receive.response_required = CCAPI_TRUE;  

    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    expected_request.buffer = (void *)data;
    expected_request.length = sizeof data;
    ccapi_receive_data_expected_request = &expected_request;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    expected_response.buffer = (void *)exp_response;
    expected_response.length = sizeof exp_response;
    ccapi_receive_data_desired_response = &expected_response;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    ccfsm_receive_reply_data.transport = connector_transport_tcp;
    ccfsm_receive_reply_data.user_context = &svc_receive;
    ccfsm_receive_reply_data.buffer = response;
    ccfsm_receive_reply_data.bytes_available = MAX_RESPONSE_SIZE;
    ccfsm_receive_reply_data.bytes_used = 0;
    ccfsm_receive_reply_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_reply_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_reply_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == sizeof data);

        CHECK(svc_receive.response_buffer_info.buffer != NULL);
        CHECK(svc_receive.response_buffer_info.length == sizeof exp_response);
        CHECK(svc_receive.response_processing.length == 0);

        CHECK(memcmp(response, exp_response, sizeof exp_response) == 0);
        CHECK(ccfsm_receive_reply_data.bytes_used == sizeof exp_response);
        CHECK(ccfsm_receive_reply_data.more_data == connector_false);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_TwoDataCalls)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;
    uint8_t const data2[] = DATA2;
    ccapi_buffer_info_t expected_request;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive.user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive.max_request_size = CCAPI_RECEIVE_NO_LIMIT;
    svc_receive.response_required = CCAPI_TRUE;
    
    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    expected_request.buffer = (void *)data2;
    expected_request.length = sizeof data2;
    ccapi_receive_data_expected_request = &expected_request;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);

    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == 2 * sizeof data);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_ResponseNotRequired)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;
    ccapi_buffer_info_t expected_request;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive.user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive.max_request_size = CCAPI_RECEIVE_NO_LIMIT;
    svc_receive.response_required = CCAPI_FALSE;
    
    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    expected_request.buffer = (void *)data;
    expected_request.length = sizeof data;
    ccapi_receive_data_expected_request = &expected_request;
    ccapi_receive_data_expected_response = NULL;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == sizeof data);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_OneDataCallTwoResponseCall)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_data_service_receive_reply_data_t ccfsm_receive_reply_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;
    uint8_t const exp_response[] = RESPONSE;
    ccapi_buffer_info_t expected_request;
    ccapi_buffer_info_t expected_response;

    #define MAX_RESPONSE_SIZE 100
    uint8_t response[MAX_RESPONSE_SIZE];

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive.user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive.max_request_size = CCAPI_RECEIVE_NO_LIMIT;
    svc_receive.response_required = CCAPI_TRUE;  

    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    expected_request.buffer = (void *)data;
    expected_request.length = sizeof data;
    ccapi_receive_data_expected_request = &expected_request;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    expected_response.buffer = (void *)exp_response;
    expected_response.length = sizeof exp_response;
    ccapi_receive_data_desired_response = &expected_response;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    ccfsm_receive_reply_data.transport = connector_transport_tcp;
    ccfsm_receive_reply_data.user_context = &svc_receive;
    ccfsm_receive_reply_data.buffer = response;
    ccfsm_receive_reply_data.bytes_available = sizeof exp_response / 2;
    ccfsm_receive_reply_data.bytes_used = 0;
    ccfsm_receive_reply_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_reply_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_reply_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    ccfsm_receive_reply_data.buffer = ((uint8_t*)response) + sizeof exp_response / 2;
    ccfsm_receive_reply_data.bytes_available = sizeof exp_response / 2;
    ccfsm_receive_reply_data.bytes_used = 0;
    ccfsm_receive_reply_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_reply_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_reply_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == sizeof data);

        CHECK(svc_receive.response_buffer_info.buffer != NULL);
        CHECK(svc_receive.response_buffer_info.length == sizeof exp_response);
        CHECK(svc_receive.response_processing.length == 0);

        CHECK(memcmp(response, exp_response, sizeof exp_response) == 0);
        CHECK(ccfsm_receive_reply_data.bytes_used == sizeof exp_response / 2);
        CHECK(ccfsm_receive_reply_data.more_data == connector_false);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_OneDataCallTwoResponseCall_NoRoom)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_data_service_receive_reply_data_t ccfsm_receive_reply_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;
    uint8_t const exp_response[] = RESPONSE;
    ccapi_buffer_info_t expected_request;
    ccapi_buffer_info_t expected_response;

    #define MAX_RESPONSE_SIZE 100
    uint8_t response[MAX_RESPONSE_SIZE];

    #define NUM_BYTES_NO_ROOM  1 /* No room for one byte of the response */

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive.user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive.max_request_size = CCAPI_RECEIVE_NO_LIMIT;
    svc_receive.response_required = CCAPI_TRUE;  

    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    expected_request.buffer = (void *)data;
    expected_request.length = sizeof data;
    ccapi_receive_data_expected_request = &expected_request;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    expected_response.buffer = (void *)exp_response;
    expected_response.length = sizeof exp_response;
    ccapi_receive_data_desired_response = &expected_response;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    ccfsm_receive_reply_data.transport = connector_transport_tcp;
    ccfsm_receive_reply_data.user_context = &svc_receive;
    ccfsm_receive_reply_data.buffer = response;
    ccfsm_receive_reply_data.bytes_available = sizeof exp_response / 2;
    ccfsm_receive_reply_data.bytes_used = 0;
    ccfsm_receive_reply_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_reply_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_reply_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    ccfsm_receive_reply_data.buffer = ((uint8_t*)response) + sizeof exp_response / 2;
    ccfsm_receive_reply_data.bytes_available = sizeof exp_response / 2 - NUM_BYTES_NO_ROOM;
    ccfsm_receive_reply_data.bytes_used = 0;
    ccfsm_receive_reply_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_reply_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_reply_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == sizeof data);

        CHECK(svc_receive.response_buffer_info.buffer != NULL);
        CHECK(svc_receive.response_buffer_info.length == sizeof exp_response);
        CHECK(svc_receive.response_processing.length == NUM_BYTES_NO_ROOM);

        CHECK(memcmp(response, exp_response, sizeof exp_response - NUM_BYTES_NO_ROOM) == 0);
        CHECK(ccfsm_receive_reply_data.bytes_used == sizeof exp_response / 2 - NUM_BYTES_NO_ROOM);
        CHECK(ccfsm_receive_reply_data.more_data == connector_true);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testRequestTooBig)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_data_service_receive_reply_data_t ccfsm_receive_reply_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;
    uint8_t const exp_response[] = RESPONSE;
    ccapi_buffer_info_t expected_response;

    #define MAX_RESPONSE_SIZE 100
    uint8_t response[MAX_RESPONSE_SIZE];

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;

    svc_receive.user_callbacks.data_cb = ccapi_data_single_instance->service.receive.user_callbacks.data_cb;
    svc_receive.user_callbacks.status_cb = ccapi_data_single_instance->service.receive.user_callbacks.status_cb;
    svc_receive.max_request_size = 5; /* Set limit here instead of calling ccapi_receive_add_target() */
    svc_receive.response_required = CCAPI_TRUE;  

    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_data_expected_request = NULL;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_REQUEST_TOO_BIG;

    expected_response.buffer = (void *)exp_response;
    expected_response.length = sizeof exp_response;
    ccapi_receive_data_desired_response = &expected_response;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);

    ccfsm_receive_reply_data.transport = connector_transport_tcp;
    ccfsm_receive_reply_data.user_context = &svc_receive;
    ccfsm_receive_reply_data.buffer = response;
    ccfsm_receive_reply_data.bytes_available = MAX_RESPONSE_SIZE;
    ccfsm_receive_reply_data.bytes_used = 0;
    ccfsm_receive_reply_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_reply_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_reply_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_REQUEST_TOO_BIG);

        CHECK(svc_receive.request_buffer_info.buffer == NULL);

        CHECK(svc_receive.response_buffer_info.buffer != NULL);
        CHECK(svc_receive.response_buffer_info.length == sizeof exp_response);
        CHECK(svc_receive.response_processing.length == 0);

        CHECK(memcmp(response, exp_response, sizeof exp_response) == 0);
        CHECK(ccfsm_receive_reply_data.bytes_used == sizeof exp_response);
        CHECK(ccfsm_receive_reply_data.more_data == connector_false);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}