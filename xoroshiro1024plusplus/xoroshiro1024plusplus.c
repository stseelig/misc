//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// xoroshiro1024plusplus.c                                                  //
//                                                                          //
// Original:                                                                //
//	https://prng.di.unimi.it/xoroshiro1024plusplus.c                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// SPDX-License-Identifier: CC0-1.0                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

#include "xoroshiro1024plusplus.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef __has_builtin
#define HAS_BUILTIN(x)			__has_builtin(x)
#else
#define HAS_BUILTIN(x)			0
#endif

#define BUILTIN_ROTL64			__builtin_rotateleft64
#define BUILTIN_MEMCPY			__builtin_memcpy

//==========================================================================//

static inline __attribute__((always_inline))
__attribute__((const))
uint64_t
rotl64(const uint64_t x, const uint8_t k) /*@*/ {
#if HAS_BUILTIN(BUILTIN_ROTL64)
	return (uint64_t) BUILTIN_ROTL64(x, k);
#else
	return (uint64_t) ((x << k) | (x >> (((uint8_t) 64u) - k)));
#endif
}

//--------------------------------------------------------------------------//

#if HAS_BUILTIN(BUILTIN_MEMCPY)
#define MEMCPY(dest, src, n)		BUILTIN_MEMCPY((dest), (src), (n))
#else
#include <string.h>
#define MEMCPY(dest, src, n)		((void) memcpy((dest), (src), (n)))
#endif

//////////////////////////////////////////////////////////////////////////////

// returns a high-quality pseudo-random number
__attribute__((noinline))
uint64_t
xoroshiro1024plusplus_next(
	struct X_Xoroshiro1024PlusPlus_Seed *const restrict seed
)
/*@modifies	*seed@*/
{
	uint64_t     *const restrict s =  seed->s;
	unsigned int *const restrict p = &seed->p;

	const unsigned int pF = *p;
	const unsigned int p0 = *p = (*p + 1u) & 0xFu;
	const uint64_t     s0 = s[p0];
	const uint64_t     sF = s[pF];
	const uint64_t     sX = s0 ^ sF;

	s[pF] = rotl64(s0, (uint8_t) 25u) ^ sX ^ (uint64_t) (sX << 27u);
	s[p0] = rotl64(sX, (uint8_t) 36u);

	return rotl64(s0 + sF, (uint8_t) 23u) + sF;
}

// returns 0 on success; 'src' cannot be all zeros
int
xoroshiro1024plusplus_seed(
	/*@out@*/ struct X_Xoroshiro1024PlusPlus_Seed *const restrict dest,
	const struct X_Xoroshiro1024PlusPlus_SrcBuf *const restrict src
)
/*@modifies	*dest@*/
{
	unsigned int i;

	// check if the seed is all zeroes
	#pragma nounroll
	for ( i = 0; i < (unsigned int) (sizeof src->buf); ++i ){
		if ( src->buf[i] != 0 ){
			goto valid_seed;
		}
	}
	return 1;

valid_seed:
	MEMCPY(dest->s, src->buf, sizeof dest->s);
	dest->p = 0;
	return 0;
}

// equivalent to 2^512 calls to xoroshiro1024plusplus_next()
void
xoroshiro1024plusplus_jump(
	struct X_Xoroshiro1024PlusPlus_Seed *const restrict seed
)
/*@modifies	*seed@*/
{
	uint64_t     *const restrict s =  seed->s;
	unsigned int *const restrict p = &seed->p;

	const uint64_t jump[16u] = {
	(uint64_t) 0x931197D8E3177F17u, (uint64_t) 0xB59422E0B9138C5Fu,
	(uint64_t) 0xF06A6AFB49D668BBu, (uint64_t) 0xACB8A6412C8A1401u,
	(uint64_t) 0x12304EC85F0B3468u, (uint64_t) 0xB7DFE7079209891Eu,
	(uint64_t) 0x405B7EEC77D9EB14u, (uint64_t) 0x34EAD68280C44E4Au,
	(uint64_t) 0xE0E4BA3E0AC9E366u, (uint64_t) 0x8F46EDA8348905B7u,
	(uint64_t) 0x328BF4DBAD90D6FFu, (uint64_t) 0xC8FD6FB31C9EFFC3u,
	(uint64_t) 0xE899D452D4B67652u, (uint64_t) 0x45F387286ADE3205u,
	(uint64_t) 0x03864F454A8920BDu, (uint64_t) 0xA68FA28725B1B384u
	};

	uint64_t tmp[16u] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint64_t bit;
	unsigned int i, j;

	#pragma nounroll
	for ( i = 0; i < 16u; ++i ){
		#pragma nounroll
		for ( bit = (uint64_t) 0x1u; bit != 0; bit <<= 1u ) {
			if ( (jump[i] & bit) != 0 ) {
				#pragma nounroll
				for ( j = 0; j < 16u; ++j ){
					tmp[j] ^= s[(j + *p) & 0xFu];
				}
			}
			(void) xoroshiro1024plusplus_next(seed);
		}
	}
	#pragma nounroll
	for ( i = 0; i < 16u; ++i ) {
		s[(i + *p) & 0xFu] = tmp[i];
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
