// (C) 2019 Srimanta Barua

#pragma once

// Compute max of two numbers
#define MAX(x, y) ((x) > (y) ? (x) : (y))

// Compute min of two numbers
#define MIN(x, y) ((x) < (y) ? (x) : (y))

// Compute positive difference of two numbers
#define DIFF(x, y) ((x) > (y) ? (x) - (y) : (y) - (x))

// Round up x to be a multiple of y (y should be a multiple of 2)
#define ROUND_UP_2(x, y) ((x) & ~((y) - 1))

// Round down value x to be a multiple of y
#define ROUND_DOWN_2(x, y) (((x) + ((y) - 1)) & ((y) - 1))
