z80pack includes an ICE-like monitor to allow inspection and/or
modification of the CPU registers, memory and I/O ports, single
stepping and tracing of instructions, etc.

It also optionally supports breakpoints, register history, and T-state
counting inside an address range. Type '?' when it is enabled to see a
complete command list.

It is included by default, with all features enabled, in z80sim and
mosteksim.

To include the ICE in cpmsim, altairsim, cromemcosim, or imsaisim you
need to uncomment some "#define"s in the corresponding "sim.h" file:

WANT_ICE	to enable the ICE
WANT_TIM	to enable T-state counting
HISIZE		to enable register history and optionally change the
		size of the history table
SBSIZE		to enable breakpoints and optionally change the size
		of the breakpoints table

For cpmsim see "README-cpm.txt" on how to build it. The simulators
which include a frontpanel (altairsim, cromemcosim, or imsaisim) need
to be build without it, as described in "README-frontpanel.txt".
