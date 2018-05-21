#ifndef UDP_OP_H
#define UDP_OP_H

#include <netdb.h>

typedef struct udp_peer {
	char *address;
	char *port;
	int sockfd;
	struct addrinfo peer_addr_info;	// Only used in case of client
} udp_peer_t;

udp_peer_t *udp_create_peer(const char *address, const char *port);
void udp_destroy_peer(udp_peer_t *udp_peer);
void udp_create_client_socket(udp_peer_t *udp_peer);
void udp_create_server_socket_and_bind(udp_peer_t *udp_peer);
int udp_send_data(tun_tap_dev_t *dev, char *buf, ssize_t buf_len);
int udp_receive_data(int sockfd, char *buf, ssize_t buf_len);

#endif //UDP_OP_H

