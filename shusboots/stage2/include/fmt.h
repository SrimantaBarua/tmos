// Interface for text formatting

#pragma once

#include <stdarg.h>

// Format text according to the formatting arguments and then write using the callback function
void fmt_write(void (*cb) (const char *str), const char *fmt, va_list ap);
