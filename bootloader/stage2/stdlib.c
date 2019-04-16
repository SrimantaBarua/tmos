#include <ctype.h>
#include <stdlib.h>


// String to int. Return 0 if could not parse integer
int atod(const char *s) {
	int ret = 0, mul = 1;
	if (!s) {
		return 0;
	}
	while (isspace(*s)) {
		s++;
	}
	if (*s == '-') {
		mul = -1;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + ((*s - '0') * mul);
		s++;
	}
	return ret;
}


// String to unsigned int. Return 0 if could not parse integer
unsigned atou(const char *s) {
	unsigned ret = 0;
	if (!s) {
		return 0;
	}
	while (isspace(*s)) {
		s++;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + (*s - '0');
		s++;
	}
	return ret;
}


// String to long long int. Return 0 if could not parse integer
long long atolld(const char *s) {
	long long ret = 0, mul = 1;
	if (!s) {
		return 0;
	}
	while (isspace(*s)) {
		s++;
	}
	if (*s == '-') {
		mul = -1;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + ((*s - '0') * mul);
		s++;
	}
	return ret;
}


// String to unsigned long long int. Return 0 if could not parse integer
unsigned long long atollu(const char *s) {
	unsigned long long ret = 0;
	if (!s) {
		return 0;
	}
	while (isspace(*s)) {
		s++;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + (*s - '0');
		s++;
	}
	return ret;
}


// String to int. Return 0 if could not parse integer. Otherwise, move string forward
int skip_atod(const char **sptr) {
	const char *s;
	int ret = 0, mul = 1;
	if (!sptr || !*sptr) {
		return 0;
	}
	s = *sptr;
	while (isspace(*s)) {
		s++;
	}
	if (*s == '-') {
		mul = -1;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + ((*s - '0') * mul);
		s++;
	}
	*sptr = s;
	return ret;
}


// String to unsigned int. Return 0 if could not parse integer. Otherwise, move string forward
unsigned skip_atou(const char **sptr) {
	const char *s;
	unsigned ret = 0;
	if (!sptr || !*sptr) {
		return 0;
	}
	s = *sptr;
	while (isspace(*s)) {
		s++;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + (*s - '0');
		s++;
	}
	*sptr = s;
	return ret;
}


// String to long long int. Return 0 if could not parse integer. Otherwise, move string forward
long long skip_atolld(const char **sptr) {
	const char *s;
	long long ret = 0, mul = 1;
	if (!sptr || !*sptr) {
		return 0;
	}
	s = *sptr;
	while (isspace(*s)) {
		s++;
	}
	if (*s == '-') {
		mul = -1;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + ((*s - '0') * mul);
		s++;
	}
	*sptr = s;
	return ret;
}


// String to unsigned long long int. Return 0 if could not parse integer. Otherwise, move string forward
unsigned long long skip_atollu(const char **sptr) {
	const char *s;
	unsigned long long ret = 0;
	if (!sptr || !*sptr) {
		return 0;
	}
	s = *sptr;
	while (isspace(*s)) {
		s++;
	}
	while (isdigit(*s)) {
		ret = ret * 10 + (*s - '0');
		s++;
	}
	*sptr = s;
	return ret;
}
