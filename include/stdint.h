// (C) 2018 Srimanta Barua

#pragma once

#include <limits.h>


#ifdef __INT8_TYPE__
typedef __INT8_TYPE__ int8_t;
#else
#error "__INT8_TYPE__ not defined"
#endif

#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ uint8_t;
#else
typedef unsigned int8_t uint8_t;
#endif

#define INT8_MAX  127
#define INT8_MIN  -128
#define UINT8_MAX 255


#ifdef __INT16_TYPE__
typedef __INT16_TYPE__ int16_t;
#else
#error "__INT16_TYPE__ not defined"
#endif

#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ uint16_t;
#else
typedef unsigned int16_t uint16_t;
#endif

#define INT16_MAX  32767
#define INT16_MIN  -32768
#define UINT16_MAX 65535


#ifdef __INT32_TYPE__
typedef __INT32_TYPE__ int32_t;
#else
#error "__INT32_TYPE__ not defined"
#endif

#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ uint32_t;
#else
typedef unsigned int32_t uint32_t;
#endif

#define INT32_MAX  2147483647
#define INT32_MIN  -2147483648
#define UINT32_MAX 4294967295


#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ int64_t;
#else
#error "__INT64_TYPE__ not defined"
#endif

#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ uint64_t;
#else
typedef unsigned int64_t uint64_t;
#endif

#define INT64_MAX  9223372036854775807LL
#define INT64_MIN  -9223372036854775808LL
#define UINT64_MAX 18446744073709551615ULL


#ifdef __INT_LEAST8_TYPE__
typedef __INT_LEAST8_TYPE__ int_least8_t;
#else
#error "__INT_LEAST8_TYPE__ not defined"
#endif

#ifdef __UINT_LEAST8_TYPE__
typedef __UINT_LEAST8_TYPE__ uint_least8_t;
#else
typedef unsigned int_least8_t uint_least8_t;
#endif


#ifdef __INT_LEAST16_TYPE__
typedef __INT_LEAST16_TYPE__ int_least16_t;
#else
#error "__INT_LEAST16_TYPE__ not defined"
#endif

#ifdef __UINT_LEAST32_TYPE__
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
#else
typedef unsigned int_least32_t uint_least32_t;
#endif


#ifdef __INT_LEAST32_TYPE__
typedef __INT_LEAST32_TYPE__ int_least32_t;
#else
#error "__INT_LEAST32_TYPE__ not defined"
#endif

#ifdef __UINT_LEAST32_TYPE__
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
#else
typedef unsigned int_least32_t uint_least32_t;
#endif


#ifdef __INT_LEAST64_TYPE__
typedef __INT_LEAST64_TYPE__ int_least64_t;
#else
#error "__INT_LEAST64_TYPE__ not defined"
#endif

#ifdef __UINT_LEAST64_TYPE__
typedef __UINT_LEAST64_TYPE__ uint_least64_t;
#else
typedef unsigned int_least64_t uint_least64_t;
#endif


#ifdef __INTPTR_TYPE__
typedef __INTPTR_TYPE__ intptr_t;
#else
#error "__INTPTR_TYPE__ not defined"
#endif

#ifdef __UINTPTR_TYPE__
typedef __UINTPTR_TYPE__ uintptr_t;
#else
typedef unsigned intptr_t uintptr_t;
#endif

#if (!defined __INTPTR_MAX__) || (!defined __UINTPTR_MAX__)
#error "__INTPTR_MAX__ and __UINTPTR_MAX__ should be defined"
#else
#define INTPTR_MAX __INTPTR_MAX__
#define INTPTR_MIN (-__INTPTR_MAX__ - 1)
#define UINTPTR_MAX __UINTPTR_MAX__
#endif


#ifdef __SIZE_MAX__
#define SIZE_MAX __SIZE_MAX__
#else
#error "__SIZE_MAX__ not defined"
#endif
