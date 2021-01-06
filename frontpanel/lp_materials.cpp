/* lp_materials.C  materials functions 

   Copyright (c) 2007-2008, John Kichury

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

#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include "lp_materials.h"

#define MAX_MATERIALS 200

#define AMBIENT    0
#define DIFFUSE    4
#define SPECULAR   8
#define SHININESS 12
#define EMISSION  13

static float materials[MAX_MATERIALS][17];
static int material_flag[MAX_MATERIALS];   /* true = material set by user */
static int material_used_flag[MAX_MATERIALS]; /* true = material is used */
static int material_alpha_flag[MAX_MATERIALS]; /* true = material has an alpha component < 1.0*/
static int list_offset = 0;

void lp_init_materials_flags(void)
{ int i;

  for(i=0;i<MAX_MATERIALS;i++)
   { material_flag[i] = GL_FALSE;
     material_used_flag[i] = GL_FALSE;
     material_alpha_flag[i] = GL_FALSE;
   }
}

void lp_init_materials_dlist(void )
{
 if(list_offset) glDeleteLists(list_offset,MAX_MATERIALS);
 list_offset = glGenLists(MAX_MATERIALS);
}

int lp_is_material_alpha(int n)
{
     return(material_alpha_flag[n]);

}
void lp_init_materials(void )
{
 lp_init_materials_flags();

lp_set_material_params(1, 0.01, 0.01, 0.01, 1.0,	/* black */
		       0.0, 0.0, 0.0, 1.0,
		       0.0, 0.0, 0.0, 1.0, 00.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(2, 0.1, 0.0, 0.0, 1.0,		/* red */
		       1.0, 0.0, 0.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(3, 0.2, 0.12, 0.0, 1.0,		/* orange */
		       1.0, 0.6, 0.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(4, 0.2, 0.2, 0.0, 1.0,		/* yellow */
		       1.0, 1.0, 0.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(5, 0.0, 0.2, 0.0, 1.0,		/* green */
		       0.0, 0.5, 0.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(6, 0.0, 0.2, 0.2, 1.0,		/* cyan */
		       0.0, 1.0, 1.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(7, 0.0, 0.0, 0.2, 1.0,		/* blue */
		       0.0, 0.0, 1.0, 1.0,
		       0.0, 0.0, 0.0, 1.0, 0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(8, 0.2, 0.0, 0.2, 1.0,		/* violet */
		       1.0, 0.0, 1.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(9, 0.12, 0.02, 0.0, 1.0,		/* brown */
		       0.63, 0.10, 0.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(10, 0.2, 0.2, 0.2, 1.0,		/* white */
		       1.0, 1.0, 1.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(11, 0.2, 0.02, 0.02, 1.0,	/* pink */
		       1.0, 0.5, 0.5, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(12, 0.06, 0.0, 0.18, 1.0,	/* mauve */
		       0.3, 0.0, 0.9, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(13, 0.0, 0.2, 0.2, 1.0,		/* turquoise */
		       0.0, 1.0, 1.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(14, 0.078, 0.0, 0.2, 1.0,	/* indigo */
		       0.39, 0.0, 1.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(15, 0.15, 0.15, 0.15, 1.0,	/* grey */
		       0.78, 0.78, 0.78, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(16, 0.2, 0.0, 0.2, 1.0,		/* magenta */
		       1.0, 0.0, 1.0, 1.0,
		       0.0, 0.0, 0.0, 1.0,  0.0, 0.0,0.0,0.0,1.0);

/* specular materials */

lp_set_material_params(17, 0.02, 0.02, 0.02, 1.0,	/* shiny black */
		       0.0, 0.0, 0.0, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(18, 0.2, 0.0, 0.0, 1.0,		/* shiny red */
		       1.0, 0.0, 0.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(19, 0.2, 0.1, 0.0, 1.0,		/* shiny orange */
		       1.0, 0.6, 0.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(20, 0.2, 0.2, 0.0, 1.0,		/* shiny yellow */
		       1.0, 1.0, 0.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(21, 0.0, 0.2, 0.0, 1.0,		/* shiny green */
		       0.0, 1.0, 0.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(22, 0.0, 0.2, 0.2, 1.0,		/* shiny cyan */
		       0.0, 1.0, 1.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(23, 0.0, 0.0, 0.2, 1.0,		/* shiny blue */
		       0.0, 0.0, 1.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(24, 0.2, 0.0, 0.2, 1.0,		/* shiny violet */
		       1.0, 0.0, 1.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(25, 0.12, 0.02, 0.0, 1.0,	/* shiny brown */
		       0.63, 0.10, 0.0, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(26, 0.2, 0.2, 0.2, 1.0,		/* shiny white */
		       1.0, 1.0, 1.0, 1.0,
		       0.4, 0.4, 0.4, 1.0,  10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(27, 0.2, 0.02, 0.02, 1.0,	/* shiny pink */
		       1.0, 0.5, 0.5, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(28, 0.06, 0.0, 0.18, 1.0,	/* shiny mauve */
		       0.3, 0.0, 0.9, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(29, 0.0, 0.2, 0.2, 1.0,		/* shiny turquoise */
		       0.0, 1.0, 1.0, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(30, 0.078, 0.0, 0.2, 1.0,	/* shiny indigo */
		       0.39, 0.0, 1.0, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(31, 0.15, 0.15, 0.15, 1.0,	/* shiny grey */
		       0.78, 0.78, 0.78, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);

lp_set_material_params(32, 0.2, 0.0, 0.2, 1.0,		/* shiny magenta */
		       1.0, 0.0, 1.0, 1.0,
		       0.4, 0.4, 0.4, 1.0, 10.0, 0.0,0.0,0.0,1.0);


/* special materials */

lp_set_material_params(33,0.35, 0.25, 0.1, 1.0,		/* brass */
		       0.65, 0.5, 0.35, 1.0,
		       0.0, 0.0, 0.0, 1.0,  2.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(34,0.25, 0.15, 0.0, 1.0,		/* shiny brass */
		       0.65, 0.5, 0.35, 1.0,
		       0.9, 0.6, 0.0, 1.0,  20.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(35, 0.0, 0.0,  0.0, 1.0,		/* pewter */
                        0.6, 0.55 , 0.65, 1.0,
                        0.9, 0.9, 0.95, 1.0,
                        10.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(36,0.4, 0.4,  0.4, 1.0,		/* silver */
                         0.3, 0.3, 0.3, 1.0,
                         0.9, 0.9, 0.95, 1.0,
                         30.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(37,0.4, 0.2, 0.0, 1.0,		/* gold */
                         0.9, 0.5, 0.0, 1.0,
                         0.5, 0.5, 0.0, 1.0,
                         2.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(38,0.4, 0.2,  0.0, 1.0,		/* shiny gold */
                         0.9, 0.5, 0.0, 1.0,
                         0.9, 0.9, 0.0, 1.0,
                         20.0, 0.0,0.0,0.0, 1.0);


lp_set_material_params(39,0.3, 0.1, 0.1, 1.0,		/* red plastic */
                         0.5, 0.1, 0.1, 1.0,
                         0.45, 0.45, 0.45, 1.0,
                         30.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(40,0.1, 0.3, 0.1, 1.0,		/* green plastic */
                         0.1, 0.5, 0.1, 1.0,
                         0.45, 0.45, 0.45, 1.0,
                         30.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(41,0.1, 0.1, 0.3, 1.0,		/* blue plastic */
                         0.1, 0.1, 0.5, 1.0,
                         0.45, 0.45, 0.45, 1.0,
                         30.0, 0.0,0.0,0.0, 1.0);

lp_set_material_params(42,0.2, 0.2,  0.2, 1.0,		/* plaster */
                         0.95, 0.95, 0.95, .80,
                         0.0, 0.0, 0.0, 1.,
                         1.0, 0.0,0.0,0.0, 1.0);

}

void lp_set_material_params(int n,		/* material number */
	float Ar,float Ag,float Ab,float Aa,	/* Ambient rgba */
	float  Dr, float Dg,float Db,float Da,  /* Diffuse rgba */
	float  Sr,float Sg,float Sb,float Sa,	/* Specular rgba */
	float  shine,				/* shinyness    */
	float Er,float Eg,float Eb,float Ea)	/* emission rgba */
{
  if(n >= MAX_MATERIALS)
   { printf("Attempt to exceed Max amount of defined materials.\n");
     printf("Max allowed is %d.\n",MAX_MATERIALS);
     return;
   }

  /* if the material flag is TRUE, ignore this material, user has this defined
     as a custom material. */

  if(material_flag[n] ) return;

  materials[n][AMBIENT]   = Ar;
  materials[n][AMBIENT+1] = Ag;
  materials[n][AMBIENT+2] = Ab;
  materials[n][AMBIENT+3] = Aa;

  materials[n][DIFFUSE] = Dr;
  materials[n][DIFFUSE+1] = Dg;
  materials[n][DIFFUSE+2] = Db;
  materials[n][DIFFUSE+3] = Da;

  materials[n][SPECULAR] = Sr;
  materials[n][SPECULAR+1] = Sg;
  materials[n][SPECULAR+2] = Sb;
  materials[n][SPECULAR+3] = Sa;

  materials[n][SHININESS] = shine;

  materials[n][EMISSION] = Er;
  materials[n][EMISSION+1] = Eg;
  materials[n][EMISSION+2] = Eb;
  materials[n][EMISSION+3] = Ea;

  if(Aa < 1.0 || Da < 1.0 || Sa < 1.0 || Ea < 1.0) 
	material_alpha_flag[n] = GL_TRUE;
} /* end set_material_params() */




void lp_set_user_material_params(int n,		/* material num */
	float Ar,float Ag,float Ab,float Aa,	/* Ambient rgba */
	float  Dr, float Dg,float Db,float Da,  /* Diffuse rgba */
	float  Sr,float Sg,float Sb,float Sa,	/* Specular rgba */
	float  shine,				/* shinyness    */
	float Er,float Eg,float Eb,float Ea)	/* emission rgba */
{
  if(n >= MAX_MATERIALS)
   { printf("Attempt to exceed Max amount of defined materials.\n");
     printf("Max allowed is %d.\n",MAX_MATERIALS);
     return;
   }
  lp_set_material_params(n,Ar,Ag,Ab,Aa,Dr,Dg,Db,Da,Sr,Sg,Sb,Sa,shine,Er,Eg,Eb,Ea);
  material_flag[n] = GL_TRUE; /* mark it as a custom material */

  if(Aa < 1.0 || Da < 1.0 || Sa < 1.0 || Ea < 1.0) 
	material_alpha_flag[n] = GL_TRUE;
}


void lp_bind_material(int n)
{
 static int currmat = -1;


 if(n >= MAX_MATERIALS) return;

 if(n != currmat)
 {
  if(material_used_flag[n])
   {
    glCallList(n + list_offset);
   }
  else
   { glNewList(n + list_offset,GL_COMPILE_AND_EXECUTE);
     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &materials[n][AMBIENT]);
     glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &materials[n][DIFFUSE]);
     glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &materials[n][SPECULAR]);
     glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &materials[n][SHININESS]);
     glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &materials[n][EMISSION]);
     glEndList();
     material_used_flag[n] = GL_TRUE;
   }
  currmat = n;
 }
}

