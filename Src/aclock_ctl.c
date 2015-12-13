#include "aclock_ctl.h"
#include "aclock.h"

void aclock_idle_handler(struct alarm_clock* ac)
{
	switch (btn_get_event(&ac->btn_mode)) {
	case btn_released:
		break;
	case btn_long_pressed:
		aclock_set_mode(ac, displ_off, aclock_idle_handler);
		return;
	default:
		return;
	}
	switch (ac->display_mode) {
	case displ_off:
		aclock_set_mode(ac, displ_show_hm, aclock_idle_handler);
		break;
	case displ_show_hm:
		aclock_set_mode(ac, displ_show_ms, aclock_idle_handler);
		break;
	case displ_show_ms:
		aclock_set_mode(ac, displ_off, aclock_idle_handler);
		break;
	}
}

