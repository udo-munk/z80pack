/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2019-2021 by Udo Munk
 *
 * Emulation of a RTC to provide date and time information.
 * This doesn't emulate a specific RTC card or chip, in 1980
 * there was no standard yet for such things.
 *
 * History:
 * 24-OCT-2019 moved out of the cpmsim machine
 * 30-JUN-2021 clock read now returns time format instead of last command
 */

#include <stdint.h>
#include <time.h>
#include "sim.h"

static BYTE clkcmd;		/* clock command */
static BYTE clkfmt;		/* clock format, 0 = BCD, 1 = decimal */

/*
 *	Convert an integer to BCD
 */
static int to_bcd(int val)
{
	register int i = 0;

	while (val >= 10) {
		i += val / 10;
		i <<= 4;
		val %= 10;
	}
	i += val;
	return (i);
}

/*
 *	Calculate number of days since 1.1.1978
 *	CP/M 3 and MP/M 2 are Y2K bug fixed and can handle the date,
 *	the Y2K bug is intentional, so don't try to fix it here.
 */
static int get_date(struct tm *t)
{
	register int i;
	register int val = 0;

	for (i = 1978; i < 1900 + t->tm_year; i++) {
		val += 365;
		if (i % 4 == 0)
			val++;
	}
	val += t->tm_yday + 1;
	return (val);
}

/*
 *	I/O handler for read clock command:
 *	return format of time, 0 = BCD - 1 = decimal
 */
BYTE clkc_in(void)
{
	return (clkfmt);
}

/*
 *	I/O handler for write clock command:
 *	set the wanted clock command
 *	toggle BCD/decimal format if toggle command (255)
 */
void clkc_out(BYTE data)
{
	clkcmd = data;
	if (data == 255)
		clkfmt = clkfmt ^ 1;
}

/*
 *	I/O handler for read clock data:
 *	dependent on the last clock command the following
 *	information is returned from the system clock:
 *		0 - seconds in BCD or decimal
 *		1 - minutes in BCD or decimal
 *		2 - hours in BCD or decimal
 *		3 - low byte number of days since 1.1.1978
 *		4 - high byte number of days since 1.1.1978
 *		5 - day of month in BCD or decimal
 *		6 - month in BCD or decimal
 *		7 - year in BCD or decimal
 *	for every other clock command a 0 is returned
 */
BYTE clkd_in(void)
{
	register struct tm *t;
	register int val;
	time_t Time;

	time(&Time);
	t = localtime(&Time);
	switch (clkcmd) {
	case 0:			/* seconds */
		if (clkfmt)
			val = t->tm_sec;
		else
			val = to_bcd(t->tm_sec);
		break;
	case 1:			/* minutes */
		if (clkfmt)
			val = t->tm_min;
		else
			val = to_bcd(t->tm_min);
		break;
	case 2:			/* hours */
		if (clkfmt)
			val = t->tm_hour;
		else
			val = to_bcd(t->tm_hour);
		break;
	case 3:			/* low byte days */
		val = get_date(t) & 255;
		break;
	case 4:			/* high byte days */
		val = get_date(t) >> 8;
		break;
	case 5:			/* day of month */
		if (clkfmt)
			val = t->tm_mday;
		else
			val = to_bcd(t->tm_mday);
		break;
	case 6:			/* month */
		if (clkfmt)
			val = t->tm_mon;
		else
			val = to_bcd(t->tm_mon);
		break;
	case 7:			/* year */
		if (clkfmt)
			val = t->tm_year;
		else
			val = to_bcd(t->tm_year);
		break;
	default:
		val = 0;
		break;
	}
	return ((BYTE) val);
}

/*
 *	I/O handler for write clock data:
 *	under UNIX the system clock only can be set by the
 *	super user, so we do nothing here
 */
void clkd_out(BYTE data)
{
	UNUSED(data);
}
