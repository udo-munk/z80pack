// lp_utils.h

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

#ifndef _LP_UTILS_DEFS
#define _LP_UTILS_DEFS

#include <stdio.h>

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

extern int	gtoken(char *string, char *token, int maxlen, int *pos);
extern int	freadlin(FILE *funit, char *buffer, int bufsize);

extern int	xpand(const char *s, char ***namelist);

extern double	frate_gettime(void);
extern void	framerate_set(double r);
extern void	framerate_start_frame(void);
extern void	framerate_wait(void);

enum parser_rules_enums { PARSER_STRING, PARSER_INT, PARSER_FLOAT };
enum parser_state_enums { PARSER_GETVAR, PARSER_GETVAL };

enum parser_error_nums_enums {
	PARSER_ERR_NONE,
	PARSER_ERR_NO_VAR,
	PARSER_ERR_NO_VAL,
	PARSER_ERR_VAR_NOT_FOUND,
	PARSER_ERR_BAD_VAL,
	PARSER_ERR_TOO_MANY_VALS,
	PARSER_ERR_TOO_FEW_VALS
};

#define PARSER_DONE	-2
#define PARSER_OK	-1

typedef struct parser_rules {
	const char	*cmd;
	int		minvals,
			maxvals,
			valtype;
} parser_rules_t;

typedef struct parser_result {
	int		cmd_idx,	// command index within rules of found command string
			var_pos,	// position in parse string where the variable was found
			num_args,	// number of arguments found for current variable
			max_args,	// max args that can exist for current var
			*stringlengths;	// lengths of each argument in string form
	char 		**strings;	// array of text values for each argument
	float 		*floats;
	int		*ints;

} parser_result_t;

typedef struct Parser {
	// private variables
	parser_rules_t	*rules;
	int		state, error, eol;
	char		c;
	const char	*cp, *cp1, *cp2, *buff;
	int		curr_rule,
			curr_arg;

	// public variables
	parser_result_t	results;
} Parser_t;

extern Parser_t	*Parser_new(void);
extern void	Parser_delete(Parser_t *p);
extern void	Parser_init(Parser_t *p);
extern void	Parser_fini(Parser_t *p);

/* private functions */
extern int	Parser_addArg(Parser_t *p, const char *s, const char *cpos1, const char *cpos2);
extern int	Parser_findCmd(Parser_t *p, const char *s);
extern void	Parser_growArgs(Parser_t *p, int n);

/* public functions */
extern void	Parser_setParseString(Parser_t *p, const char *s);
extern void	Parser_setRules(Parser_t *p, parser_rules_t *rules);
extern void	Parser_printError(Parser_t *p);
extern void	Parser_printRules(Parser_t *p);
extern int	Parser_parse(Parser_t *p, parser_result_t **returned_result);

#endif /* !_LP_UTILS_DEFS */
