#include "htu21d.h"
#include "main.h"

#include <stdio.h>

#define HTU21D_ADDR 0x80
#define HTU21D_TOUT 1000
#define HTU21D_T_CMD 0xE3
#define HTU21D_H_CMD 0xE5

static int htu21d_get(unsigned what)
{
	uint8_t cmd = what;
	uint8_t rx_buff[3];
	HAL_StatusTypeDef res;
	res = HAL_I2C_Master_Transmit(&I2CxHandle, HTU21D_ADDR, &cmd, sizeof(cmd), HTU21D_TOUT);
	if (res != HAL_OK)
		return -1;
	res = HAL_I2C_Master_Receive(&I2CxHandle, HTU21D_ADDR, rx_buff, sizeof(rx_buff), HTU21D_TOUT);
	if (res != HAL_OK)
		return -1;
	return (rx_buff[0] << 8) | (rx_buff[1] & ~3);
}

int htu21d_get_humidity(void)
{
	int v = htu21d_get(HTU21D_H_CMD);
	if (v < 0) {
		return -1;
	} else {
		return (125 * v) / 0x10000 - 6;
	}
}

int htu21d_get_temperature(float* t)
{
	int v = htu21d_get(HTU21D_T_CMD);
	if (v < 0) {
		return -1;
	} else {
		*t = (175.72 * v) / 0x10000 - 46.85;
		return 0;
	}	
}

void htu21d_get_humidity_str(char* buff, int sz)
{
	int h = htu21d_get_humidity();
	if (h < 0) {
		snprintf(buff, sz, "err");
	} else {
		snprintf(buff, sz, "%d%%", h);
	}
}

void htu21d_get_temperature_str(char* buff, int sz)
{
	float t;
	int r = htu21d_get_temperature(&t);
	if (r < 0) {
		snprintf(buff, sz, "err");
	} else {
		r = snprintf(buff, sz, "%.1f C", t);
		buff[r-2] = 176;
	}
}
