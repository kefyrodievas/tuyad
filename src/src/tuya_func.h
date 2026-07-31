#include "tuyalink_core.h"
#include "mqtt_client_interface.h"
#include "system_interface.h"
#include "cJSON.h"
#include "tuya_error_code.h"
#include "tuya_log.h"

void on_connected(tuya_mqtt_context_t *context, void *user_data);

void on_disconnect(tuya_mqtt_context_t *context, void *user_data);

void on_messages(tuya_mqtt_context_t *context, void *user_data,
                 const tuyalink_message_t *msg);