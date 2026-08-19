#include "tuya_func.h"
#include "actions.h"
#include <syslog.h>
#include "devices.h"

void on_connected(tuya_mqtt_context_t *context, void *user_data) {
}

void on_disconnect(tuya_mqtt_context_t *context, void *user_data) {
}

void on_messages(tuya_mqtt_context_t *context, void *user_data,
	const tuyalink_message_t *msg) {
	int status;
	cJSON *action_data = NULL;
	cJSON *action_code = NULL;
	switch (msg->type) {
		case THING_TYPE_MODEL_RSP:
			break;

		case THING_TYPE_PROPERTY_SET:
			break;

		case THING_TYPE_PROPERTY_REPORT_RSP:
			break;
		case THING_TYPE_ACTION_EXECUTE:
			action_data = cJSON_Parse(msg->data_string);
			action_code = cJSON_GetObjectItem(action_data, "actionCode");
			// if (strcmp(action_code->valuestring, "action_log")) {
			//   status = action_log(action_data);
			// } else {
			//   status = 1;
			// }
			cJSON *action_params = cJSON_GetObjectItem(action_data, "inputParams");
			if (!strcmp(action_code->valuestring, "on")) {
				struct data_output out;
				status = set_device_pin_state(cJSON_GetObjectItem(action_params, "port")->valuestring, ON,
					cJSON_GetObjectItem(action_params, "pin")->valueint, &out);
				char *str = data_to_json(out);
				tuyalink_thing_property_report_with_ack(context, NULL, str);
				free(str);
			}
			else if (!strcmp(action_code->valuestring, "off")) {
				struct data_output out;
				status = set_device_pin_state(cJSON_GetObjectItem(action_params, "port")->valuestring, OFF,
					cJSON_GetObjectItem(action_params, "pin")->valueint, &out);
				char *str = data_to_json(out);
				tuyalink_thing_property_report_with_ack(context, NULL, str);
				free(str);
			}
			else if (!strcmp(action_code->valuestring, "get")) {
				struct data_output out;
				status = get_sensor_data(cJSON_GetObjectItem(action_params, "port")->valuestring,
					cJSON_GetObjectItem(action_params, "pin")->valueint, cJSON_GetObjectItem(action_params, "sensor")->valuestring, cJSON_GetObjectItem(action_params, "model")->valuestring, &out);
				char *str = data_to_json(out);
				tuyalink_thing_property_report_with_ack(context, NULL, str);
				free(str);
			}
			else if (strcmp(action_code->valuestring, "devices")) {

			}
			if (status != 0) {
				syslog(LOG_ERROR, "Action %s returned %d: ", action_code->valuestring,
					status);
			}
			else {
				syslog(LOG_INFO, "Action %s completed successfully",
					action_code->valuestring);
			}
			cJSON_Delete(action_data);
			break;
		default:
			break;
	}
	printf("\r\n");
}