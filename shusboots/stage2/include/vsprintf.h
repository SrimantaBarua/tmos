// (C) 2019 Srimanta Barua
//
// Interface for text formatting


#pragma once


#include <stddef.h>
#include <stdarg.h>


int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

int vsprintf(char *buf, const char *fmt, va_list ap);

int snprintf(char *buf, size_t size, const char *fmt, ...);

int sprintf(char *buf, const char *fmt, ...);
