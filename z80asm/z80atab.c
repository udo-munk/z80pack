/*
 *	Z80 - Macro - Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022 by Thomas Eberhardt
 *
 *	History:
 *	17-SEP-1987 Development under Digital Research CP/M 2.2
 *	28-JUN-1988 Switched to Unix System V.3
 *	22-OCT-2006 changed to ANSI C for modern POSIX OS's
 *	03-FEB-2007 more ANSI C conformance and reduced compiler warnings
 *	18-MAR-2007 use default output file extension dependent on format
 *	04-OCT-2008 fixed comment bug, ';' string argument now working
 *	22-FEB-2014 fixed is...() compiler warnings
 *	13-JAN-2016 fixed buffer overflow, new expression parser from Didier
 *	02-OCT-2017 bug fixes in expression parser from Didier
 *	28-OCT-2017 added variable symbol length and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 *	04-OCT-2022 new expression parser (TE)
 *	25-OCT-2022 Intel-like macros (TE)
 */

/*
 *	module with table operations on opcode and symbol tables
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "z80a.h"
#include "z80aglb.h"

struct sym *look_sym(char *);
struct sym *get_sym(char *);
struct sym *new_sym(char *);
int hash(char *);
char *strsave(char *);
int namecmp(const void *, const void *);
int valcmp(const void *, const void *);

/* z80amain.c */
extern void fatal(int, const char *);

/* z80aout.c */
extern void asmerr(int);

/*
 *	binary search in sorted table opctab of opset and OPSET_PSD
 *	search opset first since pseudo ops occur less often
 *
 *	Input: pointer to string with opcode
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct opc *search_op(char *op_name)
{
	register int cond, i;
	register struct opc *low, *high, *mid;
	register struct opset *p;

	for (i = opset; i >= 0; i = (i == opset ? OPSET_PSD : -1)) {
		p = &opsettab[i];
		low = &p->opctab[0];
		high = &p->opctab[p->no_opcodes - 1];
		while (low <= high) {
			mid = low + (high - low) / 2;
			if ((cond = strcmp(op_name, mid->op_name)) < 0)
				high = mid - 1;
			else if (cond > 0)
				low = mid + 1;
			else if (!undoc_flag && (mid->op_flags & OP_UNDOC))
				return(NULL);
			else
				return(mid);
		}
	}
	return(NULL);
}

/*
 *	binary search in sorted table opetab of opset
 *
 *	Input: pointer to string with operand
 *
 *	Output: symbol for operand, NOOPERA if empty operand,
 *		NOREG if operand not found
 */
BYTE get_reg(char *s)
{
	register int cond;
	register struct ope *low, *high, *mid;
	register struct opset *p;

	if (s == NULL || *s == '\0')
		return(NOOPERA);
	p = &opsettab[opset];
	low = &p->opetab[0];
	high = &p->opetab[p->no_operands - 1];
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(s, mid->ope_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else if (!undoc_flag && (mid->ope_flags & OPE_UNDOC))
			return(NOREG);
		else
			return(mid->ope_sym);
	}
	return(NOREG);
}

/*
 *	hash search on symbol table symtab
 *
 *	Input: pointer to string with symbol
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct sym *look_sym(char *sym_name)
{
	register struct sym *sp;

	for (sp = symtab[hash(sym_name)]; sp != NULL; sp = sp->sym_next)
		if (strcmp(sym_name, sp->sym_name) == 0)
			return(sp);
	return(NULL);
}

/*
 *	hash search on symbol table symtab, increase refcnt if found
 *
 *	Input: pointer to string with symbol
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct sym *get_sym(char *sym_name)
{
	register struct sym *sp;

	if ((sp = look_sym(sym_name)) != NULL) {
		sp->sym_refcnt++;
		return(sp);
	}
	return(NULL);
}

/*
 *	add symbol to symbol table symtab
 *
 *	Input: sym_name pointer to string with symbol name
 *	       sym_val  value of symbol
 *
 *	Output: pointer to table element
 */
struct sym *new_sym(char *sym_name)
{
	register int hashval, n;
	register struct sym *sp;

	sp = (struct sym *) malloc(sizeof (struct sym));
	if (sp == NULL || ((sp->sym_name = strsave(sym_name)) == NULL))
		fatal(F_OUTMEM, "symbols");
	hashval = hash(sym_name);
	sp->sym_next = symtab[hashval];
	symtab[hashval] = sp;
	sp->sym_refcnt = 0;
	n = strlen(sym_name);
	if (n > symmax)
		symmax = n;
	return(sp);
}

/*
 *	add symbol to symbol table symtab, or modify existing symbol
 *	and increase refcnt
 *
 *	Input: sym_name pointer to string with symbol name
 *	       sym_val  value of symbol
 */
void put_sym(char *sym_name, int sym_val)
{
	register struct sym *sp;

	if ((sp = get_sym(sym_name)) == NULL)
		sp = new_sym(sym_name);
	sp->sym_val = sym_val;
}

/*
 *	add label to symbol table, error if symbol already exists
 *	and differs in value
 */
void put_label(void)
{
	register struct sym *sp;

	if ((sp = look_sym(label)) == NULL)
		new_sym(label)->sym_val = pc;
	else if (sp->sym_val != pc)
		asmerr(E_LBLDIF);
}

/*
 *	hash algorithm
 *
 *	Input: pointer to string with name
 *
 *	Output: hash value
 */
int hash(char *name)
{
	register int hashval;

	for (hashval = 0; *name;)
		hashval += *name++;
	return(hashval % HASHSIZE);
}

/*
 *	save string into allocated memory
 *
 *	Input: pointer to string
 *
 *	Output: pointer to allocated memory with string
 */
char *strsave(char *s)
{
	register char *p;

	if ((p = (char *) malloc((unsigned) strlen(s) + 1)) != NULL)
		strcpy(p, s);
	return(p);
}

/*
 *	copy whole symbol hash table into allocated pointer array
 *	used for sorting the symbol table later
 */
int copy_sym(void)
{
	register int i, j;
	register struct sym *np;

	symarray = (struct sym **) malloc(sizeof(struct sym *) * SYMINC);
	if (symarray == NULL)
		fatal(F_OUTMEM, "sorting symbol table");
	symsize = SYMINC;
	for (i = 0, j = 0; i < HASHSIZE; i++) {
		if (symtab[i] != NULL) {
			for (np = symtab[i]; np != NULL; np = np->sym_next) {
				symarray[j++] = np;
				if (j == symsize) {
					symarray = (struct sym **)
						realloc(symarray,
							sizeof(struct sym *)
							* (symsize + SYMINC));
					if (symarray == NULL)
						fatal(F_OUTMEM,
						      "sorting symbol table");
					symsize += SYMINC;
				}
			}
		}
	}
	return(j);
}

/*
 *	sort symbol table by name
 */
void n_sort_sym(int len)
{
	qsort(symarray, len, sizeof(struct sym *), namecmp);
}

/*
 *	compares two symbol names for qsort()
 */
int namecmp(const void *p1, const void *p2)
{
	return(strcmp((*(const struct sym **) p1)->sym_name,
		      (*(const struct sym **) p2)->sym_name));
}

/*
 *	sort symbol table by address
 */
void a_sort_sym(int len)
{
	qsort(symarray, len, sizeof(struct sym *), valcmp);
}

/*
 *	compares two symbol values for qsort(), result like strcmp()
 *	if equal compares symbol names
 */
int valcmp(const void *p1, const void *p2)
{
	register int n1, n2;

	n1 = (*(const struct sym **) p1)->sym_val;
	n2 = (*(const struct sym **) p2)->sym_val;
	if (n1 < n2)
		return(-1);
	else if (n1 > n2)
		return(1);
	else
		return(namecmp(p1, p2));
}
