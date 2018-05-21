#include "common.h"
#include "tun_tap.h"
#include "vxlan.h"
#include "udp_op.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

udp_peer_t *udp_create_peer(const char *address, const char *port)
{
	udp_peer_t *udp_peer = (udp_peer_t *)calloc(sizeof(udp_peer_t), 1);
	if (udp_peer == NULL)
	{
		printf("Failed to allocate memory for udp_peer\n");
		return NULL;
	}

	udp_peer->address = (char *)malloc(strlen(address) + 1);
	if (udp_peer->address == NULL)
	{
		printf("Failed to allocate memory for udp peer address\n");
		free(udp_peer);
		return NULL;
	}
	strncpy(udp_peer->address, address, strlen(address));

	udp_peer->port = (char *)malloc(strlen(port) + 1);
	if (udp_peer->port == NULL)
	{
		printf("Failed to allocate memory for udp peer port\n");
		free(udp_peer->address);
		free(udp_peer);
		return NULL;
	}
	strncpy(udp_peer->port, port, strlen(port));

	return udp_peer;
}

void udp_destroy_peer(udp_peer_t *udp_peer)
{
	if (udp_peer == NULL)
	{
		return;
	}

	close(udp_peer->sockfd);

	free(udp_peer->address);
	free(udp_peer->port);
	free(udp_peer);
}

void udp_create_client_socket(udp_peer_t *udp_peer)
{
	int sockfd = 0;
	struct addrinfo hints, *servinfo = NULL, *ptr = NULL;
	int rc = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rc = getaddrinfo(udp_peer->address, udp_peer->port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
		return;
	}

	for (ptr = servinfo; ptr != NULL; ptr = ptr->ai_next)
	{
		if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
		{
			continue;
		}
		break;
	}

	if (ptr == NULL)
	{
		printf("udp_create_client_socket: Failed to create socket\n");
		return;
	}

	udp_peer->sockfd = sockfd;
	(udp_peer->peer_addr_info).ai_addr = ptr->ai_addr;
	(udp_peer->peer_addr_info).ai_addrlen = ptr->ai_addrlen;
}

void udp_create_server_socket_and_bind(udp_peer_t *udp_peer)
{
	int sockfd = 0, rc = 0;
	struct addrinfo hints, *servinfo = NULL, *ptr = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rc = getaddrinfo(udp_peer->address, udp_peer->port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
		return;
	}

	for (ptr = servinfo; ptr != NULL; ptr = ptr->ai_next)
	{
		if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
		{
			continue;
		}

		rc = bind(sockfd, ptr->ai_addr, ptr->ai_addrlen);
		if (rc == -1)
		{
			fprintf(stderr, "Error in bind socket: %s\n", strerror(errno));
			close(sockfd);
			continue;
		}

		break;
	}

	if (ptr == NULL)
	{
		printf("udp_create_server_socket: Failed to create/bind socket\n");
		return;
	}

	udp_peer->sockfd = sockfd;
	(udp_peer->peer_addr_info).ai_addr = ptr->ai_addr;
	(udp_peer->peer_addr_info).ai_addrlen = ptr->ai_addrlen;
}

int udp_send_data(tun_tap_dev_t *dev, char *buf, ssize_t buf_len)
{
	int numbytes = 0;

	numbytes = sendto(
			(dev->udp_peer_remote)->sockfd, buf, buf_len, 0,
			((dev->udp_peer_remote)->peer_addr_info).ai_addr,
			((dev->udp_peer_remote)->peer_addr_info).ai_addrlen);

	if (numbytes <= 0)
	{
		fprintf(stderr, "Error in sending data over udp: %s\n", strerror(errno));
	}

	return numbytes;
}

int udp_receive_data(int sockfd, char *buf, ssize_t buf_len)
{
	int numbytes = 0;
	struct sockaddr_storage their_addr;
	socklen_t addr_len = sizeof(struct sockaddr_storage);

	numbytes = recvfrom(sockfd, buf, buf_len, 0,
			(struct sockaddr *)&their_addr, &addr_len);

	if (numbytes == -1)
	{
		fprintf(stderr, "Error in receiving data over udp: %s\n", strerror(errno));
	}

	return numbytes;
}

