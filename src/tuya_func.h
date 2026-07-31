#include "../lib/tuya-iot-core/include/tuyalink_core.h"
#include "../lib/tuya-iot-core/interface/mqtt_client_interface.h"
#include "../lib/tuya-iot-core/interface/system_interface.h"
#include "../lib/tuya-iot-core/utils/cJSON.h"
#include "../lib/tuya-iot-core/utils/tuya_error_code.h"
#include "../lib/tuya-iot-core/utils/tuya_log.h"

void on_connected(tuya_mqtt_context_t *context, void *user_data);

void on_disconnect(tuya_mqtt_context_t *context, void *user_data);

void on_messages(tuya_mqtt_context_t *context, void *user_data,
                 const tuyalink_message_t *msg);