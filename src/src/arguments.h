#pragma once
// #include <argp.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

struct arguments
{
  char *deviceId;
  char *deviceSecret;
  char *productId;
  bool daemonize;
  int interval;
};

int parse_args(int argc, char **argv, struct arguments *args, struct option *long_opts);