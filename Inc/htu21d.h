#pragma once

int htu21d_get_humidity(void);
int htu21d_get_temperature(float* t);
void htu21d_get_humidity_str(char* buff, int sz);
void htu21d_get_temperature_str(char* buff, int sz);
