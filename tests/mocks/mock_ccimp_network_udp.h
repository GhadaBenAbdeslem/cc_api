#ifndef _MOCK_CCIMP_NETWORK_UDP_H_
#define _MOCK_CCIMP_NETWORK_UDP_H_

void Mock_ccimp_network_udp_open_create(void);
void Mock_ccimp_network_udp_open_destroy(void);
void Mock_ccimp_network_udp_open_expectAndReturn(ccimp_network_open_t * expect, ccimp_status_t retval);

void Mock_ccimp_network_udp_send_create(void);
void Mock_ccimp_network_udp_send_destroy(void);
void Mock_ccimp_network_udp_send_expectAndReturn(ccimp_network_send_t * expect, ccimp_status_t retval);

void Mock_ccimp_network_udp_receive_create(void);
void Mock_ccimp_network_udp_receive_destroy(void);
void Mock_ccimp_network_udp_receive_expectAndReturn(ccimp_network_receive_t * expect, ccimp_status_t retval);

void Mock_ccimp_network_udp_close_create(void);
void Mock_ccimp_network_udp_close_destroy(void);
void Mock_ccimp_network_udp_close_expectAndReturn(ccimp_network_close_t * expect, ccimp_status_t retval);

#endif