#pragma once

#include <stdbool.h>

enum {
    ON,
    OFF
};

enum {
    RC,
    MSG,
    DATA,
    GET_MAX,
    SET_MAX = DATA
};

enum {
    HUMIDITY,
    TEMP,
    DATA_MAX
};

struct data_output {
    int rc;
    char *msg;
    bool sensor_value;
    double humidity;
    double temperature;
};

struct port {
    char *port;
    int vid, pid;
};

int set_device_pin_state(char *port, int state, int pin, struct data_output *output);
int get_sensor_data(char *port, int pin, char *sensor, char *model, struct data_output *output);
char *data_to_json(struct data_output out);