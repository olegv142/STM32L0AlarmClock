#include "aclock_ctl.h"
#include "aclock.h"
#include "main.h"

static void aclock_hm_handler(struct alarm_clock* ac);
static void aclock_ms_handler(struct alarm_clock* ac);
static void aclock_alarm_handler(struct alarm_clock* ac);
static void aclock_set_time_handler(struct alarm_clock* ac);
static void aclock_set_alarm_handler(struct alarm_clock* ac);

static inline void set_idle(struct alarm_clock* ac)
{
	aclock_set_mode(ac, displ_off, aclock_idle_handler);
	aclock_refresh(ac);
}

void aclock_idle_handler(struct alarm_clock* ac)
{
	switch (btn_get_event(&ac->btn_mode)) {
	case btn_long_pressed:
		ac->alarm_enabled = !ac->alarm_enabled;
		set_idle(ac);
		return;
	case btn_released:
		aclock_set_mode(ac, displ_show_hm, aclock_hm_handler);
		return;
	default:
		return;
	}
}

static void aclock_hm_handler(struct alarm_clock* ac)
{
	switch (btn_get_event(&ac->btn_mode)) {
	case btn_long_pressed:
		set_idle(ac);
		return;
	case btn_released:
		aclock_set_mode(ac, displ_show_ms, aclock_ms_handler);
		return;
	}
	switch (btn_get_event(&ac->btn_set)) {
	case btn_long_pressed:
		aclock_set_mode(ac, displ_show_hm, aclock_set_time_handler);
		ac->display.blink_mask = 1;
		return;
	}
}

static void aclock_ms_handler(struct alarm_clock* ac)
{
	switch (btn_get_event(&ac->btn_mode)) {
	case btn_long_pressed:
		set_idle(ac);		
		return;
	case btn_released:
		aclock_set_mode(ac, displ_show_alarm, aclock_alarm_handler);
		return;
	}
	switch (btn_get_event(&ac->btn_set)) {
	case btn_long_pressed:
	case btn_released:
		if (ac->clock.sec > 30) {
			++ac->clock.min;
		}
		ac->clock.sec = 0;
		return;
	}
}

static void aclock_alarm_handler(struct alarm_clock* ac)
{
	switch (btn_get_event(&ac->btn_mode)) {
	case btn_long_pressed:
	case btn_released:
		set_idle(ac);
		return;
	}
	switch (btn_get_event(&ac->btn_set)) {
	case btn_long_pressed:
		aclock_set_mode(ac, displ_show_alarm, aclock_set_alarm_handler);
		ac->display.blink_mask = 1;
		return;
	}
}

static int display_pos(struct alarm_clock* ac)
{
	int i = 0;
	unsigned b = 1;
	for (; i < LED_DIGS; ++i, b <<= 1) {
		if (ac->display.blink_mask & b)
			return i;
	}
	Error_Handler();
	return -1;
}

static void aclock_set_time_handler(struct alarm_clock* ac)
{
	int i;
	switch (btn_get_event(&ac->btn_set)) {
	case btn_long_pressed:
		ac->clock.hou = led_display_high(&ac->display);
		ac->clock.min = led_display_low(&ac->display);
		aclock_set_mode(ac, displ_show_hm, aclock_hm_handler);
		return;
	case btn_released:
		i = display_pos(ac);
		if (++ac->display.dig[i] > (i == LED_DIGS-1 ? '2' : '9')) {
			ac->display.dig[i] = '0';
		}
		return;
	}
	switch (btn_get_event(&ac->btn_mode)) {
	case btn_long_pressed:
	case btn_released:
		ac->display.blink_mask <<= 1;
		if (ac->display.blink_mask >= 1 << LED_DIGS) {
			ac->display.blink_mask = 1;
		}
		return;
	}
}

static void aclock_set_alarm_handler(struct alarm_clock* ac)
{
	int i;
	switch (btn_get_event(&ac->btn_set)) {
	case btn_long_pressed:
		ac->alarm.hou = led_display_high(&ac->display);
		ac->alarm.min = led_display_low(&ac->display);
		aclock_set_mode(ac, displ_show_alarm, aclock_alarm_handler);
		return;
	case btn_released:
		i = display_pos(ac);
		if (++ac->display.dig[i] > (i == LED_DIGS-1 ? '2' : '9')) {
			ac->display.dig[i] = '0';
		}
		return;
	}
	switch (btn_get_event(&ac->btn_mode)) {
	case btn_long_pressed:
	case btn_released:
		ac->display.blink_mask <<= 1;
		if (ac->display.blink_mask >= 1 << LED_DIGS) {
			ac->display.blink_mask = 1;
		}
		return;
	}
}
