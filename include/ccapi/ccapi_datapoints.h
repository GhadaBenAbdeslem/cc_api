#ifndef _CCAPI_DATAPOINTS_H_
#define _CCAPI_DATAPOINTS_H_

#if (defined CCIMP_DATA_SERVICE_ENABLED) && (defined CCIMP_DATA_POINTS_ENABLED)

#define CCAPI_NO_ELEVATION   -6378000 /* Negative maximum Earth's radio */

#define CCAPI_DP_KEY_DATA_INT32     "int32"
#define CCAPI_DP_KEY_DATA_INT64     "int64"
#define CCAPI_DP_KEY_DATA_FLOAT     "float"
#define CCAPI_DP_KEY_DATA_DOUBLE    "double"
#define CCAPI_DP_KEY_DATA_STRING    "string"

#define CCAPI_DP_KEY_TS_EPOCH       "ts_epoch"
#define CCAPI_DP_KEY_TS_EPOCHMS     "ts_epoch_ms"
#define CCAPI_DP_KEY_TS_ISO8601     "ts_iso"

#define CCAPI_DP_KEY_LOCATION       "loc"
#define CCAPI_DP_KEY_QUALITY        "qual"

typedef enum {
    CCAPI_DP_ERROR_NONE,
    CCAPI_DP_ERROR_INVALID_ARGUMENT,
    CCAPI_DP_ERROR_INVALID_STREAM_ID,
    CCAPI_DP_ERROR_INVALID_FORMAT,
    CCAPI_DP_ERROR_INVALID_UNITS,
    CCAPI_DP_ERROR_INVALID_FORWARD_TO,
    CCAPI_DP_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_DP_ERROR_SYNCR_FAILED
} ccapi_dp_error_t;

typedef struct {
    float latitude;
    float longitude;
    float elevation;
} ccapi_location_t;

typedef union {
    struct {
        uint32_t seconds;
        uint32_t milliseconds;
    } epoch;
    uint64_t epoch_msec;
    char const * iso8601;
} ccapi_timestamp_t;


typedef struct ccapi_dp_collection * ccapi_dp_collection_handle_t;
typedef struct ccapi_dp_data_stream * ccapi_dp_data_stream_handle_t;

ccapi_dp_error_t ccapi_dp_create_collection(ccapi_dp_collection_handle_t * const dp_collection);
ccapi_dp_error_t ccapi_dp_clear_collection(ccapi_dp_collection_handle_t const dp_collection);
ccapi_dp_error_t ccapi_dp_destroy_collection(ccapi_dp_collection_handle_t const dp_collection);
ccapi_dp_error_t ccapi_dp_send_collection(ccapi_transport_t transport, ccapi_dp_collection_handle_t const dp_collection);

ccapi_dp_error_t ccapi_dp_add_data_stream_to_collection(ccapi_dp_collection_handle_t const dp_collection, char const * const stream_id, char const * const format_string);
ccapi_dp_error_t ccapi_dp_add_data_stream_to_collection_extra(ccapi_dp_collection_handle_t const dp_collection, char const * const stream_id, char const * const format_string, char const * const units, char const * const forward_to);
ccapi_dp_error_t ccapi_dp_remove_data_stream_from_collection(ccapi_dp_collection_handle_t const dp_collection, char const * const stream_id);

ccapi_dp_error_t ccapi_dp_add(ccapi_dp_collection_handle_t const collection, char const * const stream_id, ...);
#endif

#endif
