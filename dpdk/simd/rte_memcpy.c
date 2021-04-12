
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <mmintrin.h>

#ifndef likely
#define likely(exp) __builtin_expect(!!(exp), 1)
#endif 


void rte_mov16(uint8_t *dst, const uint8_t *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	*dst128 = *src128;
}

void rte_mov32(uint8_t *dst, const uint8_t *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	const __uint128_t x0 = src128[0], x1 = src128[1];
	dst128[0] = x0;
	dst128[1] = x1;
}

void rte_mov48(uint8_t *dst, const uint8_t *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	const __uint128_t x0 = src128[0], x1 = src128[1], x2 = src128[2];
	dst128[0] = x0;
	dst128[1] = x1;
	dst128[2] = x2;
}

void rte_mov64(uint8_t *dst, const uint8_t *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	const __uint128_t
		x0 = src128[0], x1 = src128[1], x2 = src128[2], x3 = src128[3];
	dst128[0] = x0;
	dst128[1] = x1;
	dst128[2] = x2;
	dst128[3] = x3;
}

void rte_mov128(uint8_t *dst, const uint8_t *src)
{
	__uint128_t *dst128 = (__uint128_t *)dst;
	const __uint128_t *src128 = (const __uint128_t *)src;
	/* Keep below declaration & copy sequence for optimized instructions */
	const __uint128_t
		x0 = src128[0], x1 = src128[1], x2 = src128[2], x3 = src128[3];
	dst128[0] = x0;
	__uint128_t x4 = src128[4];
	dst128[1] = x1;
	__uint128_t x5 = src128[5];
	dst128[2] = x2;
	__uint128_t x6 = src128[6];
	dst128[3] = x3;
	__uint128_t x7 = src128[7];
	dst128[4] = x4;
	dst128[5] = x5;
	dst128[6] = x6;
	dst128[7] = x7;
}

void rte_mov256(uint8_t *dst, const uint8_t *src)
{
	rte_mov128(dst, src);
	rte_mov128(dst + 128, src + 128);
}

rte_memcpy_lt16(uint8_t *dst, const uint8_t *src, size_t n)
{
	if (n & 0x08) {
		/* copy 8 ~ 15 bytes */
		*(uint64_t *)dst = *(const uint64_t *)src;
		*(uint64_t *)(dst - 8 + n) = *(const uint64_t *)(src - 8 + n);
	} else if (n & 0x04) {
		/* copy 4 ~ 7 bytes */
		*(uint32_t *)dst = *(const uint32_t *)src;
		*(uint32_t *)(dst - 4 + n) = *(const uint32_t *)(src - 4 + n);
	} else if (n & 0x02) {
		/* copy 2 ~ 3 bytes */
		*(uint16_t *)dst = *(const uint16_t *)src;
		*(uint16_t *)(dst - 2 + n) = *(const uint16_t *)(src - 2 + n);
	} else if (n & 0x01) {
		/* copy 1 byte */
		*dst = *src;
	}
}
void rte_memcpy_ge16_lt64(uint8_t *dst, const uint8_t *src, size_t n)
{
	if (n == 16) {
		rte_mov16(dst, src);
	} else if (n <= 32) {
		rte_mov16(dst, src);
		rte_mov16(dst - 16 + n, src - 16 + n);
	} else if (n <= 48) {
		rte_mov32(dst, src);
		rte_mov16(dst - 16 + n, src - 16 + n);
	} else {
		rte_mov48(dst, src);
		rte_mov16(dst - 16 + n, src - 16 + n);
	}
}
#define USE_RTE_MEMCPY(dst, src, n) (1)

void rte_memcpy_ge64(uint8_t *dst, const uint8_t *src, size_t n)
{
	do {
		rte_mov64(dst, src);
		src += 64;
		dst += 64;
		n -= 64;
	} while (likely(n >= 64));

	if (likely(n)) {
		if (n <= 16)
			rte_mov16(dst - 16 + n, src - 16 + n);
		else if (n <= 32)
			rte_mov32(dst - 32 + n, src - 32 + n);
		else if (n <= 48)
			rte_mov48(dst - 48 + n, src - 48 + n);
		else
			rte_mov64(dst - 64 + n, src - 64 + n);
	}
}

void *rte_memcpy(void *dst, const void *src, size_t n)
{
	if (n < 16) {
		rte_memcpy_lt16((uint8_t *)dst, (const uint8_t *)src, n);
		return dst;
	}
	if (n < 64) {
		rte_memcpy_ge16_lt64((uint8_t *)dst, (const uint8_t *)src, n);
		return dst;
	}
	__builtin_prefetch(src, 0, 0);
	__builtin_prefetch(dst, 1, 0);
	if (likely(USE_RTE_MEMCPY(dst, src, n))) {
		rte_memcpy_ge64((uint8_t *)dst, (const uint8_t *)src, n);
		return dst;
	} else
		return memcpy(dst, src, n);
}

int main() 
{
    char s1[1024] = {123};
    char s2[1024] = {0};

    rte_memcpy(s2, s1, 1024);
}
