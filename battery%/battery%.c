//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// battery%.c - print the battery charge and status                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright 2023-2024, Shane Seelig                                        //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////

// may need to modify these, if yours differ
#define FILE_CHARGE_NOW		"/sys/class/power_supply/BAT1/charge_now"
#define FILE_CHARGE_FULL	"/sys/class/power_supply/BAT1/charge_full"
#define FILE_STATUS		"/sys/class/power_supply/BAT1/status"

//////////////////////////////////////////////////////////////////////////////

static void
fgets_check(const char *filename, /*@out@*/ char *buf, size_t size)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*buf
@*/
{
	FILE *fh;

	fh = fopen(filename, "r");	// exit handles fclose
	if ( fh == NULL ){
		exit(EXIT_FAILURE);
	}
	(void) fgets(buf, (int) size, fh);
	return;
}

//--------------------------------------------------------------------------//

int
main(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char plus_array[]  = {'+','+','+'};
	const char space_array[] = {' ',' ',' '};
	const char *array;
	size_t array_nmemb;
	//
	char buf[80u];
	long now, full;
	unsigned int percent, frac;
	bool discharging;

	fgets_check(FILE_CHARGE_FULL, buf, sizeof buf);
	full = atol(buf);

	fgets_check(FILE_CHARGE_NOW, buf, sizeof buf);
	now = 100uL * atol(buf);

	percent = (unsigned int) (now / full);
	frac = (unsigned int) ((100u * (now % full)) / full);

	fgets_check(FILE_STATUS, buf, sizeof buf);
	discharging = (bool) (strcmp(buf, "Discharging\n") == 0);

	array = discharging ? space_array : plus_array;
	if ( percent == 100u ){
		array_nmemb = (size_t) 1u;
	}
	else if ( percent >= 10u ){
		array_nmemb = (size_t) 2u;
	}
	else {	// percent < 10u
		array_nmemb = (size_t) 3u;
	}

	(void) fwrite(array, sizeof *array, array_nmemb, stdout);
	(void) printf("%u.%0.2u%%\n", percent, frac);

	return EXIT_SUCCESS;
}

// EOF ///////////////////////////////////////////////////////////////////////
