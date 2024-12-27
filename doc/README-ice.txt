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
SBSIZE		to enable software breakpoints and optionally change
		the size of the breakpoints table
WANT_HB		to enable the hardware breakpoint

For cpmsim see "README-cpm.txt" on how to build it. The simulators
which include a frontpanel (altairsim, cromemcosim, or imsaisim) need
to be build without it, as described in "README-frontpanel.txt".

The ICE also can be included when running on bare metal, if the device
has enough memory. This is shown in picosim running on a Raspberry Pi
Pico. Because on bare metal there is no operating system all commands
that require one are disabled. To enable bare metal support add the
following to the sim.h file for the machine:

#define BAREMETAL       /* disable ICE commands that require a full OS */

Also watch out what code you are going to execute, because without an
operating system that provides signal handling for applications, we
have no way to break a runaway program with CTL-C, as it works with
z80sim on workstations. This requires at least a switch connected to
an interrupt pin and an interrupt handler, which signals the CPU
emulation to stop. This is implemented in picosim, see also the
picture of breadboard wiring. With picosim it is also possible to
trigger an interrupt over USB with a serial line BREAK signal (for
example, CTL-A F in minicom or CTL-Pause/Break in putty).
