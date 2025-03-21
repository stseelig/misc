/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// asspf.c - Async-Signal-Safe Print Functions                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2025, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>	/* memcpy(), memmove(), memset(), strlen() */

/* //////////////////////////////////////////////////////////////////////// */

#if CHAR_BIT != 8
#error "CHAR_BIT != 8"
#endif

/* //////////////////////////////////////////////////////////////////////// */

#define TRUE	(0 == 0)
#define FALSE	(0 != 0)

/* ======================================================================== */

#if __STDC_VERSION__ >= 199901L && !defined(S_SPLINT_S)
#define STATIC(Xsize)	static Xsize
#else
#define STATIC(Xsize)
#endif

/* ======================================================================== */

#ifdef __GNUC__

#ifdef __has_attribute
#define HAS_ATTRIBUTE(x)	__has_attribute(x)
#else
#define HAS_ATTRIBUTE(x)	0
#endif

#else	/* !defined(__GNUC__) */

#define HAS_ATTRIBUTE(x)	0

#endif	/* __GNUC__ */

/* ------------------------------------------------------------------------ */

#if __STDC_VERSION__ >= 199901L
#define INLINE			static inline
#elif defined(__GNUC__)
#define INLINE			static __inline__
#else
#define INLINE			static
#endif

#if HAS_ATTRIBUTE(always_inline)
#define ALWAYS_INLINE		INLINE __attribute__((always_inline))
#else
#define ALWAYS_INLINE		INLINE
#endif

#if HAS_ATTRIBUTE(noinline)
#define NOINLINE		__attribute__((noinline))
#else
#define NOINLINE
#endif

#if HAS_ATTRIBUTE(pure)
#define PURE			__attribute__((pure))
#else
#define PURE
#endif

#if HAS_ATTRIBUTE(const)
#define CONST			__attribute__((const))
#else
#define CONST
#endif

#if HAS_ATTRIBUTE(unused)
#define UNUSED			/*@unused@*/   __attribute__((unused))
#else
#define UNUSED			/*@unused@*/
#endif

#define DEBUG_ONLY		UNUSED

/* ======================================================================== */

#ifdef __GNUC__

#ifdef __has_builtin
#define HAS_BUILTIN(x)			__has_builtin(x)
#else
#define HAS_BUILTIN(x)			0
#endif

#define BUILTIN_EXPECT			__builtin_expect

#else /* !defined(__GNUC__) */

#define HAS_BUILTIN(x)			0

#define BUILTIN_EXPECT			nil

#endif /* __GNUC__ */

/* ------------------------------------------------------------------------ */

#if HAS_BUILTIN(BUILTIN_EXPECT)
#define LIKELY(cond)		(BUILTIN_EXPECT((cond), TRUE))
#define UNLIKELY(cond)		(BUILTIN_EXPECT((cond), FALSE))
#else
#define LIKELY(cond)		(cond)
#define UNLIKELY(cond)		(cond)
#endif

/* //////////////////////////////////////////////////////////////////////// */

struct ASSPF_WriteBuf {
	/*@temp@*/
	char		*buf;
	unsigned short	limit;
	unsigned short	nmemb;
	int		fd;
};

/* ------------------------------------------------------------------------ */

/* returns a defined struct */
ALWAYS_INLINE CONST struct ASSPF_WriteBuf
writebuf_get(char *buf, unsigned short size, int fd)
/*@*/
{
	struct ASSPF_WriteBuf retval;

	retval.buf	= buf;
	retval.limit	= size;
	retval.nmemb	= 0;
	retval.fd	= fd;

	return retval;
}

/* //////////////////////////////////////////////////////////////////////// */

#if defined(__unix__)
#include <unistd.h>
#define WRITE(fd, buf, count)	((size_t)  write(fd, buf, count))

#elif defined(__WIN32__)
#include <io.h>
#define WRITE(fd, buf, count)	((size_t) _write(fd, buf, count))

#else
#error "unsupported system"
#endif

/* ======================================================================== */

/* returns 0 on success */
int
asspf_writebuf_autoinit(
	/*@out@*/ struct ASSPF_WriteBuf *writebuf, int fd,
	/*@reldef@*/ char *buf, unsigned short limit
)
/*@modifies	*writebuf@*/
{
	*writebuf = writebuf_get(buf, limit, fd);
	return 0;
}

/* ======================================================================== */

/* returns the number of bytes written */
NOINLINE size_t
asspf_sys_write(int fd, const void *buf, size_t size)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char *buf_c = buf;
	size_t retval = 0, result;

try_again:
	result = WRITE(fd, buf_c, size);
	if LIKELY ( result != SIZE_MAX ){
		assert(result <= size);
		retval += result;
		if UNLIKELY ( result < size ){
			buf_c = &buf_c[result];
			size -= result;
			goto try_again;
		}
	}
#ifdef __unix__
	else {	if ( (errno == EAGAIN) || (errno == EINTR) ){
			goto try_again;
		}
	}
#endif
	return retval;
}

/* returns the number of bytes left in the buffer (0 == success) */
NOINLINE unsigned short
asspf_flush(struct ASSPF_WriteBuf *writebuf)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	size_t nbytes_writ, writ_diff;

	nbytes_writ = asspf_sys_write(
		writebuf->fd, writebuf->buf, (size_t) writebuf->nmemb
	);
	assert(nbytes_writ <= (size_t) writebuf->nmemb);

	/* check if the write failed */
	writ_diff = ((size_t) writebuf->nmemb) - nbytes_writ;
	if UNLIKELY ( writ_diff != 0 ){
		(void) memmove(
			writebuf->buf, &writebuf->buf[nbytes_writ], writ_diff
		);
	}
	writebuf->nmemb = (unsigned short) writ_diff;

	return (unsigned short) writ_diff;
}

/* returns the number of chars written to the buffer */
NOINLINE size_t
asspf_write(struct ASSPF_WriteBuf *writebuf, const void *src, size_t size)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	size_t nbytes_writ = 0;
	unsigned short nmemb;

	/* check if the writebuf needs to be flushed */
	if ( (writebuf->nmemb != 0)
	    &&
	     (size > (size_t) (writebuf->limit - writebuf->nmemb))
	){
		nmemb = asspf_flush(writebuf);
		if UNLIKELY ( nmemb != 0 ){
			return 0;
		}
	}

	/* check if the source is too big for the writebuf */
	if ( size > (size_t) writebuf->limit ){
		nbytes_writ = asspf_sys_write(writebuf->fd, src, size);
	}
	else {	(void) memcpy(&writebuf->buf[writebuf->nmemb], src, size);
		writebuf->nmemb += (unsigned short) size;
		nbytes_writ = size;
	}

	return nbytes_writ;
}

/* returns the number of chars written to the buffer */
size_t
asspf_puts(struct ASSPF_WriteBuf *writebuf, const char *s)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return asspf_write(writebuf, s, strlen(s));
}

/* returns the number of chars written to the buffer */
size_t
asspf_putc(struct ASSPF_WriteBuf *writebuf, char c)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return asspf_write(writebuf, &c, sizeof c);
}

/* //////////////////////////////////////////////////////////////////////// */

#ifndef ASSPF_OPT_NO_PRINTF

/* //////////////////////////////////////////////////////////////////////// */

#undef writebuf
static NOINLINE size_t printf_int(
	struct ASSPF_WriteBuf *writebuf, const char *, uintmax_t, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;

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

/* returns the number of bytes written to the writebuf */
size_t
asspf_printf_int8least(
	struct ASSPF_WriteBuf *writebuf, const char *format,
	uint_least8_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return printf_int(writebuf, format, (uintmax_t) value, sizeof value);
}

#ifdef X_ASSPS_U16LEAST_NEEDED
/* returns the number of bytes written to the writebuf */
size_t
asspf_printf_int16least(
	struct ASSPF_WriteBuf *writebuf, const char *format,
	uint_least16_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return printf_int(writebuf, format, (uintmax_t) value, sizeof value);
}
#endif	/* X_ASSPS_U16LEAST_NEEDED */

#ifdef X_ASSPS_U32LEAST_NEEDED
/* returns the number of bytes written to the writebuf */
size_t
asspf_printf_int32least(
	struct ASSPF_WriteBuf *writebuf, const char *format,
	uint_least32_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return printf_int(writebuf, format, (uintmax_t) value, sizeof value);
}
#endif	/* X_ASSPS_U32LEAST_NEEDED */

#ifdef X_ASSPS_U64LEAST_NEEDED
/* returns the number of bytes written to the writebuf */
size_t
asspf_printf_int64least(
	struct ASSPF_WriteBuf *writebuf, const char *format,
	uint_least64_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return printf_int(writebuf, format, (uintmax_t) value, sizeof value);
}
#endif	/* X_ASSPS_U64LEAST_NEEDED */

#ifdef X_ASSPS_UINTMAX_NEEDED
/* returns the number of bytes written to the writebuf */
size_t
asspf_printf_intmax(
	struct ASSPF_WriteBuf *writebuf, const char *format, uintmax_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return printf_int(writebuf, format, (uintmax_t) value, sizeof value);
}
#endif /* X_ASSPS_UINTMAX_NEEDED */

#ifdef X_ASSPS_UINTPTR_NEEDED
/* returns the number of bytes written to the writebuf */
size_t
asspf_printf_intptr(
	struct ASSPF_WriteBuf *writebuf, const char *format, uintptr_t value
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	return printf_int(writebuf, format, (uintmax_t) value, sizeof value);
}
#endif /* X_ASSPS_UINTPTR_NEEDED */

/* //////////////////////////////////////////////////////////////////////// */

/* checks if the targeted arch has an arithmetic (signed) right shift */
#define HAS_ASR(Xtype)	( \
	/*@-shiftimplementation@*/ \
	(Xtype) (((Xtype) UINTMAX_MAX) >> 1u) == (Xtype) UINTMAX_MAX \
	/*@=shiftimplementation@*/ \
)

/* returns 'value' arithmetically shifted 'count' bits right */
ALWAYS_INLINE CONST intmax_t
asr_imax(intmax_t x, uint_fast8_t k)
/*@*/
{
	assert((size_t) k < (sizeof x) * CHAR_BIT);

	if ( HAS_ASR(intmax_t) ){
		/*@-shiftimplementation@*/
		return (intmax_t) (x >> k);
		/*@=shiftimplementation@*/
	}
	else {	return (x < 0
			? (intmax_t) ~((~((uintmax_t) x)) >> k)
			: (intmax_t) (((uintmax_t) x) >> k)
		);
	}
}

/* returns 'value' arithmetically shifted 'count' bits left */
ALWAYS_INLINE CONST intmax_t
asl_imax(intmax_t x, uint_fast8_t k)
/*@*/
{
	assert((size_t) k < (sizeof x) * CHAR_BIT);

	return (intmax_t) (((uintmax_t) x) << k);
}

/* returns the absolute value of an integer */
ALWAYS_INLINE CONST uintmax_t
abs_imax(intmax_t x)
/*@*/
{
	/* the compiler does better with this than imaxabs() or bit-tricks */
	return (x < 0 ? -((uintmax_t) x) : (uintmax_t) x);
}

/* returns the 'ilog10 + 1' of an integer */
/* only used on pre-defined values (like a constexpr) */
ALWAYS_INLINE CONST size_t
ilog10p1(uintmax_t limit)
/*@*/
{	size_t retval = (size_t) 1u;

	while ( (limit /= 10u) != 0 ){
		++retval;
	}
	return retval;
}

/* returns whether 'c' is a decimal digit */
/* @note isdigit() is not on the whitelist */
ALWAYS_INLINE CONST int
ascii_isdigit(char c)
/*@*/
{
	return (int) ((c >= '0') && (c <= '9'));
}

/* returns the number of chars read from 'str' and sets '*dest' on success */
/* returns 0 on error */
/* @note atoi() is not on the whitelist */
static size_t
ascii_a2uint(const char *str, size_t limit, /*@out@*/ unsigned int *dest)
/*@modifies	*dest@*/
{
	const size_t ndigits_max = ilog10p1((uintmax_t) UINT_MAX);
	size_t ndigits;
	unsigned int power, number, temp;
	size_t i;

	/* count the digits */
	for ( ndigits = 0; ndigits < limit; ++ndigits ){
		if ( ascii_isdigit(str[ndigits]) == 0 ){
			break;
		}
	}
	if ( (ndigits == 0) || (ndigits > ndigits_max) ){
		/*@-mustmod@*/ /*@-mustdefine@*/
		return 0;
		/*@=mustmod@*/ /*@=mustdefine@*/
	}

	/* sum the digits */
	number = 0, power = 1u;
	for ( i = ndigits; i-- != 0; power *= 10u ){
		temp = power * ((unsigned int) (str[i] - '0'));
		if UNLIKELY ( UINT_MAX - number < temp ){
			number  = UINT_MAX;
			break;
		}
		else {	number += temp; }
	}

	*dest = number;
	return ndigits;
}

/* //////////////////////////////////////////////////////////////////////// */

#define FORMATFLAG_NONE			0x00u
#define FORMATFLAG_ALTFORM_C		0x01u
#define FORMATFLAG_ALTFORM_M		0x02u
#define FORMATFLAG_ZERO_PAD		0x04u
#define FORMATFLAG_LEFT_ADJUST		0x08u
#define FORMATFLAG_BLANK_SIGN		0x10u
#define FORMATFLAG_ALWAYS_SIGN		0x20u

#define FORMATFLAG_ALTFORM_ALL		\
	(FORMATFLAG_ALTFORM_C | FORMATFLAG_ALTFORM_M)

#define FORMATFIELDWIDTH_UNSET		0u

#define FORMATPRECISION_UNSET		1u
#define FORMATPRECISION_NATURAL		0u

enum FormatConvSpec_Int {
	FORMATCONVSPEC_INT_UNSET,
	FORMATCONVSPEC_INT_d,
	FORMATCONVSPEC_INT_u,
	FORMATCONVSPEC_INT_b,
	FORMATCONVSPEC_INT_o,
	FORMATCONVSPEC_INT_x,
	FORMATCONVSPEC_INT_X
};

/* ======================================================================== */

struct ItemFormat_Int {
	unsigned int		flags;
	unsigned int		fieldwidth;
	unsigned int		precision;
	enum FormatConvSpec_Int	convspec;
};

/* ------------------------------------------------------------------------ */

/* returns a defined struct */
ALWAYS_INLINE CONST struct ItemFormat_Int
itemformat_int_get(
	unsigned int flags, unsigned int fieldwidth, unsigned int precision,
	enum FormatConvSpec_Int convspec
)
/*@*/
{
	struct ItemFormat_Int retval;

	retval.flags		= flags;
	retval.fieldwidth	= fieldwidth;
	retval.precision	= precision;
	retval.convspec		= convspec;

	return retval;
}

/* ======================================================================== */

#define FORMATREAD_ERROR { \
	if UNLIKELY ( TRUE ) { \
		/*@-mustmod@*/ /*@-mustdefine@*/\
		return SIZE_MAX; \
		/*@=mustmod@*/ /*@=mustdefine@*/\
	} \
}
#define CHECK_FORMATBUF { \
	if UNLIKELY ( format_idx == format_len ){ \
		FORMATREAD_ERROR; \
	} \
}
#define SET_FORMATFIELD_A2INT(Xfield){ \
	size_t x_setffa2int_ndigits; \
	x_setffa2int_ndigits = ascii_a2uint( \
		&format[format_idx], format_len - format_idx, \
		&Xfield \
	); \
	if ( (x_setffa2int_ndigits == 0) \
	    || \
	     (Xfield > (unsigned int) INT_MAX) \
	){ \
		FORMATREAD_ERROR; \
	} \
	format_idx += x_setffa2int_ndigits; \
}

/* returns the number of bytes processed, or SIZE_MAX on error */
static size_t
format_scan_flags(
	/*@out@*/ unsigned int *flags_out, const char *format,
	size_t format_len
)
/*@modifies	*flags_out@*/
{
	size_t       format_idx		= 0;
	unsigned int format_flags	= FORMATFLAG_NONE;

	#define SET_FORMATFLAG(Xflag) { \
		if UNLIKELY ( (format_flags & Xflag) != 0 ){ \
			FORMATREAD_ERROR; \
		} \
		format_flags |= Xflag; \
		format_idx   += 1u; \
	}

	get_flag:
	CHECK_FORMATBUF;
	switch ( format[format_idx] ){
	case '#':
		SET_FORMATFLAG(FORMATFLAG_ALTFORM_C);
		goto get_flag;
	case '$':
		SET_FORMATFLAG(FORMATFLAG_ALTFORM_M);
		goto get_flag;
	case '0':
		SET_FORMATFLAG(FORMATFLAG_ZERO_PAD);
		goto get_flag;
	case '-':
		SET_FORMATFLAG(FORMATFLAG_LEFT_ADJUST);
		goto get_flag;
	case ' ':
		SET_FORMATFLAG(FORMATFLAG_BLANK_SIGN);
		goto get_flag;
	case '+':
		SET_FORMATFLAG(FORMATFLAG_ALWAYS_SIGN);
		goto get_flag;
	default:
		break;
	}

	*flags_out = format_flags;
	return format_idx;
}

/* returns the number of bytes processed, or SIZE_MAX on error */
static size_t
format_scan_fieldwidth(
	/*@out@*/ unsigned int *fieldwidth_out, const char *format,
	size_t format_len
)
/*@modifies	*fieldwidth_out@*/
{
	size_t       format_idx		= 0;
	unsigned int format_fieldwidth	= FORMATFIELDWIDTH_UNSET;

	CHECK_FORMATBUF;
	if ( ascii_isdigit(format[format_idx]) != 0 ){
		SET_FORMATFIELD_A2INT(format_fieldwidth);
	}

	*fieldwidth_out = format_fieldwidth;
	return format_idx;
}

/* returns the number of bytes processed, or SIZE_MAX on error */
static size_t
format_scan_precision(
	/*@out@*/ unsigned int *precision_out, const char *format,
	size_t format_len
)
/*@modifies	*precision_out@*/
{
	size_t       format_idx		= 0;
	unsigned int format_precision	= FORMATPRECISION_UNSET;

	CHECK_FORMATBUF;
	if ( format[format_idx] == '.' ){
		format_idx += 1u;
		CHECK_FORMATBUF;
		if ( ascii_isdigit(format[format_idx]) != 0 ){
			SET_FORMATFIELD_A2INT(format_precision);
		}
		else {	FORMATREAD_ERROR; }
	}

	*precision_out = format_precision;
	return format_idx;
}

/* returns the number of bytes processed, or SIZE_MAX on error */
static size_t
format_scan_convspec_int(
	/*@out@*/ enum FormatConvSpec_Int *convspec_out, const char *format,
	size_t format_len
)
/*@modifies	*convspec_out@*/
{
	size_t                  format_idx	= 0;
	enum FormatConvSpec_Int format_convspec	= FORMATCONVSPEC_INT_UNSET;

	CHECK_FORMATBUF;
	switch ( format[format_idx] ){
	case 'd':
		format_convspec = FORMATCONVSPEC_INT_d;
		break;
	case 'u':
		format_convspec = FORMATCONVSPEC_INT_u;
		break;
	case 'b':
		format_convspec = FORMATCONVSPEC_INT_b;
		break;
	case 'o':
		format_convspec = FORMATCONVSPEC_INT_o;
		break;
	case 'x':
		format_convspec = FORMATCONVSPEC_INT_x;
		break;
	case 'X':
		format_convspec = FORMATCONVSPEC_INT_X;
		break;
	default:
		FORMATREAD_ERROR;
	}
	format_idx += 1u;

	*convspec_out = format_convspec;
	return format_idx;
}

/* ======================================================================== */

#define FORMATSCAN(Xfn, Xvar){ \
	size_t x_formatscan_nbytesread; \
	x_formatscan_nbytesread = Xfn( \
		&Xvar, &format[format_idx], format_len - format_idx \
	); \
	if UNLIKELY ( x_formatscan_nbytesread == SIZE_MAX ){ \
		return 1; \
	} \
	format_idx += x_formatscan_nbytesread; \
}

/* returns 0 on success */
static int
format_scan_int(/*@out@*/ struct ItemFormat_Int *itemfmt, const char *format)
/*@modifies	*itemfmt@*/
{
	const size_t            format_len        = strlen(format);
	size_t                  format_idx        = 0;
	unsigned int            format_flags      = FORMATFLAG_NONE;
	unsigned int            format_fieldwidth = FORMATFIELDWIDTH_UNSET;
	unsigned int            format_precision  = FORMATPRECISION_UNSET;
	enum FormatConvSpec_Int format_convspec   = FORMATCONVSPEC_INT_UNSET;

	FORMATSCAN(format_scan_flags, format_flags);
	FORMATSCAN(format_scan_fieldwidth, format_fieldwidth);
	FORMATSCAN(format_scan_precision, format_precision);
	FORMATSCAN(format_scan_convspec_int, format_convspec);

	if UNLIKELY ( format_idx != format_len ){
		return 1;
	}
	*itemfmt = itemformat_int_get(
		format_flags, format_fieldwidth, format_precision,
		format_convspec
	);
	return 0;
}

/* //////////////////////////////////////////////////////////////////////// */

#undef writebuf
static size_t printf_int_body(
	struct ASSPF_WriteBuf *writebuf, const struct ItemFormat_Int *,
	uintmax_t, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
;

/* ------------------------------------------------------------------------ */

/* returns the number of bytes written to the writebuf */
static NOINLINE size_t
printf_int(
	struct ASSPF_WriteBuf *writebuf, const char *format, uintmax_t value,
	size_t value_size
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	size_t retval = 0;
	struct ItemFormat_Int itemfmt;
	int err;

	/* scan the format string */
	err = format_scan_int(&itemfmt, format);
	if ( err != 0 ){
		return 0;
	}

	/* format and print */
	switch ( itemfmt.convspec ){
	case FORMATCONVSPEC_INT_UNSET:
		assert(FALSE);
		break;
	case FORMATCONVSPEC_INT_d:
	case FORMATCONVSPEC_INT_u:
	case FORMATCONVSPEC_INT_b:
	case FORMATCONVSPEC_INT_o:
	case FORMATCONVSPEC_INT_x:
	case FORMATCONVSPEC_INT_X:
		retval = printf_int_body(
			writebuf, &itemfmt, value, value_size
		);
		break;
	}

	return retval;
}

/* ======================================================================== */

/* returns value converted to a signed integer */
/* only works with 2's-complement */
static CONST intmax_t
printf_read_raw_d(uintmax_t value, size_t value_size)
/*@*/
{
	const uint_fast8_t shift_cnt = (uint_fast8_t) (
		((sizeof value) - value_size) * CHAR_BIT
	);

	assert((value_size != 0) && (value_size <= sizeof value));

	return asr_imax(asl_imax((intmax_t) value, shift_cnt), shift_cnt);
}

/* returns the number of characters written to 'dest' */
static size_t
printf_w2b_u(/*@out@*/ char dest[], size_t dest_size, uintmax_t value)
/*@modifies	*dest@*/
{
	size_t retval = 0;
	unsigned char digit;
	size_t i;

	for ( i = (size_t) 1u; i <= dest_size; ++i ){
		digit   = (unsigned char) (value % 10u);
		value  /= 10u;
		dest[dest_size - i] = (char) ('0' + (char) digit);
		retval += 1u;
	}
	return retval;
}

/* returns the number of bytes written to 'dest' */
ALWAYS_INLINE size_t
printf_w2b_b_nibble(/*@out@*/ char dest[STATIC(4u)], uint_fast8_t value)
/*@modifies	dest[]@*/
{
	const char str[16u][4u] = {
		{'0','0','0','0'}, {'0','0','0','1'},
		{'0','0','1','0'}, {'0','0','1','1'},
		{'0','1','0','0'}, {'0','1','0','1'},
		{'0','1','1','0'}, {'0','1','1','1'},
		{'1','0','0','0'}, {'1','0','0','1'},
		{'1','0','1','0'}, {'1','0','1','1'},
		{'1','1','0','0'}, {'1','1','0','1'},
		{'1','1','1','0'}, {'1','1','1','1'}
	};

	(void) memcpy(dest, str[value & 0xFu], sizeof str[0]);
	return sizeof str[0];
}

/* returns the number of bytes written to 'dest' */
static size_t
printf_w2b_b(/*@out@*/ char dest[], size_t dest_size, uintmax_t value)
/*@modifies	dest[]@*/
{
	size_t retval = 0;
	size_t i;

	for ( i = (size_t) 4u; i <= dest_size; i += 4u ){
		retval += printf_w2b_b_nibble(
			&dest[dest_size - i], (uint_fast8_t) value
		);
		value >>= 4u;
	}
	return retval;
}

/* returns the number of bytes written to 'dest' */
static size_t
printf_w2b_o(/*@out@*/ char dest[], size_t dest_size, uintmax_t value)
/*@modifies	dest[]@*/
{
	size_t retval = 0;
	unsigned char digit;
	size_t i;

	for ( i = (size_t) 1u; i <= dest_size; ++i ){
		digit   = (unsigned char) (value & 0x7u);
		value >>= 3u;
		dest[dest_size - i] = (char) ('0' + (char) digit);
		retval += 1u;
	}
	return retval;
}

/* returns the number of characters written to 'dest' */
ALWAYS_INLINE size_t
printf_w2b_x_nibble(/*@out@*/ char dest[STATIC(1u)], uint_fast8_t value)
/*@modifies	dest[]@*/
{
	const char c[16u] = {
		'0','1','2','3','4','5','6','7',
		'8','9','a','b','c','d','e','f'
	};

	dest[0] = c[value & 0xFu];
	return sizeof c[0];
}

/* returns the number of characters written to 'dest' */
static size_t
printf_w2b_x(/*@out@*/ char dest[], size_t dest_size, uintmax_t value)
/*@modifies	dest[]@*/
{
	size_t retval = 0;
	size_t i;

	for ( i = (size_t) 1u; i <= dest_size; ++i ){
		retval += printf_w2b_x_nibble(
			&dest[dest_size - i], (uint_fast8_t) value
		);
		value >>= 4u;
	}
	return retval;
}

/* returns the number of characters written to 'dest' */
ALWAYS_INLINE size_t
printf_w2b_X_nibble(/*@out@*/ char dest[STATIC(1u)], uint_fast8_t value)
/*@modifies	dest[]@*/
{
	const char c[16u] = {
		'0','1','2','3','4','5','6','7',
		'8','9','A','B','C','D','E','F'
	};

	dest[0] = c[value & 0xFu];
	return sizeof c[0];
}

/* returns the number of characters written to 'dest' */
static size_t
printf_w2b_X(/*@out@*/ char dest[], size_t dest_size, uintmax_t value)
/*@modifies	dest[]@*/
{
	size_t retval = 0;
	size_t i;

	for ( i = (size_t) 1u; i <= dest_size; ++i ){
		retval += printf_w2b_X_nibble(
			&dest[dest_size - i], (uint_fast8_t) value
		);
		value >>= 4u;
	}
	return retval;
}

/* ------------------------------------------------------------------------ */

/* returns the maximum number of printable digits a value could need */
static CONST size_t
printf_ndigits_max(size_t value_size, enum FormatConvSpec_Int convspec)
/*@*/
{
	size_t retval = 0;

	switch ( convspec ){
	case FORMATCONVSPEC_INT_UNSET:
		assert(FALSE);
		break;
	case FORMATCONVSPEC_INT_d:
		if ( value_size <= sizeof(int_least8_t) ){
			retval = ilog10p1((uintmax_t) INT_LEAST8_MAX);
		}
		else if ( value_size <= sizeof(int_least16_t) ){
			retval = ilog10p1((uintmax_t) INT_LEAST16_MAX);
		}
		else if ( value_size <= sizeof(int_least32_t) ){
			retval = ilog10p1((uintmax_t) INT_LEAST32_MAX);
		}
		else if ( value_size <= sizeof(int_least64_t) ){
			retval = ilog10p1((uintmax_t) INT_LEAST64_MAX);
		}
		else {	retval = ilog10p1((uintmax_t) INTMAX_MAX); }
		break;
	case FORMATCONVSPEC_INT_u:
		if ( value_size <= sizeof(uint_least8_t) ){
			retval = ilog10p1((uintmax_t) UINT_LEAST8_MAX);
		}
		else if ( value_size <= sizeof(uint_least16_t) ){
			retval = ilog10p1((uintmax_t) UINT_LEAST16_MAX);
		}
		else if ( value_size <= sizeof(uint_least32_t) ){
			retval = ilog10p1((uintmax_t) UINT_LEAST32_MAX);
		}
		else if ( value_size <= sizeof(uint_least64_t) ){
			retval = ilog10p1((uintmax_t) UINT_LEAST64_MAX);
		}
		else {	retval = ilog10p1(UINTMAX_MAX); }
		break;
	case FORMATCONVSPEC_INT_b:
		retval = 8u * value_size;
		break;
	case FORMATCONVSPEC_INT_o:
		retval = 3u * value_size;
		break;
	case FORMATCONVSPEC_INT_x:
	case FORMATCONVSPEC_INT_X:
		retval = 2u * value_size;
		break;
	}

	return retval;
}

/* returns the base number of digits for a printf-integer */
static PURE size_t
printf_ndigits_base(const char buf[], size_t buf_size)
/*@*/
{
	size_t i;

	for ( i = 0; i < buf_size; ++i ){
		if ( buf[i] != '0' ){
			break;
		}
	}
	return (i != buf_size ? buf_size - i : (size_t) 1u);
}

/* ------------------------------------------------------------------------ */

/* returns the number of bytes written to dest */
static size_t
printf_flag_sigil_int_c(
	/*@out@*/ char dest[STATIC(2u)], enum FormatConvSpec_Int convspec
)
/*@modifies	dest[]@*/
{
	/* const char sigil_d[0u] = {}; */
	const char sigil_b[2u] = {'0','b'};
	const char sigil_o[1u] = {'0'};
	const char sigil_x[2u] = {'0','x'};

	size_t retval = 0;

	switch ( convspec ){
	case FORMATCONVSPEC_INT_UNSET:
		assert(FALSE);
		break;
	case FORMATCONVSPEC_INT_d:
	case FORMATCONVSPEC_INT_u:
		break;
	case FORMATCONVSPEC_INT_b:
		(void) memcpy(dest, sigil_b, sizeof sigil_b);
		retval = sizeof sigil_b;
		break;
	case FORMATCONVSPEC_INT_o:
		(void) memcpy(dest, sigil_o, sizeof sigil_o);
		retval = sizeof sigil_o;
		break;
	case FORMATCONVSPEC_INT_x:
	case FORMATCONVSPEC_INT_X:
		(void) memcpy(dest, sigil_x, sizeof sigil_x);
		retval = sizeof sigil_x;
		break;
	}

	/*@-mustdefine@*/
	return retval;
	/*@-mustdefine@*/
}

/* returns the number of bytes written to dest */
static size_t
printf_flag_sigil_int_m(
	/*@out@*/ char dest[STATIC(1u)], enum FormatConvSpec_Int convspec
)
/*@modifies	dest[]@*/
{
	const char sigil_d = '#';
	const char sigil_b = '%';
	const char sigil_o = '@';
	const char sigil_x = '$';

	size_t retval = 0;

	switch ( convspec ){
	case FORMATCONVSPEC_INT_UNSET:
		assert(FALSE);
		break;
	case FORMATCONVSPEC_INT_d:
	case FORMATCONVSPEC_INT_u:
		dest[0] = sigil_d;
		retval  = sizeof sigil_d;
		break;
	case FORMATCONVSPEC_INT_b:
		dest[0] = sigil_b;
		retval  = sizeof sigil_b;
		break;
	case FORMATCONVSPEC_INT_o:
		dest[0] = sigil_o;
		retval  = sizeof sigil_o;
		break;
	case FORMATCONVSPEC_INT_x:
	case FORMATCONVSPEC_INT_X:
		dest[0] = sigil_x;
		retval  = sizeof sigil_x;
		break;
	}

	/*@-mustdefine@*/
	return retval;
	/*@-mustdefine@*/
}

/* returns the number of bytes written to dest */
static size_t
printf_flag_sigil_int(
	/*@out@*/ char dest[STATIC(2u)], const struct ItemFormat_Int *itemfmt
)
/*@modifies	dest[]@*/
{
	const unsigned int            flags    = itemfmt->flags;
	const enum FormatConvSpec_Int convspec = itemfmt->convspec;

	size_t retval = 0;

	if ( (flags & FORMATFLAG_ALTFORM_C) != 0 ){
		retval = printf_flag_sigil_int_c(dest, convspec);
	}
	else if ( (flags & FORMATFLAG_ALTFORM_M) != 0 ){
		retval = printf_flag_sigil_int_m(dest, convspec);
	}
	else {	assert(FALSE); }

	/*@-mustdefine@*/
	return retval;
	/*@-mustdefine@*/
}

/* ------------------------------------------------------------------------ */

/* returns the number of bytes written to the buffer */
static size_t
printf_put_repchar(
	struct ASSPF_WriteBuf *writebuf, char c, size_t count
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	char cbuf[64u];
	size_t size = sizeof cbuf;
	size_t retval = 0;

	memset(cbuf, (int) c, sizeof cbuf);

	do {	size = (count > size ? size : count);
		for ( ; count >= size; count -= size ){
			retval += asspf_write(writebuf, cbuf, size);
		}
	} while ( count != 0 );

	return retval;
}

/* ------------------------------------------------------------------------ */

/* returns the number of bytes written to the writebuf */
static size_t
printf_int_body(
	struct ASSPF_WriteBuf *writebuf, const struct ItemFormat_Int *itemfmt,
	uintmax_t value, const size_t value_size
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*writebuf
@*/
{
	size_t retval = 0;
	char   digit_buf[CHAR_BIT * (sizeof value)];	/* %b uintmax_t */
	size_t ndigits_max;
	unsigned int ndigits;
	char   sigil_buf[2u];
	size_t sigil_size = 0;
	char   sign_c;
	size_t sign_size = 0;
	unsigned int precision_pad = 0;
	size_t printed_size;
	int    adjust_type = 0;	/* 0: none, >0: right <0: left */
	size_t adjust_size = 0;
	char   adjust_c = '\0';	/* only for right adjustment */
	DEBUG_ONLY size_t result;

	assert((value_size != 0) && (value_size <= sizeof value));

	#define PRINTFINT_WRITE_SIGIL { \
		if ( sigil_size != 0 ){ \
			retval += asspf_write( \
				writebuf, sigil_buf, sigil_size \
			); \
		} \
	}
	#define PRINTFINT_WRITE_SIGN { \
		if ( (itemfmt->convspec == FORMATCONVSPEC_INT_d) \
		    && \
		     (sign_size != 0) \
		){ \
			retval += asspf_write( \
				writebuf, &sign_c, sizeof sign_c \
			); \
		} \
	}

	/* maximum number of digit_buf bytes to use */
	ndigits_max = printf_ndigits_max(value_size, itemfmt->convspec);
	assert(ndigits_max != 0);

	/* base number */
	switch ( itemfmt->convspec ){
	case FORMATCONVSPEC_INT_UNSET:
		assert(FALSE);
		break;
	case FORMATCONVSPEC_INT_d:
		value  = (uintmax_t) printf_read_raw_d(value, value_size);
		sign_c = ((intmax_t) value < 0 ? '-' : '+');
		value  = abs_imax((intmax_t) value);
		/*@fallthrough@*/
	case FORMATCONVSPEC_INT_u:
		result = printf_w2b_u(digit_buf, ndigits_max, value);
		assert(result == ndigits_max);
		break;
	case FORMATCONVSPEC_INT_b:
		result = printf_w2b_b(digit_buf, ndigits_max, value);
		assert(result == ndigits_max);
		break;
	case FORMATCONVSPEC_INT_o:
		result = printf_w2b_o(digit_buf, ndigits_max, value);
		assert(result == ndigits_max);
		break;
	case FORMATCONVSPEC_INT_x:
		result = printf_w2b_x(digit_buf, ndigits_max, value);
		assert(result == ndigits_max);
		break;
	case FORMATCONVSPEC_INT_X:
		result = printf_w2b_X(digit_buf, ndigits_max, value);
		assert(result == ndigits_max);
		break;
	}

	/* altform sigil */
	assert(sigil_size == 0);
	if ( (itemfmt->flags & FORMATFLAG_ALTFORM_ALL) != 0 ){
		sigil_size = printf_flag_sigil_int(sigil_buf, itemfmt);
	}

	/* sign ('-', '+', or ' ') */
	assert(sign_size == 0);
	if ( itemfmt->convspec == FORMATCONVSPEC_INT_d ){
		switch ( sign_c ){
		default:
			assert(FALSE);
			break;
		case '-':
			sign_size = (size_t) 1u;
			break;
		case '+':
			if ( (itemfmt->flags & FORMATFLAG_ALWAYS_SIGN) != 0 ){
				sign_size = (size_t) 1u;
				break;
			}
			if ( (itemfmt->flags & FORMATFLAG_BLANK_SIGN) != 0 ){
				sign_c    = ' ';
				sign_size = (size_t) 1u;
				break;
			}
			break;
		}
	}

	/* precision calculation */
	ndigits = (unsigned int) printf_ndigits_base(digit_buf, ndigits_max);
	if ( itemfmt->precision == FORMATPRECISION_NATURAL ){
		ndigits = (unsigned int) ndigits_max;
	}
	else if ( itemfmt->precision > ndigits ){
		ndigits = itemfmt->precision;
		if ( itemfmt->precision > (unsigned int) ndigits_max ){
			ndigits       = (unsigned int) ndigits_max;
			precision_pad =	itemfmt->precision - ndigits;
		}
	} else{;}


	/* pre-adjustment calculations */
	printed_size = sigil_size + sign_size + precision_pad + ndigits;
	if ( printed_size < (size_t) itemfmt->fieldwidth ){
		adjust_type = ((itemfmt->flags & FORMATFLAG_LEFT_ADJUST) == 0
			? 1 : -1
		);
		adjust_size = ((size_t) itemfmt->fieldwidth) - printed_size;
		adjust_c    = ((itemfmt->flags & FORMATFLAG_ZERO_PAD) == 0
			? ' ' : '0'
		);
	}

	/* ----------- */

	/* sigil & sign 1 */
	if ( (adjust_type > 0) && (adjust_c == '0') ){
		PRINTFINT_WRITE_SIGIL;
		PRINTFINT_WRITE_SIGN;
	}
	/* right adjustment */
	if ( adjust_type > 0 ){
		retval += printf_put_repchar(writebuf, adjust_c, adjust_size);
	}
	/* sigil & sign 2 */
	if ( (adjust_type <= 0) || (adjust_c == ' ') ){
		PRINTFINT_WRITE_SIGIL;
		PRINTFINT_WRITE_SIGN;
	}
	/* precision padding */
	if ( precision_pad != 0 ){
		retval += printf_put_repchar(
			writebuf, '0', (size_t) precision_pad
		);
	}
	/* base */
	retval += asspf_write(
		writebuf, &digit_buf[ndigits_max - ndigits], (size_t) ndigits
	);
	/* left adjustment */
	if ( adjust_type < 0 ){
		retval += printf_put_repchar(writebuf, ' ', adjust_size);
	}

	return retval;
}

/* //////////////////////////////////////////////////////////////////////// */

#endif	/* ASSPF_OPT_NO_PRINTF */

/* EOF //////////////////////////////////////////////////////////////////// */
