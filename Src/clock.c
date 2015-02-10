#include "clock.h"

/* Updates current time, returns 1 in case the time in hou:min is changed */ 
int clock_sec(struct time* t)
{
	++t->sec;
	if (t->sec < 60)
		return 0;
	t->sec = 0;
	/* next minute */
	++t->min;
	if (t->min < 60)
		return 1;
	t->min = 0;
	/* next hour */
	++t->hou;
	if (t->hou < 24)
		return 1;
	t->hou = 0;
	return 1;
}

/* Place time in hou:min onto the str argument */
void get_time_str(struct time const* t, char str[TIME_BUFF_SZ])
{
	int i = 0;
	unsigned v;
	v = t->hou / 10;
	if (v != 0) {
		str[i] = '0' + v;
		++i;
	}
	v = t->hou % 10;
	str[i] = '0' + v;
	++i;
	str[i] = ':';
	++i;
	v = t->min / 10;
	str[i] = '0' + v;
	++i;
	v = t->min % 10;
	str[i] = '0' + v;
	++i;
	str[i] = 0;
}
