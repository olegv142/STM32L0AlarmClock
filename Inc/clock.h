#pragma once

struct time {
	unsigned sec;
	unsigned min;
	unsigned hou;
};

/* Updates current time, returns 1 in case the time in hou:min is changed */ 
int clock_sec(struct time* t);

#define TIME_BUFF_SZ 6

/* Place time in hou:min onto the str argument */
void get_time_str(struct time const* t, char str[TIME_BUFF_SZ]);
