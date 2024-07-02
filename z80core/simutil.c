/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 Udo Munk
 */

#include <ctype.h>

/*
 *	atoi for hexadecimal numbers
 */
int exatoi(char *str)
{
	register int num = 0;

	while (isxdigit((unsigned char) *str)) {
		num *= 16;
		if (*str <= '9')
			num += *str - '0';
		else
			num += toupper((unsigned char) *str) - '7';
		str++;
	}
	return num;
}
