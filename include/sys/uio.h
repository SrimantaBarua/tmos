// (C) 2018 Srimanta Barua

#pragma once

#include <stddef.h>

struct iovec {
	void *iov_base; // Base address of a memory region for input or output
	size_t iov_len; // The size of memory pointed to by iov_base
};

ssize_t readv(int, const struct iovec*, int);
ssize_t writev(int, const struct iovec*, int);
