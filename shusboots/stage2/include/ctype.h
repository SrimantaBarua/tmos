// (C) 2019 Srimanta Barua
//
// Test for character types

#pragma once

#define _U    0x01    // Uppercase
#define _L    0x02    // Lowercase
#define _D    0x04    // Digit
#define _C    0x08    // Cntrl
#define _P    0x10    // Punctuation
#define _S    0x20    // Space
#define _X    0x40    // Hex digit
#define _SP   0x80    // Hard space (0x20)

extern unsigned char _ctype[];

#define isalnum(c) ((_ctype + 1)[(unsigned) c] & (_U | _L | _D))
#define isalpha(c) ((_ctype + 1)[(unsigned) c] & (_U | _L))
#define iscntrl(c) ((_ctype + 1)[(unsigned) c] & (_C))
#define isdigit(c) ((_ctype + 1)[(unsigned) c] & (_D))
#define isgraph(c) ((_ctype + 1)[(unsigned) c] & (_P | _U | _L | _D))
#define islower(c) ((_ctype + 1)[(unsigned) c] & (_L))
#define isprint(c) ((_ctype + 1)[(unsigned) c] & (_P | _U | _L | _D | _SP))
#define ispunct(c) ((_ctype + 1)[(unsigned) c] & (_P))
#define isspace(c) ((_ctype + 1)[(unsigned) c] & (_S))
#define isupper(c) ((_ctype + 1)[(unsigned) c] & (_U))
#define isxdigit(c) ((_ctype + 1)[(unsigned) c] & (_D | _X))

#define isascii(c) (((unsigned) (c)) <= 0x7f)
#define toascii(c) (((unsigned) (c)) & 0x7f)

#define tolower(c) (isupper(c) ? (c) - ('A' - 'a') : (c))
#define toupper(c) (islower(c) ? (c) - ('a' - 'A') : (c))
