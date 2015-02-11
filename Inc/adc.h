#pragma once

int adc_init(unsigned ch[], unsigned nch);
int adc_conv(unsigned res[], unsigned nch);

struct adc_tv {
	union {
		struct {
			unsigned v;
			unsigned t;
		};
		unsigned conv_res[2];
	};
	int  t_val;
	int  v_val;
};

#define TV_BUFF_SZ 8

struct adc_tv_str {
	char t_str[TV_BUFF_SZ];
	char v_str[TV_BUFF_SZ];
};

int adc_tv_init(void);
int adc_tv_get(struct adc_tv* tv);
int adc_tv_str(struct adc_tv const* tv, struct adc_tv_str* tvs);
