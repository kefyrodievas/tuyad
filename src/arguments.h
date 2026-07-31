#pragma once
#include <argp.h>
#include <stdbool.h>
#include <string.h>

struct arguments {
  char *deviceId;
  char *deviceSecret;
  char *productId;
  bool daemonize;
};

error_t parse_opt(int key, char *arg, struct argp_state *state);
