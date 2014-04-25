#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_os)
{
    void setup()
    {
        Mock_create_all();
        th_start_ccapi();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_os, testMalloc)
{
    connector_request_id_t request;
    connector_os_malloc_t malloc_structure = {1024, NULL};
    connector_callback_status_t status;
    void * pre_allocated_buffer = malloc(1024);

    Mock_ccimp_os_malloc_expectAndReturn(malloc_structure.size, pre_allocated_buffer);

    request.os_request = connector_request_id_os_malloc;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &malloc_structure, ccapi_data_single_instance);

    CHECK(status == connector_callback_continue);
    CHECK(malloc_structure.ptr == pre_allocated_buffer);
}

TEST(test_ccapi_os, testFreeOk)
{
    connector_request_id_t request;
    void * pre_allocated_buffer = malloc(1024);
    connector_os_free_t free_structure = {pre_allocated_buffer};
    connector_callback_status_t status;

    Mock_ccimp_os_free_expectAndReturn(pre_allocated_buffer, CCIMP_STATUS_OK);

    request.os_request = connector_request_id_os_free;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &free_structure, ccapi_data_single_instance);
    CHECK(status == connector_callback_continue);
}

TEST(test_ccapi_os, testFreeAbort)
{
    connector_request_id_t request;
    void * pre_allocated_buffer = malloc(1024);
    connector_os_free_t free_structure = {pre_allocated_buffer};
    connector_callback_status_t status;

    Mock_ccimp_os_free_expectAndReturn(pre_allocated_buffer, CCIMP_STATUS_ERROR);

    request.os_request = connector_request_id_os_free;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &free_structure, ccapi_data_single_instance);
    CHECK(status == connector_callback_error);
}

TEST(test_ccapi_os, testYield)
{
    /* Trying to mock ccimp_os_yield() was a complete failure, the function is being called from connector_thread()
     * making it quite difficult to check expectations */
    connector_request_id_t request;
    connector_callback_status_t status;

    request.os_request = connector_request_id_os_yield;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, NULL, ccapi_data_single_instance);
    CHECK(status == connector_callback_continue);
}

TEST(test_ccapi_os, testSystemUptime)
{
    connector_request_id_t request;
    connector_os_system_up_time_t uptime;
    connector_callback_status_t status;

    Mock_ccimp_os_get_system_time_return(0);
    request.os_request = connector_request_id_os_system_up_time;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &uptime, ccapi_data_single_instance);
    CHECK(status == connector_callback_continue);
}

TEST(test_ccapi_os, testReboot)
{
    /* This is not entirely OK, reboot would never return */
    connector_request_id_t request;
    connector_callback_status_t status;

    request.os_request = connector_request_id_os_reboot;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, NULL, ccapi_data_single_instance);
    CHECK(status == connector_callback_continue);
}

TEST(test_ccapi_os, testRealloc)
{
    /* This is not entirely OK, reboot would never return */
    connector_request_id_t request;
    connector_callback_status_t status;
    void * pointer_to_be_reallocated = malloc(512);
    connector_os_realloc_t realloc_info = {512, 1024, pointer_to_be_reallocated};
    uint8_t aux_buffer[1024] = {'A'};

    memcpy(realloc_info.ptr, aux_buffer, 512);
    request.os_request = connector_request_id_os_realloc;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &realloc_info, ccapi_data_single_instance);
    CHECK(status == connector_callback_continue);
    memcpy(realloc_info.ptr, aux_buffer, 1024); /* If realloc is not done, this would throw a SIGSEV */
}
