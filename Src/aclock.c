#include "main.h"
#include "clock.h"
#include "led_display.h"
#include "glcd_font.h"
#include "glcd_fonts.h"
#include "adc.h"

static struct time        g_clock;
static int                g_clock_updated;
static struct led_display g_led_display;

void aclock_init(void)
{
	BSP_LED_Init(LED3);

	if (adc_tv_init())
	{
		Error_Handler();
	}

	BSP_EPD_Init();
	led_display_init(&g_led_display);
	g_led_display.dp_mask = 1 << 2;
	led_display_on(&g_led_display);
}

void aclock_sec_handler(void)
{
	BSP_LED_Toggle(LED3);
	if (clock_sec(&g_clock)) {
		g_clock_updated = 1;
	}
	led_display_show(&g_led_display, g_clock.sec, g_clock.min);
}

void aclock_tick_handler(void)
{
	led_display_refresh(&g_led_display);
}

static void aclock_epd_update(void)
{
	struct adc_tv tv;
	struct adc_tv_str tvs;
	char str[TIME_BUFF_SZ];

	if (adc_tv_get(&tv))
	{
		Error_Handler();
	}
	if (adc_tv_str(&tv, &tvs))
	{
		Error_Handler();
	}

	get_time_str(&g_clock, str);
	BSP_EPD_Clear(EPD_COLOR_WHITE);
	glcd_print_str(0,  15, "00:00",   &g_font_Tahoma9x12Clk, 2);
	glcd_print_str(0,  12, tvs.v_str, &g_font_Tahoma12x11Bld, 2);
	glcd_print_str(55, 13, tvs.t_str, &g_font_Tahoma19x20, 1);
	glcd_print_str_r(LCD_WIDTH, 13, "86%", &g_font_Tahoma19x20, 1);
	glcd_print_str_r(LCD_WIDTH, 0, str, &g_font_Tahoma29x48Clk, 6);
	BSP_EPD_RefreshDisplay();
}

void aclock_loop(void)
{
	/* Infinite loop */
	g_clock_updated = 1;
	for (;;)
	{
		if (g_clock_updated)
		{
			g_clock_updated = 0;
			aclock_epd_update();
		}
	}
}
