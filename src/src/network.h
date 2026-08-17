#pragma once
#include <stdint.h>

struct net_device
{
  char *name;
  char *device;
  char *address_v4;
  char *address_v6;
  uint8_t netmask_v4;
  uint8_t netmask_v6;
  uint32_t rx_bytes;
  uint32_t tx_bytes;
};

struct net_list
{
  struct net_device device;
  struct net_list *next;
};

struct net_list *network_lookup_uci();
int netinfo(struct net_list **list);
void net_list_free(struct net_list *list);
