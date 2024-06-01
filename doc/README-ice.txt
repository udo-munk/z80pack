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

The ICE also can be included when running on bare metal, if the device
has enough memory. This is shown in picosim running on Raspberry Pi Pico.
Because the bare metal is not running an operating system all
commands that require one are disabled. Also this is done with a define
in sim.h for the machine:

ICE_BAREMETAL	don't include commands that require an OS

Also watch out what code you are going to execute, because without an
operating system that provides signal handling for applications, we have
no way to break a runnaway program with CTL-C, as it works with z80sim
on workstations. This requires at least a switch connected to an
interrupt pin and an interrupt handler, which signals the CPU emulation
to stop. This is implemented in picosim, also see picture of breadboard
wiring.
