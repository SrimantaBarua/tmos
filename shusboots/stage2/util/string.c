// (C) 2018 Srimanta Barua

#include <string.h>
#include <stdint.h>

void* memcpy(void *dstptr, const void *srcptr, size_t len) {
	uintptr_t dst = (uintptr_t) dstptr;
	uintptr_t src = (uintptr_t) srcptr;
	if (dst == src || len == 0) {
		return dstptr;
	}
	if (!(len & 3) && !(dst & 3) && !(src & 3)) {
		len >>= 2;
		__asm__ __volatile__ ("cld; rep movsd;\n"
				      : :"S"(src), "D"(dst), "c"(len)
				      :"memory");
		return dstptr;
	}
	__asm__ __volatile__ ("cld; rep movsb;\n"
			      : :"S"(src), "D"(dst), "c"(len)
			      :"memory");
	return dstptr;
}

void* memmove(void *dstptr, const void *srcptr, size_t len) {
	uintptr_t dst = (uintptr_t) dstptr;
	uintptr_t src = (uintptr_t) srcptr;
	if (dst <= src) {
		return memcpy (dstptr, srcptr, len);
	}
	if (!(len & 3) && !(dst & 3) && !(src & 3)) {
		dst += len - 4;
		src += len - 4;
		len >>= 2;
		__asm__ __volatile__ ("std; rep movsd; cld;\n"
				      : :"S"(src), "D"(dst), "c"(len)
				      :"memory");
		return dstptr;
	}
	dst += len - 1;
	src += len - 1;
	__asm__ __volatile__ ("std; rep movsb; cld;\n"
			      : :"S"(src), "D"(dst), "c"(len)
			      :"memory");
	return dstptr;
}

int memcmp(const void *a_ptr, const void *b_ptr, size_t len) {
        const uint8_t* a = (const uint8_t*) a_ptr;
        const uint8_t* b = (const uint8_t*) b_ptr;
        size_t index;
        for (index = 0; index < len; index++) {
                if (a[index] != b[index]) {
                        return ((int) a[index]) - ((int) b[index]);
                }
        }
        return 0;
}

void* memset(void *dstptr, int val, size_t len) {
	uintptr_t dst = (uintptr_t) dstptr;
	uint8_t byte = (uint8_t) (val & 0xff);
        if (!(dst & 3) && !(len & 3)) {
                uint32_t data = (uint32_t) byte << 24 | (uint32_t) byte << 16
			| (uint32_t) byte << 8 | (uint32_t) byte;
		len >>= 2;
                __asm__ __volatile__ ("rep stosd;\n"
				      : :"D"(dst), "a"(data), "c"(len)
				      :"memory");
                return dstptr;
        }
        __asm__ __volatile__ ("rep stosb;\n"
			      : :"D"(dst), "a"(byte), "c"(len)
			      :"memory");
	return dstptr;
}

size_t strlen(const char *s) {
	size_t len = 0;
	if (s) {
		while (s[len]) {
			len++;
		}
	}
	return len;
}

