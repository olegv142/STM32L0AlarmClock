#include "main.h"
#include "glcd_font.h"
#include "glcd_fonts.h"
#include "aclock_ctl.h"
#include "..\Components\gde021a1\gde021a1.h"
#include "htu21d.h"

/* MODE button */
#define MODE_PORT GPIOA
#define MODE_PIN  1

/* SET button */
#define SET_PORT GPIOH
#define SET_PIN  2

/* Alarm port */
#define ALARM_PORT GPIOH
#define ALARM_PIN  1

/* Alarm signal timing parameters */
#define ALARM_PULSE_BIT (1 << 6)
#define ALARM_PULSES 6

#define UI_TIMEOUT    120 /* 2 min */
#define ALARM_TIMEOUT 300 /* 5 min */

struct alarm_clock g_aclock;

static EPD_DrvTypeDef *epd_drv = &gde021a1_drv;

static void aclock_btn_init(GPIO_TypeDef* port, unsigned pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Configure Button pin as input */
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
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

static void aclock_alarm_pin_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Configure alarm pin as output */
	GPIO_InitStruct.Pin = ALARM_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(ALARM_PORT, &GPIO_InitStruct);
}

static inline void aclock_alarm_init(void)
{
	g_aclock.alarm_signalling = -1;
	aclock_alarm_pin_init();
}

static inline void aclock_alarm_signal_on(void)
{
	HAL_GPIO_WritePin(ALARM_PORT, ALARM_PIN, GPIO_PIN_SET);
}

static inline void aclock_alarm_signal_off(void)
{
	HAL_GPIO_WritePin(ALARM_PORT, ALARM_PIN, GPIO_PIN_RESET);
}

static inline int aclock_alarming(void)
{
	return g_aclock.alarm_signalling >= 0;
}

static inline int aclock_btn_read(GPIO_TypeDef* port, unsigned pin)
{
	return port->IDR & pin; 
}

static inline void aclock_set_idle_mode(void)
{
	aclock_set_mode(&g_aclock, displ_off, aclock_idle_handler);
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
	aclock_alarm_init();
	led_display_init(&g_aclock.display);
	aclock_set_idle_mode();
}

/* Called every second */
void aclock_sec_handler(void)
{
	++g_aclock.sec_clock;
	g_aclock.sec_ticks = 0;
	if (clock_sec(&g_aclock.clock)) {
		g_aclock.clock_updated = 1;
		if (g_aclock.display_mode == displ_show_hm) {
			led_display_show(&g_aclock.display, g_aclock.clock.min, g_aclock.clock.hou);
		}
	}
	if (g_aclock.display_mode == displ_show_ms) {
		led_display_show(&g_aclock.display, g_aclock.clock.sec, g_aclock.clock.min);
	}
	if (aclock_alarming()) {
		if (g_aclock.alarm_signalling < ALARM_PULSES) {
			++g_aclock.alarm_signalling;
		}
		g_aclock.alarm_pulse = 0;
	}
}

/* Called every millisecond */
void aclock_tick_handler(void)
{
	unsigned sec_ticks_ = g_aclock.sec_ticks;
	g_aclock.sec_ticks  = sec_ticks_ + 1;
	led_display_refresh(&g_aclock.display);
	btn_update(&g_aclock.btn_mode, aclock_btn_read(MODE_PORT, MODE_PIN));
	btn_update(&g_aclock.btn_set,  aclock_btn_read(SET_PORT,  SET_PIN));
	if (aclock_alarming() && g_aclock.alarm_pulse < g_aclock.alarm_signalling)
	{
		int a_ = sec_ticks_ & ALARM_PULSE_BIT, a = g_aclock.sec_ticks & ALARM_PULSE_BIT;
		if (a != a_) {
			if (a) {
				aclock_alarm_signal_on();
			} else {
				aclock_alarm_signal_off();
				++g_aclock.alarm_pulse;
			}
		}
	}
}

#define BUFF_SZ TIME_BUFF_SZ

/* Update EP display */
static void aclock_epd_update(void)
{
	struct adc_tv tv;
	struct adc_tv_str tvs;
	char buff[BUFF_SZ+1];
	buff[BUFF_SZ] = 0;
	/* Measure temperature / voltage */
	if (adc_tv_get(&tv))
	{
		Error_Handler();
	}
	if (adc_tv_str(&tv, &tvs))
	{
		Error_Handler();
	}
	/* Clear display */
	BSP_EPD_Clear(EPD_COLOR_WHITE);
	if (g_aclock.alarm_enabled) {
		/* Alarm time */
		get_time_str(&g_aclock.alarm, buff);
		glcd_print_str(0,  15, buff, &g_font_Tahoma9x12Clk, 2);
	}
	/* Temperature / voltage */
	glcd_print_str(0,  12, tvs.v_str, &g_font_Tahoma12x11Bld, 2);
	glcd_print_str(55, 13, tvs.t_str, &g_font_Tahoma19x20, 1);
	/* Humidity */
	htu21d_get_humidity_str(buff, BUFF_SZ);
	glcd_print_str_r(LCD_WIDTH, 13, buff, &g_font_Tahoma19x20, 1);
	/* Current time */
	get_time_str(&g_aclock.clock, buff);
	glcd_print_str_r(LCD_WIDTH, 0, buff, &g_font_Tahoma29x48Clk, 6);
	BSP_EPD_RefreshDisplay();
	/* Halt EPD charge pump to reduce power consumption */
	epd_drv->CloseChargePump();
}

/* Stop ticks and goes to sleep to save power */
static inline void aclock_sleep(void)
{
	HAL_SuspendTick();
	/* Enter Sleep Mode, wake up is done once MODE push button is pressed */
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); 
	HAL_ResumeTick();
}

/* Stop alarming */
static inline void aclock_alarm_stop(void)
{
	g_aclock.alarm_signalling = -1;
	aclock_alarm_signal_off();
	aclock_set_idle_mode();
}

/* Event handler for alarming */
static void aclock_alarm_handler(struct alarm_clock* ac)
{
	/* Stop on any event */
	aclock_alarm_stop();
}

/* Start alarming */
static inline void aclock_alarm_start(void)
{
	g_aclock.alarm_pulse = 0;
	g_aclock.alarm_signalling = 0;
	g_aclock.last_evt_sec = g_aclock.sec_clock;
	aclock_set_mode(&g_aclock, displ_show_hm, aclock_alarm_handler);
}

/* Check alarm activation */
static inline void aclock_check_alarm(void)
{
	if (!g_aclock.alarm_enabled)
		return;
	if (aclock_alarming())
		return;
	if (g_aclock.clock.hou == g_aclock.alarm.hou && g_aclock.clock.min == g_aclock.alarm.min)
		aclock_alarm_start();
}

/* The main loop, never return */
void aclock_loop(void)
{
	/* Infinite loop */
	g_aclock.clock_updated = 1;
	for (;;)
	{
		if (g_aclock.clock_updated)
		{
			g_aclock.clock_updated = 0;
			aclock_check_alarm();
			aclock_epd_update();
		}
		if (btn_has_event(&g_aclock.btn_mode) || btn_has_event(&g_aclock.btn_set))
		{
			g_aclock.handler(&g_aclock);
			btn_set_event_handled(&g_aclock.btn_mode);
			btn_set_event_handled(&g_aclock.btn_set);
			g_aclock.last_evt_sec = g_aclock.sec_clock;
		}
		if (!btn_is_pending(&g_aclock.btn_mode) && !btn_is_pending(&g_aclock.btn_set))
		{
			if (g_aclock.display_mode == displ_off) {
				aclock_sleep();
			} else {
				if (aclock_alarming()) {
					if ((int)(g_aclock.sec_clock - g_aclock.last_evt_sec) > ALARM_TIMEOUT) {
						aclock_alarm_stop();
					}
				} else {
					if ((int)(g_aclock.sec_clock - g_aclock.last_evt_sec) > UI_TIMEOUT) {
						aclock_set_idle_mode();
					}
				}
			}
		}
	}
}

/* Set mode and event handler routine */
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
