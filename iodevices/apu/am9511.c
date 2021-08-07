/* am9511.c
 *
 * First cut am9511 emulation. This version is NOT cycle accurate,
 * or even algorithm accurate. It should be a somewhat reasonable
 * stand-in, which should allow us to run base-line comparisions with
 * the real device.
 */


#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "am9511.h"
#include "floatcnv.h"
#include "ova.h"
#include "types.h"


/* Define fp_na() -- fp to native and
 *        na_fp() -- native to fp
 */
#ifdef z80
#define fp_na(x,y) fp_hi(x,y)
#define na_fp(x,y) hi_fp(x,y)
#else
#define fp_na(x,y) fp_ie(x,y)
#define na_fp(x,y) ie_fp(x,y)
#endif

/* Stack is 16 bytes long. sp is the stack pointer.
 * Points to next location to use.
 *
 * AM9511 status and operator latch
 */

struct am_context {
    unsigned char stack[16];
    int sp;
    void *fptmp;
    unsigned char status;
    unsigned char op_latch;
#ifndef NDEBUG
    unsigned char last_latch;
#endif
};


#define AM_OP    0x1f


/* Add to sp
 */
#define sp_add(n) ((ctx->sp + (n)) & 0xf)


/* Return pointer into stack
 */
#define stpos(offset) (ctx->stack + sp_add(offset))


/* Increment stack pointer
 */
#define inc_sp(n) ctx->sp = sp_add(n)


/* Decrement stack pointer
 */
#define dec_sp(n) ctx->sp = sp_add(-(n))


/* Push byte to am9511 stack
 */
void am_push(void *amp, unsigned char v) {
    struct am_context *ctx = (struct am_context *)amp;
    *stpos(0) = v;
    inc_sp(1);
}


/* Pop byte from am9511 stack
 */
unsigned char am_pop(void *amp) {
    struct am_context *ctx = (struct am_context *)amp;
    dec_sp(1);
    return *stpos(0);
}



/* Return status of am9511
 */
unsigned char am_status(void *amp) {
    struct am_context *ctx = (struct am_context *)amp;
    return ctx->status;
}


#define IS_SINGLE ((ctx->op_latch & AM_SINGLE) == AM_SINGLE)
#define IS_FIXED (ctx->op_latch & AM_FIXED)


/* Set SIGN and ZERO according to op type and top of stack.
 * Zero detect for integer is or'ing together all the bytes.
 * Zero detect for float is testing bit 23 for 0.
 * The sign bit for all types is the top-most bit. If 1 then
 * negative.
 */
static void sz(struct am_context *ctx) {
    if (IS_SINGLE) {
	if ((*stpos(-1) | *stpos(-2)) == 0)
	    ctx->status |= AM_ZERO;
    } else if (IS_FIXED) {
	if ((*stpos(-1) | *stpos(-2) | *stpos(-3) | *stpos(-4)) == 0)
	    ctx->status |= AM_ZERO;
    } else {
	if ((*stpos(-2) & 0x80) == 0)
	    ctx->status |= AM_ZERO;
    }
    if (*stpos(-1) & 0x80)
	ctx->status |= AM_SIGN;
}


/* PUPI
 */
static void pupi(struct am_context *ctx) {
    am_push(ctx, 0xda); /* little end to big end */
    am_push(ctx, 0x0f);
    am_push(ctx, 0xc9);
    am_push(ctx, 0x02);
    sz(ctx);
}


/* PTOS PTOD PTOF
 *
 * This relies on the stack data not moving during a push.
 */
static void pto(struct am_context *ctx) {
    unsigned char *s; 

    if (IS_SINGLE) {
        s = stpos(-2);
	am_push(ctx, *s++);
	am_push(ctx, *s);
    } else {
        s = stpos(-4);
        am_push(ctx, *s++);
	am_push(ctx, *s++);
	am_push(ctx, *s++);
	am_push(ctx, *s);
    }
    sz(ctx);
}


/* POPS POPD POPF
 *
 * Note that the SIGN and ZERO flags are set from the element that
 * is next on stack. But... it may be wrong! We do not know what the
 * new tos element really is! (in terms of type)
 * The guide states and SIGN and ZERO are affected, but no more than that.
 */
static void pop(struct am_context *ctx) {
    if (IS_SINGLE)
    	dec_sp(2);
    else
    	dec_sp(4);
    sz(ctx);
}


/* XCHS XCHD XCHF
 */
static void xch(struct am_context *ctx) {
    unsigned char *s, *t, v;

    if (IS_SINGLE) {
	s = stpos(-2);
	t = stpos(-4);
	v = *t; *t++ = *s; *s++ = v;
	v = *t; *t   = *s; *s   = v;
    } else {
	s = stpos(-4);
	t = stpos(-8);
	v = *t; *t++ = *s; *s++ = v;
	v = *t; *t++ = *s; *s++ = v;
	v = *t; *t++ = *s; *s++ = v;
	v = *t; *t   = *s; *s   = v;
    }
    sz(ctx);
}


/* CHSF
 */
static void chsf(struct am_context *ctx) {
    /* Floating point sign change - only flip sign
     * (if not zero). And, as with the AM9511 chip, CHSF
     * is even faster than CHSS.
     */
    if (*stpos(-2) & 0x80)
        *stpos(-1) ^= 0x80;
    sz(ctx);
}


/* CHSS CHSD
 */
static void chs(struct am_context *ctx) {
    if (IS_SINGLE) {
        if (cm16(stpos(-2), stpos(-2)))
	    ctx->status |= AM_ERR_OVF;
    } else {
	if (cm32(stpos(-4), stpos(-4)))
	    ctx->status |= AM_ERR_OVF;
    }
    sz(ctx);
}


/* Push float to stack, set SIGN and ZERO
 */
static void push_float(struct am_context *ctx, float x) {
    unsigned char v[4];

    na_fp(&x, ctx->fptmp);
    fp_am(ctx->fptmp, v);
    am_push(ctx, v[0]);
    am_push(ctx, v[1]);
    am_push(ctx, v[2]);
    am_push(ctx, v[3]);
    ctx->op_latch = AM_FLOAT;
    sz(ctx);
}


/* FLTS
 */
static void flts(struct am_context *ctx) {
    int16 n;
    float x;

    n = am_pop(ctx);
    n = (n << 8) | am_pop(ctx);
    x = n;
    push_float(ctx, x);
}


/* FLTD
 */
static void fltd(struct am_context *ctx) {
    int32 n;
    float x;
    int b;

    /* HI-TECH C long shift bug
     */
    b = am_pop(ctx);
    n = b;

    n = n << 8;
    b = am_pop(ctx);
    n = n | b;

    n = n << 8;
    b = am_pop(ctx);
    n = n | b;

    n = n << 8;
    b = am_pop(ctx);
    n = n | b;

    x = n;
    push_float(ctx, x);
}


/* FIXS
 */
static void fixs(struct am_context *ctx) {
    float x;
    unsigned char *s;
    int n;

    s = stpos(-4);
    am_fp(s, ctx->fptmp);
    fp_na(ctx->fptmp, &x);
    if ((x < -32768.0) || (x > 32767.0)) {
	ctx->status |= AM_ERR_OVF;
	sz(ctx);
	return;
    }
    dec_sp(4);
    n = (int)x;
    am_push(ctx, n);
    am_push(ctx, n >> 8);
    ctx->op_latch = AM_SINGLE;
    sz(ctx);

}


/* FIXD
 */
static void fixd(struct am_context *ctx) {
    float x;
    unsigned char *s;
    int32 n;
    float xl, xh;

    s = stpos(-4);
    am_fp(s, ctx->fptmp);
    fp_na(ctx->fptmp, &x);
    n = -2147483648;
    xl = (float)n;
    n = 2147483647;
    xh = (float)n;
    if ((x < xl) || (x > xh)) {
	ctx->status |= AM_ERR_OVF;
	sz(ctx);
	return;
    }
    dec_sp(4);
    n = (int32)x;
    am_push(ctx, n);
    am_push(ctx, n >> 8);
    am_push(ctx, n >> 16);
    am_push(ctx, n >> 24);
    ctx->op_latch = AM_DOUBLE;
    sz(ctx);
}


/* SADD DADD
 */
static void add(struct am_context *ctx) {
    int carry;
    int overflow;

    if (IS_SINGLE) {
        carry    = add16( stpos(-4), stpos(-2), stpos(-4));
        overflow = oadd16(stpos(-4), stpos(-2), stpos(-4));
	dec_sp(2);
    } else {
        carry    = add32( stpos(-8), stpos(-4), stpos(-8));
        overflow = oadd32(stpos(-8), stpos(-4), stpos(-8));
	dec_sp(4);
    }
    if (carry)
	ctx->status |= AM_CARRY;
    if (overflow)
	ctx->status |= AM_ERR_OVF;
    sz(ctx);
}


/* SSUB DSUB
 */
static void sub(struct am_context *ctx) {
    int carry;
    int overflow;

    if (IS_SINGLE) {
        carry    = sub16( stpos(-4), stpos(-2), stpos(-4));
        overflow = osub16(stpos(-4), stpos(-2), stpos(-4));
	dec_sp(2);
    } else {
        carry    = sub32( stpos(-8), stpos(-4), stpos(-8));
        overflow = osub32(stpos(-8), stpos(-4), stpos(-8));
	dec_sp(4);
    }
    if (carry)
	ctx->status |= AM_CARRY;
    if (overflow)
	ctx->status |= AM_ERR_OVF;
    sz(ctx);
}


/* MUL
 */
static void mul(struct am_context *ctx) {
    int overflow;

    if (IS_SINGLE) {
        overflow = mull16(stpos(-4), stpos(-2), stpos(-4));
        dec_sp(2);
    } else {
        overflow = mull32(stpos(-8), stpos(-4), stpos(-8));
        dec_sp(4);
    }
    if (overflow)
	ctx->status |= AM_ERR_OVF;
    sz(ctx);
}


/* MUU
 */
static void muu(struct am_context *ctx) {
    int overflow;

    if (IS_SINGLE) {
        overflow = mulu16(stpos(-4), stpos(-2), stpos(-4));
        dec_sp(2);
    } else {
        overflow = mulu32(stpos(-8), stpos(-4), stpos(-8));
        dec_sp(4);
    }
    if (overflow)
	ctx->status |= AM_ERR_OVF;
    sz(ctx);
}


/* DIV
 */
static void divi(struct am_context *ctx) {
    int div0;

    if (IS_SINGLE) {
        div0 = div16(stpos(-4), stpos(-2), stpos(-4));
        dec_sp(2);
    } else {
        div0 = div32(stpos(-8), stpos(-4), stpos(-8));
        dec_sp(4);
    }
    if (div0)
	ctx->status |= AM_ERR_DIV0;
    sz(ctx);
}


/* Detect and report float overflow/underflow
 */
static int fov(struct am_context *ctx, double r) {
    int e;

    frexp(r, &e);
    if (e > 63) {
	ctx->status |= AM_ERR_OVF;
	return 1;
    } else if (e < -64) {
	ctx->status |= AM_ERR_UND;
	return 1;
    }
    return 0;
}


/* basicf - basic FADD/FSUB/FMUL/FDIV
 *
 * The guide says that overflow and underflow are detected on the
 * exponent. The mantissa is maintained, and the exponent is offset
 * by 128. So... that is what we do. Note that frexp() and ldexp()
 * should be implemented via bit operations, not arithmetic.
 */
static void basicf(struct am_context *ctx) {
    unsigned char *ap, *bp;
    float a, b, r;
    double m;
    int e;
 
    ap = stpos(-4);
    am_fp(ap, ctx->fptmp);
    fp_na(ctx->fptmp, &a);

    bp = stpos(-8);
    am_fp(bp, ctx->fptmp);
    fp_na(ctx->fptmp, &b);

    switch (ctx->op_latch & AM_OP) {
    case AM_FADD:
        r = a + b;
	break;
    case AM_FSUB:
        r = b - a;
	break;
    case AM_FMUL:
        r = a * b;
	break;
    case AM_FDIV:
	if (a == 0.0) {
	    r = b;
	    ctx->status |= AM_ERR_DIV0;
	} else
            r = b / a;
	break;
    }

    /* We do not use fov() because we want to bias exponent by 128
     * on OVF/UND per the guide.
     */
    m = frexp(r, &e);
    if (e > 63) {
	ctx->status |= AM_ERR_OVF;
	e -= 128;
	r = ldexp(m, e);
    } else if (e < -64) {
	ctx->status |= AM_ERR_UND;
	e += 128;
	r = ldexp(m, e);
    }
    na_fp(&r, ctx->fptmp);
    fp_am(ctx->fptmp, bp);
    dec_sp(4);
    ctx->op_latch = AM_FLOAT;
    sz(ctx);
}


/* SQRT EXP SIN COS TAN LN LOG etc (functions with single arg)
 *
 * Note that we use the -lm math library with GCC, and the -LF library
 * with HI-TECH C. This means we are limited to only using functions
 * that are in both. This explains the strange shenanigans with double
 * here.
 */
static void ffunc(struct am_context *ctx) {
    unsigned char *ap;
    float a;
    double x;

    ap = stpos(-4);
    am_fp(ap, ctx->fptmp);
    fp_na(ctx->fptmp, &a);

    x = a;
    switch (ctx->op_latch & AM_OP) {
    case AM_SQRT:
        if (a < 0.0) {
	    ctx->status |= AM_ERR_NEG;
	    goto err;
	}
        x = sqrt(x);
	break;
    case AM_EXP:
        /* -1.0 x 2^5 .. 1.0 x 2^5 */
        if ((a < -32.0) || (a > 32.0)) {
	    ctx->status |= AM_ERR_ARG;
	    goto err;
	}
        x = exp(x);
	break;
    case AM_SIN:
	x = sin(x);
	break;
    case AM_COS:
	x = cos(x);
	break;
    case AM_TAN:
	/* less than 2^-12 : return A as tan(A) */
	if (a >= (1.0 / 4096.0))
            x = tan(x);
	break;
    case AM_LN:
	if (a < 0.0) {
	    ctx->status |= AM_ERR_NEG;
	    goto err;
	}
	x = log(x);
	break;
    case AM_LOG:
	if (a < 0.0) {
	    ctx->status |= AM_ERR_NEG;
	    goto err;
	}
	x = log10(x);
	break;
    case AM_ASIN:
	if ((a < -1.0) || (a > 1.0)) {
	   ctx->status |= AM_ERR_ARG;
	   goto err;
	}
	x = asin(x);
	break;
    case AM_ACOS:
	if ((a < -1.0) || (a > 1.0)) {
	   ctx->status |= AM_ERR_ARG;
	   goto err;
	}
	x = acos(x);
	break;
    case AM_ATAN:
	x = atan(x);
	break;
    }
    if (fov(ctx, x))
	goto err;
    a = x;
    na_fp(&a, ctx->fptmp);
    fp_am(ctx->fptmp, ap);
err:
    ctx->op_latch = AM_FLOAT;
    sz(ctx);
}


/* PWR
 *
 * B^A = EXP( A * LN(B) )
 */
static void pwr(struct am_context *ctx) {
    /* B^A = EXP( A * LN(B) ) */
    unsigned char *ap, *bp;
    float a, b;
    double x;

    /* A */
    ap = stpos(-4);
    am_fp(ap, ctx->fptmp);
    fp_na(ctx->fptmp, &a);

    /* B */
    bp = stpos(-8);
    am_fp(bp, ctx->fptmp);
    fp_na(ctx->fptmp, &b);

    /* LN(B) */
    if (b < 0.0) {
	ctx->status |= AM_ERR_NEG;
	goto err;
    }
    x = b;
    x = log(x);

    /* A * LN(B) */
    x = (double)a * x;

    /* EXP( A * LN(B) ) */
    if ((x < -32.0) || (x > 32.0)) {
        ctx->status |= AM_ERR_ARG;
	goto err;
    }
    x = exp(x);

    if (fov(ctx, x))
        goto err;

    /* replace B with result */
    b = x;
    na_fp(&b, ctx->fptmp);
    fp_am(ctx->fptmp, bp);

    /* roll stack */
    dec_sp(4);
err:
    sz(ctx);
}


/* Issue am9511 command. Does not return until command
 * is complete.
 */
void am_command(void *amp, unsigned char op) {
    struct am_context *ctx = (struct am_context *)amp;

    ctx->op_latch = op;

#ifndef NDEBUG
    ctx->last_latch = op;
#endif

    ctx->status = AM_BUSY;

    switch (ctx->op_latch & AM_OP) {

    case AM_NOP:  /* no operation */
	ctx->status = 0;
        break;

    case AM_PUPI: /* push pi */
	pupi(ctx);
	break;

    case AM_CHS:  /* change sign */
	chs(ctx);
	break;

    case AM_CHSF: /* float change sign */
	chsf(ctx); /* per Wayne Hortensius */
	break;

    case AM_POP:  /* pop */
	pop(ctx);
        break;

    case AM_PTO:  /* push tos (copy) */
	pto(ctx);
        break;

    case AM_XCH:  /* exchange tos and nos */
	xch(ctx);
        break;

    case AM_FLTD: /* 32 bit to float */
	fltd(ctx);
        break;

    case AM_FLTS: /* 16 bit to float */
	flts(ctx);
        break;

    case AM_FIXD: /* float to 32 bit */
	fixd(ctx);
        break;

    case AM_FIXS: /* float to 16 bit */
	fixs(ctx);
        break;

    case AM_ADD:  /* add */
	add(ctx);
	break;

    case AM_SUB:  /* subtract nos-tos */
	sub(ctx);
        break;

    case AM_MUL:  /* multiply, lower half */
	mul(ctx);
        break;

    case AM_MUU:  /* multiply, upper half */
	muu(ctx);
        break;

    case AM_DIV:  /* divide nos/tos */
	divi(ctx);
        break;

    case AM_FADD: /* floating add */
    case AM_FSUB: /* floating subtract */
    case AM_FMUL: /* floating multiply */
    case AM_FDIV: /* floating divide */
	basicf(ctx);
        break;

    case AM_SQRT: /* square root */
    case AM_EXP:  /* exponential (e^x) */
    case AM_SIN:  /* sine */
    case AM_COS:  /* cosine */
    case AM_TAN:  /* tangent */
    case AM_LOG:  /* common logarithm (base 10) */
    case AM_LN:   /* natural logarthm (base e) */
    case AM_ASIN: /* inverse sine */
    case AM_ACOS: /* inverse cosine */
    case AM_ATAN: /* inverse tangent */
	ffunc(ctx);
        break;

    case AM_PWR:  /* power nos^tos */
	pwr(ctx);
        break;

    default:
        break;
    }

    ctx->status &= ~AM_BUSY;
}


/* Reset the am9511 emulator
 */
void am_reset(void *amp) {
    struct am_context *ctx = (struct am_context *)amp;
    int i;

    ctx->sp = 0;
    ctx->status = 0;
    ctx->op_latch = 0;
    ctx->last_latch = 0;
    for (i = 0; i < 16; ++i)
	ctx->stack[i] = 0;
}


/* Create chip.
 */
void *am_create(int status, int data) {
    struct am_context *p;
    void *fpp;
    fpp = malloc(apu_fp_size());
    if (fpp == NULL)
	return NULL;
    p = malloc(sizeof (struct am_context));
    if (p == NULL)
	return NULL;
    p->fptmp = fpp;
    am_reset(p);
    return (void *)p;
}


/* Dump stack A..H or A..D, format depends on arg (AM_SINGLE,
 * AM_DOUBLE, AM_FLOAT). Dump status and last op_latch.
 */
void am_dump(void *amp, unsigned char op) {
    struct am_context *ctx = (struct am_context *)amp;
    int i;
    int16 n;
    int32 nl;
    float x;
    unsigned char t = ctx->status;
    int b;
    static char *opnames[] = {
        "NOP",  "SQRT", "SIN",  "COS",
        "TAN",  "ASIN", "ACOS", "ATAN",
        "LOG",  "LN",   "EXP",  "PWR",
        "ADD",  "SUB",  "MUL",  "DIV",
        "FADD", "FSUB", "FMUL", "FDIV",
        "CHS",  "CHSF", "MUU",  "PTO",
        "POP",  "XCH",  "PUPI", "",
        "FLTD", "FLTS", "FIXD", "FIXS"
    };

    printf("AM9511 STATUS: %02x ", ctx->status);
        if (t & AM_BUSY)  printf("BUSY ");
        if (t & AM_SIGN)  printf("SIGN ");
        if (t & AM_ZERO)  printf("ZERO ");
        if (t & AM_CARRY) printf("CARRY ");
        printf("ERROR: ");
        t &= AM_ERR_MASK;
        if (t == AM_ERR_NONE) printf("NONE");
        if (t & AM_ERR_DIV0)  printf("DIV0");
        if (t & AM_ERR_NEG)   printf("NEG");
        if (t & AM_ERR_ARG)   printf("ARG");
        if (t & AM_ERR_ARG)   printf("ARG");
        if (t & AM_ERR_UND)   printf("UND");
        if (t & AM_ERR_OVF)   printf("OVF");
        printf("\n");
    t = ctx->last_latch;
    printf("LAST COMMAND: ");
        if (t & AM_SR)                    printf("SR ");
        if ((t & AM_SINGLE) == AM_SINGLE) printf("SINGLE ");
        else if (t & AM_FIXED)            printf("DOUBLE ");
        else                              printf("FLOAT ");
        printf("%s\n", opnames[t & AM_OP]);
    t = ctx->op_latch;
    ctx->op_latch = op;
    printf("AM9511 STACK ");
    if (IS_SINGLE) {
	printf("(SINGLE)\n");
	for (i = 0; i < 8; ++i) {
            n =            *stpos(-(i * 2) - 1);
            n = (n << 8) | *stpos(-(i * 2) - 2);
	    printf("%c: %02x %02x %d\n", 'A' + i,
                                         *stpos(-(i * 2) - 1),
                                         *stpos(-(i * 2) - 2),
					 n);
	}
    } else {
        if (IS_FIXED)
	    printf("(DOUBLE)\n");
	else
	    printf("(FLOAT)\n");
	for (i = 0; i < 4; ++i) {
    	    printf("%c: %02x %02x %02x %02x ", 'A' + i,
                                               *stpos(-(i * 4) - 1),
                                               *stpos(-(i * 4) - 2),
                                               *stpos(-(i * 4) - 3),
                                               *stpos(-(i * 4) - 4));
	    if (IS_FIXED) {
#if 0
		/* Borked -- HI-TECH C bug
		 *
		 * We have seen this before -- use a "fix" (work-around)
		 */
                nl =             *stpos(-(i * 4) - 1);
                nl = (nl << 8) | *stpos(-(i * 4) - 2);
                nl = (nl << 8) | *stpos(-(i * 4) - 3);
                nl = (nl << 8) | *stpos(-(i * 4) - 4);
#else
		/* We use the following instead, which seems to work
		 */
                b = *stpos(-(i * 4) - 1);
		nl = b;

		nl = nl << 8;
                b = *stpos(-(i * 4) - 2);
		nl = nl | b;

		nl = nl << 8;
                b = *stpos(-(i * 4) - 3);
		nl = nl | b;

		nl = nl << 8;
                b = *stpos(-(i * 4) - 4);
		nl = nl | b;
#endif
		printf("%ld\n", (long)nl);
	    } else {
		am_fp(stpos(-(i * 4) - 4), ctx->fptmp);
		fp_na(ctx->fptmp, &x);
		printf("%g\n", x);
	    }
	}
    }
    ctx->op_latch = t;
}
