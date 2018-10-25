// (C) 2018 Srimanta Barua
//
// Interface for writing formatted text to buffer

#pragma once

#include <stdarg.h>
#include <stddef.h>

int vsprintf(char *buf, const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, size_t size, const char *fmt, ...);
