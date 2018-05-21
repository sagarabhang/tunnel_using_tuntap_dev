#include "common.h"
#include "tun_tap.h"
#include "vxlan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

vxlan_t *vxlan_create(char *source_vtep, char *destination_vtep, char *vni)
{
	if (source_vtep == NULL || destination_vtep == NULL || vni == NULL)
	{
		printf("Invalid values for vxlan_create\n");
		return NULL;
	}

	vxlan_t *vxlan = (vxlan_t *)malloc(sizeof(vxlan_t));
	if (vxlan == NULL)
	{
		printf("Cannot allocate the memory for vxlan_t\n");
		return NULL;
	}

#if 0
	memset(vxlan, 0, sizeof(vxlan_t));

	rc = inet_pton(AF_INET, source_vtep, &(sa.sin_addr));
	if (rc <= 0)
	{
		printf("Error converting source vtep address into network address\n");
		return NULL;
	}

	vxlan->source_vtep = sa.sin_addr.s_addr;
	rc = inet_pton(AF_INET, destination_vtep, &(sa.sin_addr));
	if (rc <= 0)
	{
		printf("Error converting destination vtep address into network address\n");
		return NULL;
	}
	vxlan->destination_vtep = sa.sin_addr.s_addr;
#endif

	vxlan->source_vtep = (char *)malloc(strlen(source_vtep) + 1);
	if (vxlan->source_vtep == NULL)
	{
		printf("Failed to allocate memory for source_vtep\n");
		free(vxlan);
		return NULL;
	}

	strncpy(vxlan->source_vtep, source_vtep, strlen(source_vtep));

	vxlan->destination_vtep = (char *)malloc(strlen(destination_vtep) + 1);
	if (vxlan->destination_vtep == NULL)
	{
		printf("Failed to allocate memory for destination_vtep\n");
		free(vxlan->source_vtep);
		free(vxlan);
		return NULL;
	}
	strncpy(vxlan->destination_vtep, destination_vtep, strlen(destination_vtep));

	strncpy(vxlan->vni, vni, sizeof(vxlan->vni));

	return vxlan;
}

void vxlan_encap(tun_tap_dev_t *dev, char *buf, ssize_t buf_len)
{
	if (dev == NULL || buf == NULL || buf_len <= 0)
	{
		return;
	}

	vxlan_header_t *vxlan_hdr = (vxlan_header_t *)buf;
	vxlan_hdr->flags = 0x08; /* '00001000' */
	strncpy(vxlan_hdr->vni, (dev->vxlan)->vni, sizeof(vxlan_hdr->vni));
}

void vxlan_destroy(vxlan_t *vxlan)
{
	if (vxlan == NULL)
	{
		return;
	}

	free(vxlan->source_vtep);
	free(vxlan->destination_vtep);
	free(vxlan);
}

