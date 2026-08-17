#include "deviceinfo.h"
#include "memory.h"
#include "network.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

long get_uptime()
{
  struct sysinfo info;
  sysinfo(&info);
  return info.uptime;
}

int get_cpu_info(union cpuinfo *info)
{ // to be fixed, percentages are weird
  FILE *input = fopen("/proc/stat", "r");
  if (input == NULL)
    return -1;
  char buf[10];
  fscanf(input, "%s", buf);
  for (int i = 0; i < 10; i++)
  {
    fscanf(input, "%u", &(info->vals[i]));
  }
  fclose(input);
  return 0;
}

double get_cpu_usage(union cpuinfo info)
{
  long double cpu_work = info.cpustat.user + info.cpustat.nice +
                         info.cpustat.system + info.cpustat.irq +
                         info.cpustat.softirq + info.cpustat.steal +
                         info.cpustat.guest + info.cpustat.guest_nice;
  long double cpu_total = cpu_work + info.cpustat.idle;
  return (cpu_work / cpu_total) * 100;
}

union cpuinfo cpu_diff(union cpuinfo prev, union cpuinfo curr)
{
  union cpuinfo res = curr;
  for (int i = 0; i < 10; i++)
  {

    res.vals[i] -= prev.vals[i];
  }
  return res;
}

char *device_data_to_json(struct meminfo mem_info, union cpuinfo cpu_info,
                          struct net_list *network_info, long uptime_info)
{
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
  for (struct net_list *ptr = network_info; ptr != NULL; ptr = ptr->next)
  {
    cJSON *net_device = cJSON_CreateObject();
    if (cJSON_AddStringToObject(net_device, "name", ptr->device.device) == NULL)
    {
      goto end;
    }
    if (ptr->device.address_v4 != NULL)
    {
      if (cJSON_AddStringToObject(net_device, "address_v4", ptr->device.address_v4) == NULL)
      {
        goto end;
      }

      if (cJSON_AddNumberToObject(net_device, "netmask_v4", ptr->device.netmask_v4) ==
          NULL)
      {
        goto end;
      }
    }
    if (ptr->device.address_v6 != NULL)
    {
      if (cJSON_AddStringToObject(net_device, "address_v6", ptr->device.address_v6) == NULL)
      {
        goto end;
      }

      if (cJSON_AddNumberToObject(net_device, "netmask_v6", ptr->device.netmask_v6) ==
          NULL)
      {
        goto end;
      }
    }
    if (cJSON_AddNumberToObject(net_device, "rx_bytes", ptr->device.rx_bytes) ==
        NULL)
    {
      goto end;
    }
    if (cJSON_AddNumberToObject(net_device, "tx_bytes", ptr->device.tx_bytes) ==
        NULL)
    {
      goto end;
    }
    cJSON_AddItemToArray(net_devices, net_device);
  }

  cJSON *cpu = NULL;
  cpu = cJSON_AddNumberToObject(data, "cpu_util", get_cpu_usage(cpu_info));
  if (cpu == NULL)
    goto end;
  string = cJSON_PrintUnformatted(data);
  if (string == NULL)
  {
    fprintf(stderr, "Failed to print device data.\n");
  }
end:
  cJSON_Delete(data);
  return string;
}