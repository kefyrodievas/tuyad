#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "devices.h"

static const struct blobmsg_policy set_policy[] = {
    [RC] = {.name = "rc", .type = BLOBMSG_TYPE_INT32},
    [MSG] = {.name = "msg", .type = BLOBMSG_TYPE_STRING} };

static const struct blobmsg_policy get_policy[] = {
    [RC] = {.name = "rc", .type = BLOBMSG_TYPE_INT32},
    [MSG] = {.name = "msg", .type = BLOBMSG_TYPE_STRING},
    [DATA] = {.name = "data", .type = BLOBMSG_TYPE_TABLE},
};

static const struct blobmsg_policy data_policy[] = {
    [HUMIDITY] = {.name = "humidity", .type = BLOBMSG_TYPE_DOUBLE},
    [TEMP] = {.name = "temperature", .type = BLOBMSG_TYPE_DOUBLE},
};

static void action_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
    struct data_output *pin_data = (struct data_output *)req->priv;
    struct blob_attr *tb[GET_MAX];
    struct blob_attr *data[DATA_MAX];

    blobmsg_parse(get_policy, GET_MAX, tb, blob_data(msg), blob_len(msg));
    printf("after parse\n");
    if (tb[DATA]) {
        printf("data exists\n");
        pin_data->sensor_value = true;
        blobmsg_parse(data_policy, DATA_MAX, data, blobmsg_data(tb[DATA]),
            blobmsg_data_len(tb[DATA]));
        printf("data exists\n");
        pin_data->humidity = blobmsg_get_double(data[HUMIDITY]);
        printf("data exists\n");
        pin_data->temperature = blobmsg_get_double(data[TEMP]);
        printf("data exists\n");
    }
    else {
        printf("data does not exist\n");
        pin_data->sensor_value = false;
    }
    printf("msg\n");
    pin_data->msg = strdup(blobmsg_get_string(tb[MSG]));
    printf("rc\n");
    pin_data->rc = blobmsg_get_u32(tb[RC]);

    printf("ubus returned: rc=%d, msg=\'%s\'\n", pin_data->rc, pin_data->msg);
}

int set_device_pin_state(char *port, int state, int pin, struct data_output *output) {

    struct ubus_context *ctx;
    uint32_t id;
    int rc;
    output->humidity = 0;
    output->msg = NULL;
    output->rc = 0;
    output->sensor_value = false;
    output->temperature = 0;
    static struct blob_buf attr;

    blobmsg_buf_init(&attr);

    ctx = ubus_connect(NULL);
    if (!ctx) {
        printf("ubus connection failed!\n");
        rc = -1;
        goto end;
    }

    char *pin_state = state == ON ? "on" : "off";

    printf("%s\n", pin_state);

    blobmsg_add_string(&attr, "port", strdup(port));
    blobmsg_add_u32(&attr, "pin", pin);

    printf("%s\n", blobmsg_format_json(attr.head, true));
    int testing_var;
    if (ubus_lookup_id(ctx, "esp_controller", &id) ||
        (testing_var = ubus_invoke(ctx, id, pin_state, attr.head, action_cb, output, 4000))) {
        printf("ubus lookup or invoke failed! rc: %d\n", testing_var);
        output->rc = 2;
        output->msg = strdup("Bad port value");
        output->sensor_value = false;
        rc = -1;
        goto end;
    }

end:
    if (ctx)
        ubus_free(ctx);
    blob_buf_free(&attr);
    return rc;
}

int get_sensor_data(char *port, int pin, char *sensor, char *model, struct data_output *output) {
    struct ubus_context *ctx;
    uint32_t id;
    int rc;
    output->humidity = 0;
    output->msg = NULL;
    output->rc = 0;
    output->sensor_value = false;
    output->temperature = 0;
    static struct blob_buf attr;

    blobmsg_buf_init(&attr);

    ctx = ubus_connect(NULL);
    if (!ctx) {
        printf("ubus connection failed!\n");
        rc = -1;
        goto end;
    }

    printf("1");

    blobmsg_add_string(&attr, "port", strdup(port));
    blobmsg_add_u32(&attr, "pin", pin);
    blobmsg_add_string(&attr, "sensor", strdup(sensor));
    blobmsg_add_string(&attr, "model", strdup(model));

    printf("2");

    printf("%s\n", blobmsg_format_json(attr.head, true));
    int testing_var;
    if (ubus_lookup_id(ctx, "esp_controller", &id) ||
        (testing_var = ubus_invoke(ctx, id, "get", attr.head, action_cb, output, 4000))) {
        printf("ubus lookup or invoke failed! rc: %d\n", testing_var);
        output->rc = 2;
        output->msg = strdup("Bad port value");
        output->sensor_value = false;
        rc = -1;
        goto end;
    }printf("1");


end:
    if (ctx)
        ubus_free(ctx);
    blob_buf_free(&attr);
    return rc;
}

char *data_to_json(struct data_output out) {
    static struct blob_buf buffer;
    blob_buf_init(&buffer, 0);
    blobmsg_add_u16(&buffer, "rc", out.rc);
    // printf("data_to_json() added rc: %d\n", out.rc);
    blobmsg_add_string(&buffer, "msg", out.msg);
    // printf("data_to_json() added msg: %s\n", out.msg);
    if (out.sensor_value) {
        blobmsg_add_double(&buffer, "humidity", out.humidity);
        blobmsg_add_double(&buffer, "temperature", out.temperature);
    }
    char *str = strdup(blobmsg_format_json(buffer.head, true));
    blob_buf_free(&buffer);
    // printf("data_to_json() returns: %s\n", str);
    return str;
}