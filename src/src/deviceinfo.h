#pragma once
#include <stdint.h>
#include "memory.h"
#include "network.h"

union cpuinfo
{
  uint32_t vals[10];
  struct
  {
    uint32_t user, nice, system, idle, iowait, irq, softirq, steal, guest,
        guest_nice;
  } cpustat;
};

struct devinfo get_device_info();
double get_cpu_usage(union cpuinfo info);
int get_cpu_info(union cpuinfo *info);
long get_uptime();
// struct meminfo get_mem_info();
union cpuinfo cpu_diff(union cpuinfo prev, union cpuinfo curr);
char *device_data_to_json(struct meminfo mem_info, union cpuinfo cpu_info,
                          struct net_list *network_info, long uptime_info);