// lp_materials.h

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

#ifndef _LP_MATERIALS_DEFS_
#define _LP_MATERIALS_DEFS_

#include <stdbool.h>

extern void	lp_set_material_params(int n,			/* material num */
			float Ar, float Ag, float Ab, float Aa,	/* Ambient rgba */
			float Dr, float Dg, float Db, float Da,	/* Diffuse rgba */
			float Sr, float Sg, float Sb, float Sa,	/* Specular rgba */
			float shine,				/* shinyness */
			float Er, float Eg, float Eb, float Ea); /* emission rgba */

extern void	lp_set_user_material_params(int n,		/* material num */
			float Ar, float Ag, float Ab, float Aa,	/* Ambient rgba */
			float Dr, float Dg, float Db, float Da,	/* Diffuse rgba */
			float Sr, float Sg, float Sb, float Sa,	/* Specular rgba */
			float shine,				/* shinyness */
			float Er, float Eg, float Eb, float Ea); /* emission rgba */

extern void	lp_bind_material(int n);
extern void	lp_init_materials(void);
extern void	lp_init_materials_dlist(void);
extern bool	lp_is_material_alpha(int n);

#endif /* !_LP_MATERIALS_DEFS */
