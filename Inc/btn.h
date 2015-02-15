#pragma once

#include <stdint.h>

#define LONG_PRESS 2000

struct btn {
	uint32_t ticks;
	uint16_t history;
	union {
		uint16_t events;
		struct {
			uint16_t released:1;
			uint16_t long_pressed:1;
		};
	};
};

void btn_init(struct btn* b);

void btn_update(struct btn* b, int pressed);

static inline int btn_is_pending(struct btn* b)
{
	if (b->history != 0)
		return 1;
	if (b->events != 0)
		return 1;
	return 0;
}

static inline int btn_has_events(struct btn* b)
{
	return b->events != 0;
}
