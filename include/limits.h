// (C) 2018 Srimanta Barua

#pragma once


#define CHAR_BIT   8        // Number of bits in a 'char'
#define CHAR_MAX   255      // Max. value of 'char'
#define CHAR_MIN   0        // Min. value of 'char'
#define UCHAR_MAX  CHAR_MAX // Max. value of 'unsigned char'
#define SCHAR_MAX  127      // Max. value of 'signed char'
#define SCHAR_MIN  -128     // Min. value of 'signed char'


#define SHRT_MAX   32767    // Max. value of 'short'
#define SHRT_MIN   -32768   // Min. value of 'short'
#define USHRT_MAX  65535    // Max. value of 'unsigned short'


#define INT_MAX    2147483647
#define INT_MIN    -2147483648
#define UINT_MAX   4294967295


#define LONG_BIT   32
#define LONG_MAX   2147483647
#define LONG_MIN   -2147483648
#define ULONG_MAX  4294967295


#if defined(__LONG_LONG_MAX__) && (__LONG_LONG_MAX > 0x7fffffff)

#define LLONG_MAX  __LONG_LONG_MAX
#define LLONG_MIN  (-__LONG_LONG_MAX - 1)
#define ULLONG_MAX (__LONG_LONG_MAX__ * 2ULL) + 1

#else

#define LLONG_MAX  9223372036854775807LL
#define LLONG_MIN  -9223372036854775808LL
#define ULLONG_MAX 18446744073709551615ULL

#endif


#define MB_LEN_MAX 1

//#define SSIZE_MAX
//#define WORD_BIT
