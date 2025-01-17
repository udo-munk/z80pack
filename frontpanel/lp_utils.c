// lp_utils.c	utility functions

/* Copyright (c) 2007-2008, John Kichury
   C conversion and SDL2 support is Copyright (c) 2024-2025, Thomas Eberhardt

   This software is freely distributable free of charge and without license fees with the
   following conditions:

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   JOHN KICHURY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The above copyright notice must be included in any copies of this software.

*/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "lp_utils.h"

#define UNUSED(x) (void) (x)

/* ---------------------
   gtoken - token parser
   ---------------------*/

/* Token parser. Returns string tokens.


  Gtoken scans the string argument <string> starting at the character
  position specified by the argument <pos>. The next token is returned
  in the return argument string <token> and the value of <pos> is updated
  to reflect the current parsing position. The value of the function is
  the integer character length of the returned token.

  Tokens are generated using the following rules:

  Delimiters: Space and Comma.

   A single space terminates a token. A multispace field is equivalent to
   a single space. Leading spaces are ignored.

   A single comma terminates a token and is equivalent to a space. Two or
   more commas in succession generates null tokens.

   Delimiters may be specified in a token by enclosing the token in single
   quotes. Quotes are specified in a token if they are doubled.

   examples: 'this is one token'
             'this token string contains a ''quoted'' word'.
             one, two, three are individual tokens.
*/

int gtoken(char *string,	/* input character string */
	   char *token,		/* return token string */
	   int  maxlen,		/* max token length */
	   int  *pos)		/* starting char position to parse */
{
	int  toklen;		/* token length in characters */
	bool qseen,		/* quote (') seen flag */
	     q2seen,		/* 2nd quote seen flag */
	     done,		/* done with token flag */
	     ignore_space;
	char c;

	q2seen = qseen = done = false;
	ignore_space = true;		/* ignore leading spaces */
	toklen = 0;

	while ((c = string[*pos]) != '\0' && (toklen < maxlen) && !done) {
		(*pos)++;
		switch (c) {
		case '\'':
			ignore_space = false;
			if (!qseen) {
				qseen = true;
				break;
			}
			if (!q2seen) {
				q2seen = true;
				break;
			}
			token[toklen++] = c;
			q2seen = false;
			break;

		case ',':
			ignore_space = false;
			if (qseen && !q2seen)
				token[toklen++] = c;
			else
				done = true;
			break;

		case ' ':
			if (!ignore_space) {
				if (qseen && !q2seen)
					token[toklen++] = c;
				else
					done = true;
			}
			break;

		default:
			ignore_space = false;
			if (q2seen) {
				done = true;
				break;
			}
			token[toklen++] = c;
			break;

		} /* end switch */
	} /* end while */

	token[toklen] = '\0';	/* mark end of token */
	return toklen;

} /* end of gtoken function */

/* freadlin    read line from file */

/* Read an ascii line from specified file (carret,lf or null signals end
   of line)

   Returns line in "buffer" up to specified length terminated with a NULL.

   Value of function: 0=failure 1=success.

   usage: int  freadlin(), i;
          char buffer[];
          FILE *funit;
          i = freadlin(funit, buffer, bufsize);
*/

#define CARRET	'\015'	/* carriage return */
#define LF	'\012'	/* line feed */
#define EOL	'\0'	/* end of line ("C" standard) */

int freadlin(FILE *funit, char *buffer, int bufsize)
{
	int i;
	char c;

	i = 0;

	c = getc(funit);

	if (feof(funit))
		return 0;

	while (c != EOL && c != CARRET && c != LF) {
		if (i < bufsize)
			buffer[i++] = c;
		c = getc(funit);
		if (feof(funit))
			return 0;
	}
	buffer[i] = EOL;
	return 1;
} /* end freadlin() */

/* xpand */

enum xpnd_states { XPN_START, XPN_PREFIX, XPN_FRMVAL, XPN_TOVAL, XPN_INC, XPN_SUFFIX };

int xpand(const char *s, char **namelist[])
{
	const char *cp = s,
		*cp1 = NULL;
	char	c,
		*prefix = NULL,
		*suffix = NULL,
		*from	= NULL,
		*to	= NULL,
		*inc	= NULL;
	int	n,
		// error	= 0,
		state 	= XPN_START;

	int	ival,
		from_ival	= 0,
		from_digits	= 0,
		to_ival		= 0,
		to_digits	= 0,
		inc_ival	= 1,
		ndigits		= 0;

	char	format[10],
		dbuf[10],
		obuf[100];

	int	max_names,	// computed number of expanded names
		num_names = 0;
	char	**new_namelist;

	while ((c = *cp)) {

		switch (state) {
		case XPN_START:
			if (c == '{') {
				state = XPN_FRMVAL;
				cp1 = cp + 1;
			} else {
				cp1 = cp;
				state = XPN_PREFIX;
			}
			cp++;
			break;

		case XPN_PREFIX:
			if (c == '{') {
				state = XPN_FRMVAL;
				n = cp - cp1;
				if (n > 0) {
					prefix = (char *) malloc(n + 1);
					strncpy(prefix, s, n);
					prefix[n] = 0;
					cp1 = cp + 1;
				}
			}
			cp++;
			break;

		case XPN_FRMVAL:
			if (c == '-') {
				state = XPN_TOVAL;
				n = cp - cp1;
				if (n > 0) {
					from = (char *) malloc(n + 1);
					strncpy(from, cp1, n);
					from[n] = 0;
					cp1 = cp + 1;
				}
			} else if (c == '}') {
				state = XPN_SUFFIX;
				n = cp - cp1;
				if (n > 0) {
					from = (char *) malloc(n + 1);
					strncpy(from, cp1, n);
					from[n] = 0;
				}
				cp1 = cp + 1;
			} else if (c == '+') {
				// error = 1;
			}
			cp++;
			break;

		case XPN_TOVAL:
			if (c == '+') {
				state = XPN_INC;
				n = cp - cp1;
				if (n > 0) {
					to = (char *) malloc(n + 1);
					strncpy(to, cp1, n);
					to[n] = 0;
					cp1 = cp + 1;
				}
			} else if (c == '}') {
				state = XPN_SUFFIX;
				n = cp - cp1;
				if (n > 0) {
					to = (char *) malloc(n + 1);
					strncpy(to, cp1, n);
					to[n] = 0;
				}
				cp1 = cp + 1;
			}
			cp++;
			break;

		case XPN_INC:
			if (c == '}') {
				state = XPN_SUFFIX;
				n = cp - cp1;
				if (n > 0) {
					inc = (char *) malloc(n + 1);
					strncpy(inc, cp1, n);
					inc[n] = 0;
					cp1 = cp + 1;
				}
			}
			cp++;
			break;

		case XPN_SUFFIX:
			cp++;
			break;
		} /* end switch */

	} /* end while */

	switch (state) {
	case XPN_START:
	case XPN_PREFIX:
		n = cp - cp1;
		if (n > 0) {
			prefix = (char *) malloc(n + 1);
			strncpy(prefix, s, n);
			prefix[n] = 0;
		}
		break;
	case XPN_FRMVAL:
		n = cp - cp1;
		if (n > 0) {
			from = (char *) malloc(n + 1);
			strncpy(from, cp1, n);
			from[n] = 0;
		}
		break;
	case XPN_TOVAL:
		n = cp - cp1;
		if (n > 0) {
			to = (char *) malloc(n + 1);
			strncpy(to, cp1, n);
			to[n] = 0;
		}
		break;
	case XPN_INC:
		n = cp - cp1;
		if (n > 0) {
			inc = (char *) malloc(n + 1);
			strncpy(inc, cp1, n);
			inc[n] = 0;
		}
		break;
	case XPN_SUFFIX:
		n = cp - cp1;
		if (n > 0) {
			suffix = (char *) malloc(n + 1);
			strncpy(suffix, cp1, n);
			suffix[n] = 0;
		}
		break;
	} /* end switch(state) */

	/* set up to iterate */

	if (from && to) {
		from_ival = atoi(from);
		to_ival = atoi(to);
		if (to_ival != from_ival) {
			if (inc)
				inc_ival = atoi(inc);
			if (to_ival < from_ival)
				inc_ival = -inc_ival;

			from_digits = strlen(from);
			to_digits = strlen(to);
			ndigits = max(from_digits, to_digits);

			ival = from_ival;

			snprintf(format, sizeof(format), "%%0%dd", ndigits);

			max_names = ((max(ival, to_ival)) - (min(ival, to_ival))) /
				    abs(inc_ival) + 1;

			new_namelist = (char **) malloc(sizeof(char *) * max_names);
			*namelist = new_namelist;
			num_names = 0;

			do {
				obuf[0] = 0;
				snprintf(dbuf, sizeof(dbuf), format, ival);
				if (prefix)
					strcpy(obuf, prefix);
				strcat(obuf, dbuf);
				if (suffix)
					strcat(obuf, suffix);

				new_namelist[num_names] = (char *) malloc(strlen(obuf) + 1);
				strcpy(new_namelist[num_names], obuf);
				num_names++;
				ival += inc_ival;
			} while (ival != to_ival);

			snprintf(dbuf, sizeof(dbuf), format, ival);
			obuf[0] = 0;
			if (prefix)
				strcpy(obuf, prefix);
			strcat(obuf, dbuf);
			if (suffix)
				strcat(obuf, suffix);
			new_namelist[num_names] = (char *) malloc(strlen(obuf) + 1);
			strcpy(new_namelist[num_names], obuf);
			num_names++;
		}
	} else {
		obuf[0] = 0;
		if (prefix)
			strcpy(obuf, prefix);
		if (from)
			strcat(obuf, from);
		if (suffix)
			strcat(obuf, suffix);

		new_namelist = (char **) malloc(sizeof(char *));
		*namelist = new_namelist;
		num_names = 0;

		new_namelist[num_names] = (char *) malloc(strlen(obuf) + 1);
		strcpy(new_namelist[num_names], obuf);
		num_names++;
	}

	if (prefix)
		free(prefix);
	if (from)
		free(from);
	if (to)
		free(to);
	if (inc)
		free(inc);
	if (suffix)
		free(suffix);

	return num_names;

} /* end xpand() */

/* framerate control */

/*
 usage:

    init:
	framerate_set(30.0);
	framerate_start();

    draw()
    {
	...

	glFinish();
	framerate_wait();
	swapbuffers();
	framerate_start();
    }
*/

typedef struct frate {
	double	fps,
		frametime,
		start,
		et;
} frate_t;

static frate_t frate_parms = { 30.0, 1.0 / 30., 0., 0. };

double frate_gettime(void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);
	return (double) tp.tv_sec + (double) tp.tv_usec / 1000000.0;
}

/* funcs */

void framerate_set(double r)
{
	frate_parms.fps = r;
	frate_parms.frametime = (float) 1.0 / frate_parms.fps;
}

void framerate_start_frame(void)
{
	frate_parms.start = frate_gettime();
}

void framerate_wait(void)
{
	struct timespec ts, rem;
	double delta;
	double t;

	t = frate_gettime();

	frate_parms.et = t - frate_parms.start;

	delta = frate_parms.frametime - frate_parms.et;

	// if (delta < 0.)
	// 	printf("missed rendering frame\n");

	if (delta > 0.0) {
		delta = delta * 10e8;
		ts.tv_sec = 0;
		ts.tv_nsec = (long) delta;

		for (;;)
			if (nanosleep(&ts, &rem) == -1 && errno == EINTR && rem.tv_nsec > 0L) {
				ts = rem;
				continue;
			} else
				break;
	}
}

// parser class

static const char *parser_errmsgs[] = {
	"No error.",
	"No variable",
	"No value.",
	"Bad Varable.",
	"Bad value.",
	"Too many values.",
	"Too few values."
};

Parser_t *Parser_new(void)
{
	Parser_t *p = (Parser_t *) calloc(1, sizeof(Parser_t));

	if (p)
		Parser_init(p);

	return p;
}

void Parser_delete(Parser_t *p)
{
	if (p) {
		Parser_fini(p);
		free(p);
	}
}

void Parser_init(Parser_t *p)
{
	parser_result_t *resp = &p->results;

	p->curr_arg = 0;
	p->eol = 0;
	resp->max_args = 0;
	resp->strings = NULL;
	resp->stringlengths = NULL;
	resp->floats = NULL;
	resp->ints = NULL;
}

void Parser_fini(Parser_t *p)
{
	parser_result_t *resp = &p->results;
	int i;

	if (resp->strings) {
		for (i = 0; i < resp->max_args; i++)
			if (resp->strings[i])
				free(resp->strings[i]);
	}

	if (resp->strings)
		free(resp->strings);
	if (resp->stringlengths)
		free(resp->stringlengths);
	if (resp->floats)
		free(resp->floats);
	if (resp->ints)
		free(resp->ints);
}

int Parser_addArg(Parser_t *p, const char *s, const char *cpos1, const char *cpos2)
{
	parser_result_t *resp = &p->results;
	int len;
	float fval;
	char *endptr;

	UNUSED(s);

	if (resp->num_args + 1 > resp->max_args)
		Parser_growArgs(p, 4);

	if ((len = cpos2 - cpos1) > 0) {

		if (resp->stringlengths[resp->num_args] < len + 1) {
			if (resp->strings[resp->num_args]) {
				free(resp->strings[resp->num_args]);
				resp->strings[resp->num_args] = NULL;
			}
		}
		if (resp->strings[resp->num_args] == NULL)
			resp->strings[resp->num_args] = (char *) malloc(len + 1);
		resp->stringlengths[resp->num_args] = len + 1;
		strncpy(resp->strings[resp->num_args], cpos1, len);
		resp->strings[resp->num_args][len] = 0;
		p->curr_arg = resp->num_args;

		// convert the string value to int/float if rules dictate it

		switch (p->rules[p->curr_rule].valtype) {
		case PARSER_STRING:	// no conversion, already a string
			break;

		case PARSER_INT:

			fval = strtof(resp->strings[resp->num_args], &endptr);

			if (endptr == resp->strings[resp->num_args]) {
				p->error = PARSER_ERR_BAD_VAL;
				return cpos1 - p->buff;
			}

			resp->ints[resp->num_args] = (int) fval;
			break;

		case PARSER_FLOAT:

			fval = strtof(resp->strings[resp->num_args], &endptr);
			resp->floats[resp->num_args] = fval;
			if (endptr == resp->strings[resp->num_args]) {
				p->error = PARSER_ERR_BAD_VAL;
				return cpos1 - p->buff;
			}
			break;
		}

		resp->num_args++;
	}
	return -1;
}

int Parser_findCmd(Parser_t *p, const char *s)
{
	int i = 0;

	while (p->rules[i].cmd) {
		if (!strcmp(s, p->rules[i].cmd))
			return i;
		i++;
	}

	return -1; // not found
}

void Parser_growArgs(Parser_t *p, int n)
{
	parser_result_t *resp = &p->results;
	char **new_strings;
	float *new_floats;
	int   *new_ints;
	int   *new_stringlengths;
	int i;

	new_strings = (char **) realloc(resp->strings, sizeof(char *) * (resp->max_args + n));
	new_stringlengths = (int *) realloc(resp->stringlengths, sizeof(int) *
					    (resp->max_args + n));
	new_floats  = (float *) realloc(resp->floats, sizeof(float) * (resp->max_args + n));
	new_ints    = (int *) realloc(resp->ints, sizeof(int) * (resp->max_args + n));

	resp->max_args += n;

	for (i = resp->num_args; i < resp->max_args; i++) {
		new_strings[i] = NULL;
		new_stringlengths[i] = 0;
	}

	resp->strings = new_strings;
	resp->stringlengths = new_stringlengths;
	resp->floats = new_floats;
	resp->ints = new_ints;
}

void Parser_setRules(Parser_t *p, parser_rules_t *rules)
{
	p->rules = rules;
}

void Parser_printError(Parser_t *p)
{
	fprintf(stderr, "%s\n", parser_errmsgs[p->error]);
}

void Parser_printRules(Parser_t *p)
{
	int i = 0;

	while (p->rules[i].cmd) {
		printf("%s, %d, %d, %d\n",
		       p->rules[i].cmd, p->rules[i].minvals,
		       p->rules[i].maxvals, p->rules[i].valtype);
		i++;
	}
}

void Parser_setParseString(Parser_t *p, const char *s)
{
	p->curr_arg = 0;
	p->eol = 0;
	p->cp = p->cp1 = p->cp2 = p->buff = s;
	p->state = PARSER_GETVAR;
}

int Parser_parse(Parser_t *p, parser_result_t **returned_result)
{
	parser_result_t *resp = &p->results;
	int n,
	    leadspace = 1;

	resp->num_args = 0;
	*returned_result = resp;
	p->error = PARSER_ERR_NONE;

	// while ((p->c = *p->cp))
	while (!p->eol) {
		p->c = *p->cp;
		if (p->c == 0) {
			p->eol = 1;
		}
		switch (p->state) {
		case PARSER_GETVAR:

			if (leadspace) {
				if (p->eol)
					return PARSER_DONE;
				if (p->c == ' ' || p->c == '\t') {
					p->cp1 = p->cp + 1;
					break;
				} else
					leadspace = 0;
			}

			if (p->eol || p->c == '=') {
				if ((n = (p->cp - p->cp1)) > 0) {
					char *var;

					var = (char *) malloc(n + 1);
					strncpy(var, p->cp1, n);
					var[n] = 0;
					p->curr_rule = resp->cmd_idx = Parser_findCmd(p, var);
					free(var);
					if (p->curr_rule < 0) {
						p->error = PARSER_ERR_VAR_NOT_FOUND;
						return p->cp1 - p->buff;
					}

					resp->var_pos = p->cp1 - p->buff;

					if (p->eol) {
						if (resp->num_args <
						    p->rules[p->curr_rule].minvals) {
							p->error = PARSER_ERR_TOO_FEW_VALS;
							return p->cp1 - p->buff;
						}
						return PARSER_OK;
					}

					p->state = PARSER_GETVAL;
				} else {
					// null var
					// return error pointing to position of bad var/val
					p->error = PARSER_ERR_NO_VAR;
					return p->cp1 - p->buff;
				}
				p->cp1 = p->cp + 1;
			} else if (p->eol || p->c == ' ' || p->c == '\t') {
				if ((n = (p->cp - p->cp1)) > 0) {
					char *var;

					var = (char *) malloc(n + 1);
					strncpy(var, p->cp1, n);
					var[n] = 0;
					p->curr_rule = resp->cmd_idx = Parser_findCmd(p, var);
					free(var);
					if (p->curr_rule < 0) {
						p->error = PARSER_ERR_VAR_NOT_FOUND;
						return p->cp1 - p->buff;
					}

					// leadspace = 1;
					if (resp->num_args < p->rules[p->curr_rule].minvals) {
						p->error = PARSER_ERR_TOO_FEW_VALS;
						return p->cp1 - p->buff;
					}
					return PARSER_OK;
				}
			} else if (p->c == ',') {
				// error, var with no value, return pointing to char pos
				p->error = PARSER_ERR_NO_VAL;
				return p->cp - p->buff;
			}
			break;

		case PARSER_GETVAL:
			if (p->c == ',') {
				// got an argument
				if (resp->num_args + 1 > p->rules[p->curr_rule].maxvals) {
					p->error = PARSER_ERR_TOO_MANY_VALS;
					return p->cp1 - p->buff;
				}

				// copy value to string and convert to datatype
				// dictated by the rule

				n = Parser_addArg(p, p->buff, p->cp1, p->cp);

				if (n >= 0)
					return n;
				p->cp1 = p->cp + 1;
			} else if (p->eol || p->c == ' ' || p->c == '\t') {
				// got an argument
				if (resp->num_args + 1 > p->rules[p->curr_rule].maxvals) {
					p->error = PARSER_ERR_TOO_MANY_VALS;
					return p->cp1 - p->buff;
				}

				// copy value to string and convert to datatype
				// dictated by the rule

				n = Parser_addArg(p, p->buff, p->cp1, p->cp);

				if (n >= 0)
					return n;
				p->cp1 = p->cp + 1;

				p->state = PARSER_GETVAR;
				// leadspace = 1;
				p->cp1 = p->cp + 1;
				p->cp2 = p->cp;
				return PARSER_OK;
			}
			break;
		} // end switch(p->state)

		p->cp++;
	}
	return PARSER_DONE;
} // end Parser_parse()
