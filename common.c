#include "common.h"
#include "tun_tap.h"
#include "vxlan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

char send_buffer[2000];
char receive_buffer[2000];

void handle_outgoing_frame(tun_tap_dev_t *dev, char *buf, ssize_t buf_len)
{
	int num_bytes = 0;

	if (dev == NULL || buf == NULL || buf_len <= 0)
	{
		return;
	}

#if 0
	ether_header_t *eth_hdr = (ether_header_t *)(buf + sizeof(vxlan_header_t));
	switch(htons(eth_hdr->ethertype))
	{
		case ETHER_TYPE_IPV4:
			printf("IPv4 packet\n");
			break;

		case ETHER_TYPE_ARP:
			printf("ARP packet\n");
			break;

		case ETHER_TYPE_IPV6:
			printf("IPv6 packet\n");
			break;

		case ETHER_TYPE_VLAN:
			printf("VLAN packet\n");
			break;

		default:
			printf("Not supported ETHER_TYPE %x\n", eth_hdr->ethertype);
	}
#endif

	vxlan_encap(dev, buf, buf_len);
	num_bytes = udp_send_data(dev, buf, buf_len);
	if (num_bytes > 0)
	{
		if (debug_code == 1)
		{
			printf("\tSent %d bytes over udp\n", num_bytes);;
		}
	}
}

void receive_loop(tun_tap_dev_t *dev_addr)
{
	int i = 0, n = 0, j = 0;
	int rc = 0;
	ssize_t num_bytes = 0;
	char *frame_ptr = NULL;
	vxlan_header_t *vxlan_hdr_ptr = NULL;
	fd_set readfds;
	fd_set selectrfds;
	tun_tap_dev_t *dev = NULL;

	FD_ZERO(&readfds);

	tun_tap_dev_t **devices = (tun_tap_dev_t **)dev_addr;

	for (i = 0; i < MAX_NUM_TUN_TAP_DEV; i++)
	{
		dev = devices[i];
		if (dev == NULL)
		{
			return;
		}

		FD_SET((dev->udp_peer_local)->sockfd, &readfds);
	}
	n = (dev->udp_peer_local)->sockfd + 1;
	
	if (debug_code == 1)
	{
		printf("In receive loop\n");
	}

	while(1)
	{
		selectrfds = readfds;
		rc = select(n, &selectrfds, NULL, NULL, NULL);
		if (rc == -1)
		{
			printf("Receive loop: Error in select\n");
			exit(1);
		}

		for (i = 0; i < MAX_NUM_TUN_TAP_DEV; i++)
		{
			dev = devices[i];
			if (dev && FD_ISSET((dev->udp_peer_local)->sockfd, &selectrfds))
			{
				int sd = (dev->udp_peer_local)->sockfd;
				
				memset(receive_buffer, 0, sizeof(receive_buffer));
				num_bytes = 0;
				num_bytes = udp_receive_data(sd, receive_buffer, sizeof(receive_buffer));
				if (num_bytes == -1)
				{
					continue;
				}

				if (debug_code == 1)
				{
					printf("\t\t\t\t\tReceived %zd bytes over UDP\n", num_bytes);
				}

				vxlan_hdr_ptr = (vxlan_header_t *)receive_buffer;
				tun_tap_dev_t* rcv_dev = NULL;
				for (j = 0; j < MAX_NUM_TUN_TAP_DEV; j++)
				{
					if (strcmp(vxlan_hdr_ptr->vni, (devices[j]->vxlan)->vni) == 0)
					{
						rcv_dev = devices[j];
						break;
					}
				}

				if (rcv_dev == NULL)
				{
					if (debug_code == 1)
					{
						printf("\t\t\t\t\tData from unknown source %s\n", vxlan_hdr_ptr->vni);
					}
					continue;
				}

				frame_ptr = receive_buffer + sizeof(vxlan_header_t);
				num_bytes = write(rcv_dev->fd, frame_ptr, num_bytes - sizeof(vxlan_header_t));
				if (num_bytes < 0)
				{
					if (debug_code == 1)
					{
						printf("Error writing to interface '%s'\n", rcv_dev->ifr.ifr_name);
					}
				}

				if (debug_code == 1)
				{
					printf("\t\t\t\t\tWrote %zd bytes to interface '%s'\n", num_bytes, rcv_dev->ifr.ifr_name);
				}
			}
		}
	}
}

void transmit_loop(tun_tap_dev_t *dev_addr)
{
	int i = 0, j = 0, n = 0;
	int rc = 0;
	ssize_t num_bytes = 0;
	char *frame_ptr = NULL;
	fd_set readfds;
	fd_set selectxfds;
	vxlan_header_t *vxlan_hdr_ptr = NULL;
	tun_tap_dev_t *dev = NULL;

	FD_ZERO(&readfds);

	tun_tap_dev_t **devices = (tun_tap_dev_t **)dev_addr;

	for (i = 0; i < MAX_NUM_TUN_TAP_DEV; i++)
	{
		dev = devices[i];
		if (dev == NULL)
		{
			return;
		}

		FD_SET(dev->fd, &readfds);
	}
	n = dev->fd + 1;

	if (debug_code == 1)
	{
		printf("In transmit loop\n");
	}

	while(1)
	{
		selectxfds = readfds;
		rc = select(n, &selectxfds, NULL, NULL, NULL);
		if (rc == -1)
		{
			printf("Transmit loop: Error in select\n");
			exit(1);
		}

		for (i = 0; i < MAX_NUM_TUN_TAP_DEV; i++)
		{
			dev = devices[i];
			if (dev && FD_ISSET(dev->fd, &selectxfds))
			{
				int xd = dev->fd;
				tun_tap_dev_t* xmit_dev = NULL;

				for (j = 0; j < MAX_NUM_TUN_TAP_DEV; j++)
				{
					if (xd == devices[j]->fd)
					{
						xmit_dev = devices[j];
						break;
					}
				}

				if (xmit_dev == NULL)
				{
					if (debug_code == 1)
					{
						printf("\tData from unknown source %s - ignoring\n", vxlan_hdr_ptr->vni);
					}
					continue;
				}

				memset(send_buffer, 0, sizeof(send_buffer));
				frame_ptr = send_buffer + sizeof(vxlan_header_t);

				num_bytes = 0;
				num_bytes = read(xmit_dev->fd, frame_ptr, sizeof(send_buffer) - sizeof(vxlan_header_t));
				if (num_bytes < 0)
				{
					if (debug_code == 1)
					{
						printf("Error reading from device = %s\n", xmit_dev->ifr.ifr_name);
					}
				}

				if (debug_code == 1)
				{
					printf("\tRead %zd bytes from device %s\n", num_bytes, xmit_dev->ifr.ifr_name);
				}
				handle_outgoing_frame(xmit_dev, send_buffer, num_bytes + sizeof(vxlan_header_t));
			}
		}
	}
}

