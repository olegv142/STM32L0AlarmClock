#pragma once

#include <stdint.h>

#define LONG_PRESS_TOUT 2000

typedef enum {
	btn_none = 0,
	btn_released,
	btn_long_pressed,
} btn_evt_t;

struct btn {
	uint16_t  ticks;
	uint16_t  history;
	btn_evt_t evt;
	uint32_t  evt_epoch;
	uint32_t  handled_epoch;
};

void btn_init(struct btn* b);

void btn_update(struct btn* b, int pressed);

static inline int btn_has_event(struct btn* b)
{
	return b->handled_epoch != b->evt_epoch;
}

static inline btn_evt_t btn_get_event(struct btn* b)
{
	return btn_has_event(b) ? b->evt : btn_none;
}

static inline void btn_set_event_handled(struct btn* b)
{
	b->handled_epoch = b->evt_epoch;
}

static inline int btn_is_pending(struct btn* b)
{
	return b->history != 0 || btn_has_event(b);
}
