#ifndef BITSET_INLINE_H
#define BITSET_INLINE_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// bitset_inline.h                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>
#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#ifndef INLINE
#if __STDC_VERSION__ >= 199901L
#define INLINE				/*@unused@*/ static inline
#elif defined(__GNUC__)
#define INLINE				/*@unused@*/ static __inline__
#else
#define INLINE				/*@unused@*/ static
#endif
#endif

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

#undef bitset
INLINE int bitset_set_0(uint8_t *RESTRICT bitset, size_t)
/*@modifies	*bitset@*/
;

#undef bitset
INLINE int bitset_set_1(uint8_t *RESTRICT bitset, size_t)
/*@modifies	*bitset@*/
;

__attribute__((pure))
INLINE size_t bitset_next_raw_0(const uint8_t *RESTRICT, size_t)
/*@*/
;

__attribute__((pure))
INLINE size_t bitset_next_raw_1(const uint8_t *RESTRICT, size_t)
/*@*/
;

__attribute__((pure))
INLINE size_t bitset_nextish_raw_0(const uint8_t *RESTRICT, size_t)
/*@*/
;

__attribute__((pure))
INLINE size_t bitset_nextish_raw_1(const uint8_t *RESTRICT, size_t)
/*@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/** @fn bitset_get
  * @brief gets the bit at the index
  *
  * @param bitset[in] the bitset
  * @param index the index of the bit
  *
  * @retval 0|1 - the bit
 **/
__attribute__((pure))
INLINE int
bitset_get(const uint8_t *const RESTRICT bitset, const size_t index)
/*@*/
{
	const size_t  byte_index = (size_t)  (index >> 3u);
	const uint8_t bit_index  = (uint8_t) (index & 0x7u);
	const uint8_t mask       = (uint8_t) (((uint8_t) 0x1u) << bit_index);

	return (int) ((bitset[byte_index] & mask) != 0);
}

/* ======================================================================== */

/** @fn bitset_set
  * @brief sets the bit at the index to the value
  *
  * @param bitset[in] the bitset
  * @param index the index of the bit
  * @param value 0|1
  *
  * @retval 0|1 - the new bit
 **/
INLINE int
bitset_set(
	uint8_t *const RESTRICT bitset, const size_t index, const int value
)
/*@modifies	*bitset@*/
{

	return (value == 0
		? bitset_set_0(bitset, index)
		: bitset_set_1(bitset, index)
	);
}

/* ------------------------------------------------------------------------ */

/** @fn bitset_set_0
  * @brief sets the bit at the index to 0
  *
  * @param bitset[in] the bitset
  * @param index the index of the bit
  *
  * @retval 0
 **/
INLINE int
bitset_set_0(uint8_t *const RESTRICT bitset, const size_t index)
/*@modifies	*bitset@*/
{
	const size_t  byte_index = (size_t)  (index >> 3u);
	const uint8_t bit_index  = (uint8_t) (index & 0x7u);
	const uint8_t mask       = (uint8_t) (((uint8_t) 0x1u) << bit_index);

	bitset[byte_index] &= ~mask;
	return 0;
}

/** @fn bitset_set_1
  * @brief sets the bit at the index to 1
  *
  * @param bitset[in] the bitset
  * @param index the index of the bit
  *
  * @retval 1
 **/
INLINE int
bitset_set_1(uint8_t *const RESTRICT bitset, const size_t index)
/*@modifies	*bitset@*/
{
	const size_t  byte_index = (size_t)  (index >> 3u);
	const uint8_t bit_index  = (uint8_t) (index & 0x7u);
	const uint8_t mask       = (uint8_t) (((uint8_t) 0x1u) << bit_index);

	bitset[byte_index] |= mask;
	return 1;
}

/* ======================================================================== */

/** @fn bitset_flip
  * @brief flips the bit at the index
  *
  * @param bitset[in] the bitset
  * @param index the index of the bit
  *
  * @retval 0|1 - the new bit
 **/
INLINE int
bitset_flip(uint8_t *const RESTRICT bitset, const size_t index)
/*@modifies	*bitset@*/
{
	const size_t  byte_index = (size_t)  (index >> 3u);
	const uint8_t bit_index  = (uint8_t) (index & 0x7u);
	const uint8_t mask       = (uint8_t) (((uint8_t) 0x1u) << bit_index);

	bitset[byte_index] ^= mask;
	return (int) ((bitset[byte_index] & mask) != 0);
}

/* ======================================================================== */

/** @fn bitset_next_raw
  * @brief finds the first 0|1 in the bitset starting at an index
  *
  * @note assumes that the bit being searched for exists
  *
  * @param bitset[in] the bitset
  * @param start the index to start at
  * @param value 0|1
  *
  * @return the index of the first 0|1 from the start
 **/
__attribute__((pure))
INLINE size_t
bitset_next_raw(
	const uint8_t *const RESTRICT bitset,
	const size_t start,
	const int value
)
/*@*/
{
	return (value == 0
		? bitset_next_raw_0(bitset, start)
		: bitset_next_raw_1(bitset, start)
	);
}

/* ------------------------------------------------------------------------ */

/** @fn bitset_next_raw_0
  * @brief finds the first 0 in the bitset starting at an index
  *
  * @note assumes that the bit being searched for exists
  *
  * @param bitset[in] the bitset
  * @param start the index to start at
  *
  * @return the index of the first 0 from the start
 **/
__attribute__((pure))
INLINE size_t
bitset_next_raw_0(const uint8_t *const RESTRICT bitset, const size_t start)
/*@*/
{
	const size_t  byte_index = (size_t)  (start >> 3u);
	const uint8_t bit_index  = (uint8_t) (start & 0x7u);
	const uint8_t nbits      = (uint8_t) __builtin_ctz(
		(unsigned int) ((0xFF00u | ~bitset[byte_index]) >> bit_index)
	);

	return (nbits + bit_index < (uint8_t) 8u
		? (size_t) (nbits + start)
		: bitset_nextish_raw_0(bitset, start + 8u)
	);
}

/** @fn bitset_next_raw_1
  * @brief finds the first 1 in the bitset starting at an index
  *
  * @note assumes that the bit being searched for exists
  *
  * @param bitset[in] the bitset
  * @param start the index to start at
  *
  * @return the index of the first 1 from the start
 **/
__attribute__((pure))
INLINE size_t
bitset_next_raw_1(const uint8_t *const RESTRICT bitset, const size_t start)
/*@*/
{
	const size_t  byte_index = (size_t)  (start >> 3u);
	const uint8_t bit_index  = (uint8_t) (start & 0x7u);
	const uint8_t nbits      = (uint8_t) __builtin_ctz(
		(unsigned int) ((0xFF00u | bitset[byte_index]) >> bit_index)
	);

	return (nbits + bit_index < (uint8_t) 8u
		? (size_t) (nbits + start)
		: bitset_nextish_raw_1(bitset, start + 8u)
	);
}

/* ======================================================================== */

/** @fn bitset_nextish_raw
  * @brief finds the first 0|1 in the bitset starting in the same byte as an
  *   index
  *
  * @note assumes that the bit being searched for exists
  *
  * @param bitset[in] the bitset
  * @param start the index to start near
  * @param value 0|1
  *
  * @return the index of the first 0|1 near the start
 **/
__attribute__((pure))
INLINE size_t
bitset_nextish_raw(
	const uint8_t *const RESTRICT bitset,
	const size_t start,
	const int value
)
/*@*/
{
	return (value == 0
		? bitset_nextish_raw_0(bitset, start)
		: bitset_nextish_raw_1(bitset, start)
	);
}

/* ------------------------------------------------------------------------ */

/** @fn bitset_nextish_raw_0
  * @brief finds the first 0 in the bitset starting in the same byte as an
  *   index
  *
  * @note assumes that the bit being searched for exists
  *
  * @param bitset[in] the bitset
  * @param start the index to start near
  *
  * @return the index of the first 0 near the start
 **/
__attribute__((pure))
INLINE size_t
bitset_nextish_raw_0(const uint8_t *const RESTRICT bitset, const size_t start)
/*@*/
{
	size_t  byte_index;
	uint8_t bit_index;

	byte_index = (size_t) (start >> 3u);
	while ( bitset[byte_index] == UINT8_MAX ){
		++byte_index;
	}

	bit_index  = (uint8_t) __builtin_ctz(
		~((unsigned int) bitset[byte_index])
	);

	return (size_t) ((byte_index << 3u) | bit_index);
}

/** @fn bitset_nextish_raw_1
  * @brief finds the first 1 in the bitset starting in the same byte as an
  *   index
  *
  * @note assumes that the bit being searched for exists
  *
  * @param bitset[in] the bitset
  * @param start the index to start near
  *
  * @return the index of the first 1 near the start
 **/
__attribute__((pure))
INLINE size_t
bitset_nextish_raw_1(const uint8_t *const RESTRICT bitset, const size_t start)
/*@*/
{
	size_t  byte_index;
	uint8_t bit_index;

	byte_index = (size_t) (start >> 3u);
	while ( bitset[byte_index] == 0 ){
		++byte_index;
	}

	bit_index  = (uint8_t) __builtin_ctz(
		(unsigned int) bitset[byte_index]
	);

	return (size_t) ((byte_index << 3u) | bit_index);
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* BITSET_INLINE_H */
