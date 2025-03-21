
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

#include "xoroshiro1024plusplus.h"

int
main(void)
{
	const uint64_t init_seed[16u] = {
		 1u,  2u,  3u,  4u,  5u,  6u,  7u,  8u,
		 9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u
	};
	Xoroshiro1024PlusPlus_Seed seed;
	volatile uint64_t value;
	int test;
	uint32_t i;

	assert(sizeof init_seed == sizeof(Xoroshiro1024PlusPlus_SrcBuf));
	test = xoroshiro1024plusplus_seed(
		&seed, (Xoroshiro1024PlusPlus_SrcBuf *) &init_seed
	);
	assert(test == 0);
	xoroshiro1024plusplus_jump(&seed);

	for ( i = UINT32_MAX; i-- != 0; ){
		value = xoroshiro1024plusplus_next(&seed);
		//printf("0x%016"PRIX64"\n", value);
	}

	return 0;
}
