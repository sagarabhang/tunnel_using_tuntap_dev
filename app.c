#if 0
ip link set tun_tap_1 up
ip addr add 1.1.1.5/24 dev tun_tap_1
ip link set tun_tap_2 up
ip addr add 2.2.2.5/24 dev tun_tap_2
ip addr
#endif

#include "common.h"
#include "tun_tap.h"
#include "vxlan.h"
#include "udp_op.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int debug_code = 0;

void *start_transmit(void *dev)
{
	transmit_loop((tun_tap_dev_t *)dev);
	pthread_exit(NULL);
}

void *start_receive(void *dev)
{
	receive_loop((tun_tap_dev_t *)dev);
	pthread_exit(NULL);
}

tun_tap_dev_t *create_device(char *src_vtep, char *dest_vtep, char *vni, char *tun_tap_name)
{
	vxlan_t *vxlan = vxlan_create(src_vtep, dest_vtep, vni);
	if (vxlan == NULL)
	{
		exit(1);
	}

	udp_peer_t *udp_peer_local = udp_create_peer(src_vtep, "9550");
	if (udp_peer_local == NULL)
	{
		vxlan_destroy(vxlan);
		exit(1);
	}

	udp_peer_t *udp_peer_remote = udp_create_peer(dest_vtep, "9550");
	if (udp_peer_remote == NULL)
	{
		vxlan_destroy(vxlan);
		udp_destroy_peer(udp_peer_remote);
		exit(1);
	}

	tun_tap_dev_t* dev = tun_tap_create(tun_tap_name);
	if (dev == NULL)
	{
		vxlan_destroy(vxlan);
		udp_destroy_peer(udp_peer_local);
		udp_destroy_peer(udp_peer_remote);
		exit(1);
	}

	tun_tap_register_vxlan(dev, vxlan);
	tun_tap_register_udp_peer(dev, udp_peer_local, 1);
	tun_tap_register_udp_peer(dev, udp_peer_remote, 0);

	return dev;
}

void destroy_device(tun_tap_dev_t *dev)
{
	tun_tap_unregister_udp_peer(dev, 1);
	tun_tap_unregister_udp_peer(dev, 0);
	tun_tap_unregister_vxlan(dev);
	vxlan_destroy(dev->vxlan);
	tun_tap_destroy(dev);
}

void create_udp_client_socket(tun_tap_dev_t *dev)
{
	udp_create_client_socket(dev->udp_peer_remote);
	if ((dev->udp_peer_remote)->sockfd <= 0)
	{
		destroy_device(dev);
		exit(1);
	}
}

void create_udp_server_socket(tun_tap_dev_t *dev)
{
	udp_create_server_socket_and_bind(dev->udp_peer_local);
	if ((dev->udp_peer_local)->sockfd <= 0)
	{
		destroy_device(dev);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	int i = 0;
	pthread_t transmitter, receiver;
	tun_tap_dev_t *devices[MAX_NUM_TUN_TAP_DEV] = {NULL, NULL};

	if (argc != 5)
	{
		printf("Syntax: './app.out local_ip_vtep1 remote_ip_vtep1 local_ip_vtep2 remote_ip_vtep2'\n");
		exit(0);
	}

	devices[0] = create_device(argv[1], argv[2], "1", "tun_tap_1");
	create_udp_client_socket(devices[0]);
	create_udp_server_socket(devices[0]);
	devices[1] = create_device(argv[3], argv[4], "2", "tun_tap_2");
	create_udp_client_socket(devices[1]);
	create_udp_server_socket(devices[1]);

	pthread_create(&transmitter, NULL, start_transmit, (void *)devices);
	pthread_create(&receiver, NULL, start_receive, (void *)devices);

	pthread_join(transmitter, NULL);
	pthread_join(receiver, NULL);

	for (i = 0; i < MAX_NUM_TUN_TAP_DEV; i++)
	{
		destroy_device(devices[i]);
	}

	return 0;
}

