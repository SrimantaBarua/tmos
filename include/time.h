// (C) 2018 Srimanta Barua

#pragma once

#include <sys/types.h>


struct timespec {
	time_t tv_sec;  // Seconds
	long   tv_nsec; // Nanoseconds
};
