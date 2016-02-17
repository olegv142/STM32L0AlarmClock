#include "led_display.h"
#include "stm32l0xx_hal.h"
#include <string.h>

/*
  cathodes:
   seg a    PA2   P3.13
   seg b    PA3   P3.14
   seg c    PA4   P3.15
   seg d    PA5   P3.16
   seg e    PA6   P3.17
   seg f    PA7   P3.18
   seg g    PB0   P3.19
   seg dp   PB1   P3.20
  anodes (dig 0 is the rightmost one):
   dig 0    PB12  P2.23
   dig 1    PB13  P2.22
   dig 2    PB14  P2.21
   dig 3    PB15  P2.20
*/

#define SEG_A_MASK  0xfc
#define SEG_B_MASK  0x3
#define DIG_B_MASK  0xf000

#define SEG_A_MASK_  0x5550
#define SEG_B_MASK_  0x5
#define DIG_B_MASK_  0x55000000

#define SEG_A_MASK2  (SEG_A_MASK_|(SEG_A_MASK_<<1))
#define SEG_B_MASK2  (SEG_B_MASK_|(SEG_B_MASK_<<1))
#define DIG_B_MASK2  (DIG_B_MASK_|(DIG_B_MASK_<<1))

#define A_MASK SEG_A_MASK
#define B_MASK (SEG_B_MASK|DIG_B_MASK)

#define A_MASK_ SEG_A_MASK_
#define B_MASK_ (SEG_B_MASK_|DIG_B_MASK_)

#define A_MASK2 SEG_A_MASK2
#define B_MASK2 (SEG_B_MASK2|DIG_B_MASK2)

#define BLINK_BIT 0x200

enum {
	_a_ = 1,
	_b_ = 2,
	_c_ = 4,
	_d_ = 8,
	_e_ = 16,
	_f_ = 32,
	_g_ = 64,
};

static unsigned char disp_char_map[] = {
	
	['0'] = _a_|_b_|_c_|_d_|_e_|_f_ ,
	['1'] = _b_|_c_,
	['2'] = _a_|_b_|_g_|_e_|_d_,
	['3'] = _a_|_b_|_c_|_d_|_g_,
	['4'] = _f_|_g_|_b_|_c_,
	['5'] = _a_|_f_|_g_|_c_|_d_,
	['6'] = _a_|_f_|_g_|_c_|_d_|_e_,
	['7'] = _a_|_b_|_c_,
	['8'] = _a_|_b_|_c_|_d_|_e_|_f_|_g_,
	['9'] = _a_|_b_|_c_|_d_|_f_|_g_,
	['-'] = _g_,
};

static inline void set_reg_bits(uint32_t volatile* r, uint32_t val, uint32_t mask)
{
	*r = (*r & ~mask) | (val & mask);
}

static void led_display_init_ports(void)
{
	/* Configure all as outputs */
	set_reg_bits(&GPIOA->MODER, A_MASK_, A_MASK2); 
	set_reg_bits(&GPIOB->MODER, B_MASK_, B_MASK2);
	/* Open drain for segments */
	set_reg_bits(&GPIOA->OTYPER, SEG_A_MASK, A_MASK);
	set_reg_bits(&GPIOB->OTYPER, SEG_B_MASK, B_MASK);
	/* Low seed */
	set_reg_bits(&GPIOA->OSPEEDR, 0, A_MASK2);
	set_reg_bits(&GPIOB->OSPEEDR, 0, B_MASK2);
	/* No pullup */
	set_reg_bits(&GPIOA->PUPDR, 0, A_MASK2);
	set_reg_bits(&GPIOB->PUPDR, 0, B_MASK2);
}

static void led_display_update(struct led_display* ld)
{
	unsigned dig = 1 << ld->active;
	unsigned segs = disp_char_map[ld->dig[ld->active]];
	int blinking = ld->blink_mask & dig;
	if (ld->dp_mask & dig)
		segs |= 1 << 7;
	/* Turn off for update */
	set_reg_bits(&GPIOB->ODR, 0, DIG_B_MASK);
	if (blinking && (HAL_GetTick() & BLINK_BIT))
		return;
	/* Update segment outputs */
	segs = ~segs;
	set_reg_bits(&GPIOA->ODR, segs << 2, SEG_A_MASK);
	set_reg_bits(&GPIOB->ODR, segs >> 6, SEG_B_MASK);
	/* Turn on digit */
	set_reg_bits(&GPIOB->ODR, dig << 12, DIG_B_MASK);
}

void led_display_init(struct led_display* ld)
{
	memset(ld, 0, sizeof(*ld));
	led_display_init_ports();
	led_display_off(ld);
}

void led_display_on(struct led_display* ld)
{
	ld->active = 0;
	led_display_update(ld);
}

void led_display_off(struct led_display* ld)
{
	ld->active = -1;
	/* Turn off all digits / segments */
	set_reg_bits(&GPIOA->ODR, 0, A_MASK);
	set_reg_bits(&GPIOB->ODR, 0, B_MASK);
}

void led_display_refresh(struct led_display* ld)
{
	if (ld->active < 0)
		return;
	if (++ld->active >= LED_DIGS)
		ld->active = 0;
	led_display_update(ld);
}

void led_display_show(struct led_display* ld, unsigned low, unsigned high)
{
	ld->dig[0] = '0' + (low % 10);
	ld->dig[1] = '0' + (low / 10);
	ld->dig[2] = '0' + (high % 10);
	ld->dig[3] = '0' + (high / 10);
}

void led_display_clear(struct led_display* ld)
{
	int i;
	for (i = 0; i < LED_DIGS; ++i)
		ld->dig[i] = 0;
	ld->dp_mask = ld->blink_mask = 0;
}
