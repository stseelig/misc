#ifndef XOROSHIRO1024PLUSPLUS_H
#define XOROSHIRO1024PLUSPLUS_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// xoroshiro1024plusplus.h                                                  //
//                                                                          //
// Original:                                                                //
//	https://prng.di.unimi.it/xoroshiro1024plusplus.c                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// SPDX-License-Identifier: CC0-1.0                                         //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#ifndef RESTRICT
#if __STDC_VERSION__ >= 199901L
#define RESTRICT			restrict
#elif defined(__GNUC__)
#define RESTRICT			__restrict__
#else
#define RESTRICT
#endif
#endif

/* //////////////////////////////////////////////////////////////////////// */

struct X_Xoroshiro1024PlusPlus_Seed {
	uint64_t	s[16u];
	unsigned int	p;	/* gcc does better with uint than uint8_t */
};
typedef /*@abstract@*/ struct X_Xoroshiro1024PlusPlus_Seed
	Xoroshiro1024PlusPlus_Seed
;

struct X_Xoroshiro1024PlusPlus_SrcBuf {
	uint8_t		buf[128u];
};
typedef struct X_Xoroshiro1024PlusPlus_SrcBuf	Xoroshiro1024PlusPlus_SrcBuf;

/* //////////////////////////////////////////////////////////////////////// */

#undef seed
/*@external@*/ /*@unused@*/
extern uint64_t xoroshiro1024plusplus_next(
	Xoroshiro1024PlusPlus_Seed *RESTRICT seed
)
/*@modifies	*seed@*/
;

#undef dest
#undef src
/*@external@*/ /*@unused@*/
extern int xoroshiro1024plusplus_seed(
	/*@out@*/
	Xoroshiro1024PlusPlus_Seed *RESTRICT dest,
	const Xoroshiro1024PlusPlus_SrcBuf *RESTRICT src
)
/*@modifies	*dest@*/
;

#undef seed
/*@external@*/ /*@unused@*/
extern void xoroshiro1024plusplus_jump(
	Xoroshiro1024PlusPlus_Seed *RESTRICT seed
)
/*@modifies	*seed@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* XOROSHIRO1024PLUSPLUS_H */
