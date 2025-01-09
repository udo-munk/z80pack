// lp_main.h

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

#ifndef _LP_MAIN_DEFS
#define _LP_MAIN_DEFS

#ifndef WANT_SDL

#include <pthread.h>

typedef struct thread_info {
	int thread_no;
	int quit;
	int run;
	int running;
	pthread_t thread_id;
} thread_info_t;

#endif

#endif /* !_LP_MAIN_DEFS */
