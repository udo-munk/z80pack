/*
 * Program to generate pixmap font file from BDF font.
 * Not very robust parsing, works with the Terminus fonts.
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>

#define STRSIZE		100
#define NUMCHARS	128

/* uni/x11gr.uni and uni/ascii-h.uni */
static uint16_t codepts[NUMCHARS] = {
	0x25AE, 0x25C6, 0x2592, 0x2409, 0x240C, 0x240D, 0x240A, 0x00B0,
	0x00B1, 0x2424, 0x240B, 0x2518, 0x2510, 0x250C, 0x2514, 0x253C,
	0x23BA, 0x23BB, 0x2500, 0x23BC, 0x23BD, 0x251C, 0x2524, 0x2534,
	0x252C, 0x2502, 0x2264, 0x2265, 0x03C0, 0x2260, 0x00A3, 0x00B7,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x2302
};

static char *prog;

#define MISSFONT	"missing font file name"
#define CANTOPEN	"can't open font file"
#define OUTOFMEM	"out of memory"
#define EOFINPUT	"end of file on input"

static void fatal(char *s)
{
	fprintf(stderr, "%s: %s\n", prog, s);
	exit(EXIT_FAILURE);
}

static char *get_token(char *s, char *token)
{
	while (!isspace((unsigned char) *s) && *s != '\n' && *s != '\0')
		*token++ = *s++;
	while (isspace((unsigned char) *s))
		s++;
	*token = '\0';
	return s;
}

static char *copy_str(char *s)
{
	char *p, *t;
	int n;

	if (*s == '"')
		s++;
	p = s;
	while (*p != '"' && *p != '\n' && *p != '\0')
		p++;
	n = p - s;

	t = (char *) malloc(n + 1);
	if (t != NULL) {
		strncpy(t, s, n);
		t[n] = '\0';
	}
	return t;
}

int main(int argc, char *argv[])
{
	char line[STRSIZE], token[STRSIZE];
	FILE *fp;
	char *copyright = NULL, *notice = NULL;
	char *family = NULL, *weight = NULL;
	char *s;
	int size = 0, fbboxw = 0, fbboxh = 0, fbboxl = 0, fbboxb = 0;
	int ch = 0, bboxw = 0, bboxh = 0, bboxl = 0, bboxb = 0;
	int stride, codept, i, j, off;
	uint8_t *bitmap, *p, *p0, m, m0, c, mc;

	prog = argv[0];
	if (argc < 2)
		fatal(MISSFONT);

	if ((fp = fopen(argv[1], "r")) == NULL)
		fatal(CANTOPEN);

	/* read font properties */
	while (true) {
		if (fgets(line, STRSIZE, fp) == NULL)
			fatal(EOFINPUT);
		s = get_token(line, token);
		if (!strcmp(token, "CHARS"))
			break;
		else if (!strcmp(token, "FONTBOUNDINGBOX")) {
			sscanf(s, "%d %d %d %d",
			       &fbboxw, &fbboxh, &fbboxl, &fbboxb);
		} else if (!strcmp(token, "PIXEL_SIZE")) {
			sscanf(s, "%d", &size);
		} else if (!strcmp(token, "COPYRIGHT")) {
			if ((copyright = copy_str(s)) == NULL)
				fatal(OUTOFMEM);
		} else if (!strcmp(token, "NOTICE")) {
			if ((notice = copy_str(s)) == NULL)
				fatal(OUTOFMEM);
		} else if (!strcmp(token, "FAMILY_NAME")) {
			if ((family = copy_str(s)) == NULL)
				fatal(OUTOFMEM);
		} else if (!strcmp(token, "WEIGHT_NAME")) {
			if ((weight = copy_str(s)) == NULL)
				fatal(OUTOFMEM);
		}
	}

	if (fbboxw == 0 || fbboxh == 0 || size == 0 || copyright == NULL ||
	    notice == NULL || family == NULL || weight == NULL)
		fatal("missing font properties");

	/* allocate bitmap */
	stride = (fbboxw * NUMCHARS + 1) / 8;
	bitmap = (uint8_t *) calloc(stride * fbboxh, sizeof(uint8_t));
	if (bitmap == NULL)
		fatal(OUTOFMEM);

	/* read font characters and draw them into the bitmap */
	while (true) {
		if (fgets(line, STRSIZE, fp) == NULL)
			fatal(EOFINPUT);
		s = get_token(line, token);
		if (!strcmp(token, "ENDFONT"))
			break;
		else if (!strcmp(token, "ENCODING")) {
			sscanf(s, "%d", &codept);
			/* convert codepoint to font character index */
			for (ch = 0; ch < NUMCHARS; ch++)
				if (codept == codepts[ch])
					break;
		} else if (!strcmp(token, "BBX")) {
			sscanf(s, "%d %d %d %d",
			       &bboxw, &bboxh, &bboxl, &bboxb);
		} else if (!strcmp(token, "BITMAP") && ch < NUMCHARS) {
			off = ch * bboxw;
			p0 = bitmap + (off >> 3);
			m0 = 0x80 >> (off & 7);
			for (j = 0; j < bboxh; j++) {
				m = m0;
				p = p0;
				if ((s = fgets(line, STRSIZE, fp)) == NULL)
					fatal(EOFINPUT);
				mc = c = 0;
				for (i = 0; i < bboxw; i++) {
					if ((mc >>= 1) == 0) {
						/* get next hex char */
						c = *s++;
						c -= (c <= '9' ? '0'
							       : 'A' - 10);
						mc = 0x8;
					}
					if (c & mc)
						*p |= m;
					if ((m >>= 1) == 0) {
						m = 0x80;
						p++;
					}
				}
				p0 += stride;
			}
		}
	}

	fclose(fp);

	/* generate font C code */
	printf("/*\n");
	printf(" * Automatically generated from %s with %s\n",
	       basename(argv[1]), basename(prog));
	printf(" * ASCII subset of %s %s %d\n", family, weight, size);
	printf(" *\n");
	printf(" * %s\n", copyright);
	printf(" *\n");
	printf(" * %s\n", notice);
	printf(" */\n\n");
	printf("static const uint8_t font%d_bits[] = {", size);
	p = bitmap;
	for (i = 0; i < stride * fbboxh; i++) {
		if (i == 0)
			printf("\n\t");
		else {
			printf(",");
			if (i % 12 == 0)
				printf("\n\t");
			else
				printf(" ");
		}
		printf("0x%02x", *p++);
	}
	printf("\n};\n\n");
	printf("static const font_t font%d = {\n", size);
	printf("\t.bits = font%d_bits,\n", size);
	printf("\t.depth = 1,\n");
	printf("\t.width = %d,\n", fbboxw);
	printf("\t.height = %d,\n", fbboxh);
	printf("\t.stride = %d\n", stride);
	printf("};\n");

	exit(EXIT_SUCCESS);
}
