/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#include "custom/custom_connector_config.h"

/* +++ Layer2 Digi Controlled defines +++ */
/* Transports */
#define CCIMP_TCP_TRANSPORT_ENABLED
#define CONNECTOR_TRANSPORT_TCP

/* Transports configuration */
#define CONNECTOR_NETWORK_TCP_START connector_connect_manual
#define CONNECTOR_NETWORK_UDP_START connector_connect_manual
#define CONNECTOR_NETWORK_SMS_START connector_connect_manual

#define CONNECTOR_DEVICE_ID_METHOD connector_device_id_method_manual

#undef CONNECTOR_DEVICE_TYPE
#undef CONNECTOR_CLOUD_URL
#undef CONNECTOR_TX_KEEPALIVE_IN_SECONDS
#undef CONNECTOR_RX_KEEPALIVE_IN_SECONDS
#undef CONNECTOR_WAIT_COUNT
#undef CONNECTOR_VENDOR_ID
#undef CONNECTOR_MSG_MAX_TRANSACTION
#undef CONNECTOR_CONNECTION_TYPE
#undef CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND
#undef CONNECTOR_WAN_PHONE_NUMBER_DIALED

#undef CONNECTOR_WAN_TYPE
#undef CONNECTOR_IDENTITY_VERIFICATION

#undef CONNECTOR_CLOUD_PHONE
#undef CONNECTOR_CLOUD_SERVICE_ID

/* Services: Undefine services support so we are asked dinamically */
#undef CONNECTOR_DATA_SERVICE_SUPPORT
#undef CONNECTOR_REMOTE_CONFIGURATION_SUPPORT
#undef CONNECTOR_FIRMWARE_SUPPORT
#undef CONNECTOR_FILE_SYSTEM_SUPPORT

/* We always enable CLI support if any SM transport is enabled */
#if (defined CCIMP_UDP_TRANSPORT_ENABLED || defined CCIMP_SMS_TRANSPORT_ENABLED)
#define CONNECTOR_SM_CLI
#endif

/* Limits */
#undef CONNECTOR_SM_UDP_MAX_SESSIONS
#undef CONNECTOR_SM_SMS_MAX_SESSIONS

#undef CONNECTOR_SM_UDP_RX_TIMEOUT
#undef CONNECTOR_SM_SMS_RX_TIMEOUT

#define CONNECTOR_SM_MULTIPART

#undef CONNECTOR_NO_MALLOC
#undef CONNECTOR_NO_MALLOC_MAX_SEND_SESSIONS
/* --- Layer2 Digi Controlled defines --- */


/* +++ Match Layer2 User Controlled defines to Layer1 defines +++ */
/* Transports */
#ifdef CCIMP_UDP_TRANSPORT_ENABLED
#define CONNECTOR_TRANSPORT_UDP
#endif

#ifdef CCIMP_SMS_TRANSPORT_ENABLED
#define CONNECTOR_TRANSPORT_SMS
#endif

/* Services */
#ifdef CCIMP_DATA_SERVICE_ENABLED
#define CONNECTOR_DATA_SERVICE
#endif

#ifdef CCIMP_DATA_POINTS_ENABLED
#define CONNECTOR_DATA_POINTS
#endif

#ifdef CCIMP_RCI_SERVICE_ENABLED
#define CONNECTOR_RCI_SERVICE
#endif

#ifdef CCIMP_FIRMWARE_SERVICE_ENABLED
#define CONNECTOR_FIRMWARE_SERVICE
#endif

#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
#define CONNECTOR_FILE_SYSTEM
#endif

/* OS Features */
#ifdef CCIMP_LITTLE_ENDIAN
#define CONNECTOR_LITTLE_ENDIAN
#endif

#ifdef CCIMP_COMPRESSION_ENABLED
#define CONNECTOR_COMPRESSION
#endif

#ifdef CCIMP_64_BIT_INTEGERS_SUPPORTED
#define CONNECTOR_SUPPORTS_64_BIT_INTEGERS
#endif

#ifdef CCIMP_FLOATING_POINT_SUPPORTED
#define CONNECTOR_SUPPORTS_FLOATING_POINT
#endif

#ifdef CCIMP_HAS_STDINT_HEADER
#define CONNECTOR_HAVE_STDINT_HEADER
#endif

/* Debugging (Logging / Halt) */
#ifdef CCIMP_DEBUG_ENABLED
#define CONNECTOR_DEBUG
#endif

/* Limits */
#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
#ifdef CCIMP_FILE_SYSTEM_MAX_PATH_LENGTH
#define CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH CCIMP_FILE_SYSTEM_MAX_PATH_LENGTH
#else
#error "You must define CCIMP_FILE_SYSTEM_MAX_PATH_LENGTH in custom_connector_config.h if you have CCIMP_FILE_SYSTEM_SERVICE_ENABLED"
#endif
#endif

#ifdef CCIMP_FILE_SYSTEM_LARGE_FILES_SUPPORTED
#define CONNECTOR_FILE_SYSTEM_HAS_LARGE_FILES
#endif

#ifdef CCIMP_UDP_TRANSPORT_ENABLED
#ifdef CCIMP_SM_UDP_MAX_RX_SEGMENTS
#define CONNECTOR_SM_UDP_MAX_RX_SEGMENTS CCIMP_SM_UDP_MAX_RX_SEGMENTS
#else
#error "You must define CCIMP_SM_UDP_MAX_RX_SEGMENTS in custom_connector_config.h if you have CCIMP_UDP_TRANSPORT_ENABLED"
#endif
#endif

#ifdef CCIMP_SMS_TRANSPORT_ENABLED
#define CONNECTOR_SMS_TRANSPORT_ENABLED
#ifdef CCIMP_SM_SMS_MAX_RX_SEGMENTS
#define CONNECTOR_SM_SMS_MAX_RX_SEGMENTS CCIMP_SM_SMS_MAX_RX_SEGMENTS
#else
#error "You must define CCIMP_SM_SMS_MAX_RX_SEGMENTS in custom_connector_config.h if you have CCIMP_SMS_TRANSPORT_ENABLED"
#endif
#endif

/* --- Match Layer2 User Controlled defines to Layer1 defines --- */
