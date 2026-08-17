#include "network.h"
#include <stdio.h>
#include <string.h>
#include <uci.h>
#include <stdlib.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <netdb.h>

// netlist part

struct net_list *net_list(struct net_device device)
{
  struct net_list *list = malloc(sizeof(struct net_list));
  list->device = device;
  list->next = NULL;
  return list;
}

struct net_device net_device(char *name, char *device, char *addr_v4, char *addr_v6,
                             uint8_t netmask_v4, uint8_t netmask_v6, uint32_t rx, uint32_t tx)
{
  struct net_device ret;
  ret.name = name != NULL ? strdup(name) : NULL;
  ret.device = device != NULL ? strdup(device) : NULL;
  ret.address_v4 = addr_v4 != NULL ? strdup(addr_v4) : NULL;
  ret.address_v6 = addr_v6 != NULL ? strdup(addr_v6) : NULL;
  ret.netmask_v4 = netmask_v4;
  ret.netmask_v6 = netmask_v6;
  ret.rx_bytes = rx;
  ret.tx_bytes = tx;
  return ret;
}

void net_list_free(struct net_list *list)
{
  if (list == NULL)
    return;
  struct net_list *prev = list;
  list = list->next;
  for (list;; list = list->next)
  {
    free(prev->device.address_v4);
    free(prev->device.address_v6);
    free(prev->device.device);
    free(prev->device.name);
    free(prev);
    if (list == NULL)
      break;
    prev = list;
  }
}

// lookup part

int check_uci_ptr(int rc, struct uci_ptr ptr)
{
  if (rc != UCI_OK)
  {
    return rc;
  }
  if (!ptr.o)
  {
    return 1;
  }
  return 0;
}

struct net_list *network_lookup_uci()
{
  int rc = 0;
  struct uci_context *cursor;
  struct uci_ptr ptr;
  struct uci_package *config;
  struct uci_section *section;
  struct uci_option *option;
  char opt[34] = {0};
  struct net_list *head = NULL;
  struct net_list *curr = NULL;

  cursor = uci_alloc_context();
  rc = uci_load(cursor, "network", &config);
  if (rc != UCI_OK)
  {
    uci_perror(cursor, "UCI ");
    goto clean_up;
  }
  // printf("Path -> %s\n", config->path);

  // section = uci_lookup_section(cursor, config, "loopback");

  for (int i = 0;; i++)
  {
    sprintf(opt, "network.@interface[%d].device", i);
    int tmp = uci_lookup_ptr(cursor, &ptr, opt, true);
    if (!check_uci_ptr(tmp, ptr))
    {
      if (!strncmp(ptr.s->e.name, "loopback", 8))
        continue;
      struct net_device dev = net_device(ptr.s->e.name, NULL, NULL, NULL, 0, 0, 0, 0);
      if (head == NULL)
      {
        head = net_list(dev);
        curr = head;
      }
      else
      {
        curr->next = net_list(dev);
        curr = curr->next;
      }
      // printf("interface name -> %s\n", ptr.s->e.name); // interface name
    }
    else
      break;
  }

clean_up:
  uci_free_context(cursor);
  return head;
}

// UBUS bullshit

void get_net_transfers(struct net_list **list);

enum
{
  INTERFACE,
  DEVICE,
  IPV4,
  IPV6,
  INTERFACE_MAX
};

enum
{
  INTERFACES,
  _INTERFACES_MAX
};

enum
{
  ADDRESS,
  MASK,
  _ADDR_MAX
};

static const struct blobmsg_policy array_policy[] = {
    [INTERFACES] = {.name = "interface", .type = BLOBMSG_TYPE_ARRAY}};

static const struct blobmsg_policy interface_policy[] = {
    [INTERFACE] = {.name = "interface", .type = BLOBMSG_TYPE_STRING},
    [DEVICE] = {.name = "device", .type = BLOBMSG_TYPE_STRING},
    [IPV4] = {.name = "ipv4-address", .type = BLOBMSG_TYPE_ARRAY},
    [IPV6] = {.name = "ipv6-address", .type = BLOBMSG_TYPE_ARRAY},
};

static const struct blobmsg_policy address_policy[] = {
    [ADDRESS] = {.name = "address", .type = BLOBMSG_TYPE_STRING},
    [MASK] = {.name = "mask", .type = BLOBMSG_TYPE_INT32},
};

static void callback(struct ubus_request *req, int type, struct blob_attr *msg)
{
  struct blob_attr *tb[_INTERFACES_MAX];
  struct blob_attr *interface[INTERFACE_MAX];
  struct blob_attr *address[_ADDR_MAX];
  struct net_list **list = req->priv;
  struct net_list *list_head = *list;

  blobmsg_parse(array_policy, _INTERFACES_MAX, tb, blob_data(msg), blob_len(msg));
  if (tb[INTERFACES])
  {
    struct blob_attr *cur;
    int i = 0;
    size_t rem;

    blobmsg_for_each_attr(cur, tb[INTERFACES], rem)
    {
      blobmsg_parse(interface_policy, INTERFACE_MAX, interface, blobmsg_data(cur),
                    blobmsg_data_len(cur));
      struct net_list *current;
      for (current = list_head; current != NULL; current = current->next)
      {
        char *name = blobmsg_get_string(interface[INTERFACE]);

        if (strcmp(name, current->device.name) == 0)
        {
          current->device.device = strdup(blobmsg_get_string(interface[DEVICE]));
          struct blob_attr *cur_addr;
          size_t addr_rem;
          blobmsg_for_each_attr(cur_addr, interface[IPV4], addr_rem)
          {
            blobmsg_parse(address_policy, _ADDR_MAX, address, blobmsg_data(cur_addr), blobmsg_data_len(cur_addr));
            current->device.address_v4 = strdup(blobmsg_get_string(address[ADDRESS]));
            current->device.netmask_v4 = blobmsg_get_u32(address[MASK]);
          }
          blobmsg_for_each_attr(cur_addr, interface[IPV6], addr_rem)
          {
            blobmsg_parse(address_policy, _ADDR_MAX, address, blobmsg_data(cur_addr), blobmsg_data_len(cur_addr));
            current->device.address_v6 = strdup(blobmsg_get_string(address[ADDRESS]));
            current->device.netmask_v6 = blobmsg_get_u32(address[MASK]);
          }
        }
      }
    }
  }
}

int netinfo(struct net_list **list)
{
  int rc = 0;
  if (list == NULL)
    return -1;
  else if (*list == NULL)
    return -1;

  struct ubus_context *ctx;
  uint32_t id;

  ctx = ubus_connect(NULL);
  if (!ctx)
  {
    rc = -1;
    goto end;
  }

  if (ubus_lookup_id(ctx, "network.interface", &id) ||
      ubus_invoke(ctx, id, "dump", NULL, callback, list, 3000))
  {
    rc = -1;
    goto end;
  }
  get_net_transfers(list);
end:
  if (ctx)
    ubus_free(ctx);
  return rc;
}

void get_net_transfers(struct net_list **list)
{
  char host[1025];
  int family, s;
  struct ifaddrs *addrs_head;
  getifaddrs(&addrs_head);
  struct ifaddrs *addrs = addrs_head;

  if (list == NULL)
  {
    freeifaddrs(addrs_head);
    return;
  }

  while (addrs != NULL)
  {
    // printf("%s\n", addrs->ifa_name);
    if (addrs->ifa_flags & IFF_LOOPBACK)
    { // omit loopback devices
      addrs = addrs->ifa_next;
      continue;
    }
    if (addrs->ifa_addr == NULL)
    {
      addrs = addrs->ifa_next;
      continue;
    }
    family = addrs->ifa_addr->sa_family;
    struct rtnl_link_stats *stats = addrs->ifa_data;
    struct net_list *current_node;

    if (addrs->ifa_data != NULL && family == AF_PACKET)
    {
      for (current_node = *list; current_node != NULL; current_node = current_node->next)
      {
        if (strcmp(current_node->device.device, addrs->ifa_name) == 0)
        {
          // printf("%s\n", addrs->ifa_name);
          break;
        }
      }
      if (current_node == NULL)
      {
        addrs = addrs->ifa_next;
        continue;
      }

      struct rtnl_link_stats *stats = addrs->ifa_data;
      current_node->device.rx_bytes = stats->rx_bytes;
      current_node->device.tx_bytes = stats->tx_bytes;
    }
    addrs = addrs->ifa_next;
  }
  freeifaddrs(addrs_head);
}