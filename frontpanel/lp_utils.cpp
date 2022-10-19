// lp_utils.cpp    utility functions


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

#ifdef __CYGWIN__
#include <windef.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>
#include "lp_utils.h"

#define TRUE  1
#define FALSE 0

#ifndef max
#define max(a,b) (a) > (b) ? (a) : (b)
#endif
#ifndef min
#define min(a,b) (a) < (b) ? (a) : (b)
#endif

#define UNUSED(x) (void)(x)


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
  
   examples:  'this is one token'
              'this token string contains a ''quoted'' word'.
              one, two, three are individual tokens.
*/
  
int
gtoken(char *string,char *token,int maxlen,int *pos)
  
#if 0
char *string, /* input character string */
     *token;  /* return token string    */
  
int  *pos,    /* starting char position to parse  */
      maxlen; /* max token length */
#endif 
{
  
  
 int qseen,    /* quote (') seen flag */
     q2seen,   /* 2nd quote seen flag */
     toklen,   /* token length in characters */
     done,     /* done with token flag */
     ignore_space;
  
 char c;
  
 q2seen = qseen = done = FALSE;
 ignore_space = TRUE;             /* ignore leading spaces */
 toklen = 0;
  
 while((c=string[*pos]) != '\0' && (toklen < maxlen) && !done)
  { (*pos)++;
   switch(c)
   { case '\'':
       ignore_space = FALSE;
       if(!qseen) { qseen = TRUE; break; }
       if(!q2seen) { q2seen = TRUE; break; }
       token[toklen++] = c;
       q2seen = FALSE;
       break;
  
     case ',':
       ignore_space = FALSE;
       if(qseen && !q2seen) token[toklen++] = c;
         else done = TRUE;
       break;
  
     case ' ':
       if(!ignore_space)
        { if(qseen && !q2seen) token[toklen++] = c;
           else done = TRUE;
        }
       break;
  
     default:
       ignore_space = FALSE;
       if(q2seen)
         { done = TRUE;
           break;
         }
       token[toklen++] = c;
       break;
  
   } /* end switch */
  } /* end while */
  
 token[toklen] = '\0';    /* mark end of token */
 return(toklen);
  
} /* end of gtoken function */
  

/* -------------------------------
   case_a       change string case
   -------------------------------

  Converts a character string from upper to lower or lower to upper case.
   key;   0=conv to lower, 1= convert to upper case 
*/


void case_a(int key,char *string)
{ int i;
  char c;

  i=0;
  switch(key)
   { case 0: while((c = string[i]) != '\0') string[i++] = tolower(c);
             break;
     case 1: while((c = string[i]) != '\0') string[i++] = toupper(c);
             break;
     default: break;
   }
  return;
} /* end case_a */

/* prefix.c    test for string prefix */
 
/* Returns true (1) if string2  is a prefix of string1.
 
   usage: int prefix();
          char string1[LEN],string2[LEN];
          if(prefix(string1,string2)) ...
          i = prefix("testing","test")
*/
 
int
prefix(char *str1,char *str2)
{
  int i;
  char c;
 
  i=0;
  while((c=str2[i]) != '\0')
    if(c != str1[i++]) return(0);

  return(1);

}

/* freadlin.c    read line from file */
  
/* Read an ascii line from specified file (carret,lf or null signals end
   of line)
  
   Returns line in "buffer" up to specified length terminated with a NULL.
  
   Value of function: 0=failure 1=success.
  
   usage: int  freadlin(), i;
          char buffer[];
          FILE *funit;
          i=freadlin(funit,buffer,bufsize);
*/
  
/* #include <stdio.h> */
  
#define CARRET '\015'  /* carriage return */
#define LF     '\012'  /* line feed       */
#define EOL    '\0'    /* end of line ("C" standard) */
  
int
freadlin(FILE *funit,char *buffer,int bufsize)
{
  int i;
  char c;
  
  i = 0;
  
  c = getc(funit);
  
  if(feof(funit)) return(0);
  
  while( c != EOL && c != CARRET && c != LF)
   { if(i < bufsize) buffer[i++] = c;
     c = getc(funit);
     if(feof(funit)) return(0);
   }
  buffer[i] = EOL;
  return(1);
  
} /* end freadlin() */



/* strindex: return index of t in s, -1 if none */

int strindex(char s[], char t[])
{
 int i,j,k;

 for(i=0; s[i] != '\0'; i++)
  {
    for (j=i, k=0; t[k] != '\0' && s[j]==t[k]; j++, k++)
	;
    if(k>0 && t[k] == '\0')
	return i;

  }

  return -1;
}


/*  get power of 2 */

int
GetPowerOf2i(int n)
{ int result = 0x2;

 while(result < n) result = result << 1;
 return(result);
}



/* xpand */



enum xpnd_states  { XPN_START, XPN_PREFIX, XPN_FRMVAL, XPN_TOVAL, XPN_INC, XPN_SUFFIX };

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif


//int xpand(const char *s, int (*func)(char *m) )

//int xpand(const char *s, char ***namelist )
int xpand(const char *s, char **namelist[] )
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
 char **new_namelist;

 while ( (c = *cp) )
 {

  switch(state)
  {
	case XPN_START:
		if( c == '{' )
		 {
			state = XPN_FRMVAL;
			cp1 = cp + 1;
		 }
		else
		 {
			cp1 = cp;
			state = XPN_PREFIX;
		 }
		cp++;
		break;

	case XPN_PREFIX:
		if( c == '{' )
		 {
			state = XPN_FRMVAL;
			n = cp - cp1;
			if(n > 0)
			 {
			  //prefix = (char *) malloc(n+1);
			  prefix = new char[n+1];
			  strncpy(prefix, s, n);
			  prefix[n] = 0;
			  cp1 = cp + 1;
			 }
		 }
		cp++;
		break;

	case XPN_FRMVAL:

		if( c == '-' )
		 {
			state = XPN_TOVAL;
			n = cp - cp1;
			if(n > 0)
			 {
			  //from = (char *) malloc(n+1);
			  from = new char[n+1];
			  strncpy(from, cp1, n);
			  from[n] = 0;
			  cp1 = cp + 1;
			 }
		 }
		else if( c == '}' )
		 {
			state = XPN_SUFFIX;
			n = cp - cp1;
			if(n > 0)
			 {
			  //from = (char *) malloc(n+1);
			  from = new char[n+1];
			  strncpy(from, cp1, n);
			  from[n] = 0;
			 }
			  cp1 = cp + 1;
		 }
		else if( c == '+' )
		 {
			// error = 1;
		 }
	
		cp++;
		break;

	case XPN_TOVAL:
		if( c == '+' )
		 {
			state = XPN_INC;
			n = cp - cp1;
			if(n > 0)
			 {
			  //to = (char *) malloc(n+1);
			  to = new char[n+1];
			  strncpy(to, cp1, n);
			  to[n] = 0;
			  cp1 = cp + 1;
			 }
		 }

		else if( c == '}' )
		 {
			state = XPN_SUFFIX;
			n = cp - cp1;
			if(n > 0)
			 {
			  //to = (char *) malloc(n+1);
			  to = new char[n+1];
			  strncpy(to, cp1, n);
			  to[n] = 0;
			 }
			  cp1 = cp + 1;
		 }

		cp++;
		break;

	case XPN_INC:
		if( c == '}' )
		 {
			state = XPN_SUFFIX;
			n = cp - cp1;
			if(n > 0)
			 {
			  //inc = (char *) malloc(n+1);
			  inc = new char[n+1];
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

 switch( state )
 {
	case XPN_START:
	case XPN_PREFIX:
			n = cp - cp1;
			//prefix = (char *) malloc(n+1);
			//prefix = new char[n+1];
			if(n > 0)
			 {
			  //prefix = (char *) malloc(n+1);
			  prefix = new char[n+1];
			  strncpy(prefix, s, n);
			  prefix[n] = 0;
			 }
		break;
	case XPN_FRMVAL:
			n = cp - cp1;
			if(n > 0)
			 {
			  //from = (char *) malloc(n+1);
			  from = new char[n+1];
			  strncpy(from, cp1, n);
			  from[n] = 0;
			 }
		break;
	case XPN_TOVAL:
			n = cp - cp1;
			if(n > 0)
			 {
			  //to = (char *) malloc(n+1);
			  to = new char[n+1];
			  strncpy(to, cp1, n);
			  to[n] = 0;
			 }
		break;
	case XPN_INC:
			n = cp - cp1;
			if(n > 0)
			 {
			  //inc = (char *) malloc(n+1);
			  inc = new char[n+1];
			  strncpy(inc, cp1, n);
			  inc[n] = 0;
			 }
		break;
	case XPN_SUFFIX:
			n = cp - cp1;
			if(n > 0)
			 {
			  //suffix = (char *) malloc(n+1);
			  suffix = new char[n+1];
			  strncpy(suffix, cp1, n);
			  suffix[n] = 0;
			 }
		break;

 } /* end switch(state) */


 /* set up to iterate */

  if(from && to) 
  {
   from_ival = atoi(from);
   to_ival = atoi(to);
   if(to_ival != from_ival)
   {
    if(inc) inc_ival = atoi(inc);
    if( to_ival < from_ival) inc_ival = -inc_ival;

    from_digits = strlen(from);
    to_digits = strlen(to);
    ndigits = max(from_digits, to_digits);

    ival = from_ival;

    sprintf(format, "%%0%dd",ndigits);

   max_names = ( (max(ival, to_ival)) - (min(ival,to_ival))) / abs(inc_ival) + 1;

   new_namelist = new char *[max_names];
   *namelist = &new_namelist[0];
   num_names = 0;

   do
    {
     obuf[0]=0;
     sprintf(dbuf,format,ival);
     if(prefix) strcpy(obuf,prefix);
     strcat(obuf,dbuf);
     if(suffix) strcat(obuf,suffix);

     new_namelist[num_names] = new char[ strlen(obuf)+1];
     strcpy(new_namelist[num_names], obuf);
     num_names++;
     ival += inc_ival;
    } while (ival != to_ival);

     sprintf(dbuf,format,ival);
     obuf[0]=0;
     if(prefix) strcpy(obuf,prefix);
     strcat(obuf,dbuf);
     if(suffix) strcat(obuf,suffix);
     new_namelist[num_names] = new char[ strlen(obuf)+1];
     strcpy(new_namelist[num_names], obuf);
     num_names++;
   }
  }
  else
  {
     obuf[0]=0;
     if(prefix) strcpy(obuf,prefix);
     if(from ) strcat(obuf,from);
     if(suffix) strcat(obuf,suffix);

     new_namelist = new char *[1];
     *namelist = &new_namelist[0];
     num_names = 0;

     new_namelist[num_names] = new char[ strlen(obuf)+1];
     strcpy(new_namelist[num_names], obuf);
     num_names++;
  }

  if(prefix) delete[] prefix;
  if(from)  delete[] from;
  if(to) delete[] to;
  if(inc) delete[] inc;
  if(suffix) delete[] suffix;

  return(num_names);

} /* end xpand() */


int check_glerror(void)
{
int n = glGetError();
 if(n)
  { printf("glError:%d %s\n",n, gluErrorString(n));
  }
 return n;
}


/* framerate control */


/* framerate.c */


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

typedef struct {
    struct timeval              bsdtime;
    float                       dt;
    int                         stopped;
} watch_t;

typedef struct 
{
  double	fps,
		frametime,
  		start,
		et;
} frate_t;

static frate_t frate_parms = {30.0, 1.0/30., 0., 0.};

static watch_t syswatch;

double frate_gettime(void)
{
   struct timeval      tp;
    struct timezone     tzp;
    int                 sec;
    int                 usec;
    watch_t             *t;

    double	secf, usecf, dt;

    t = &syswatch;

    gettimeofday(&tp, &tzp);
    sec = tp.tv_sec - t->bsdtime.tv_sec;
    usec = tp.tv_usec - t->bsdtime.tv_usec;
    if (usec < 0) 
     {
        sec--;
        usec += 1000000;
     }

    secf = (double) sec;
    usecf = (double) usec;
    usecf = usecf / 1000000.0; 
    dt = secf + usecf; 
    return (dt);
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
 unsigned int usec;
 double delta;
 double t;

 t = frate_gettime();

 frate_parms.et = t - frate_parms.start;

 delta =  frate_parms.frametime - frate_parms.et;

// if(delta < 0.) printf("missed rendering frame\n");

 if( delta > 0.0 )
  {
    delta = delta * 10e5;
    usec = (unsigned int) delta;
    usleep(usec);
  }

}


// parser class


// arg_parser.cpp	argument parser class


#include <string.h>
#include <stdlib.h>

static const char *parser_errmsgs[] = {
"No error.",
"No variable",
"No value.",
"Bad Varable.",
"Bad value.",
"Too many values.",
"Too few values."
};



Parser::Parser(void)
{
 results.max_args = 0;
 curr_arg = 0;
 eol = 0;
 results.strings = NULL;
 results.stringlengths = NULL;
 results.floats = NULL;
 results.ints = NULL;
}

Parser::~Parser(void)
{
 int i;

 if(results.strings)
  {
    for(i=0;i<results.max_args;i++)
       if(results.strings[i]) delete[] results.strings[i];
  }

 if(results.strings) delete[] results.strings;
 if(results.stringlengths) delete[] results.stringlengths;
 if(results.floats) delete[] results.floats;
 if(results.ints) delete[] results.ints;
}

int
Parser::addArg(const char *s, const char *cpos1, const char *cpos2)
{
 int len; 
 float fval;
 char *endptr;

 UNUSED(s);

 if(results.num_args + 1 > results.max_args)
   growArgs(4);

 if((len = cpos2 - cpos1) > 0)
  {

    if(results.stringlengths[results.num_args] < len+1)
     {
	if(results.strings[results.num_args]) delete[] results.strings[results.num_args];
     }
    results.strings[results.num_args] = new char[ len + 1];
    results.stringlengths[results.num_args] = len + 1;
    strncpy(results.strings[results.num_args], cpos1,len);
    results.strings[results.num_args][len] = 0;
    curr_arg = results.num_args;


    // convert the string value to int/float if rules dictate it

    switch(rules[curr_rule].valtype)
    {
	case PARSER_STRING:	// no conversion, already a string
		break;

	case PARSER_INT:

		fval = strtof(results.strings[results.num_args],&endptr);

		if(endptr == results.strings[results.num_args])
		 {
		   error = PARSER_ERR_BAD_VAL;
		   return(cpos1 - buff);
		 }

		results.ints[results.num_args] = (int) fval;
		break;

	case PARSER_FLOAT:

		fval = strtof(results.strings[results.num_args],&endptr);
		results.floats[results.num_args] = fval;
		if(endptr == results.strings[results.num_args])
		 {
		   error = PARSER_ERR_BAD_VAL;
		   return(cpos1 - buff);
		 }
		break;
    }

    results.num_args++;
  }
    return -1; 
}

int
Parser::findCmd(char *s)
{
  int i = 0;

  while( rules[i].cmd )
  {
    if(!strcmp(s, rules[i].cmd)) return i;
   i++;
  }

 return -1; // not found
}

void Parser:: growArgs(int n)
{
 char **new_strings;
 float *new_floats;
 int   *new_ints;
 int   *new_stringlengths;
 int i;

 results.strings = results.strings;
 results.floats = results.floats;
 results.ints= results.ints;
 results.stringlengths = results.stringlengths;

 new_strings = new char *[results.max_args + n]; 
 new_stringlengths = new int[results.max_args + n]; 
 new_floats  = new float[results.max_args + n]; 
 new_ints    = new int[results.max_args + n]; 


 for(i=0;i<results.max_args;i++)
 { new_strings[i] = results.strings[i];
   new_stringlengths[i] = results.stringlengths[i];
   new_floats[i] = results.floats[i];
   new_ints[i] = results.ints[i];
 }

 for(i=results.num_args; i<results.max_args + n; i++)
  {
   new_strings[i] = NULL;
   new_stringlengths[i] = 0;
  }

 if(results.strings) delete[] results.strings;
 if(results.stringlengths) delete[] results.stringlengths;
 if(results.floats) delete[] results.floats;
 if(results.ints) delete[] results.ints;
 results.strings = new_strings;
 results.stringlengths = new_stringlengths;
 results.floats = new_floats;
 results.ints = new_ints;

 results.max_args += n;

}

void
Parser::setRules(parser_rules_t *_rules)
{
  rules = _rules;
}

void
Parser::printError(void)
{
 fprintf(stderr,"%s\n", parser_errmsgs[error]);
}

void
Parser::printRules(void)
{
  int i = 0;

  while( rules[i].cmd )
  {
   printf("%s, %d, %d, %d\n",
       rules[i].cmd, rules[i].minvals, rules[i].maxvals,  rules[i].valtype);
   i++;
  }

}

void
Parser::setParseString(const char *s)
{
  curr_arg = 0; 
  eol = 0;
  cp = cp1 = cp2 = buff = s;
  state = PARSER_GETVAR;
}


int
Parser::parse(parser_result_t **returned_result)
{
 int n,
     leadspace = 1;
  
  results.num_args = 0;
  *returned_result = &results;
  error=PARSER_ERR_NONE;

  //while( (c = *cp) )
  while( !eol)
  {
    c = *cp;
    if(c == 0) 
	{ eol = 1;
	}
    switch(state)
    {
      case PARSER_GETVAR:

		if(leadspace)
		{
  		  if(eol) return  PARSER_DONE;
		  if(c == ' ' || c == '\t')
		    {
			cp1 = cp+1;
			break;
		    }
		  else leadspace = 0;
	   	}


		if(eol || c  == '=')
		 {
		   if(( n  =  ( cp - cp1)) > 0 )
		    {
			char *var;
			 var = new char[n+1];
			 strncpy(var,cp1,n);
			 var[n]=0;
			 curr_rule= results.cmd_idx = findCmd(var);
			 delete[] var;
			 if(curr_rule < 0) 
			  {
  		  	    error = PARSER_ERR_VAR_NOT_FOUND;
			    return (cp1-buff);
			  }

			results.var_pos = cp1 - buff;

			if(eol)
			 {
			   if(results.num_args < rules[curr_rule].minvals)
			    {
				error=PARSER_ERR_TOO_FEW_VALS;
			        return (cp1-buff);
			    }
		           return PARSER_OK;
			  }

		   	 state = PARSER_GETVAL;
		    }
 		   else
		    {			// null var
  		  	error = PARSER_ERR_NO_VAR;
			return(cp1 - buff);	// return error pointing to position of bad var/val
		    }
		    cp1 = cp+1;
		 }
		else if(eol || c == ' ' || c == '\t')
		{
		   if(( n  =  ( cp - cp1)) > 0 )
		    {
			char *var;
			 var = new char[n+1];
			 strncpy(var,cp1,n);
			 var[n]=0;
			 curr_rule= results.cmd_idx = findCmd(var);
			 delete[] var;
			 if(curr_rule < 0) 
			  {
  		  	    error = PARSER_ERR_VAR_NOT_FOUND;
			    return (cp1-buff);
			  }

			   leadspace = 1;
			   if(results.num_args < rules[curr_rule].minvals)
			    {
				error=PARSER_ERR_TOO_FEW_VALS;
			        return (cp1-buff);
			    }
		           return PARSER_OK;

		     }
		}
		else if( c == ',')
		{
  		  error = PARSER_ERR_NO_VAL;
		  return( (cp-buff));	// error, var with no value, return pointing to char pos 
		}
 		break; 

      case PARSER_GETVAL:
		if( c == ',')
		 {

		   // got an argument
		   if(results.num_args +1 > rules[curr_rule].maxvals)
		    {
			error = PARSER_ERR_TOO_MANY_VALS;
			return (cp1-buff);
		    }

		   // copy value to string and convert to datatype dictated by the rule

   		   n = addArg(buff, cp1, cp);

		   if(n >= 0) return n;
		   cp1 = cp+1;

		 }
		else if(eol || c == ' ' || c == '\t')
		 {
		   // got an argument
		   if(results.num_args +1 > rules[curr_rule].maxvals)
		    {
			error = PARSER_ERR_TOO_MANY_VALS;
			return (cp1-buff);
		    }

		   // copy value to string and convert to datatype dictated by the rule

   		   n = addArg(buff, cp1, cp);

		   if(n >= 0) return n;
		   cp1 = cp+1;

		    state = PARSER_GETVAR;
		    leadspace = 1;
		    cp1 = cp+1;
		    cp2 = cp;
		    return PARSER_OK;
		 }

		break;
    } // end switch(state)

   cp++;

   }
  return  PARSER_DONE;

} // end parse()



