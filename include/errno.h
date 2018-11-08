// (C) 2018 Srimanta Barua

#pragma once

// __errno is a function which return the address of the actual errno variable
extern int* __errno();
#define errno (*__errno())

#define EAGAIN 1           // Resource unavailable, try again
#define EEXIST 2           // File exists
#define EINTR  3           // Interrupted system call
#define EWOULDBLOCK EAGAIN // Same as EAGAIN
