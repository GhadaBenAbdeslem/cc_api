#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"
#include "mocks/mock_ccimp_os.h"
#include "mocks/mock_connector_api.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "internal/ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

using namespace std;

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

static void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[DEVICE_ID_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
    start->vendor_id = 0x12345678; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    memcpy(start->device_id, device_id, sizeof start->device_id);
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = NULL;
    start->service.rci = NULL;
}



TEST_GROUP(ccapi_config_test)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_init_error_t error;
        void * malloc_for_ccapi_config = malloc(sizeof (ccapi_config_t));
        void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
        void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
        connector_handle_t handle = &handle; /* Not-NULL */

        Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_config_t), malloc_for_ccapi_config);
        Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
        Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
        Mock_connector_init_expectAndReturn(ccapi_connector_callback, handle);

        fill_start_structure_with_good_parameters(&start);
        error = ccapi_start(&start);
        CHECK(error == CCAPI_INIT_ERROR_NONE);
    }

    void teardown()
    {
        Mock_ccimp_malloc_destroy();
    }
};

TEST(ccapi_config_test, testDeviceID)
{
    connector_request_id_t request;
    uint8_t device_id_buf[DEVICE_ID_LENGTH] = {0};
    connector_config_pointer_data_t device_id = {NULL, DEVICE_ID_LENGTH};

    device_id.data = device_id_buf;

    request.config_request = connector_request_id_config_device_id;
    ccapi_connector_callback(connector_class_id_config, request, &device_id);
    CHECK(memcmp(ccapi_config->device_id, device_id_buf, device_id.bytes_required) == 0);
}

TEST(ccapi_config_test, testCloudURL)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_cloud_url = {0};

    request.config_request = connector_request_id_config_device_cloud_url;
    ccapi_connector_callback(connector_class_id_config, request, &device_cloud_url);
    STRCMP_EQUAL(device_cloud_url.string, ccapi_config->device_cloud_url);
    CHECK(strlen(ccapi_config->device_cloud_url) == device_cloud_url.length);
}

TEST(ccapi_config_test, testVendorID)
{
    connector_request_id_t request;
    connector_config_vendor_id_t vendor_id = {0};

    request.config_request = connector_request_id_config_vendor_id;
    ccapi_connector_callback(connector_class_id_config, request, &vendor_id);
    CHECK(vendor_id.id == ccapi_config->vendor_id);
}

TEST(ccapi_config_test, testDeviceType)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_type = {0};

    request.config_request = connector_request_id_config_device_type;
    ccapi_connector_callback(connector_class_id_config, request, &device_type);
    STRCMP_EQUAL(device_type.string, ccapi_config->device_type);
    CHECK(strlen(ccapi_config->device_type) == device_type.length);
}