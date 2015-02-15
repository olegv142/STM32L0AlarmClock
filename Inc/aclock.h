#pragma once

#include "clock.h"
#include "led_display.h"
#include "adc.h"
#include "btn.h"

typedef enum {
	displ_off,
	displ_on,
	displ_show_hm,
	displ_show_ms,
} display_mode_t;

struct alarm_clock;

typedef void (*aclock_handler_t)(struct alarm_clock*);

struct alarm_clock {
	/* The clock */
	struct time        clock;
	struct time        alarm;	
	/* LED display */
	struct led_display display;
	display_mode_t     display_mode;
	/* Buttons */
	struct btn         btn_mode;
	struct btn         btn_set;
	/* Button handler */
	aclock_handler_t   handler;
	/* Seconds clock */
	unsigned           sec_clock;
	unsigned           sec_ticks;
	/* Last events time */
	unsigned           last_evt_sec;
	/* Status flags */
	int8_t             clock_updated;
	int8_t             alarm_enabled;
	int8_t             alarm_signalling;
	int8_t             alarm_pulse;
};

void aclock_init(void);
void aclock_sec_handler(void);
void aclock_tick_handler(void);
void aclock_loop(void);

void aclock_set_mode(struct alarm_clock* ac, display_mode_t dm, aclock_handler_t h);
