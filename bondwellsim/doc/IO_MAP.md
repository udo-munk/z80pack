# Bondwell I/O map

Only the low eight address bits are decoded. Source: MAME bw12.cpp
`bw12_io`, `common`, and original-ROM port traces; schematic verification pending.

| Ports | Device | Read | Write |
| --- | --- | --- | --- |
| 00–0F | LS259 | update addressed latch, return 00 | update addressed latch; ignore data |
| 10–1E even | MC6845 index | FF | select register |
| 11–1F odd | MC6845 data | selected register | selected register |
| 20–2E even | uPD765 | main status | ignored |
| 21–2F odd | uPD765 | execution/result data | command/parameters/execution data |
| 30/34/38/3C | 6821 A | DDRA or PA | DDRA or output A |
| 31/35/39/3D | 6821 CRA | control/IRQ flags | control |
| 32/36/3A/3E | 6821 B | DDRB or PB; clear IRQ flags on data read | DDRB or printer data |
| 33/37/3B/3F | 6821 CRB | control/IRQ flags | control and keyboard shift clock |
| 40/44/48/4C | SIO A data | receive byte | transmit byte |
| 41/45/49/4D | SIO A control | RR0/1/2 | register pointer/command/WR0–7 |
| 42/46/4A/4E | SIO B data | receive byte | transmit byte |
| 43/47/4B/4F | SIO B control | RR0/1/2 | register pointer/command/WR0–7 |
| 50–5F | MC1408 DAC | FF | 8-bit sample |
| 60/64/68/6C | 8253 counter 0 | counter/latch | reload |
| 61/65/69/6D | 8253 counter 1 | counter/latch | reload |
| 62/66/6A/6E | 8253 counter 2 | counter/latch | reload |
| 63/67/6B/6F | 8253 control | FF | mode/access/latch |
| 70–FF | unmapped | FF | ignored |

The CRTC currently returns stored values for all selected registers rather than
enforcing the original chip's write-only register restrictions. SIO and PIA
support the polled firmware paths; see README for incomplete peripheral modes.
