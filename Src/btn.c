#include "btn.h"

void btn_init(struct btn* b)
{
	b->ticks = 0;
	b->history = 0;
	b->evt = btn_none;
	b->evt_epoch = b->handled_epoch = 0;
}

void btn_update(struct btn* b, int pressed)
{
	uint16_t prev = b->history;
	b->history <<= 1;
	if (pressed) {
		b->history |= 1;
	}
	if (b->history) {
		++b->ticks;
		if (b->evt != btn_long_pressed && b->ticks >= LONG_PRESS_TOUT) {
			/* Report long press only once */
			b->evt = btn_long_pressed;
			++b->evt_epoch;
		}
	} else {
		if (prev) {
			/* Btn released */
			if (b->evt != btn_long_pressed) {
				++b->evt_epoch;
			}
			/* Don't increment epoch in case long press was reported.
			 * The client is expected to handle either of them but not both.
			 */
			b->evt = btn_released;
		}
		b->ticks = 0;
	}
}
