#include "arguments.h"
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

int parse_args(int argc, char **argv, struct arguments *args,
               struct option *long_opts)
{
  uint8_t flags = 0;
  int option;
  while ((option = getopt_long(argc, argv, "hDp:i:s:m:", long_opts, NULL)) !=
         -1)
  {
    switch (option)
    {
    case 'h':
      break;
    case 'i':
      flags |= 0b1;
      args->deviceId = strdup(optarg);
      break;
    case 's':
      flags |= 0b10;
      args->deviceSecret = strdup(optarg);
      break;
    case 'p':
      flags |= 0b100;
      args->productId = strdup(optarg);
      break;
    case 'D':
      flags |= 8;
      args->daemonize = true;
      break;
    case 'm':
      flags |= 16;
      args->interval = atoi(optarg);
      break;
    default:
      return -1;
    }
  }
  if (!(flags & 0b10000))
  {
    args->interval = 5000;
  }
  if (!(flags & 0b1) || !(flags & 0b10) || !(flags & 0b100))
    return 1;
  return 0;
}