#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "devices.h"

static const struct blobmsg_policy set_policy[] = {
	[RC]	  = { .name = "rc", .type = BLOBMSG_TYPE_INT32 },
	[MSG]	  = { .name = "msg", .type = BLOBMSG_TYPE_STRING }
};

static const struct blobmsg_policy get_policy[] = {
	[RC]	  = { .name = "rc", .type = BLOBMSG_TYPE_INT64 },
	[MSG]	  = { .name = "msg", .type = BLOBMSG_TYPE_STRING },
	[DATA]	  = { .name = "data", .type = BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy data_policy[] = {
    [HUMIDITY] = {.name = "humidity", .type = BLOBMSG_TYPE_INT16},
    [TEMP] = {.name = "temperature", .type = BLOBMSG_TYPE_DOUBLE},
};

static void action_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct data_output *pin_data = (struct data_output *)req->priv;
	struct blob_attr *tb[GET_MAX];
	struct blob_attr *data[DATA_MAX];

	blobmsg_parse(get_policy, GET_MAX, tb, blob_data(msg), blob_len(msg));

	if (tb[DATA]) {
		pin_data->sensor_value = true;
        blobmsg_parse(data_policy, DATA_MAX, data, blobmsg_data(tb[DATA]),
		      blobmsg_data_len(tb[DATA]));
        pin_data->humidity = blobmsg_get_u64(data[HUMIDITY]);
        pin_data->temperature = blobmsg_get_double(data[TEMP]);
	}
    else {
        pin_data->sensor_value = false;
    }
	pin_data->rc = blobmsg_get_u64(tb[RC]);
	pin_data->msg = strdup(blobmsg_get_string(tb[MSG]));

}

int set_device_pin_state(char * port, int state, int pin, struct data_output * output){

    struct ubus_context *ctx;
    uint32_t id;
    int rc;
    output->humidity = 0;
    output->msg = NULL;
    output->rc = 0;
    output->sensor_value = false;
    output->temperature = 0;

    ctx = ubus_connect(NULL);
    if (!ctx) {
        rc = -1;
        goto end;
    }

    char * pin_state = state == ON ? "on" : "off" ;

    struct blob_buf attr;

    blobmsg_buf_init(&attr);

    blobmsg_add_string(&attr, "port", strdup(port));
    blobmsg_add_u16(&attr, "pin", pin);

    if (ubus_lookup_id(ctx, "esp_controller", &id) ||
        ubus_invoke(ctx, id, pin_state, attr.head, action_cb, output, 3000)) {
        rc = -1;
        goto end;
    }

end:
    if(ctx)	ubus_free(ctx);
    blob_buf_free(&attr);
    return rc;
}