#pragma once
#include <stdint.h>

struct meminfo {
  unsigned long total_mem;
  unsigned long free_mem;
};

struct netlist {
  char *name;
  char *addr;
  char *netmask;
  uint32_t rx_bytes;
  uint32_t tx_bytes;
  struct netlist *next;
};

// struct uptime {
//   long total_secs;
//   short sec, min, hour;
// };

union cpuinfo {
  uint32_t vals[10];
  struct {
    uint32_t user, nice, system, idle, iowait, irq, softirq, steal, guest,
        guest_nice;
  } cpustat;
};

// struct devinfo {
//   struct meminfo memory;
//   struct netlist *network;
//   struct uptime uptime;
//   struct cpuinfo cpu;
// };

struct devinfo get_device_info();
void freenetlist(struct netlist *list);
double get_cpu_usage(union cpuinfo info);
int get_cpu_info(union cpuinfo *info);
struct netlist *get_net_info();
long get_uptime();
struct meminfo get_mem_info();
union cpuinfo cpu_diff(union cpuinfo prev, union cpuinfo curr);
char *device_data_to_json(struct meminfo mem_info, union cpuinfo cpu_info,
                          struct netlist *network_info, long uptime_info);