#include "adc.h"
#include "stm32l0xx_hal.h"

#include <stdio.h>

ADC_HandleTypeDef AdcHandle;

int adc_init(unsigned ch[], unsigned nch)
{
	unsigned i;

	AdcHandle.Instance = ADC1;
	AdcHandle.Init.OversamplingMode      = DISABLE;
	AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV1;
	AdcHandle.Init.LowPowerAutoOff       = DISABLE;
	AdcHandle.Init.LowPowerFrequencyMode = ENABLE;
	AdcHandle.Init.LowPowerAutoWait      = ENABLE;
	AdcHandle.Init.Resolution            = ADC_RESOLUTION12b;
	AdcHandle.Init.SamplingTime          = ADC_SAMPLETIME_28CYCLES_5;
	AdcHandle.Init.ScanDirection         = ADC_SCAN_DIRECTION_UPWARD;
	AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.ContinuousConvMode    = DISABLE;
	AdcHandle.Init.DiscontinuousConvMode = ENABLE;
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIG_EDGE_NONE;
	AdcHandle.Init.EOCSelection          = EOC_SINGLE_CONV;
	AdcHandle.Init.DMAContinuousRequests = DISABLE;

	/* Initialize ADC peripheral according to the passed parameters */
	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		return -1;
	}
	/* Calibrate */
	if (HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED) != HAL_OK)
	{
		return -1;
	}
	for (i = 0; i < nch; ++i)
	{
		ADC_ChannelConfTypeDef sConfig = {.Channel = ch[i]};
		if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
		{
			return -1;
		}
	}
	return 0;
}

#define ADC_TIMEOUT 10

int adc_conv(unsigned res[], unsigned nch)
{
	unsigned i;
	for (i = 0; i < nch; ++i)
	{
		if (HAL_ADC_Start(&AdcHandle) != HAL_OK)
		{
			return -1;
		}
		if (HAL_ADC_PollForConversion(&AdcHandle, ADC_TIMEOUT) != HAL_OK)
		{
			return -1;
		}
		res[i] = HAL_ADC_GetValue(&AdcHandle);
	}
	if (HAL_ADC_Stop(&AdcHandle) != HAL_OK)
	{
		return -1;
	}
	return 0;
}

int adc_tv_init(void)
{
	unsigned tv_ch[] = {ADC_CHANNEL_VREFINT, ADC_CHANNEL_TEMPSENSOR};
	return adc_init(tv_ch, 2);
}

int adc_tv_get(struct adc_tv* tv)
{
	int vr_cal3   = *(uint16_t const*)(0x1FF80078);
	int ts_cal30  = *(uint16_t const*)(0x1FF8007A);
	int ts_cal130 = *(uint16_t const*)(0x1FF8007E);
	if (adc_conv(tv->conv_res, 2))
		return -1;
	/* Temperature in 1/10 C */
	int t = tv->t * vr_cal3 / tv->v;
	tv->t_val = 300 + (t - ts_cal30) * 1000 / (ts_cal130 - ts_cal30);
	/* Voltage in 1/100 V */
	tv->v_val = 300 * vr_cal3 / tv->v;
	return 0;
}

int adc_tv_str(struct adc_tv const* tv, struct adc_tv_str* tvs)
{
	int r;
	int f = tv->t_val % 10;
	if (f < 0) f = -f;
	r = snprintf(tvs->t_str, TV_BUFF_SZ, "%d.%d C", tv->t_val / 10, f);
	if (r >= TV_BUFF_SZ)
		return -1;
	tvs->t_str[r-2] = 176;
	r = snprintf(tvs->v_str, TV_BUFF_SZ, "%d.%dV", tv->v_val / 100, tv->v_val % 100);
	if (r >= TV_BUFF_SZ)
		return -1;
	return 0;
}

