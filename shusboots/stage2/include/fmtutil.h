// Utility functions for formatting, which could also be used elsewhere

#pragma once

#include <stdint.h>

char* itoa(int32_t val, char *buf, uint32_t base);
char* utoa(uint32_t val, char *buf, uint32_t base);
char* lltoa(int64_t val, char *buf, uint64_t base);
char* ulltoa(uint64_t val, char *buf, uint64_t base);
