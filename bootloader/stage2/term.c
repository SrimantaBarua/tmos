// (C) 2019 Srimanta Barua

#include <stdbool.h>
#include <term.h>

// Character dimensions
#define CHEIGHT 16 // Height in pixels
#define CWIDTH  8  // Width in pixels


// Static terminal state
static bool         _is_init = false;  // Has terminal been initialized?
static uint16_t     _height  = 0;      // Height in characters
static uint16_t     _width   = 0;      // Width in characters
static uint16_t     _cur_y   = 0;      // Cursor row
static uint16_t     _cur_x   = 0;      // Cursor column
static struct color _fg      = {.red = 0xff, .green = 0xff, .blue = 0xff};  // Foreground color

static const struct color _black   = {0};  // Black, terminal color


// Initialize terminal
void term_init(const struct vbe_mode_info *vbe_mode_info) {
	_height = vbe_mode_info->height / CHEIGHT;
	_width = vbe_mode_info->width / CWIDTH;
	display_init(vbe_mode_info);
	_is_init = true;
	term_reset();
	display_flush();
}


// Write string to terminal
void term_write_str(const char *str) {
	if (!_is_init || !str) {
		return;
	}
	while (*str) {
		if (*str == '\n') {
			_cur_x = 0;
			_cur_y++;
			if (_cur_y == _height) {
				display_scroll_down(CHEIGHT, _black);
				_cur_y--;
			} else {
				display_rect(_cur_y * CHEIGHT, 0, _cur_y * CHEIGHT + CHEIGHT - 1,
						CWIDTH * _width - 1, _black);
			}
		} else if (*str == '\r') {
			_cur_x = 0;
		} else if (*str == '\t') {
			_cur_x = (_cur_x + 8) & (~7);
			if (_cur_x >= _width) {
				_cur_x = 0;
				_cur_y++;
				if (_cur_y == _height) {
					display_scroll_down(CHEIGHT, _black);
					_cur_y--;
				}
			}
		} else {
			display_rect(_cur_y * CHEIGHT, _cur_x * CWIDTH,
					_cur_y * CHEIGHT + CHEIGHT - 1,
					_cur_x * CWIDTH + CWIDTH - 1,
					_black);
			display_char(*str, _cur_y * CHEIGHT, _cur_x * CWIDTH, _fg);
			_cur_x++;
			if (_cur_x == _width) {
				_cur_x = 0;
				_cur_y++;
				if (_cur_y == _height) {
					display_scroll_down(CHEIGHT, _black);
					_cur_y--;
				}
			}
		}
		str++;
	}
}


// Reset terminal, clear screen
void term_reset() {
	_cur_y = _cur_x = 0;
	display_clear(_black);
}


// Change color of text on terminal
void term_set_fg_color(struct color fg) {
	_fg = fg;
}
