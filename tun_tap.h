#ifndef TUN_TAP_H
#define TUN_TAP_H

#include "vxlan.h"
#include "udp_op.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#define	MAX_NUM_TUN_TAP_DEV	2

typedef struct vxlan vxlan_t;

typedef struct tun_tap_device {
	int fd;
	struct ifreq ifr;
	vxlan_t *vxlan;
	udp_peer_t *udp_peer_local;
	udp_peer_t *udp_peer_remote;
} tun_tap_dev_t;

tun_tap_dev_t* tun_tap_create(const char *dev_name);
void tun_tap_destroy(tun_tap_dev_t *dev);
void tun_tap_register_vxlan(tun_tap_dev_t *dev, vxlan_t *vxlan);
void tun_tap_register_udp_peer(tun_tap_dev_t *dev, udp_peer_t *udp_peer, int is_local);
void tun_tap_unregister_vxlan(tun_tap_dev_t *dev);
void tun_tap_unregister_udp_peer(tun_tap_dev_t *dev, int is_local);

#if 0
int tun_tap_read(tun_tap_dev_t *dev);
int tun_tap_write(tun_tap_dev_t *dev);
#endif

#endif //TUN_TAP_H

