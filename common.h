#ifndef COMMON_H
#define COMMON_H

#include "tun_tap.h"
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>

#define ETHER_TYPE_IPV4			0x0800
#define ETHER_TYPE_ARP			0x0806
#define ETHER_TYPE_VLAN			0x8100
#define ETHER_TYPE_IPV6			0x86DD

extern int debug_code;

void transmit_loop(tun_tap_dev_t *dev);
void receive_loop(tun_tap_dev_t *dev);
void handle_outgoing_frame(tun_tap_dev_t *dev, char *buf, ssize_t buf_len);

#endif //COMMON_H

