// lpanel.cpp   lightpanel class

#define DEBUG 0

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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "lp_utils.h"
#include "lp_materials.h"
#include "lpanel.h"

#include "lpanel_data.h"

#define UNUSED(x) (void)(x)

extern pthread_mutex_t data_sample_lock;

static parser_rules_t light_parse_rules[] =
{
  { "color",   3, 3, PARSER_FLOAT },
  { "group",   1, 1, PARSER_INT },
  { "object",  1, 1, PARSER_STRING },
  { "pos",     2, 3, PARSER_FLOAT },
  { "size",    2, 3, PARSER_FLOAT },
  { NULL,0,0,0 }
};

extern Parser parser;


void drawLightObject(lpLight *lp)
{
   lp->obj_ref->draw(2);
}

void drawLightGraphics(lpLight *lp)
{
 int i;
 int lod=1;

for(i=0;i<3;i++)
 {
   if( lp->color[i] < 0. ||
    lp->color[i] > 1. )
    printf("drawLight: color out of range color[%d] = %f\n", i, lp->color[i]);
 }

  //glColor3f(0.,0.,0.);
  glBegin(GL_POLYGON);

   for(i=0;i<cir2d_nverts-1;i+=lod)
    glVertex2fv(&cir2d_data2[i][0]);

  glEnd();

  glBegin(GL_TRIANGLE_STRIP);

  for(i=0;i<cir2d_nverts-1;i+=lod)
   { 
     glColor3fv(&lp->color[0]);
     glVertex2fv(&cir2d_data2[i][0]);
     glColor3f(0.,0.,0.);
     glVertex2fv(&cir2d_data[i][0]);
   }
  
  glColor3fv(&lp->color[0]);
  glVertex2fv(&cir2d_data2[0][0]);
  glColor3f(0.,0.,0.);
  glVertex2fv(&cir2d_data[0][0]);
  glEnd();

}

void 
sampleData8_error(lpLight *p)
{ 
 UNUSED(p);

#if 0
static int flag = 0;
 if(!flag)
  {
    printf("sampleData8: light %s has no data bound to it.  \n", p->name);
    flag = 1;
  }
#endif
}

void 
sampleData8(lpLight *p)
{
  unsigned char bit;
  uint8 *ptr = (uint8 *) p->dataptr;

  bit = (int) (*ptr >> p->bitnum) & 0x01;

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }
  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}


void 
sampleData8invert(lpLight *p)
{
  unsigned char bit;
  uint8 *ptr = (uint8 *) p->dataptr;

  bit = (int) ~(*ptr >> p->bitnum) & 0x01;

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }
  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}

void
sampleData16(lpLight *p)
{
  unsigned char bit;
  uint16 *ptr = (uint16 *) p->dataptr;
#if 0
  uint64 on_time_inc = 0;
#endif
  // int logit = 0;

  bit = (int) (*ptr >> p->bitnum) & 0x01;

#if 0
  if(bit)
  {
#if 0
    p->on_time += ( *p->simclock - p->old_clock );
#endif
   on_time_inc = ( *p->simclock - p->old_clock );
  }

  if( bit != p->state)
   {
	on_time_inc = on_time_inc >> 1;
   }
  p->on_time += on_time_inc;
#endif

 //if(p->old_clock > *p->simclock) printf("sampleData16: clock stepped backward\n");

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }

  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}

void
sampleDatafv(lpLight *p)
{
  float *ptr = (float *) p->dataptr;
  // int logit = 0;

  if(p->smoothing > 0)
  {
    p->intense_curr = p->intensity;
    p->intense_samples[p->intense_curr_idx]  = ptr[p->bitnum];

    if(p->intense_samples[p->intense_curr_idx] > 1.0) p->intense_samples[p->intense_curr_idx] = 1.0;
    else if(p->intense_samples[p->intense_curr_idx] < 0.0) p->intense_samples[p->intense_curr_idx] = 0.0;

    p->intense_incr = ( p->intense_samples[p->intense_curr_idx] - 
			p->intense_samples[!p->intense_curr_idx]) / (float) p->smoothing;
    
  }
  else
   p->intensity = ptr[p->bitnum];

}



void 
sampleData16invert(lpLight *p)
{
  unsigned char bit;
  uint16 *ptr = (uint16 *) p->dataptr;

  bit = (int) ~(*ptr >> p->bitnum) & 0x01;

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }
  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}

void
sampleData32(lpLight *p)
{
  unsigned char bit;
  uint32 *ptr = (uint32 *) p->dataptr;

  bit = (int) (*ptr >> p->bitnum) & 0x01;

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }
  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}


void 
sampleData32invert(lpLight *p)
{
  unsigned char bit;
  uint32 *ptr = (uint32 *) p->dataptr;

  bit = (int) ~(*ptr >> p->bitnum) & 0x01;

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }
  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}

void
sampleData64(lpLight *p)
{
  unsigned char bit;
  uint64 *ptr = (uint64 *) p->dataptr;

  bit = (int) (*ptr >> p->bitnum) & 0x01;

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }
  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}

void 
sampleData64invert(lpLight *p)
{
  unsigned char bit;
  uint64 *ptr = (uint64 *) p->dataptr;

  bit = (int) ~(*ptr >> p->bitnum) & 0x01;

  if(bit)
  {
    p->on_time += ( *p->simclock - p->old_clock );
  }
  p->old_clock = *p->simclock;
  p->dirty = 1;
  p->state = bit;

}


Lpanel::Lpanel(void)		 // constructor
{
 int i;

#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
#else
 window = 0;
 vi = NULL;
 cx = 0;
 dpy = 0;
#endif

 num_lights = 0;
 max_lights = 0;
 lights = NULL;

 num_switches = max_switches = 0;
 switches = NULL;
 mom_switch_pressed = NULL;

 default_clock = 0;
 old_clock = 0;
 simclock =  &default_clock;
 clock_warp = 0;

 default_runflag = 0;
 runflag = &default_runflag;

 // init light groups

 for(i=0;i<LP_MAX_LIGHT_GROUPS;i++)
 {
   light_groups[i].num_items = 0;
   light_groups[i].max_items = 0;
   light_groups[i].list = NULL;
 }

 // init light graphics

 lightcolor[0] = 1.;
 lightcolor[1] = 0.;
 lightcolor[2] = 0.;
 lightsize[0] = 0.1875;
 lightsize[1] = 0.1875;
 lightsize[2] = 0.1875;

  for(i=0;i<cir2d_nverts;i++)
   {
    cir2d_data2[i][0] = cir2d_data[i][0] * .4;
    cir2d_data2[i][1] = cir2d_data[i][1] * .4;
   }

 // init config root path

 config_root_path = NULL;
 setConfigRootPath(".");

 // init graphics objects

 envmap_detected = 0;
 num_objects = 0;
 max_objects = 0;
 objects = NULL;

 num_alpha_objects = 0;
 max_alpha_objects = 0;
 alpha_objects = NULL;

 curr_object = NULL;
 curr_element = NULL;
 curr_vertex = NULL;

 // use this for small displays like notebooks etc.
 window_xsize = 800;
 window_ysize = 325;

 // use this for Desktops/Workstations with good graphics
 //window_xsize = 1600;
 //window_ysize = 650;

 cursor[0] = 0.;
 cursor[1] = 0.;
 cursor[2] = 0.0;
 cursor_inc = .01;
 do_cursor = 0;
 do_stats = 0;
 shift_key_pressed = 0;


 // graphics view parms

 view.rot[0] = 0.;
 view.rot[1] = 0.;
 view.rot[2] = 0.;

 view.pan[0] = 0.;
 view.pan[1] = -0.0;
 view.pan[2] = -16.0;

 view.znear = 0.01;
 view.zfar  = 1000.0;
 view.fovy  = 25.0;
 view.projection = LP_ORTHO;
 view.redo_projections = 1;
 view.do_depthtest = 0;		// default to no zbuffer

 quit_callbackfunc = NULL;

} // end constructor


Lpanel::~Lpanel(void)		// destructor
{
   Quit();

}

int
Lpanel::test(int n)
{

 printf("panel test %d\n",n);

 switch(n)
 {
   case 0:
	break;
   case 1:
	break;
   default:
	break;
 }

 return 1;
}

void
Lpanel::addQuitCallback( void (*cbfunc)(void))
{
  quit_callbackfunc = cbfunc;
}

void
Lpanel::Quit(void)
{
 int i;


 for(i=0;i<num_lights;i++)
  if(lights[i]) delete lights[i];
 num_lights = max_lights = 0;


 for(i=0;i<num_objects;i++)
  if(objects[i]) delete objects[i];
 num_objects = max_objects = 0;

 for(i=0;i<num_switches;i++)
  if(switches[i]) delete switches[i];
 num_switches = max_switches = 0;

 for(i=0;i<LP_MAX_LIGHT_GROUPS;i++)
 {
   if(light_groups[i].list) 
    { delete[] light_groups[i].list;
      light_groups[i].list = NULL;
    }
   light_groups[i].num_items = 0;
   light_groups[i].max_items = 0;
 }

 //destroyWindow();

} // end destructor


int
Lpanel::addLight(const char *name, lp_obj_parm_t *obj, const char *buff)
{
  int i,n;
  parser_result_t *result;
  lpLight *light;


  if(num_lights + 1 > max_lights) growLights();

  lights[num_lights] = new lpLight;  
  light = lights[num_lights];
  lights[num_lights]->parms = obj; 
  lights[num_lights]->setName(name);
  lights[num_lights]->bindSimclock(simclock, &clock_warp);
  lights[num_lights]->panel = this;

  parser.setRules(light_parse_rules);
  parser.setParseString(buff);

  // parse config file line values such has position, color etc.
  // if n >= 0 it contains the char position in the line where an error
  // ocurred

 while( (n = parser.parse(&result)) < 0 )
 {
   if(n != PARSER_DONE)
   {

      if(!strcmp(light_parse_rules[result->cmd_idx].cmd, "color"))
       {
	for(i=0;i<result->num_args;i++)
        light->parms->color[i] =  result->floats[i];
       }
      else if(!strcmp(light_parse_rules[result->cmd_idx].cmd, "group"))
       {
        light->parms->group =  result->ints[0];
       }
      else if(!strcmp(light_parse_rules[result->cmd_idx].cmd, "object"))
       {
	light->obj_refname = new char[strlen(result->strings[0])+1];
        strcpy(light->obj_refname, result->strings[0]);
       }
      else if(!strcmp(light_parse_rules[result->cmd_idx].cmd, "pos"))
       {
	for(i=0;i<result->num_args;i++)
        light->parms->pos[i] =  result->floats[i];
       }
      else if(!strcmp(light_parse_rules[result->cmd_idx].cmd, "size"))
       {
	for(i=0;i<result->num_args;i++)
        light->parms->scale[i] =  result->floats[i];
       }
   } // end if(n != PARSER_DONE)

  if( n == PARSER_DONE) break;
 }

  if( n >= 0)
   { printf("n=%d\n",n);
     parser.printError();
     return n;
   }


  if(obj->group >=0)
   {
     addLightToGroup(num_lights, obj->group);
   }

  num_lights++;

  return(-1);
  
}

int 
Lpanel::addLightToGroup( int lightnum, int groupnum)
{

 if(groupnum >= LP_MAX_LIGHT_GROUPS)
  {
     fprintf(stderr, "error: light %s invalid group number (%d)\n", lights[lightnum]->name, groupnum);
     return 0;
  }

 if( light_groups[groupnum].num_items + 1 > light_groups[groupnum].max_items)
  {
    int *new_list,
	i;
    
    new_list = new int[light_groups[groupnum].max_items + 1];

    if(light_groups[groupnum].list) 
     { for(i=0; i < light_groups[groupnum].max_items;i++)
		new_list[i] = light_groups[groupnum].list[i];
       delete[] light_groups[groupnum].list;
     }
    light_groups[groupnum].list = new_list;
  }
 light_groups[groupnum].max_items += 1;
 light_groups[groupnum].list[light_groups[groupnum].num_items] = lightnum;
 light_groups[groupnum].num_items++;
 return 1;
}

int 
Lpanel::bindLight8(const char *name, void *loc, int start_bit_number)
{
  char **namelist; 
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
	fprintf(stderr, "bindLight8: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);     

     if(light)
      {
        light->bindData8( (uint8 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
	if(!ignore_bind_errors) fprintf(stderr, "bindLight8: light %s not found\n", namelist[i]);
	status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

// bind light and invert logic according to mask

int 
Lpanel::bindLight8invert(const char *name, void *loc, int start_bit_number, uint8 mask)
{
  char **namelist; 
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc; // bit_inv;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
	fprintf(stderr, "bindLight8invert: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);     

     if(light)
      {
	if( mask & (0x1 << (bitnum-1)))
         light->bindData8invert( (uint8 *) loc);
        else
         light->bindData8( (uint8 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
	if(!ignore_bind_errors) fprintf(stderr, "bindLight8invert: light %s not found\n", namelist[i]);
	status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

int
Lpanel::bindLight16(const char *name, void *loc, int start_bit_number)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
        fprintf(stderr, "bindLight16: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);

     if(light)
      {
        light->bindData16( (uint16 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "bindLight16: light %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}


// bindLightfv 
// bind to an array of float values
// subsequent sampling will use the bit number as an index into the array of float values
// instead of a single bit

int
Lpanel::bindLightfv(const char *name, void *loc)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  // int bitnum, bit_inc;
  lpLight *light;

  num_names = xpand(name, &namelist);

  for(i=0;i<num_names;i++)
   {

     light = findLightByName(namelist[i]);

     if(light)
      {
        light->bindDatafv( (float *) loc);
        light->setBitNumber(i);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "bindLightfv: light %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    // bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

int 
Lpanel::bindLight16invert(const char *name, void *loc, int start_bit_number, uint16 mask)
{
  char **namelist; 
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc; // bit_inv;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
	fprintf(stderr, "bindLight16invert: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);     

     if(light)
      {
	if( mask & (0x1 << (bitnum-1)))
         light->bindData16invert( (uint16 *) loc);
        else
         light->bindData16( (uint16 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
	if(!ignore_bind_errors) fprintf(stderr, "bindLight16invert: light %s not found\n", namelist[i]);
	status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}



int
Lpanel::bindLight32(const char *name, void *loc, int start_bit_number)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
        fprintf(stderr, "bindLight16: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);

     if(light)
      {
        light->bindData32( (uint32 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "bindLight32: light %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

int 
Lpanel::bindLight32invert(const char *name, void *loc, int start_bit_number, uint32 mask)
{
  char **namelist; 
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc; // bit_inv;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
	fprintf(stderr, "bindLight32invert: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);     

     if(light)
      {
	if( mask & (0x1 << (bitnum-1)))
         light->bindData32invert( (uint32 *) loc);
        else
         light->bindData32( (uint32 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
	if(!ignore_bind_errors) fprintf(stderr, "bindLight32invert: light %s not found\n", namelist[i]);
	status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}


int
Lpanel::bindLight64(const char *name, void *loc, int start_bit_number)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
        fprintf(stderr, "bindLight64: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);

     if(light)
      {
        light->bindData64( (uint64 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "bindLight64: light %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

int 
Lpanel::bindLight64invert(const char *name, void *loc, int start_bit_number, uint64 mask)
{
  char **namelist; 
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc; // bit_inv;
  lpLight *light;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
     if(bitnum <=0)
      {
	fprintf(stderr, "bindLight64invert: light %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     light = findLightByName(namelist[i]);     

     if(light)
      {
	if( mask & (0x1 << (bitnum-1)))
         light->bindData64invert( (uint64 *) loc);
        else
         light->bindData64( (uint64 *) loc);
        light->setBitNumber(bitnum-1);
      }
     else
      {
	if(!ignore_bind_errors) fprintf(stderr, "bindLight64invert: light %s not found\n", namelist[i]);
	status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

void
Lpanel::bindRunFlag(uint8 *addr)
{
 int i;

 runflag = (uint8 *) addr;

  for(i=0;i<num_lights;i++)
   lights[i]->bindRunFlag((uint8 *)addr);
}

void
Lpanel::bindSimclock(uint64 *addr)
{
 int i;
 simclock = (uint64 *) addr;

 for(i=0;i<num_lights;i++)
   lights[i]->bindSimclock((uint64 *) addr, &clock_warp);

}


void
Lpanel::draw(void)
{ int i;

 if(view.redo_projections) 
  {
   setProjection(0);
   setModelview(0);
   view.redo_projections = 0;
  } 
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

 // draw graphics objects 

 //glEnable(GL_LIGHTING);

 for(i=0;i<num_objects;i++)
  { 
    if(objects[i]->is_alpha) continue;

    if(objects[i]->texture_num) 
     {
	textures.bindTexture(objects[i]->texture_num);
     } 
    if(objects[i]->have_normals) glEnable(GL_LIGHTING);
    objects[i]->draw();
  } 

 // draw lights

 glDisable(GL_TEXTURE_2D);
 glDisable(GL_LIGHTING);
 glEnable(GL_POLYGON_OFFSET_FILL);
 glPolygonOffset(0.,-10.);

 for(i=0;i<num_lights;i++)
   lights[i]->draw();

 // draw switches

 for(i=0;i<num_switches;i++)
    switches[i]->drawFunc(this, switches[i]);

 if(alpha_objects)
 {
//  glDisable(GL_DEPTH_TEST);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 glEnable(GL_BLEND);

  for(i=0;i<num_alpha_objects;i++)
  { 
    if(alpha_objects[i]->texture_num) 
     {
	textures.bindTexture(alpha_objects[i]->texture_num);
     } 
    if(alpha_objects[i]->have_normals) glEnable(GL_LIGHTING);
    alpha_objects[i]->draw();
  } 
 // glEnable(GL_DEPTH_TEST);
 }

 glDisable(GL_TEXTURE_2D);
 glDisable(GL_POLYGON_OFFSET_FILL);
 glEnable(GL_POLYGON_OFFSET_LINE);
 if(do_cursor) 
  {
   glEnable(GL_POLYGON_OFFSET_LINE);
   draw_cursor();
   glDisable(GL_POLYGON_OFFSET_LINE);
  }
 if(do_stats) draw_stats();
#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
 SwapBuffers(hDC);
 //UpdateWindow(hWnd);
#else
 glXSwapBuffers(dpy, window);
#endif
}


void
Lpanel::growLights(void)
{
  int i;
  lpLight **new_lights; 

  new_lights = new lpLight * [num_lights + 8];
  for(i=0;i<num_lights;i++)
    new_lights[i] = lights[i];

  max_lights += 8;
  delete[] lights;
  lights = new_lights;
}

void
Lpanel::growSwitches(void)
{
  int i;
  lpSwitch **new_switches; 

  new_switches = new lpSwitch* [num_switches + 1];
  for(i=0;i<num_switches;i++)
    new_switches[i] = switches[i];

  max_switches += 1;
  delete[] switches;
  switches = new_switches;
}

int
Lpanel::pick(int button, int state, int x, int y)
{
 GLuint namebuf[500], *ptr;
 int i,
     num_picked = 0,
     switch_dir;
 uint32 switch_num;

 UNUSED(button);

 if(state == 0) 
  {
    if( mom_switch_pressed )
       mom_switch_pressed->action(2);
    return(num_picked);
  }


 namebuf[0]=0;
 glSelectBuffer(500,&namebuf[0]);
 glRenderMode(GL_SELECT);
 glInitNames();
 glPushName(0);
 glGetIntegerv (GL_VIEWPORT, viewport);
 glMatrixMode (GL_PROJECTION);
 glPushMatrix ();
 glLoadIdentity ();

 gluPickMatrix ((GLdouble) x, (GLdouble) (window_ysize - y), 1.0, 1.0, viewport);
 doPickProjection();
 glMatrixMode (GL_MODELVIEW);
 glPushMatrix();
 doPickModelview();

 // draw switches

 for(i=0;i<num_switches;i++)
    switches[i]->drawForPick();
  
 num_picked = glRenderMode(GL_RENDER);

 if( num_picked)
  {
   uint32 n;
   ptr = (GLuint *) namebuf;
   n = ptr[3];

   switch_num = n & LP_SW_PICK_IDMASK;		// decode the switch number
   switch_dir = ((n & LP_SW_PICK_UP_BIT) != 0);
//printf("pick: switch_num=%d dir=%d\n", switch_num, switch_dir);

   switches[switch_num]->action(switch_dir);
  }

 glPopMatrix();
 glMatrixMode (GL_PROJECTION);
 glPopMatrix();
 glMatrixMode (GL_MODELVIEW);
 glPopMatrix();
 glRenderMode(GL_RENDER);

 return(num_picked);

}


int
Lpanel::readConfig(const char *_fname)
{
 FILE *fd;
 int i,n;

#define BUFSIZE 256
#define TOKENSIZE 80

 char buffer[BUFSIZE], token[TOKENSIZE];
 int pos, lineno=0, bailout=0;
 lp_obj_parm_t *obj;

 char *fname;

 fname = new char[strlen(config_root_path) + 1 + strlen(_fname) + 1];
 strcpy(fname, config_root_path);
 strcat(fname, "/");
 strcat(fname, _fname);

 if( (fd=fopen(fname,"r")) == 0)
  {
    fprintf(stderr,"readFile: could not open file %s\n",fname);
    delete[] fname;
    return 0;
  }

 lp_init_materials();

 while(!feof(fd) && !bailout)
 {
  lineno++;
  if(!freadlin(fd,buffer,BUFSIZE)) continue;
  pos = 0;
  if(!gtoken(buffer,token,TOKENSIZE,&pos)) continue;   // blank line
  if(token[0] == '#') continue;                        // comment

  if(!strcmp(token, "color"))		// color
   {  int n;
 
      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("color defined outside of an object\n");
	 bailout = 1;
       }
      else
       {
	 n = sscanf(&buffer[pos],"%f %f %f",
	    &curr_object->color[0],
	    &curr_object->color[1],
	    &curr_object->color[2]);

	if(n < 3)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("color must have 3 (r g b) values in the range of 0.0 - 1.0.\n");
	   bailout = 1;
	 }	

       }
   }
  else if(!strcmp(token, "zbuffer"))
   {
     view.do_depthtest = 1;
   }
  else
  if(!strcmp(token, "envmap"))		// environment mapped reflection
   { 
      if(!curr_object)
	{ printf("Error on line %d of config file %s\n",lineno, fname);
          printf("envmap defined outside of an object\n");
	  bailout = 1;
	}
	else
	{
	 envmap_detected = 1;
	 curr_object->envmapped = 1;
	}
   }
  else if(!strcmp(token, "instance"))
   {
     char s[100];

      if(!curr_object)
	{ printf("Error on line %d of config file %s\n",lineno, fname);
          printf("instance defined outside of an object\n");
	  bailout = 1;
	}
      else
	{	
  	  if(!gtoken(buffer,s,100,&pos))
	   { printf("Error on line %d of config file %s\n",lineno, fname);
             printf("instance with no object name.\n");
	     bailout = 1;
	   }
	  else
	    curr_object->setInstanceName(s);
	}
   }
  else if(!strcmp(token, "light"))
   {
      obj = new lp_obj_parm_t;
      obj->type = LP_LED;
      obj->subtype = LP_LED_3D;
      obj->group = -1;		// does not belong to group
      obj->pos[0] = 0.;
      obj->pos[1] = 0.;
      obj->pos[2] = 0.;
      obj->color[0] = lightcolor[0];
      obj->color[1] = lightcolor[1];
      obj->color[2] = lightcolor[2];
      obj->scale[0] = lightsize[0];
      obj->scale[1] = lightsize[1];
      obj->scale[2] = lightsize[2];

      gtoken(buffer,token,TOKENSIZE,&pos);		// get name

      n = addLight(token, obj, &buffer[pos]);
      if(n >= 0) 
       { bailout = 1;
	 n += pos;
  	 printf("Error on line %d of config file %s\n",lineno, fname);

  	 printf("%s\n",buffer);
	 for(i=0;i<n;i++)
	   putchar(' ');
	 printf("^\n"); 
       }

   }
  else if(!strcmp(token, "lightcolor"))           // default light size
   {  int n;

         n = sscanf(&buffer[pos],"%f %f %f",
            &lightcolor[0],
            &lightcolor[1],
            &lightcolor[2]);

	if(n < 3)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("lightcolor must have 3 (r g b) values in the range of 0.0 - 1.0.\n");
	   bailout = 1;
	 }	

   }
  else if(!strcmp(token, "lightsize"))           // default light size
   {  int n;

         n = sscanf(&buffer[pos],"%f %f %f",
            &lightsize[0],
            &lightsize[1],
            &lightsize[2]);

        if(n < 1)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("lightsize must have 1, 2, or 3 values (x y z).\n");
           bailout = 1;
         }      

	if(n == 1)  lightsize[1] = lightsize[2] = lightsize[0];
	else if(n == 2)  lightsize[2] = lightsize[0];
   
   }
  else if(!strcmp(token, "line"))
   {
      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("line defined outside of an object\n");
	 bailout = 1;
       }
      else
       {
	if(!(curr_element = curr_object->addElement(curr_object)))
         { printf("could not allocate memory for line.\n");
	   bailout = 1;
	 }	
        else
	 { curr_element->type = LP_LINE;
           curr_element->setTextureManager(&textures);
  	 }
       }
   }
  else if(!strcmp(token, "material"))           // material reference or definition
   {  int n, matnum;
      float Ar, Ag, Ab, Aa,   // Ambient rgba 
	    Dr,  Dg, Db, Da,  // Diffuse rgba
	    Sr, Sg, Sb, Sa,   // Specular rgba
	    shine,            // shinyness
	    Er, Eg, Eb, Ea;   // emission rgba

	if(curr_object)		// material reference
	{
         n = sscanf(&buffer[pos],"%d",&curr_object->material);

         if(n != 1)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("material within an object definition must have one integer value.\n");
           bailout = 1;
         }      
	 curr_object->is_alpha = lp_is_material_alpha(curr_object->material);
	}
	else		// material definition
	{

         n = sscanf(&buffer[pos],"%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
	    &matnum, &Ar,&Ag,&Ab,&Aa, &Dr,&Dg,&Db,&Da, &Sr,&Sg,&Sb,&Sa, &shine, &Er,&Eg,&Eb,&Ea);

        if(n < 18)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("material definition must have 18 values.\n");
           bailout = 1;
         }      
	else
          lp_set_material_params(matnum, Ar,Ag,Ab,Aa, Dr,Dg,Db,Da, Sr,Sg,Sb,Sa, shine, Er,Eg,Eb,Ea);
   
	}
   }
  else if(!strcmp(token, "message"))		// message
   { 
	printf("%s\n", &buffer[pos]);
   }
  else if(!strcmp(token, "n"))		// vertex normal vector
   {  int n;
 
      if(!curr_element || !curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("normal defined outside of a polygon or line\n");
	 bailout = 1;
       }
      else
       {
	if(!curr_vertex)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("normal defined before a vertex definition\n");
	   bailout = 1;
	 }	

	 n = sscanf(&buffer[pos],"%f %f %f",
	    &curr_vertex->norm[0],
	    &curr_vertex->norm[1],
	    &curr_vertex->norm[2]);

	if(n < 3)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("normal must have 3 values.\n");
	   bailout = 1;
	 }	
	else
	 curr_object->have_normals = 1;
       }
   }
  else if(!strcmp(token, "object"))
   {
     curr_object = addObject();
     curr_object->setTextureManager(&textures);
     if( gtoken(buffer,token,TOKENSIZE,&pos))
	curr_object->setName(token);
     curr_element = NULL;
     curr_vertex = NULL;
   }
  else if(!strcmp(token, "perspective"))
   {
     view.projection = LP_PERSPECTIVE;
   }
  else if(!strcmp(token, "polygon"))
   {
      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("polygon defined outside of an object\n");
	 bailout = 1;
       }
      else
       {
	if(!(curr_element = curr_object->addElement(curr_object)))
         { printf("could not allocate memory for polygon.\n");
	   bailout = 1;
	 }	
        else
	 { curr_element->type = LP_POLYGON;
           curr_element->setTextureManager(&textures);
  	 }
       }
   }
  else if(!strcmp(token, "referenced"))		// referenced
   {
      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("color defined outside of an object\n");
	 bailout = 1;
       }
      else
	curr_object->referenced = 1;
   }
  else if(!strcmp(token, "rotate"))		// rotate hpr
   {  int n;
 
      if(!curr_object)
       { 
	 n = sscanf(&buffer[pos],"%f %f",
	    &view.rot[0],
	    &view.rot[1]);

	if(n < 2)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("scene rotate must have 2 values.\n");
	   bailout = 1;
	 }	

       }
      else
       {
	 n = sscanf(&buffer[pos],"%f %f %f",
	    &curr_object->rotate[0],
	    &curr_object->rotate[1],
	    &curr_object->rotate[2]);

	if(n < 3)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("object rotate must have 3 values.\n");
	   bailout = 1;
	 }	
       }
   }
  else if(!strcmp(token, "switch"))
   {
      obj = new lp_obj_parm_t;
      obj->group = -1;		// does not belong to group
      obj->pos[0] = 0.;
      obj->pos[1] = 0.;
      obj->pos[2] = 0.;
      obj->color[0] = 1.;
      obj->color[1] = 0.;
      obj->color[2] = 0.;
      obj->scale[0] = 0.1875;
      obj->scale[1] = 0.1875;
      obj->scale[2] = 0.1875;

      gtoken(buffer,token,TOKENSIZE,&pos);		// get name

      n = addSwitch(token, obj, &buffer[pos], this);
      if(n >= 0) 
       { bailout = 1;
	 n += pos;
  	 printf("Error on line %d of config file %s\n",lineno, fname);

  	 printf("%s\n",buffer);
	 for(i=0;i<n;i++)
	   putchar(' ');
	 printf("^\n"); 
       }

   }
  else if(!strcmp(token, "t"))		// texture coordinate
   {  int n;
 
      if(!curr_element || !curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("texture coord defined outside of a polygon or line\n");
	 bailout = 1;
	 break;
       }
      else
       {
	if(!curr_vertex)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("texture coord defined before a vertex coordinate was defined.\n");
	   bailout = 1;
	   break;
         }

	 n = sscanf(&buffer[pos],"%f %f",
	    &curr_vertex->st[0],
	    &curr_vertex->st[1]);

	if(n < 2)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("texture coordinate must have 2 coordinates (e.g. t <s> <t>).\n");
	   bailout = 1;
	   break;
	 }	
	 curr_element->have_tcoords = 1;
       }
   }
  else if(!strcmp(token, "texture"))		// texture
   {  
      char *texture_path = NULL;
      int len;

      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("texture defined outside of an object.\n");
	 bailout = 1;
	 break;
       }

     if(!gtoken(buffer,token,TOKENSIZE,&pos))
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("texture with no path to jpeg file defined.\n");
	 bailout = 1;
	 break;
       }

     len = strlen(config_root_path) + strlen(token) + 1;

     texture_path = new char[len+1];
     strcpy(texture_path, config_root_path);
     strcat(texture_path, "/");
     strcat(texture_path, token);
     texture_path[len] = 0;


     if( !(curr_object->texture_num = textures.addTexture(texture_path)))
       { printf("Error on line %d of config file %s\n",lineno, texture_path);
         printf("could not load texture '%s'.\n", texture_path);
	 bailout = 1;
	 break;
       }

     delete[] texture_path;

   }
  else if(!strcmp(token, "texture_scale"))
   {  int n;
 
      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("texture_scale defined outside of an object\n");
	 bailout = 1;
       }
      else
       {
	 n = sscanf(&buffer[pos],"%f %f",
	    &curr_object->texture_scale[0],
	    &curr_object->texture_scale[1]);

	if(n < 2)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("texture_scale must have 2 (x y) values.\n");
	   bailout = 1;
	 }	

       }
   }
  else if(!strcmp(token, "texture_translate"))
   {  int n;
 
      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("texture_translate defined outside of an object\n");
	 bailout = 1;
       }
      else
       {
	 n = sscanf(&buffer[pos],"%f %f",
	    &curr_object->texture_translate[0],
	    &curr_object->texture_translate[1]);

	if(n < 2)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("texture_translate must have 2 (x y) values.\n");
	   bailout = 1;
	 }	

       }
   }
  else if(!strcmp(token, "translate"))
   {  int n;
 
      if(!curr_object)
       { 
	 n = sscanf(&buffer[pos],"%f %f %f",
	    &view.pan[0],
	    &view.pan[1],
	    &view.pan[2]);

	if(n < 3)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("scene translate must have 3 values.\n");
	   bailout = 1;
	 }	

       }
   }
  else if(!strcmp(token, "tristrip"))
   {
      if(!curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("tristrip defined outside of an object\n");
	 bailout = 1;
       }
      else
       {
	if(!(curr_element = curr_object->addElement(curr_object)))
         { printf("could not allocate memory for tristrip.\n");
	   bailout = 1;
	 }	
        else
	 { curr_element->type = LP_TRISTRIP;
           curr_element->setTextureManager(&textures);
  	 }
       }
   }
  else if(!strcmp(token, "v"))		// vertex
   {  int n;
 
      if(!curr_element || !curr_object)
       { printf("Error on line %d of config file %s\n",lineno, fname);
         printf("vertex defined outside of a polygon or line\n");
	 bailout = 1;
       }
      else
       {
	if(!(curr_vertex = curr_element->addVertex()))
         { printf("could not allocate memory for vertex.\n");
	   bailout = 1;
	 }	

	 n = sscanf(&buffer[pos],"%f %f %f",
	    &curr_vertex->xyz[0],
	    &curr_vertex->xyz[1],
	    &curr_vertex->xyz[2]);

	if(n < 2)
         { printf("Error on line %d of config file %s\n",lineno, fname);
           printf("vertex must have 2 or 3 coordinates.\n");
	   bailout = 1;
	 }	

       }
   }
  else
   {
     printf("Error: unknown statement on line %d of config file %s\n",lineno, fname);
     bailout = 1;

   }
 } // end while(!feof)

 delete[] fname;
 fclose(fd);

 if(!bailout)
  { 
    genGraphicsData();
    resolveObjectInstances();

    for(i=0;i<num_lights;i++)
      lights[i]->setupData();

    for(i=0;i<num_switches;i++)
      switches[i]->setupData(i);
  }
 return(!bailout);
} // end readConfig()

lpLight *
Lpanel::findLightByName(char *name)
{ int i;

 for(i=0;i<num_lights;i++)
  { if( !strcmp(lights[i]->name, name))
      return lights[i];
  }

return 0;
}

// parse var=v,v,v

 enum parse_states { GET_VAR, GET_VAL };



void
Lpanel::printLights(void)
{ int i;

 printf("lights:\n");
 for(i=0;i<num_lights;i++)
   lights[i]->print();

}

void
Lpanel::resolveObjectInstances(void)
{
 int i,j;
 lpObject *p;

 for(i=0;i<num_objects;i++)
  {
   if(objects[i]->instance_name)
    { if( (p = findObjectByName(objects[i]->instance_name)))
	objects[i]->instance_object = p;
      else
	fprintf(stderr,"Error: object %s instances object %s which cannot be found.\n", 
		objects[i]->name, objects[i]->instance_name);
    }
  }

 for(i=0;i<num_objects;i++)
  {
    p = objects[i]->instance_object;
    while(p)
    {
      for(j=0;j<3;j++)
	{
 	   objects[i]->bbox.xyz_min[j] = min(objects[i]->bbox.xyz_min[j], p->bbox.xyz_min[j]);
           objects[i]->bbox.xyz_max[j] = max(objects[i]->bbox.xyz_max[j], p->bbox.xyz_max[j]);
	}
      p = p->instance_object;
    }

  }
}

void
Lpanel::sampleData(void)
{ int i;

 // mutex lock
 pthread_mutex_lock(&data_sample_lock);

if( *simclock < old_clock )
{
  fprintf(stderr,"libfrontpanel: Warning clock went backwards (current=%lld  previous=%lld.\n", *simclock, old_clock);
    
}
 old_clock = *simclock;

 for(i=0;i<num_lights;i++)
   lights[i]->sampleData();
 // mutex unlock
pthread_mutex_unlock(&data_sample_lock);

}

void
Lpanel::sampleDataWarp(int clockwarp)
{ int i;

 clock_warp = clockwarp;

 for(i=0;i<num_lights;i++)
   lights[i]->sampleData();

 clock_warp = 0;
}

void
Lpanel::sampleLightGroup(int groupnum, int clockval)
{ int i;

 if(groupnum < 0 || groupnum >= LP_MAX_LIGHT_GROUPS)
  {
    fprintf(stderr, "sampleLightGroup: groupnum (%d) must be in the range of (0-%d). \n",
      groupnum, LP_MAX_LIGHT_GROUPS-1);
  }

 clock_warp = clockval;

 for(i=0;i<light_groups[groupnum].num_items;i++)
     lights[ light_groups[groupnum].list[i] ]->sampleData();

 clock_warp = 0;
}

void
Lpanel::setConfigRootPath(const char *path)
{
 if(config_root_path) delete[] config_root_path;
 config_root_path = new char [strlen(path) + 1];
 strcpy(config_root_path,path);

}

// -------------
// lpLight class
// -------------

lpLight::lpLight(void)
{
  bindtype = LBINDTYPE_BIT;	// default to bind to bit
  parms = NULL; 
  name  = NULL; 
  obj_refname  = NULL; 
  obj_ref = NULL; 
  sampleDataFunc = sampleData8_error;
  drawFunc = drawLightGraphics;
  t1 = t2 = on_time = 1;
  start_clock = 0;
  old_clock = 0;
  dirty = 0;
  state = 0;
  old_state = 0;
  intensity = 1.;

  smoothing = 0;		// no intensity smoothing for bindtype of FLOATV
  intense_curr_idx = 0;
  intense_samples[0] = 0.;
  intense_samples[1] = 0.;
  color[0] = 0.2;
  color[1] = 0.;
  color[2] = 0.;
  default_runflag = 0;
  runflag = &default_runflag;
}

lpLight::~lpLight(void)
{
 if(name) delete[] name;
 if(obj_refname) delete[] obj_refname;
 if(parms) delete parms;
}

void
lpLight::bindRunFlag(uint8 *addr)
{
 runflag = (uint8 *) addr;
}

void
lpLight::bindSimclock(uint64 *addr, int *clockwarp)
{
 
 simclock = addr;
 clock_warp = clockwarp;
}


void
lpLight::draw(void)
{
  int i;
  // float *fp;

 switch(bindtype)
 {

  case LBINDTYPE_BIT:

	if(*runflag)
	 { if(dirty) calcIntensity();
         }
	else
	 {
    	   for(i=0;i<3;i++)
      	   { color[i] = parms->color[i] * (float) state + (parms->color[i] * .2);
             color[i] =  min(color[i],1.0); 
           }
         } 
	break;

  case LBINDTYPE_FLOATV:

	// fp = (float *) dataptr;

	//intensity = fp[bitnum];

	//if(intensity > 1.0) intensity = 1.0;

	if(smoothing)
	{ intense_curr += intense_incr;
	  if(intense_curr > 1.0) intense_curr = 1.0;
	  else if(intense_curr < 0.0) intense_curr = 0.0;
	  intensity = intense_curr;
	}

	for(i=0;i<3;i++)
      	   { color[i] = parms->color[i] * intensity + (parms->color[i] * .2);
             color[i] =  min(color[i],1.0); 
           }
//printf("xyzzy: intense=%f bitnum=%d\n",intensity, bitnum);
	break;

 }
 
#if DEBUG
#if 0
 if(!strcmp(name,"LED_ADDR_00") ||
    !strcmp(name,"LED_ADDR_01") ||
    !strcmp(name,"LED_ADDR_10") )
#endif


 if(!strcmp(name,"LED_ADDR_15"))
  { 
    fprintf(stderr, "draw: %s %f\n",name, intensity);
  }
#endif

  glPushMatrix();
  glTranslatef(parms->pos[0], parms->pos[1], parms->pos[2]);
  glScalef(parms->scale[0], parms->scale[1], parms->scale[2]);


 glColor3fv(&color[0]);

 (*drawFunc)(this);

 glPopMatrix();
}

void
lpLight::print(void)
{
  printf("light: name=%s\n", name);

 printf("obj: pos=%f %f %f  size=%f %f %f  color=%f %f %f\n",
   parms->pos[0],
   parms->pos[1],
   parms->pos[2],
   parms->scale[0],
   parms->scale[1],
   parms->scale[2],
   parms->color[0],
   parms->color[1],
   parms->color[2]);
}

void 
lpLight::bindData8(uint8 *ptr)
{
  sampleDataFunc = sampleData8;
  dataptr = (uint8*) ptr;
}
void 
lpLight::bindData8invert(uint8 *ptr)
{
  sampleDataFunc = sampleData8invert;
  dataptr = (uint8*) ptr;
}

void 
lpLight::bindData16(uint16 *ptr)
{
// xyzzy
  sampleDataFunc = sampleData16;
  dataptr = (uint16*) ptr;
}

void 
lpLight::bindDatafv(float *ptr)
{
  sampleDataFunc = sampleDatafv;
  dataptr = (float *) ptr;
  bindtype = LBINDTYPE_FLOATV;
}

void 
lpLight::bindData16invert(uint16 *ptr)
{
  sampleDataFunc = sampleData16invert;
  dataptr = (uint16*) ptr;
}

void 
lpLight::bindData32(uint32 *ptr)
{
  sampleDataFunc = sampleData32;
  dataptr = (uint32*) ptr;
}

void 
lpLight::bindData32invert(uint32 *ptr)
{
  sampleDataFunc = sampleData32invert;
  dataptr = (uint32*) ptr;
}

void 
lpLight::bindData64(uint64 *ptr)
{
  sampleDataFunc = sampleData64;
  dataptr = (uint64*) ptr;
}

void 
lpLight::bindData64invert(uint64 *ptr)
{
  sampleDataFunc = sampleData64invert;
  dataptr = (uint64*) ptr;
}


void 
lpLight::calcIntensity(void)
{ 
  int i;
  // unsigned int dt;
  uint64 clock_delta;

  // int logit = 0;

  clock_delta = old_clock - start_clock;
  if(clock_delta == 0)
  {// intensity = 0.;
   return;
  }
  else
   intensity = (float) ((double) on_time / (double) (clock_delta) * 2.0);
  if(intensity > 1.0) intensity = 1.0;

#if 0
//printf("calcIntensity runflag = %d\n",*runflag);
 if(*runflag)
  //if(!strcmp(name,"LED_ADDR_15") && intensity > .2)
  if(!strcmp(name,"LED_ADDR_15") )
  { 

    fprintf(stderr, "dbg: draw: %s intense=%f on_time=%d  old_clock=%d  start_clock=%d  old-start=%d\n",name, intensity, on_time, old_clock, start_clock,old_clock-start_clock);
  }
#endif

  start_clock = *simclock;
  on_time = 0;
  dirty = 0;



  for(i=0;i<3;i++)
   {  color[i] = parms->color[i] * intensity + parms->color[i] * .2;
     // color[i] =  min(color[i],1.0); 
      if(color[i] > 1.0) color[i] = 1.0;
   }


}


void 
lpLight::sampleData(void)
{
 (*sampleDataFunc)(this);
}

void
lpLight::setupData(void)
{
 

 if(obj_refname)
  { if( !( obj_ref = panel->findObjectByName(obj_refname)))
   {
     fprintf(stderr, "error: light %s references object %s which cannot be found.\n",
        name, obj_refname);
     return;
   }

   drawFunc = drawLightObject;
  }

}

void 
lpLight::setName(const char *_name)
{ int n;

  n = strlen(_name);
  name = new char[n+1];
  strcpy(name,_name);
}


int 
Lpanel::smoothLight(const char *name, int nframes)
{
  char **namelist; 
  int num_names;
  int i, status = 1;
  lpLight *light;

  if( nframes <0)
   {
    fprintf(stderr, "smoothLight: light %s error. nframes = %d  must be >=0 \n", name, nframes);
    return 0;
   }

  num_names = xpand(name, &namelist);

  for(i=0;i<num_names;i++)
   {

     light = findLightByName(namelist[i]);     

     if(light)
      {
        light->smoothing = nframes;
      }
     else
      {
	if(!ignore_bind_errors) fprintf(stderr, "smoothLight: light %s not found\n", namelist[i]);
	status = 0;
      }
    if(namelist[i]) delete namelist[i];
   }
  delete[] namelist;

  return status;
}

