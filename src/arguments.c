#include "arguments.h"
#include <stdbool.h>
#include <string.h>

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = (struct arguments *)state->input;
  switch (key) {
  case 'i':
    arguments->deviceId = strdup(arg);
    break;
  case 's':
    arguments->deviceSecret = strdup(arg);
    break;
  case 'p':
    arguments->productId = strdup(arg);
    break;
  case 'D':
    arguments->daemonize = true;
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}