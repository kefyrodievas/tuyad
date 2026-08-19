#pragma once

#include <stdbool.h>

enum{
    ON,
    OFF
};

enum{
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

struct data_output{
    int rc;
    char * msg;
    bool sensor_value;
    int humidity;
    double temperature;
};


int set_device_pin_state(char * port, int state, int pin, struct data_output * output);