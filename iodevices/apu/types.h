/* types.h
 */


#ifndef TYPES_H
#define TYPES_H

#ifdef z80

#include <ansi.h>

/* Types for Hi-Tech C, z80, 8 bit
 */
#define uint8  unsigned char
#define uint16 unsigned int
#define uint32 unsigned long
#define int8   signed char
#define int16  int
#define int32  long

#else

/* Types for GCC/TCC, Linux, 64 bit
 */
#include <stdint.h>

#define uint8  uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define int8   int8_t
#define int16  int16_t
#define int32  int32_t

#endif

#endif
