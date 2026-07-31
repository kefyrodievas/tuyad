#include "deviceinfo.h"
#include "memory.h"
#include "cJSON.h"
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/types.h>


long get_uptime() {
  struct sysinfo info;
  sysinfo(&info);
  return info.uptime;
}

struct netlist *netlist(const char *addr, const char *name, const char *netmask,
                        uint32_t rx, uint32_t tx) {
  struct netlist *node = (struct netlist *)malloc(sizeof(struct netlist));
  if (addr != NULL)
    node->addr = strdup(addr);
  else
    node->addr = NULL;
  node->name = strdup(name);
  if (netmask != NULL)
    node->netmask = strdup(netmask);
  else
    node->netmask = NULL;
  node->next = NULL;
  node->rx_bytes = rx;
  node->tx_bytes = tx;
  return node;
}

struct netlist *get_net_info() {
  char host[1025];
  int family, s;
  struct netlist *list = NULL;
  struct netlist *list_head = NULL;
  struct ifaddrs *addrs_head;
  getifaddrs(&addrs_head);
  struct ifaddrs *addrs = addrs_head;
  while (addrs != NULL) {
    if (addrs->ifa_flags & IFF_LOOPBACK) { // omit loopback devices
      addrs = addrs->ifa_next;
      continue;
    }
    if (addrs->ifa_addr == NULL) {
      addrs = addrs->ifa_next;
      continue;
    }

    family = addrs->ifa_addr->sa_family;
    struct rtnl_link_stats *stats = addrs->ifa_data;
    struct netlist *node = netlist(NULL, addrs->ifa_name, NULL, 0, 0);

    if (family == AF_INET || family == AF_INET6) {
      s = getnameinfo(addrs->ifa_addr,
                      (family == AF_INET) ? sizeof(struct sockaddr_in)
                                          : sizeof(struct sockaddr_in6),
                      host, 1025, NULL, 0, NI_NUMERICHOST);
      node->addr = strdup(host);
    }
    if (addrs->ifa_netmask != NULL) {
      family = addrs->ifa_netmask->sa_family;
      s = getnameinfo(addrs->ifa_netmask,
                      (family == AF_INET) ? sizeof(struct sockaddr_in)
                                          : sizeof(struct sockaddr_in6),
                      host, 1025, NULL, 0, NI_NUMERICHOST);
      node->netmask = strdup(host);
    }
    if (addrs->ifa_data != NULL && family == AF_PACKET) {
      struct rtnl_link_stats *stats = addrs->ifa_data;
      node->rx_bytes = stats->rx_bytes;
      node->tx_bytes = stats->tx_bytes;
    }

    if (list == NULL) {
      list = node;
      addrs = addrs->ifa_next;
      list_head = list;
      continue;
    } else {
      list->next = node;
      list = list->next;
      addrs = addrs->ifa_next;
      continue;
    }
  }
  freeifaddrs(addrs_head);
  return list_head;
}

void freenetlist(struct netlist *list) {
  if (list == NULL)
    return;
  struct netlist *next = list->next;
  while (list != NULL) {
    if (list->addr != NULL)
      free(list->addr);
    if (list->name != NULL)
      free(list->name);
    if (list->netmask != NULL)
      free(list->netmask);
    free(list);
    list = next;
    if (list != NULL)
      next = list->next;
  }
}

int get_cpu_info(union cpuinfo *info) { // to be fixed, percentages are weird
  FILE *input = fopen("/proc/stat", "r");
  if (input == NULL)
    return -1;
  char buf[10];
  fscanf(input, "%s", buf);
  for (int i = 0; i < 10; i++) {
    fscanf(input, "%u", &(info->vals[i]));
  }
  fclose(input);
  return 0;
}

double get_cpu_usage(union cpuinfo info) {
  long double cpu_work = info.cpustat.user + info.cpustat.nice +
                         info.cpustat.system + info.cpustat.irq +
                         info.cpustat.softirq + info.cpustat.steal +
                         info.cpustat.guest + info.cpustat.guest_nice;
  long double cpu_total = cpu_work + info.cpustat.idle;
  return (cpu_work / cpu_total) * 100;
}

union cpuinfo cpu_diff(union cpuinfo prev, union cpuinfo curr) {
  union cpuinfo res = curr;
  for (int i = 0; i < 10; i++) {

    res.vals[i] -= prev.vals[i];
  }
  return res;
}

char *device_data_to_json(struct meminfo mem_info, union cpuinfo cpu_info,
                          struct netlist *network_info, long uptime_info) {
  cJSON *data = NULL;
  char *string = NULL;
  data = cJSON_CreateObject();
  if (data == NULL)
    goto end;

  cJSON *memory = NULL;
  memory = cJSON_AddObjectToObject(data, "memory");
  if (memory == NULL)
    goto end;
  cJSON_AddNumberToObject(memory, "total", mem_info.total);
  cJSON_AddNumberToObject(memory, "free", mem_info.free);

  cJSON *uptime = NULL;
  uptime = cJSON_AddNumberToObject(data, "uptime", uptime_info);
  if (uptime == NULL)
    goto end;

  cJSON *net_devices = NULL;
  net_devices = cJSON_AddArrayToObject(data, "network");
  if (net_devices == NULL)
    goto end;
  for (struct netlist *ptr = network_info; ptr != NULL; ptr = ptr->next) {
    cJSON *net_device = cJSON_CreateObject();
    if (cJSON_AddStringToObject(net_device, "name", ptr->name) == NULL) {
      goto end;
    }
    if (ptr->addr != NULL) {
      if (cJSON_AddStringToObject(net_device, "address", ptr->addr) == NULL) {
        goto end;
      }
    }
    if (ptr->netmask != NULL) {
      if (cJSON_AddStringToObject(net_device, "netmask", ptr->netmask) ==
          NULL) {
        goto end;
      }
    }
    if (cJSON_AddNumberToObject(net_device, "rx_bytes", ptr->rx_bytes) ==
        NULL) {
      goto end;
    }
    if (cJSON_AddNumberToObject(net_device, "tx_bytes", ptr->tx_bytes) ==
        NULL) {
      goto end;
    }
    cJSON_AddItemToArray(net_devices, net_device);
  }

  cJSON *cpu = NULL;
  cpu = cJSON_AddNumberToObject(data, "cpu_util", get_cpu_usage(cpu_info));
  if (cpu == NULL)
    goto end;
  string = cJSON_PrintUnformatted(data);
  if (string == NULL) {
    fprintf(stderr, "Failed to print device data.\n");
  }
end:
  cJSON_Delete(data);
  return string;
}