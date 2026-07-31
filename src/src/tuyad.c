#include "arguments.h"
#include "daemon.h"
#include "deviceinfo.h"
#include "tuya_cacert.h"
#include "tuya_error_code.h"
#include "tuya_func.h"
#include "tuyalink_core.h"
// #include <argp.h>
#include "memory.h"
#include <assert.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <syslog.h>

const char *argp_program_version = "tuyad 0.1";
const char *argp_program_bug_address = "<ignas.kizys@teltonika.lt>";
static char doc[] = "Tuya device daemon";

tuya_mqtt_context_t client_instance;

bool loop_break = false;

void sig_handler(int signum) {
  if (signum == SIGTERM) {
    loop_break = true;
  }
  if (signum == SIGINT) {
    loop_break = true;
  }
}

int main(int argc, char **argv) {
  signal(SIGTERM, sig_handler);
  signal(SIGINT, sig_handler);
  // make tuya stdout logs quiet
  log_set_quiet(true);

  int ret;
  const char *LOGNAME = "tuyad";
  openlog(LOGNAME, LOG_PID, LOG_USER);

  struct arguments arguments;
  arguments.daemonize = false;
  arguments.deviceId = NULL;
  arguments.deviceSecret = NULL;
  arguments.productId = NULL;
  static struct option long_opts[] = {{"deviceSecret", 1, NULL, 's'},
                                      {"deviceId", 1, NULL, 'i'},
                                      {"productId", 1, NULL, 'p'},
                                      {"interval", 1, NULL, 'm'},
                                      {"daemonize", 0, NULL, 'D'},
                                      {0, 0, 0, 0}};
  ret = parse_args(argc, argv, &arguments, long_opts);
  if (ret != 0)
  {
    if (arguments.deviceId != NULL)
      free(arguments.deviceId);
    if (arguments.deviceSecret != NULL)
      free(arguments.deviceSecret);
    if (arguments.productId != NULL)
      free(arguments.productId);
    return ret;
  }
  // argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.daemonize) {
    // turn this process into a daemon
    ret = become_daemon(0);
    if (ret) {
      syslog(LOG_USER | LOG_ERR, "Error starting daemon");
      closelog();
      return EXIT_FAILURE;
    }
  }

  syslog(LOG_USER | LOG_INFO, "Initializing MQTT context");
  ret = OPRT_OK;

  tuya_mqtt_context_t *client = &client_instance;

  ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t){
                                   .host = "m2.tuyacn.com",
                                   .port = 8883,
                                   .cacert = tuya_cacert_pem,
                                   .cacert_len = sizeof(tuya_cacert_pem),
                                   .device_id = arguments.deviceId,
                                   .device_secret = arguments.deviceSecret,
                                   .keepalive = 60,
                                   .timeout_ms = arguments.interval/2,
                                   .on_connected = on_connected,
                                   .on_disconnect = on_disconnect,
                                   .on_messages = on_messages});
  if (ret != OPRT_OK) {
    syslog(LOG_ERROR, "Could not initialize Tuya MQTT context");
    goto end;
  }

  ret = tuya_mqtt_connect(client);

  if (ret != OPRT_OK) {
    syslog(LOG_ERROR, "Could not connect to the server");
    goto end;
  }

  syslog(LOG_INFO, "Connected to server successfully");

  struct timeval start, curr;
  gettimeofday(&start, NULL);
  struct meminfo memory;
  union cpuinfo cpu_prev, cpu_curr;
  get_cpu_info(&cpu_prev);
  long uptime;
  for (;;) {
    ret = OPRT_OK;
    // get time elapsed since last loop
    gettimeofday(&curr, NULL);
    uint64_t delta_ms = (curr.tv_sec - start.tv_sec) * 1000 +
                        (curr.tv_usec - start.tv_usec) / 1000;
    if (delta_ms >= arguments.interval) {
      // reset timer
      gettimeofday(&start, NULL);

      // get system info
      ret = get_memory_info(&memory);
      get_cpu_info(&cpu_curr);
      uptime = get_uptime();
      struct netlist *networks = NULL;
      networks = get_net_info();

      char *str = device_data_to_json(memory, cpu_diff(cpu_prev, cpu_curr),
                                      networks, uptime);

      cpu_prev = cpu_curr;
      ret = tuyalink_thing_property_report_with_ack(client, NULL, str);
      free(str);
      freenetlist(networks);
      if (ret < 0) {
        syslog(LOG_ERROR,
               "Failed to send device information, property report returned %d",
               ret);
      }
    }

    /* Loop to receive packets, and handles client keepalive */
    ret = tuya_mqtt_loop(client);
    if (ret < 0) {
      syslog(LOG_ERROR, "tuya_mqtt_loop failed, return code: %d", ret);
      break;
    }
    if (loop_break)
      break;
  }
end:
  syslog(LOG_INFO, "Program is closing");
  // free everything
  tuya_mqtt_deinit(client);
  if (arguments.deviceId != NULL)
    free(arguments.deviceId);
  if (arguments.deviceSecret != NULL)
    free(arguments.deviceSecret);
  if (arguments.productId != NULL)
    free(arguments.productId);
  return ret;
}
