# Hardware model and provenance

Baseline: upstream z80pack dev `bb92b240c4c8ff0fda7d80dc23ab5ad0b0077f33`.
The machine runs the existing z80pack NMOS Z80 implementation at a nominal 4 MHz.

The implementation was independently written from the hardware connections in
[MAME bw12.cpp](https://github.com/mamedev/mame/blob/master/src/mame/bondwell/bw12.cpp)
(BSD-3-Clause, Curt Coder) and validated against the original boot ROM. Keyboard
encoding was corroborated against
[kb3600.cpp](https://github.com/mamedev/mame/blob/master/src/devices/machine/kb3600.cpp).
The user mentioned six schematic pages in prior conversation, but the PDFs
were not attached or located here. This is **not** a claim of schematic-level
verification. Motor timing, interrupt timing and analogue behaviour remain
approximations where described below.

## Memory

LS259 Q0/Q1 select the mapping of 0000–7FFF:

| Bank | Model 12 | Model 14 |
| --- | --- | --- |
| 0 (reset) | 4 KB ROM mirrored eight times, writes ignored | same |
| 1 | first 32 KB RAM bank | first 32 KB RAM bank |
| 2 | open bus FF, writes ignored | second 32 KB RAM bank |
| 3 | open bus FF, writes ignored | third 32 KB RAM bank |

8000–F7FF is common RAM. F800–FFFF is 2 KB video RAM shared across mappings.
The sum is 64 KB (Model 12) or 128 KB (Model 14), including the video region.
The ROM itself relocates its code into common RAM and tests the banked memory.

## Devices

* LS259: selected output = port bits 3:1, level = port bit 0. Reads and writes
  both update the latch; the write data byte is irrelevant. Q0/1 select RAM,
  Q3 printer initialize, Q4 Caps Lock LED, Q5/6 motor requests, Q7 FDC TC.
* Either motor request turns on both drives; final request removal delays
  motor-off by 170 ms. The FDC READY input is tied active. A: and B: are FDC
  units **1 and 2**, not 0 and 1.
* 6821 PA0/1/2 are printer busy/fault/paper status; PA3 motor status; PA4 PIT
  counter 2; PA5 key ready; PA6 serialized key bit; PA7 FDC IRQ. FDC IRQ is
  polled through the PIA, not directly wired to the Z80 interrupt input.
* Keyboard matrix index is row*10+column. Indices 64–89 wrap their low six bits
  and set bit 8. Shift sets bit 6, Control bit 7. Serialized bit order is
  **6,3,1,0,2,4,5,7,8**, advanced on CB2 rising edges; CB1 signals availability.
* MC6845 normally programs 80 columns, 25 rows, nine scanlines. Each glyph
  occupies 16 bytes in the 4 KB ROM, bit 7 leftmost. All 256 codes are retained.
* 8253 input frequency is 1.8432 MHz; counters 0/1 clock SIO A/B, counter 2
  drives PIA PA4. The implementation uses emulated T-states, not host sleeps,
  for counter and transmit timing.

See [I/O map](IO_MAP.md) and [validation](VALIDATION.md) for implemented scope.
