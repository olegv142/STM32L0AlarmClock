#include "btn.h"

void btn_init(struct btn* b)
{
	b->ticks = 0;
	b->history = 0;
	b->events = 0;
}

void btn_update(struct btn* b, int pressed)
{
	uint16_t prev = b->history;
	b->history <<= 1;
	if (pressed)
		b->history |= 1;
	++b->ticks;
	if (!prev == !b->history)
		return;
	if (!b->history) {
		b->released = 1;
		if (b->ticks >= LONG_PRESS)
			b->long_pressed = 1;
	}
	b->ticks = 0;
}
