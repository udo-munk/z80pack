#ifndef BW_SIMMEM_H
#define BW_SIMMEM_H
#include "bondwell.h"
static inline BYTE memrdr(WORD a) { return bw_mem_read(a); }
static inline void memwrt(WORD a, BYTE v) { bw_mem_write(a, v); }
static inline BYTE getmem(WORD a) { return bw_mem_read(a); }
static inline void putmem(WORD a, BYTE v) { bw_mem_write(a, v); }
static inline BYTE dma_read(WORD a) { return bw_mem_read(a); }
static inline void dma_write(WORD a, BYTE v) { bw_mem_write(a, v); }
#endif
