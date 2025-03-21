# asspf - Async-Signal-Safe Print Functions

## Caveats
The API is AS-Safe as long as:

\- 'errno' is saved before calling anything, then restored afterwards.

\- The WriteBuf object and its underlying buffer are auto (stack) variables.

The API is not MT-Safe per se,
as there is no locking used with the WriteBuf object.
(You would have to write your own wrappers.)

The printf-like functions only work with 2's-complement integers.
No floating-point either.

## Building

Source is ANSI-C89.
Use any C compiler (GNUC compatible preferred).

### Defines

##### NDEBUG
removes debug asserts

##### ASSPF_OPT_NO_PRINTF
removes the printf-like functions

## API Brief

### ASSPF_FD_STDOUT
'stdout' file-descriptor

### ASSPF_FD_STDERR
'stderr' file-descriptor

### ASSPF_WriteBuf
object used with the buffered writing functions

### asspf_writebuf_autoinit()
initializes a WriteBuf object

returns 0 on success (always succeeds)

### asspf_sys_write()
unbuffered write (error-checked system-call wrapper)

returns the number of bytes written

### asspf_flush()
flush a WriteBuf object to its file

returns 0 on success, or the number of bytes remaining in the buffer

### asspf_write()
write to a WriteBuf object

returns the number of bytes written

### asspf_puts()
write a NUL-terminated string to a WriteBuf object

returns the number of bytes written

### asspf_putc()
write a char to a WriteBuf object

returns the number of bytes written

### asspf_printf_*
write a formatted integer to a WriteBuf object

returns the number of bytes written

(most are macros for the four or so actual functions)

#### The Format String
It is mostly like printf(3)'s, but there are some differences.

The format string is made up of 4 parts (in order):

\- Flag Characters (optional)

\- Field Width (optional)

\- Precision (optional)

\- Conversion Specifier (mandatory)

##### Flag Characters
'#' - C-style sigil (2: "0xb", 8: "0", 10: "", 16: "0x").
Like printf(3), but "0x" for any hexadecimal.

'$' - Motorola-style sigil (2: "%", 8: "@", 10: "#", 16: "$").
If both '#' and '$' are present, '#' wins.

'0' - zero padded, like printf(3)

'-' - left adjusted, like printf(3)

' ' - space for a sign, like printf(3)

'+' - always print a sign, like printf(3)

##### Field Width
A decimal digit string (not starting with a '0').
Like printf(3).

##### Precision
A '.' followed by a decimal string.
Like printf(3), but a value of 0 means the type's maximum precision.

##### Conversion Specifier
'd' - decimal signed

'u' - decimal unsigned

'b' - binary unsigned

'o' - octal unsigned

'x' - hexadecimal unsigned (lowercase)

'X' - hexadecimal unsigned (uppercase)

## Example Program
```
#include <errno.h>
#include <stddef.h>
#include "asspf.h"

static size_t print_abcs(ASSPF_WriteBuf *);
static void   print_count(ASSPF_WriteBuf *, size_t);

int
main(void)
{
	const int errno_old = errno;	/* pretend this is a signal handler */

	ASSPF_WriteBuf wb_out;
	char cbuf_out[200u];
	size_t count;

	(void) asspf_writebuf_autoinit(
		&wb_out, ASSPF_FD_STDOUT, cbuf_out,
		(unsigned short) sizeof cbuf_out
	);

	count = print_abcs(&wb_out);
	print_count(&wb_out, count);
	(void) asspf_flush(&wb_out);

	errno = errno_old;
	return 0;
}

static size_t
print_abcs(ASSPF_WriteBuf *wb)
{
	const char abc_str[] = "ABCDEFG\nHIJKLMNOP\nQRS\nTUV\nWX\nY&Z\n";
	size_t count = 0;
	char c;
	size_t i;

	for ( i = 0; i < (sizeof abc_str) - 1u; ++i ){
		c = abc_str[i];
		if ( c != '\n' ){
			count += asspf_printf_char(wb, "$.0b", c);
			if ( abc_str[i + 1u] != '\n' ){
				c = ' ';
			}
			else {	continue; }
		}
		count += asspf_putc(wb, c);
	}
	count += asspf_puts(wb,
		"Now I know my ABC's.\n"
		"Next time, won't you sing with me?\n"
	);

	return count;
}

static void
print_count(ASSPF_WriteBuf *wb, size_t count)
{
	(void) asspf_putc(wb, '(');
	(void) asspf_printf_size(wb, "u", count);
	(void) asspf_puts(wb, " bytes printed)\n");
	return;
}
```
