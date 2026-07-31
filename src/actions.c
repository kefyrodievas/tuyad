#include "actions.h"
#include "../lib/tuya-iot-core/utils/cJSON.h"
#include <stdio.h>
#include <string.h>

int action_log(cJSON *data) {
  cJSON *action_params = cJSON_GetObjectItem(data, "inputParams");
  if (cJSON_GetArraySize(action_params) == 1) {
    char *arg = cJSON_GetArrayItem(action_params, 0)->valuestring;
    FILE *output = fopen("/tmp/tuya_action.log", "a");
    if (output == NULL) {
      return -1;
    }
    fprintf(output, "action sent from server, data:%s\n", arg);
    fclose(output);
    return 0;
  } else
    return 1;
}