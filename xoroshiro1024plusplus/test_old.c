
#include <stdint.h>
#include <string.h>

static int p = 0;
static uint64_t s[16];

#if 0

// original

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

__attribute__((noinline))
uint64_t next(void) {
	const int q = p;
	const uint64_t s0 = s[p = (p + 1) & 15];
	uint64_t s15 = s[q];
	const uint64_t result = rotl(s0 + s15, 23) + s15;

	s15 ^= s0;
	s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
	s[p] = rotl(s15, 36);

	return result;
}

#else

// modified

static inline uint64_t rotl(const uint64_t x, uint8_t k) {
	return (x << k) | (x >> ((uint8_t) 64u) - k);
}

__attribute__((noinline))
uint64_t next(void) {
	const unsigned int pF = p;
	const unsigned int p0 = p = (p + 1u) & 0xFu;
	const uint64_t     s0 = s[p0];
	const uint64_t     sF = s[pF];
	const uint64_t     sX = s0 ^ sF;

	s[p0] = rotl(sX, (uint8_t) 36u);
	s[pF] = rotl(s0, (uint8_t) 25u) ^ sX ^ (uint64_t) (sX << 27u);

	return rotl(s0 + sF, (uint8_t) 23u) + sF;
}
#endif

__attribute__((noinline))
void jump() {
	static const uint64_t JUMP[] = { 0x931197d8e3177f17,
		0xb59422e0b9138c5f, 0xf06a6afb49d668bb, 0xacb8a6412c8a1401,
		0x12304ec85f0b3468, 0xb7dfe7079209891e, 0x405b7eec77d9eb14,
		0x34ead68280c44e4a, 0xe0e4ba3e0ac9e366, 0x8f46eda8348905b7,
		0x328bf4dbad90d6ff, 0xc8fd6fb31c9effc3, 0xe899d452d4b67652,
		0x45f387286ade3205, 0x03864f454a8920bd, 0xa68fa28725b1b384 };

	uint64_t t[sizeof s / sizeof *s];
	memset(t, 0, sizeof t);
	for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		for(int b = 0; b < 64; b++) {
			if (JUMP[i] & UINT64_C(1) << b)
				for(int j = 0; j < sizeof s / sizeof *s; j++)
					t[j] ^= s[(j + p) & sizeof s / sizeof *s - 1];
			next();
		}

	for(int i = 0; i < sizeof s / sizeof *s; i++) {
		s[(i + p) & sizeof s / sizeof *s - 1] = t[i];
	}
}

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
int
main(void)
{
	const uint64_t init_seed[16u] = {
		 1u,  2u,  3u,  4u,  5u,  6u,  7u,  8u,
		 9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u
	};
	volatile uint64_t value;
	int test;
	uint32_t i;

	(void) memcpy(s, init_seed, sizeof s);
	jump();

	for ( i = UINT32_MAX; i-- != 0; ){
		value = next();
		//printf("0x%016"PRIX64"\n", value);
	}

	return 0;
}
