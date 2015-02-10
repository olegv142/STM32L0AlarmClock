#pragma once

#include <stdint.h>

/*
 * The purpose of this module is to provide support for the GLCD lib fonts since they can be
 * created by freely available tools like MicoElectronica GLCD fonts creator.
 */

struct glcd_font {
	uint8_t w;
	uint8_t h;
	uint8_t code_off;
	uint8_t code_num;
	uint8_t const* data;
};

static inline unsigned glcd_font_sym_valid(struct glcd_font const* font, char c)
{
	return c && (uint8_t)c >= font->code_off && (uint8_t)c < font->code_off + font->code_num;
}

static inline unsigned glcd_font_sym_bytes(struct glcd_font const* font)
{
	unsigned col_bytes = (font->h + 7) / 8;
	return 1 + col_bytes * font->w;
}

static inline uint8_t const* glcd_font_sym_data(struct glcd_font const* font, char c)
{
	return font->data + (c - font->code_off) * glcd_font_sym_bytes(font);
}

/* Print string starting from the specified position. If spacing < 0 the font will be treated as mono spacing,
 * otherwise the specified spacing will be used for variable spacing print. Note that y is in 4 pixel groups.
 */
void glcd_print_str(unsigned x, unsigned y, const char* str, struct glcd_font const* font, int spacing);

#define MONO_SPACING (-1)
