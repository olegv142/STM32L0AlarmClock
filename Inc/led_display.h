#pragma once

#define LED_DIGS 4

struct led_display {
	char     dig[LED_DIGS];
	unsigned dp_mask;
	unsigned blink_mask;
	int      active;
};

void led_display_init(struct led_display* ld);
void led_display_on(struct led_display* ld);
void led_display_off(struct led_display* ld);
void led_display_refresh(struct led_display* ld);
void led_display_clear(struct led_display* ld);
void led_display_show(struct led_display* ld, unsigned low, unsigned high);

static inline int led_display_low(struct led_display* ld)
{
	return ld->dig[0] - '0' + 10 * (ld->dig[1] - '0');
}

static inline int led_display_high(struct led_display* ld)
{
	return ld->dig[2] - '0' + 10 * (ld->dig[3] - '0');
}

