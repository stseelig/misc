#ifndef ASSPF_H
#define ASSPF_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// asspf.h - Async-Signal-Safe Print Functions                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2025, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#if CHAR_BIT != 8
#error "CHAR_BIT != 8"
#endif

/* //////////////////////////////////////////////////////////////////////// */

#if defined(__unix__)
#include <unistd.h>
#define ASSPF_FD_STDOUT		STDOUT_FILENO
#define ASSPF_FD_STDERR		STDERR_FILENO

#elif defined(__WIN32__)
#include <io.h>
#define ASSPF_FD_STDOUT		STD_OUTPUT_HANDLE
#define ASSPF_FD_STDERR		STD_ERROR_HANDLE

#else
#error "unsupported system"
#endif

/* //////////////////////////////////////////////////////////////////////// */

struct X_ASSPF_WriteBuf {
	/*@temp@*/
	char		*x_0;
	unsigned short	 x_1;
	unsigned short	 x_2;
	int		 x_3;
};
typedef /*@abstract@*/ struct X_ASSPF_WriteBuf	ASSPF_WriteBuf;

/* //////////////////////////////////////////////////////////////////////// */

#undef writebuf
#undef fd
#undef buf
#undef size
/*@external@*/ /*@unused@*/
extern int asspf_writebuf_autoinit(
	/*@out@*/
	ASSPF_WriteBuf *writebuf,
	int fd,
	/*@reldef@*/
	char *buf,
	unsigned short size
)
/*@modifies	*writebuf@*/
;

/* ======================================================================== */

#undef fd
#undef buf
#undef size
/*@external@*/ /*@unused@*/
extern size_t asspf_sys_write(int fd, const void *buf, size_t size)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

#undef writebuf
/*@external@*/ /*@unused@*/
extern unsigned short asspf_flush(ASSPF_WriteBuf *writebuf)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;

#undef writebuf
#undef src
#undef size
/*@external@*/ /*@unused@*/
extern size_t asspf_write(
	ASSPF_WriteBuf *writebuf, const void *src, size_t size
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;

#undef writebuf
#undef s
/*@external@*/ /*@unused@*/
extern size_t asspf_puts(ASSPF_WriteBuf *writebuf, const char *s)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;

#undef writebuf
#undef c
/*@external@*/ /*@unused@*/
extern size_t asspf_putc(ASSPF_WriteBuf *writebuf, char c)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;

/* ======================================================================== */

#ifndef ASSPF_OPT_NO_PRINTF

/* ======================================================================== */

/* printf - base types */

#if UINT_LEAST16_MAX != UINT_LEAST8_MAX
#define X_ASSPS_U16LEAST_NEEDED
#endif

#if UINT_LEAST32_MAX != UINT_LEAST16_MAX
#define X_ASSPS_U32LEAST_NEEDED
#endif

#if UINT_LEAST64_MAX != UINT_LEAST32_MAX
#define X_ASSPS_U64LEAST_NEEDED
#endif

#if UINTMAX_MAX != UINT_LEAST8_MAX  \
 && UINTMAX_MAX != UINT_LEAST16_MAX \
 && UINTMAX_MAX != UINT_LEAST32_MAX \
 && UINTMAX_MAX != UINT_LEAST64_MAX
#define X_ASSPS_UINTMAX_NEEDED
#endif

#if UINTPTR_MAX != UINT_LEAST8_MAX  \
 && UINTPTR_MAX != UINT_LEAST16_MAX \
 && UINTPTR_MAX != UINT_LEAST32_MAX \
 && UINTPTR_MAX != UINT_LEAST64_MAX \
 && UINTPTR_MAX != UINTMAX_MAX
#define X_ASSPS_UINTPTR_NEEDED
#endif

/* ------------------------------------------------------------------------ */

#undef writebuf
#undef format
#undef value
/*@external@*/ /*@unused@*/
extern size_t asspf_printf_int8least(
	ASSPF_WriteBuf *writebuf, const char *format, uint_least8_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;

/* ------------------------------------------------------------------------ */

#ifdef X_ASSPS_U16LEAST_NEEDED
#undef writebuf
#undef format
#undef value
/*@external@*/ /*@unused@*/
extern size_t asspf_printf_int16least(
	ASSPF_WriteBuf *writebuf, const char *format, uint_least16_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;
#else	/* !defined(X_ASSPS_U16LEAST_NEEDED) */
#define asspf_printf_int16least(writebuf, format, value) \
	asspf_printf_int8least( \
		writebuf, format, (uint_least8_t) ((uint_least16_t) (value)) \
	)
#endif	/* X_ASSPS_U16LEAST_NEEDED */

/* ------------------------------------------------------------------------ */

#ifdef X_ASSPS_U32LEAST_NEEDED
#undef writebuf
#undef format
#undef value
/*@external@*/ /*@unused@*/
extern size_t asspf_printf_int32least(
	ASSPF_WriteBuf *writebuf, const char *format, uint_least32_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;
#else	/* !defined(X_ASSPS_U32LEAST_NEEDED) */
#define asspf_printf_int32least(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, \
		(uint_least16_t) ((uint_least32_t) (value)) \
	)
#endif	/* X_ASSPS_U32LEAST_NEEDED */

/* ------------------------------------------------------------------------ */

#ifdef X_ASSPS_U64LEAST_NEEDED
#undef writebuf
#undef format
#undef value
/*@external@*/ /*@unused@*/
extern size_t asspf_printf_int64least(
	ASSPF_WriteBuf *writebuf, const char *format, uint_least64_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;
#else	/* !defined(X_ASSPS_U64LEAST_NEEDED) */
#define asspf_printf_int64least(writebuf, format value) \
	asspf_printf_int32least( \
		writebuf, format, \
		(uint_least32_t) ((uint_least64_t) (value)) \
	)
#endif	/* X_ASSPS_U64LEAST_NEEDED */

/* ------------------------------------------------------------------------ */

#ifdef X_ASSPS_UINTMAX_NEEDED
#undef writebuf
#undef format
#undef value
/*@external@*/ /*@unused@*/
extern size_t asspf_printf_intmax(
	ASSPF_WriteBuf *writebuf, const char *format, uintmax_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;
#else	/* !defined(X_ASSPS_UINTMAX_NEEDED) */
#if   UINTMAX_MAX == UINT_LEAST8_MAX
#define asspf_printf_intmax(writebuf, format, value) \
	asspf_printf_int8least(writebuf, format, (uint_least8_t) (value))
#elif UINTMAX_MAX == UINT_LEAST16_MAX
#define asspf_printf_intmax(writebuf, format, value) \
	asspf_printf_int16least(writebuf, format, (uint_least16_t) (value))
#elif UINTMAX_MAX == UINT_LEAST32_MAX
#define asspf_printf_intmax(writebuf, format, value) \
	asspf_printf_int32least(writebuf, format, (uint_least32_t) (value))
#elif UINTMAX_MAX == UINT_LEAST64_MAX
#define asspf_printf_intmax(writebuf, format, value) \
	asspf_printf_int64least(writebuf, format, (uint_least64_t) (value))
#else
#error "asspf_printf_intmax()"
#endif
#endif /* X_ASSPS_UINTMAX_NEEDED */

/* ------------------------------------------------------------------------ */

#ifdef X_ASSPS_UINTPTR_NEEDED
#undef writebuf
#undef format
#undef value
/*@external@*/ /*@unused@*/
extern size_t asspf_printf_intptr(
	ASSPF_WriteBuf *writebuf, const char *format, uintptr_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;
#else	/* !defined(X_ASSPS_UINTPTR_NEEDED) */
#if   UINTPTR_MAX == UINT_LEAST16_MAX
#define asspf_printf_intptr(writebuf, format, (uint_least16_t) (value)) \
	asspf_printf_int16least(writebuf, format, value)
#elif UINTPTR_MAX == UINT_LEAST32_MAX
#define asspf_printf_intptr(writebuf, format, value) \
	asspf_printf_int32least(writebuf, format, (uint_least32_t) (value))
#elif UINTPTR_MAX == UINT_LEAST64_MAX
#define asspf_printf_intptr(writebuf, format, value) \
	asspf_printf_int64least(writebuf, format, (uint_least64_t) (value))
#else
#define asspf_printf_intptr(writebuf, format, value) \
	asspf_printf_intmax(writebuf, format, (uintmax_t) (value))
#endif
#endif /* X_ASSPS_UINTPTR_NEEDED */

/* ======================================================================== */

/* printf - dependent types */

/* ------------------------------------------------------------------------ */

#if UINT8_MAX == UINT_LEAST8_MAX
#define asspf_printf_int8(writebuf, format, value) \
	asspf_printf_int8least( \
		writebuf, format, (uint_least8_t) ((value) & UINT8_MAX) \
	)
#endif	/* UINT8_MAX == UINT_LEAST8_MAX */

#if UINT16_MAX == UINT_LEAST16_MAX
#define asspf_printf_int16(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, (uint_least16_t) ((value) & UINT16_MAX) \
	)
#endif	/* UINT16_MAX == UINT_LEAST16_MAX */

#if UINT32_MAX == UINT_LEAST32_MAX
#define asspf_printf_int32(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((value) & UINT32_MAX) \
	)
#endif	/* UINT32_MAX == UINT_LEAST32_MAX */

#if UINT64_MAX == UINT_LEAST64_MAX
#define asspf_printf_int64(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((value) & UINT64_MAX) \
	)
#endif	/* UINT64_MAX == UINT_LEAST64_MAX */

/* ------------------------------------------------------------------------ */

#if   UINT_FAST8_MAX == UINT_LEAST8_MAX
#define asspf_printf_int8fast(writebuf, format, value) \
	asspf_printf_int8least( \
		writebuf, format, (uint_least8_t) ((uint_fast8_t) (value)) \
	)
#elif UINT_FAST8_MAX == UINT_LEAST16_MAX
#define asspf_printf_int8fast(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, (uint_least16_t) ((uint_fast8_t) (value)) \
	)
#elif UINT_FAST8_MAX == UINT_LEAST32_MAX
#define asspf_printf_int8fast(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((uint_fast8_t) (value)) \
	)
#elif UINT_FAST8_MAX == UINT_LEAST64_MAX
#define asspf_printf_int8fast(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((uint_fast8_t) (value)) \
	)
#else
#define asspf_printf_int8fast(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((uint_fast8_t) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   UINT_FAST16_MAX == UINT_LEAST16_MAX
#define asspf_printf_int16fast(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, (uint_least16_t) ((uint_fast16_t) (value)) \
	)
#elif UINT_FAST16_MAX == UINT_LEAST32_MAX
#define asspf_printf_int16fast(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((uint_fast16_t) (value)) \
	)
#elif UINT_FAST16_MAX == UINT_LEAST64_MAX
#define asspf_printf_int16fast(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((uint_fast16_t) (value)) \
	)
#else
#define asspf_printf_int16fast(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((uint_fast16_t) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   UINT_FAST32_MAX == UINT_LEAST32_MAX
#define asspf_printf_int32fast(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((uint_fast32_t) (value)) \
	)
#elif UINT_FAST32_MAX == UINT_LEAST64_MAX
#define asspf_printf_int32fast(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((uint_fast32_t) (value)) \
	)
#else
#define asspf_printf_int32fast(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((uint_fast32_t) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   UINT_FAST64_MAX == UINT_LEAST64_MAX
#define asspf_printf_int64fast(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((uint_fast64_t) (value)) \
	)
#else
#define asspf_printf_int64fast(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((uint_fast64_t) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   UCHAR_MAX == UINT_LEAST8_MAX
#define asspf_printf_char(writebuf, format, value) \
	asspf_printf_int8least( \
		writebuf, format, (uint_least8_t) ((unsigned char) (value)) \
	)
#elif UCHAR_MAX == UINT_LEAST16_MAX
#define asspf_printf_char(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, (uint_least16_t) ((unsigned char) (value)) \
	)
#elif UCHAR_MAX == UINT_LEAST32_MAX
#define asspf_printf_char(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((unsigned char) (value)) \
	)
#elif UCHAR_MAX == UINT_LEAST64_MAX
#define asspf_printf_char(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((unsigned char) (value)) \
	)
#else
#define asspf_printf_char(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((unsigned char) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   USHRT_MAX == UINT_LEAST16_MAX
#define asspf_printf_short(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, \
		(uint_least16_t) ((unsigned short) (value)) \
	)
#elif USHRT_MAX == UINT_LEAST32_MAX
#define asspf_printf_short(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, \
		(uint_least32_t) ((unsigned short) (value)) \
	)
#elif USHRT_MAX == UINT_LEAST64_MAX
#define asspf_printf_short(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, \
		(uint_least64_t) ((unsigned short) (value)) \
	)
#else
#define asspf_printf_short(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((unsigned short) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   UINT_MAX == UINT_LEAST16_MAX
#define asspf_printf_int(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, (uint_least16_t) ((unsigned int) (value)) \
	)
#elif UINT_MAX == UINT_LEAST32_MAX
#define asspf_printf_int(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((unsigned int) (value)) \
	)
#elif UINT_MAX == UINT_LEAST64_MAX
#define asspf_printf_int(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((unsigned int) (value)) \
	)
#else
#define asspf_printf_int(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((unsigned int) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   ULONG_MAX == UINT_LEAST32_MAX
#define asspf_printf_long(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((unsigned long) (value)) \
	)
#elif ULONG_MAX == UINT_LEAST64_MAX
#define asspf_printf_long(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((unsigned long) (value)) \
	)
#else
#define asspf_printf_long(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((unsigned long) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if __STDC_VERSION__ >= 199901L || defined(__GNUC__)
#if   ULLONG_MAX == UINT_LEAST64_MAX
#define asspf_printf_longlong(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, \
		(uint_least64_t) ((unsigned long long) (value)) \
	)
#else
#define asspf_printf_longlong(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((unsigned long long) (value)) \
	)
#endif
#endif	/* __STDC_VERSION__ >= 199901L || defined(__GNUC__) */

/* ------------------------------------------------------------------------ */

#if   SIZE_MAX == INT_LEAST16_MAX
#define asspf_printf_size(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, (uint_least16_t) ((size_t) (value)) \
	)
#elif SIZE_MAX == INT_LEAST32_MAX
#define asspf_printf_size(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((size_t) (value)) \
	)
#elif SIZE_MAX == INT_LEAST64_MAX
#define asspf_printf_size(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((size_t) (value)) \
	)
#else
#define asspf_printf_size(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((size_t) (value)) \
	)
#endif

/* ------------------------------------------------------------------------ */

#if   PTRDIFF_MAX == INT_LEAST16_MAX
#define asspf_printf_ptrdiff(writebuf, format, value) \
	asspf_printf_int16least( \
		writebuf, format, (uint_least16_t) ((ptrdiff_t) (value)) \
	)
#elif PTRDIFF_MAX == INT_LEAST32_MAX
#define asspf_printf_ptrdiff(writebuf, format, value) \
	asspf_printf_int32least( \
		writebuf, format, (uint_least32_t) ((ptrdiff_t) (value)) \
	)
#elif PTRDIFF_MAX == INT_LEAST64_MAX
#define asspf_printf_ptrdiff(writebuf, format, value) \
	asspf_printf_int64least( \
		writebuf, format, (uint_least64_t) ((ptrdiff_t) (value)) \
	)
#else
#define asspf_printf_ptrdiff(writebuf, format, value) \
	asspf_printf_intmax( \
		writebuf, format, (uintmax_t) ((ptrdiff_t) (value)) \
	)
#endif

/* ======================================================================== */

#endif	/* ASSPF_OPT_NO_PRINTF */

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* ASSPF_H */
