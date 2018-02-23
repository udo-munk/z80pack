// lp_utils.h

/* Copyright (c) 2007-2008, John Kichury

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

#ifndef max
#define max(a,b) (a) > (b) ? (a) : (b)
#endif
#ifndef min
#define min(a,b) (a) < (b) ? (a) : (b)
#endif



int gtoken(char *string,char *token,int maxlen,int *pos);
void case_a(int key,char *string);
int prefix(char *str1,char *str2);
int freadlin(FILE *funit,char *buffer,int bufsize);
int strindex(char s[], char t[]);

int GetPowerOf2i(int n);
int xpand(char *s, char ***namelist);

int check_glerror(void);


/* framerate.h */
double frate_gettime(void);
void framerate_set(double r);
void framerate_start_frame(void);
void framerate_wait(void);





// arg_parser.h

#ifndef _PARSER_DEFS
#define _PARSER_DEFS


enum parser_rules_enums { PARSER_STRING, PARSER_INT, PARSER_FLOAT };
enum parser_state_enums { PARSER_GETVAR, PARSER_GETVAL };

enum parser_error_nums_enums 
 { 
  PARSER_ERR_NONE, 
  PARSER_ERR_NO_VAR,
  PARSER_ERR_NO_VAL,
  PARSER_ERR_VAR_NOT_FOUND,
  PARSER_ERR_BAD_VAL,
  PARSER_ERR_TOO_MANY_VALS,
  PARSER_ERR_TOO_FEW_VALS 
 };

#define PARSER_DONE	  -2
#define PARSER_OK	  -1


typedef struct
{
 const char *cmd;
 int  minvals,
      maxvals,
      valtype;
} parser_rules_t;

typedef struct
{
 int	cmd_idx,		// command index within rules of found command string
	var_pos,		// position in parse string where the variable was found
 	num_args,		// number of arguments found for current variable 
 	max_args,		// max args that can exist for current var
 	*stringlengths;		// lengths of each argument in string form
 char 	**strings;		// array of text values for each argument
 float 	*floats;
 int   	*ints;  

} parser_result_t;


class Parser
{
 private:
   parser_rules_t *rules;
   int state, error, eol;
   char c; 
   const char *cp, *cp1, *cp2, *buff;
   int curr_rule,
       curr_arg;
   int addArg(const char *s, const char *cpos1, const char *cpos2);
   int findCmd(char *s);
   void growArgs(int n);

 public:
   Parser();
   ~Parser();

 parser_result_t results;

 void setParseString(const char *s);
 void setRules(parser_rules_t *_rules);
 void printError(void);
 void printRules(void);
 int  parse(parser_result_t **returned_result);
 int  parse();                          // continue with old parse

};


#endif

