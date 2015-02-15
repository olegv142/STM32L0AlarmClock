#include "main.h"
#include "glcd_font.h"
#include "glcd_fonts.h"
#include "aclock_ctl.h"

/* MODE button */
#define MODE_PORT GPIOA
#define MODE_PIN  1

/* SET button */
#define SET_PORT GPIOH
#define SET_PIN 2

struct alarm_clock g_aclock;

static void aclock_btn_init(GPIO_TypeDef* port, unsigned pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Configure Button pin as input */
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

static void aclock_btn_init_exti(GPIO_TypeDef* port, unsigned pin)
{	
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Configure Button pin as input with External interrupt */
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	HAL_GPIO_Init(port, &GPIO_InitStruct);

	/* Enable and set Button EXTI Interrupt to the lowest priority */
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

static inline int aclock_btn_read(GPIO_TypeDef* port, unsigned pin)
{
	return port->IDR & pin; 
}

void aclock_init(void)
{
	if (adc_tv_init())
	{
		Error_Handler();
	}

	BSP_EPD_Init();

	aclock_btn_init(SET_PORT,  SET_PIN);
	aclock_btn_init(MODE_PORT, MODE_PIN);
	aclock_btn_init_exti(MODE_PORT, MODE_PIN);

	led_display_init(&g_aclock.display);
	aclock_set_mode(&g_aclock, displ_off, aclock_idle_handler);
}

void aclock_sec_handler(void)
{
	++g_aclock.sec_clock;
	if (clock_sec(&g_aclock.clock)) {
		g_aclock.clock_updated = 1;
		if (g_aclock.display_mode == displ_show_hm)
			led_display_show(&g_aclock.display, g_aclock.clock.min, g_aclock.clock.hou);
	}
	if (g_aclock.display_mode == displ_show_ms)
		led_display_show(&g_aclock.display, g_aclock.clock.sec, g_aclock.clock.min);
}

void aclock_tick_handler(void)
{
	led_display_refresh(&g_aclock.display);
	btn_update(&g_aclock.btn_mode, aclock_btn_read(MODE_PORT, MODE_PIN));
	btn_update(&g_aclock.btn_mode, aclock_btn_read(SET_PORT,  SET_PIN));
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

	get_time_str(&g_aclock.clock, str);
	BSP_EPD_Clear(EPD_COLOR_WHITE);
	glcd_print_str(0,  15, "00:00",   &g_font_Tahoma9x12Clk, 2);
	glcd_print_str(0,  12, tvs.v_str, &g_font_Tahoma12x11Bld, 2);
	glcd_print_str(55, 13, tvs.t_str, &g_font_Tahoma19x20, 1);
	glcd_print_str_r(LCD_WIDTH, 13, "86%", &g_font_Tahoma19x20, 1);
	glcd_print_str_r(LCD_WIDTH, 0, str, &g_font_Tahoma29x48Clk, 6);
	BSP_EPD_RefreshDisplay();
}

static void aclock_sleep(void)
{
	HAL_SuspendTick();
	/* Enter Sleep Mode, wake up is done once MODE push button is pressed */
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); 
	HAL_ResumeTick();
}

void aclock_loop(void)
{
	/* Infinite loop */
	g_aclock.clock_updated = 1;
	for (;;)
	{
		if (g_aclock.clock_updated)
		{
			g_aclock.clock_updated = 0;
			aclock_epd_update();
		}
		if (btn_has_events(&g_aclock.btn_mode) || btn_has_events(&g_aclock.btn_set))
		{
			g_aclock.handler(&g_aclock);
			g_aclock.btn_mode.events = g_aclock.btn_set.events = 0;
			g_aclock.last_evt_sec = g_aclock.sec_clock;
		}
		if (g_aclock.display_mode == displ_off && !btn_is_pending(&g_aclock.btn_mode) && !btn_is_pending(&g_aclock.btn_set)) {
			aclock_sleep();
		} else if ((int)(g_aclock.sec_clock - g_aclock.last_evt_sec) > DISPLAY_TIMEOUT) {
			aclock_set_mode(&g_aclock, displ_off, aclock_idle_handler);
		}
	}
}

void aclock_set_mode(struct alarm_clock* ac, display_mode_t dm, aclock_handler_t h)
{
	ac->handler = h;
	if (dm == ac->display_mode)
		return;
	if (dm == displ_off) {
		led_display_off(&ac->display);
	} else {
		led_display_clear(&ac->display);
		switch (dm) {
		case displ_show_hm:
			led_display_show(&g_aclock.display, g_aclock.clock.min, g_aclock.clock.hou);
			break;
		case displ_show_ms:
			g_aclock.display.dp_mask = 1 << 2;
			led_display_show(&g_aclock.display, g_aclock.clock.sec, g_aclock.clock.min);
			break;
		}
		led_display_on(&ac->display);
	}
	ac->display_mode = dm;
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_0) {
		btn_update(&g_aclock.btn_mode, 1);
	}
}
