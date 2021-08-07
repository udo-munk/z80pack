/* ova.h
 *
 * Not an egg -- overflow arithmetic. 16 and 32 bit.
 */

#ifndef _OVA_H
#define _OVA_H

int oadd16(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int oadd32(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int osub16(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int osub32(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int add16(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int add32(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int cm16(unsigned char *pa, unsigned char *pb);
int cm32(unsigned char *pa, unsigned char *pb);
int sub16(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int sub32(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int mull16(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int mulu16(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int div16(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int div32(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int mull32(unsigned char *pa, unsigned char *pb, unsigned char *pc);
int mulu32(unsigned char *pa, unsigned char *pb, unsigned char *pc);

#endif

