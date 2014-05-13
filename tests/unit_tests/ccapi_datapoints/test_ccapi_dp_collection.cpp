#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_dp_collection)
{
    void setup()
    {
        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_dp_collection, testCreateCollectionInvalidArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_collection(NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_collection, testClearCollectionInvalidArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_clear_collection(NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyCollectionInvalidArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_destroy_collection(NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_collection, testCreateCollectionNotEnoughMemory)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection == NULL);
}

TEST(test_ccapi_dp_collection, testCreateCollectionSyncrFailed)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, true);
    Mock_ccimp_os_syncr_create_return(CCIMP_STATUS_ERROR);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
    CHECK(dp_collection == NULL);
}

TEST(test_ccapi_dp_collection, testCreateCollectionOk)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, false);
    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);
    CHECK(dp_collection->syncr != NULL);
    CHECK(dp_collection->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_collection, testClearCollectionSyncrFailed)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);

    Mock_ccimp_os_syncr_acquire_return(CCIMP_STATUS_ERROR);
    dp_error = ccapi_dp_clear_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
}

TEST(test_ccapi_dp_collection, testClearEmptyCollection)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);

    dp_error = ccapi_dp_clear_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyEmptyCollectionOk)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyCollectionAcquireFailed)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, false);
    Mock_ccimp_os_syncr_acquire_return(CCIMP_STATUS_ERROR);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
}

TEST_GROUP(test_ccapi_dp_collection_destroy_and_clear)
{
    ccapi_dp_collection_t * dp_collection;
    void setup()
    {
        Mock_create_all();
        ccapi_dp_error_t dp_error;

        th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, false);
        dp_error = ccapi_dp_create_collection(&dp_collection);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
        CHECK(dp_collection != NULL);

        th_expect_malloc(sizeof "string ts_iso", TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof "stream1", TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof "mph", TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof "speed", TH_MALLOC_RETURN_NORMAL, true);

        th_expect_malloc(sizeof "string ts_iso", TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof "stream2", TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof "mph", TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(sizeof "speed", TH_MALLOC_RETURN_NORMAL, true);

        dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, "stream1", CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_ISO8601, "mph", "speed");
        dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, "stream2", CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_ISO8601, "mph", "speed");

        char const * const string_data_1 = "First DP of stream1";
        char const * const string_data_2 = "Second DP of stream1";
        char const * const string_data_3 = "First DP of stream2";
        char const * const timestamp_iso_1 = "2012-01-12T06:16:55.235Z";
        char const * const timestamp_iso_2 = "2012-01-12T06:16:56.235Z";
        char const * const timestamp_iso_3 = "2012-01-12T06:16:57.235Z";
        ccapi_timestamp_t timestamp_1, timestamp_2, timestamp_3;

        timestamp_1.iso8601 = timestamp_iso_1;
        timestamp_2.iso8601 = timestamp_iso_2;
        timestamp_3.iso8601 = timestamp_iso_3;

        th_expect_malloc(sizeof (connector_data_point_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(strlen(string_data_1) + 1, TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(strlen(timestamp_iso_1) + 1, TH_MALLOC_RETURN_NORMAL, true);

        th_expect_malloc(sizeof (connector_data_point_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(strlen(string_data_2) + 1, TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(strlen(timestamp_iso_2) + 1, TH_MALLOC_RETURN_NORMAL, true);

        th_expect_malloc(sizeof (connector_data_point_t), TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(strlen(string_data_3) + 1, TH_MALLOC_RETURN_NORMAL, true);
        th_expect_malloc(strlen(timestamp_iso_3) + 1, TH_MALLOC_RETURN_NORMAL, true);

        dp_error = ccapi_dp_add(dp_collection, "stream1", string_data_1, &timestamp_1);
        dp_error = ccapi_dp_add(dp_collection, "stream1", string_data_2, &timestamp_2);
        dp_error = ccapi_dp_add(dp_collection, "stream2", string_data_3, &timestamp_3);

        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_dp_collection_destroy_and_clear, testClearCollectionOK)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_clear_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection_destroy_and_clear, testDestroyCollectionOk)
{
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_free_expectAndReturn((void*)dp_collection, CCIMP_STATUS_OK);

    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST_GROUP(test_ccapi_dp_collection_destroy_strings)
{
    ccapi_dp_collection_t * dp_collection;
    void setup()
    {
        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_dp_collection_destroy_strings, testDestroyCollectionWithStringDP)
{
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    th_expect_malloc(sizeof "string ts_iso", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof "stream1", TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream1", CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_ISO8601);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection_destroy_strings, testDestroyCollectionWithDataStreamUnitsForward)
{
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    th_expect_malloc(sizeof "string ts_iso", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof "stream1", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof "mph", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof "speed", TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, "stream1", CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_ISO8601, "mph", "speed");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}
