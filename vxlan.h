#ifndef VXLAN_H
#define VXLAN_H

#include "tun_tap.h"
#include <stdlib.h>

typedef struct vxlan {
	char *source_vtep;
	char *destination_vtep;
	char vni[3];
} vxlan_t;

typedef struct vxlan_header {
	char flags;
	char reserved_1[3];
	char vni[3];
	char reserved_2;
} vxlan_header_t;

typedef struct tun_tap_device tun_tap_dev_t;

vxlan_t *vxlan_create(char *source_vtep, char *destination_vtep, char *vni);
void vxlan_destroy(vxlan_t *vxlan);
void vxlan_encap(tun_tap_dev_t *dev, char *buf, ssize_t buf_len);
void vxlan_decap(tun_tap_dev_t *dev, char *buf, ssize_t buf_len);

#endif //VXLAN_H

