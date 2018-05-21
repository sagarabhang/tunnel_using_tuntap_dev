#include "tun_tap.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

tun_tap_dev_t* tun_tap_create(const char *dev)
{
	tun_tap_dev_t* tun_tap_dev = NULL;
	int status = 0;

	tun_tap_dev = (tun_tap_dev_t *)calloc(sizeof(tun_tap_dev_t), 1);
	if (tun_tap_dev == NULL)
	{
		printf("Failed to allocate memory for tun_tap_dev\n");
		return NULL;
	}

	if ((tun_tap_dev->fd = open("/dev/net/tun", O_RDWR)) < 0)
	{
		printf("Failed to open device /dev/net/tun\n");
		return NULL;
	}

	(tun_tap_dev->ifr).ifr_flags = IFF_TAP | IFF_NO_PI;
	if (dev && *dev)
	{
		strncpy(tun_tap_dev->ifr.ifr_name, dev, IFNAMSIZ);
	}

	status = ioctl(tun_tap_dev->fd, TUNSETIFF, (void *)&(tun_tap_dev->ifr));
	if (status < 0)
	{
		close(tun_tap_dev->fd);
		printf("Error in ioctl: %s\n", strerror(errno));
		free(tun_tap_dev);
		return NULL;
	}

	return tun_tap_dev;
}

void tun_tap_destroy(tun_tap_dev_t *dev)
{
	if (dev == NULL)
	{
		return;
	}

	close(dev->fd);
	free(dev);
}

void tun_tap_register_vxlan(tun_tap_dev_t *dev, vxlan_t *vxlan)
{
	if (dev == NULL || vxlan == NULL)
	{
		return;
	}
	dev->vxlan = vxlan;
}

void tun_tap_register_udp_peer(tun_tap_dev_t *dev, udp_peer_t *udp_peer, int is_local)
{
	if (dev == NULL || udp_peer == NULL)
	{
		return;
	}

	if (is_local)
	{
		dev->udp_peer_local = udp_peer;
	}
	else
	{
		dev->udp_peer_remote = udp_peer;
	}
}

void tun_tap_unregister_vxlan(tun_tap_dev_t *dev)
{
	if (dev == NULL)
	{
		return;
	}

	dev->vxlan = NULL;
}

void tun_tap_unregister_udp_peer(tun_tap_dev_t *dev, int is_local)
{
	if (dev == NULL)
	{
		return;
	}

	if (is_local)
	{
		dev->udp_peer_local = NULL;
	}
	else
	{
		dev->udp_peer_remote = NULL;
	}
}

