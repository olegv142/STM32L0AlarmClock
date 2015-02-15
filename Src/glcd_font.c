/*
 * The purpose of this module is to provide support for the GLCD lib fonts since they can be
 * created by freely available tools like MicoElectronica GLCD fonts creator.
 */

#include "glcd_font.h"

#include "..\Components\gde021a1\gde021a1.h"

static EPD_DrvTypeDef *epd_drv = &gde021a1_drv;

/* Put char in the specified position. Note that y is in 4 pixel groups */
static void glcd_draw_char(unsigned x, unsigned y, char c, uint8_t w, uint8_t h, uint8_t const* data)
{
	unsigned row, col, high = 1;
	unsigned col_bytes = (h + 1) / 2;

	epd_drv->SetDisplayWindow(x, y, x + w - 1, y + h - 1);

	for (col = 0; col < w; ++col)
	{
		uint8_t const* d = data + col_bytes - 1;
		high = !(h & 1);
		for (row = 0; row < h; ++row)
		{
			uint8_t b, p = ~0;
			uint8_t bit = 1, bit2 = 3;
			if (high) {
				b = *d >> 4;
				high = 0;
			} else {
				b = *d & 0xf;
				high = 1;
				--d;
			}
			for (; bit2; bit <<= 1, bit2 <<= 2) {
				if (b & bit)
					p ^= bit2;
			}
			epd_drv->WritePixel(p);
		}
		data += col_bytes;
	}
}

/* Print string starting from the specified position. If spacing < 0 the font will be treated as mono spacing,
 * otherwise the specified spacing will be used for variable spacing print.
 */
void glcd_print_str(unsigned x, unsigned y, const char* str, struct glcd_font const* font, int spacing)
{
	unsigned h = (font->h + 3) / 4;
	unsigned empty_space = spacing > 0 ? spacing : 0;
	for (;; ++str) {
		char c = *str;
		if (glcd_font_sym_valid(font, c)) {
			uint8_t const* data = glcd_font_sym_data(font, c);
			uint8_t w = spacing < 0 ? font->w : *data;
			glcd_draw_char(x, y, c, w, h, data + 1);
			x += w + empty_space;
		} else
			break;
	}
}

/* Calculate printed text length */
unsigned glcd_printed_len(const char* str, struct glcd_font const* font, int spacing)
{
	unsigned len = 0, empty_space = spacing > 0 ? spacing : 0;
	for (;; ++str) {
		char c = *str;
		if (glcd_font_sym_valid(font, c)) {
			uint8_t const* data = glcd_font_sym_data(font, c);
			uint8_t w = spacing < 0 ? font->w : *data;
			len += w + empty_space;
		} else
			break;
	}
	return len;
}

