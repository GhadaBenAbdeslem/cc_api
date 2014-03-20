#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_config_test)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;

        Mock_create_all();

        fill_start_structure_with_good_parameters(&start);
        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(ccapi_config_test, testDeviceID)
{
    connector_request_id_t request;
    uint8_t device_id_buf[DEVICE_ID_LENGTH] = {0};
    connector_config_pointer_data_t device_id = {NULL, DEVICE_ID_LENGTH};

    device_id.data = device_id_buf;

    request.config_request = connector_request_id_config_device_id;
    ccapi_connector_callback(connector_class_id_config, request, &device_id, (*spy_ccapi_data));
    CHECK(memcmp((*spy_ccapi_data)->config.device_id, device_id_buf, device_id.bytes_required) == 0);
}

TEST(ccapi_config_test, testCloudURL)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_cloud_url = {0};

    request.config_request = connector_request_id_config_device_cloud_url;
    ccapi_connector_callback(connector_class_id_config, request, &device_cloud_url, (*spy_ccapi_data));
    STRCMP_EQUAL(device_cloud_url.string, (*spy_ccapi_data)->config.device_cloud_url);
    CHECK(strlen((*spy_ccapi_data)->config.device_cloud_url) == device_cloud_url.length);
}

TEST(ccapi_config_test, testVendorID)
{
    connector_request_id_t request;
    connector_config_vendor_id_t vendor_id = {0};

    request.config_request = connector_request_id_config_vendor_id;
    ccapi_connector_callback(connector_class_id_config, request, &vendor_id, (*spy_ccapi_data));
    CHECK(vendor_id.id == (*spy_ccapi_data)->config.vendor_id);
}

TEST(ccapi_config_test, testDeviceType)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_type = {0};

    request.config_request = connector_request_id_config_device_type;
    ccapi_connector_callback(connector_class_id_config, request, &device_type, (*spy_ccapi_data));
    STRCMP_EQUAL(device_type.string, (*spy_ccapi_data)->config.device_type);
    CHECK(strlen((*spy_ccapi_data)->config.device_type) == device_type.length);
}

TEST(ccapi_config_test, testFirmwareSupport)
{
    connector_request_id_t request;
    connector_config_supported_t firmware_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_firmware_facility;
    ccapi_connector_callback(connector_class_id_config, request, &firmware_supported, (*spy_ccapi_data));
    CHECK(firmware_supported.supported == connector_false);
}

TEST(ccapi_config_test, testFileSystemSupport)
{
    connector_request_id_t request;
    connector_config_supported_t filesystem_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_file_system;
    ccapi_connector_callback(connector_class_id_config, request, &filesystem_supported, (*spy_ccapi_data));
    CHECK(filesystem_supported.supported == connector_false);
}

TEST(ccapi_config_test, testRCISupport)
{
    connector_request_id_t request;
    connector_config_supported_t rci_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_remote_configuration;
    ccapi_connector_callback(connector_class_id_config, request, &rci_supported, (*spy_ccapi_data));
    CHECK(rci_supported.supported == connector_false);
}
