// (C) 2019 Srimanta Barua
//
// A small subset (with some different functions) from C standard library's stdlib.h


#pragma once

// String to integer functions

// String to int. Return 0 if could not parse integer
int atod(const char *s);

// String to unsigned int. Return 0 if could not parse integer
unsigned atou(const char *s);

// String to long long int. Return 0 if could not parse integer
long long atolld(const char *s);

// String to unsigned long long int. Return 0 if could not parse integer
unsigned long long atollu(const char *s);


// String to integer functions (with skip)

// String to int. Return 0 if could not parse integer. Otherwise move string forward
int skip_atod(const char **sptr);

// String to unsigned int. Return 0 if could not parse integer. Otherwise move string forward
unsigned skip_atou(const char **sptr);

// String to long long int. Return 0 if could not parse integer. Otherwise move string forward
long long skip_atolld(const char **sptr);

// String to unsigned long long int. Return 0 if could not parse integer. Otherwise move string forward
unsigned long long skip_atollu(const char **sptr);
