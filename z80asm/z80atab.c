/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	symbol table module
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "z80asm.h"
#include "z80alst.h"
#include "z80atab.h"

static int hash(const char *name);
static int namecmp(const void *p1, const void *p2);
static int valcmp(const void *p1, const void *p2);

static sym_t *symtab[HASHSIZE];		/* symbol table */
static int symcnt;			/* number of symbols defined */
static sym_t **symarray;		/* sorted symbol table */
static int symsort;			/* sort mode for iterator */
static int symidx;			/* hash table index for iterator */
static sym_t *symptr;			/* symbol pointer for iterator */
static int symmax;			/* max. symbol name length observed */
static WORD last_symval;		/* value of last used symbol */

/*
 *	hash search for sym_name in symbol table symtab
 *	returns pointer to table element, or NULL if not found
 */
sym_t *look_sym(const char *sym_name)
{
	register sym_t *sp;

	for (sp = symtab[hash(sym_name)]; sp != NULL; sp = sp->sym_next)
		if (strcmp(sym_name, sp->sym_name) == 0) {
			last_symval = sp->sym_val;
			return sp;
		}
	return NULL;
}

/*
 *	hash search for sym_name in symbol table symtab
 *	set refflg when found
 *	returns pointer to table element, or NULL if not found
 */
sym_t *get_sym(const char *sym_name)
{
	register sym_t *sp;

	if ((sp = look_sym(sym_name)) != NULL)
		sp->sym_refflg = TRUE;
	return sp;
}

/*
 *	get value of last used symbol
 */
WORD sym_lastval(void)
{
	return last_symval;
}

/*
 *	add symbol sym_name with value sym_val to symbol table symtab
 */
void new_sym(const char *sym_name, WORD sym_val)
{
	register sym_t *sp;
	register int n;
	register int hashval;

	n = strlen(sym_name);
	sp = (sym_t *) malloc(sizeof(sym_t));
	if (sp == NULL || (sp->sym_name = (char *) malloc(n + 1)) == NULL)
		fatal(F_OUTMEM, "symbols");
	strcpy(sp->sym_name, sym_name);
	sp->sym_val = last_symval = sym_val;
	hashval = hash(sym_name);
	sp->sym_next = symtab[hashval];
	symtab[hashval] = sp;
	sp->sym_refflg = FALSE;
	if (n > symmax)
		symmax = n;
	symcnt++;
}

/*
 *	add symbol sym_name with value sym_val to symbol table symtab,
 *	or modify existing symbol with new value and set refflg
 */
void put_sym(const char *sym_name, WORD sym_val)
{
	register sym_t *sp;

	if ((sp = get_sym(sym_name)) == NULL)
		new_sym(sym_name, sym_val);
	else
		sp->sym_val = last_symval = sym_val;
}

/*
 *	add label to symbol table, error if symbol already exists
 *	and differs in value
 */
void put_label(const char *label, WORD addr, int pass)
{
	register sym_t *sp;

	if ((sp = look_sym(label)) == NULL)
		new_sym(label, addr);
	else if (sp->sym_val != addr)
		asmerr(pass == 1 ? E_MULSYM : E_LBLDIF);
}

/*
 *	calculate the hash value of the string name
 *	returns hash value
 */
static int hash(const char *name)
{
	register unsigned h;

	for (h = 0; *name != '\0';)
		h = ((h << 5) | (h >> (sizeof(h) * 8 - 5))) ^ (BYTE) *name++;
	return h % HASHSIZE;
}

/*
 *	return maximum symbol name length observed
 */
int get_symmax(void)
{
	return symmax;
}

/*
 *	get first symbol for listing, sorted as specified in sort_mode
 */
sym_t *first_sym(int sort_mode)
{
	register sym_t *sp;
	register int j;
	register int i;

	if (symcnt == 0)
		return NULL;
	symsort = sort_mode;
	switch (sort_mode) {
	case SYM_UNSORT:
		symidx = 0;
		while ((symptr = symtab[symidx]) == NULL)
			if (++symidx == HASHSIZE)
				break;
		return symptr;
	case SYM_SORTN:
	case SYM_SORTA:
		symarray = (sym_t **) malloc(sizeof(sym_t *) * symcnt);
		if (symarray == NULL)
			fatal(F_OUTMEM, "sorting symbol table");
		for (i = 0, j = 0; i < HASHSIZE; i++)
			for (sp = symtab[i]; sp != NULL; sp = sp->sym_next)
				symarray[j++] = sp;
		qsort(symarray, symcnt, sizeof(sym_t *),
		      sort_mode == SYM_SORTN ? namecmp : valcmp);
		symidx = 0;
		return symarray[symidx];
	default:
		fatal(F_INTERN, "unknown sort mode in first_sym");
		break;
	}
}

/*
 *	get next symbol for listing
 */
sym_t *next_sym(void)
{
	if (symsort == SYM_UNSORT) {
		if ((symptr = symptr->sym_next) == NULL) {
			do {
				if (++symidx == HASHSIZE)
					break;
				else
					symptr = symtab[symidx];
			} while (symptr == NULL);
		}
		return symptr;
	} else if (++symidx < symcnt)
		return symarray[symidx];
	return NULL;
}

/*
 *	compares two symbol names for qsort()
 */
static int namecmp(const void *p1, const void *p2)
{
	return strcmp((*(const sym_t **) p1)->sym_name,
		      (*(const sym_t **) p2)->sym_name);
}

/*
 *	compares two symbol values for qsort(), result like strcmp()
 *	if equal compares symbol names
 */
static int valcmp(const void *p1, const void *p2)
{
	register WORD n1, n2;

	n1 = (*(const sym_t **) p1)->sym_val;
	n2 = (*(const sym_t **) p2)->sym_val;
	if (n1 < n2)
		return -1;
	else if (n1 > n2)
		return 1;
	else
		return namecmp(p1, p2);
}
