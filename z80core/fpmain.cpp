/*
 * Z80SIM  -  a Z80-CPU simulator  -  C++ main()
 *
 * Copyright (C) 2022 Thomas Eberhardt
 *
 * History:
 * 19-OCT-2022 add C++ main() for linking with the C++ frontpanel library
 */

#include "simmain.h"

int main(int argc, char **argv)
{
	return sim_main(argc, argv);
}
