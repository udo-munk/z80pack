// lp_switch.cpp	light panel switch class


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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lpanel.h"
#include "lp_switch.h"
#include "lp_utils.h"

#define UNUSED(x) (void) (x)


// parse rules for switches defined in the user configuration file 

static parser_rules_t switch_parse_rules[] =
{ 
  { "type",    1, 1, PARSER_STRING },
  { "operate", 1, 1, PARSER_STRING },
  { "pos",     2, 3, PARSER_FLOAT },
  { "size",    2, 3, PARSER_FLOAT },
  { "objects", 1, 3, PARSER_STRING },
  { NULL,0,0,0 }
};

Parser parser;	// parser class for parsing config arguments
extern Parser parser;

// supporting render functions

void
lp_drawSwitchObjectDummy( Lpanel *p, lpSwitch *lpswitch)
{
 UNUSED(p);
 UNUSED(lpswitch);

 // a stub function for switches that do not have a render object defined
}

void
lp_drawSwitchObject( Lpanel *p, lpSwitch *lpswitch)
{
 int state = lpswitch->state;

 UNUSED(p);

 glPushMatrix();

 glTranslatef(  lpswitch->parms->pos[0], 
		lpswitch->parms->pos[1], 
		lpswitch->parms->pos[2]);

 glScalef( lpswitch->parms->scale[0], 
	   lpswitch->parms->scale[1], 
	   lpswitch->parms->scale[2]);

#if 1
 lpswitch->object_refs[state]->draw(1);

#else
 int i;
 int flag=0;

if(!strcmp(lpswitch->name,"SW_11"))
 flag = 1;

  glColor3f(1.,0.,0.);
  glBegin(GL_POLYGON);
   if(flag)
   printf("sw_11 targ=\n");
  for(i=0;i<4;i++)
  {
   if(flag)
   printf("= %f %f\n",
     lpswitch->up_target[i][0],
     lpswitch->up_target[i][1]);
    glVertex3fv(&lpswitch->up_target[i][0]);
  }
  glEnd();

  glColor3f(0.,1.,0.);
  glBegin(GL_POLYGON);
  for(i=0;i<4;i++)
    glVertex3fv(&lpswitch->down_target[i][0]);
  glEnd();
#endif

 glPopMatrix();
}

void
lp_drawSwitchObject_rotated( Lpanel *p, lpSwitch *lpswitch)
{
 int state = lpswitch->state;

 UNUSED(p);

 glPushMatrix();

 glTranslatef( lpswitch->parms->pos[0], lpswitch->parms->pos[1], lpswitch->parms->pos[2]);

 glScalef( lpswitch->parms->scale[0], 
	   lpswitch->parms->scale[1], 
	   lpswitch->parms->scale[2]);

 glRotatef(lpswitch->rotate[state][0],
           lpswitch->rotate[state][1],
           lpswitch->rotate[state][2],
           lpswitch->rotate[state][3]);


 lpswitch->object_refs[state]->draw(1);

 glPopMatrix();
}


void
lp_drawSwitchPaddle( Lpanel *p, lpSwitch *lpswitch)
{
 int state = lpswitch->state;

 UNUSED(p);

 glPushMatrix();

 glTranslatef( lpswitch->parms->pos[0], lpswitch->parms->pos[1], lpswitch->parms->pos[2]);

 glScalef( lpswitch->parms->scale[0], 
	   lpswitch->parms->scale[1], 
	   lpswitch->parms->scale[2]);

 glRotatef(lpswitch->rotate[state][0],
            lpswitch->rotate[state][1],
            lpswitch->rotate[state][2],
            lpswitch->rotate[state][3]);

  //draw_paddle();

  glPopMatrix();
}

void
lp_drawSwitchToggle( Lpanel *p, lpSwitch *lpswitch)
{
 int state = lpswitch->state;

 UNUSED(p);

  glPushMatrix();

  glTranslatef( lpswitch->parms->pos[0], lpswitch->parms->pos[1], lpswitch->parms->pos[2]);
 glScalef( lpswitch->parms->scale[0], 
	   lpswitch->parms->scale[1], 
	   lpswitch->parms->scale[2]);
  glRotatef(lpswitch->rotate[state][0],
            lpswitch->rotate[state][1],
            lpswitch->rotate[state][2],
            lpswitch->rotate[state][3]);

  //draw_toggle();

  glPopMatrix();
}



// lpSwitch class
// --------------

lpSwitch::lpSwitch(void)
{
 name = NULL;
 parms = NULL;
 gfx_mode = LP_SWITCH_GFX_TOGGLE;
 state = LP_SWITCH_DOWN;
 operation = LP_SWITCH_OP_ON_OFF;
 num_object_refs = 0;
 drawFunc = NULL;
 dataptr[0] = dataptr[1] = NULL;
 object_ref_names = NULL;
 select_up_name = select_dn_name = 0;
 callback = NULL;
 userdata = 0;
}

lpSwitch::~lpSwitch(void)
{
 int i;

 if(name) delete[] name;
 if(parms) delete parms;

 if(object_ref_names)
  {
    for(i=0; i<num_object_refs;i++)
      delete[] object_ref_names[i];
    delete[] object_ref_names;
  }
}


//
// switch action
// -------------
//  val  0=down  1=up  2=release (momentary )

void
lpSwitch::action(int val)
{

  switch(val)
   {
	case 0:		// down

		if (operation == LP_SWITCH_OP_OFF_MOM)
		  break;

		state = LP_SWITCH_DOWN;

		switch(operation)
		 {	
		  	case LP_SWITCH_OP_MOM_OFF_MOM:

				panel->mom_switch_pressed = this;

				if(dataptr[state])
				  switch(datatype)
				   {
				     case LP_SWITCH_DATATYPE_UINT8:
					{
					 uint8_t *ptr, val;
					 ptr = (uint8_t *) dataptr[state];
					 val = 0x1 << bitnum;
					 *ptr = *ptr | val;
					}
					break;

				     case LP_SWITCH_DATATYPE_UINT16:
					{
					 uint16_t *ptr, val;
					 ptr = (uint16_t *) dataptr[state];
					 val = 0x1 << bitnum;
					 *ptr = *ptr | val;
					}
					break;
				     case LP_SWITCH_DATATYPE_UINT32:
					{
					 uint32_t *ptr, val;
					 ptr = (uint32_t *) dataptr[state];
					 val = 0x1 << bitnum;
					 *ptr = *ptr | val;
					}
					break;
				   }
				break;

			case LP_SWITCH_OP_ON_OFF:
				if(dataptr[state])
				  switch(datatype)
				   {
				     case LP_SWITCH_DATATYPE_UINT8:
					{
					  uint8_t *ptr, val;
					  ptr = (uint8_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr & (~val);
					}
					break;
				     case LP_SWITCH_DATATYPE_UINT16:
					{
					  uint16_t *ptr, val;
					  ptr = (uint16_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr & (~val);
					}
					break;
				     case LP_SWITCH_DATATYPE_UINT32:
					{
					  uint32_t *ptr, val;
					  ptr = (uint32_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr & (~val);
					}
					break;
				   }
				break;

		 }
		break;

	case 1:		// up

		state = LP_SWITCH_UP;

		switch(operation)
		 {	
		  	case LP_SWITCH_OP_MOM_OFF_MOM:
		  	case LP_SWITCH_OP_OFF_MOM:
				panel->mom_switch_pressed = this;
				/* fallthrough */

			case LP_SWITCH_OP_ON_OFF:
				if(dataptr[state])
				  switch(datatype)
				   {
				     case LP_SWITCH_DATATYPE_UINT8:
					{
					  uint8_t *ptr, val;
					  ptr = (uint8_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr | val;
					}
					break;
				     case LP_SWITCH_DATATYPE_UINT16:
					{
					  uint16_t *ptr, val;
					  ptr = (uint16_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr | val;
					}
					break;
				     case LP_SWITCH_DATATYPE_UINT32:
					{
					  uint32_t *ptr, val;
					  ptr = (uint32_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr | val;
					}
					break;
				   }
				break;

		 }
		break;

	case 2:		// release mom switch

		switch(operation)
		 {
		  	case LP_SWITCH_OP_MOM_OFF_MOM:
		  	case LP_SWITCH_OP_OFF_MOM:
				if(dataptr[state])
				  switch(datatype)
				   {
				     case LP_SWITCH_DATATYPE_UINT8:
					{
					  uint8_t *ptr, val;
					  ptr = (uint8_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr & (~val);
					}
					break;
				     case LP_SWITCH_DATATYPE_UINT16:
					{
					  uint16_t *ptr, val;
					  ptr = (uint16_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr & (~val);
					}
					break;
				     case LP_SWITCH_DATATYPE_UINT32:
					{
					  uint32_t *ptr, val;
					  ptr = (uint32_t *) dataptr[state];
					  val = 0x1 << bitnum;
					  *ptr = *ptr & (~val);
					}
					break;
				   }
				panel->mom_switch_pressed = NULL;
				state = LP_SWITCH_CENTER;
				break;

			case LP_SWITCH_OP_ON_OFF:
				break;
		 }
		break;

   }
 if(callback) (*callback)(state, userdata);
}

void
lpSwitch::addCallback( void (*cbfunc)(int state, int val), int userval )
{
  callback = cbfunc;
  userdata = userval;
}

void
lpSwitch::bindData8(uint8_t *ptr_down, uint8_t *ptr_up, int bit_number)
{
  dataptr[LP_SWITCH_DOWN] = ptr_down;
  dataptr[LP_SWITCH_UP] = ptr_up;
  datatype = LP_SWITCH_DATATYPE_UINT8;
  bitnum = bit_number;
}

void
lpSwitch::bindData16(uint16_t *ptr_down, uint16_t *ptr_up, int bit_number)
{
  dataptr[LP_SWITCH_DOWN] = ptr_down;
  dataptr[LP_SWITCH_UP] = ptr_up;
  datatype = LP_SWITCH_DATATYPE_UINT16;
  bitnum = bit_number;
}

void
lpSwitch::bindData32(uint32_t *ptr_down, uint32_t *ptr_up, int bit_number)
{
  dataptr[LP_SWITCH_DOWN] = ptr_down;
  dataptr[LP_SWITCH_UP] = ptr_up;
  datatype = LP_SWITCH_DATATYPE_UINT32;
  bitnum = bit_number;
}

void
lpSwitch::bindData64(uint64_t *ptr_down, uint64_t *ptr_up, int bit_number)
{
  dataptr[LP_SWITCH_DOWN] = ptr_down;
  dataptr[LP_SWITCH_UP] = ptr_up;
  datatype = LP_SWITCH_DATATYPE_UINT64;
  bitnum = bit_number;
}

void
lpSwitch::setName(const char *_name)
{ int n;

  n = strlen(_name);
  name = new char[n+1];
  strcpy(name,_name);
}

void
lpSwitch::sampleData(void)
{
 if(!dataptr[0]) return;
 if(operation != LP_SWITCH_OP_ON_OFF) return;

 switch(datatype)
 {
   case LP_SWITCH_DATATYPE_UINT8:
	 { uint8_t *p, bit;
	  p = (uint8_t *) dataptr[0];
	  bit = ( 1 << bitnum) & *p;
	  if(bit) state = 1;
		else state = 0;
	 }
	break;

   case LP_SWITCH_DATATYPE_UINT16:
	 {
	  uint16_t *p, bit;
	  p = (uint16_t *) dataptr[0];
	  bit = ( 1 << bitnum) & *p;
	  if(bit) state = 1;
		else state = 0;
	 }
	break;

   case LP_SWITCH_DATATYPE_UINT32:
	 {
	  uint32_t *p, bit;
	  p = (uint32_t *) dataptr[0];
	  bit = ( 1 << bitnum) & *p;
	  if(bit) state = 1;
		else state = 0;
	 }
	break;
 }

}

void
lpSwitch::setupData(int sw_num)
{
 int i;

  // resolve referenced graphics objects

 for(i=0; i<num_object_refs;i++)
 {
  if( !( object_refs[i] = panel->findObjectByName(object_ref_names[i])))
   {
     fprintf(stderr, "error: switch %s references object %s which cannot be found.\n",
	name, object_ref_names[i]);
   }
 }

  // set the drawfunc and pick targets

  switch(type)
   {
     case LP_SWITCH_GFX_TOGGLE:
printf("setupData: toggle\n");
	break;

     case LP_SWITCH_GFX_PADDLE:
printf("setupData: paddle\n");
	break;

     case LP_SWITCH_GFX_OBJECT_REF:

	drawFunc = lp_drawSwitchObject;

       if(object_refs[0])
	{
	up_target[0][0] = object_refs[0]->bbox.xyz_min[0];
	up_target[0][1] = object_refs[0]->bbox.center[1];
	up_target[0][2] = object_refs[0]->bbox.xyz_min[2];

	up_target[1][0] = object_refs[0]->bbox.xyz_max[0];
	up_target[1][1] = object_refs[0]->bbox.center[1];
	up_target[1][2] = object_refs[0]->bbox.xyz_min[2];

	up_target[2][0] = object_refs[0]->bbox.xyz_max[0];
	up_target[2][1] = object_refs[0]->bbox.xyz_max[1];
	up_target[2][2] = object_refs[0]->bbox.xyz_min[2];

	up_target[3][0] = object_refs[0]->bbox.xyz_min[0];
	up_target[3][1] = object_refs[0]->bbox.xyz_max[1];
	up_target[3][2] = object_refs[0]->bbox.xyz_min[2];


	down_target[0][0] = object_refs[0]->bbox.xyz_min[0];
	down_target[0][1] = object_refs[0]->bbox.xyz_min[1];
	down_target[0][2] = object_refs[0]->bbox.xyz_min[2];

	down_target[1][0] = object_refs[0]->bbox.xyz_max[0];
	down_target[1][1] = object_refs[0]->bbox.xyz_min[1];
	down_target[1][2] = object_refs[0]->bbox.xyz_min[2];

	down_target[2][0] = object_refs[0]->bbox.xyz_max[0];
	down_target[2][1] = object_refs[0]->bbox.center[1];
	down_target[2][2] = object_refs[0]->bbox.xyz_min[2];

	down_target[3][0] = object_refs[0]->bbox.xyz_min[0];
	down_target[3][1] = object_refs[0]->bbox.center[1];
	down_target[3][2] = object_refs[0]->bbox.xyz_min[2];
	}
	else
	  drawFunc = lp_drawSwitchObjectDummy;

	break;
   default:
printf("setupData: invalid switch type %d\n", type);
	break;
   }

// set opengl name for mouse select
// the low order 31 bits are used to store the switch number (index within the array of
// switches). The high order bit indicates the target (1=up 0=down).

select_up_name = ((uint32_t) sw_num & LP_SW_PICK_IDMASK) | LP_SW_PICK_UP_BIT;
select_dn_name = (uint32_t) sw_num & LP_SW_PICK_IDMASK;

  switch(type)
   {
	case LP_SWITCH_OP_MOM_OFF_MOM:
	case LP_SWITCH_OP_OFF_MOM:
		break;
   }
}

int
Lpanel::addSwitch(const char *name, lp_obj_parm_t *obj, const char *buff, Lpanel *_panel)
{
  int i,n;
  lpSwitch *sw;
  parser_result_t *result;

  // parse config file line values such as position, type etc.
  // if n >= 0 it contains the char position in the line where an error
  // ocurred


  if(num_switches + 1 > max_switches) growSwitches();

  switches[num_switches] = new lpSwitch;
  switches[num_switches]->parms = obj;
  switches[num_switches]->setName(name);
  sw = switches[num_switches];
  sw->panel = _panel;
  num_switches++;

  parser.setRules(switch_parse_rules);
  parser.setParseString(buff);

  // set reasonable defaults

  obj->scale[0] = 1.;
  obj->scale[1] = 1.;
  obj->scale[2] = 1.;

  obj->pos[0] = 0.;
  obj->pos[1] = 0.;
  obj->pos[2] = 0.;

 while( (n = parser.parse(&result)) < 0 )
 {
   if(n != PARSER_DONE)
   {
   // printf("\nresult %s\n", switch_parse_rules[result->cmd_idx]);
  
#if 1
   if(!strcmp(switch_parse_rules[result->cmd_idx].cmd, "objects"))
   {
     sw->object_ref_names = new char *[result->num_args];

     for(i=0;i<result->num_args;i++)
      { 
	sw->object_ref_names[i] = new char[strlen(result->strings[i])+1];
	strcpy(sw->object_ref_names[i], result->strings[i]);
      }
     sw->num_object_refs = result->num_args;
    }
   else if(!strcmp(switch_parse_rules[result->cmd_idx].cmd, "operate"))
   {
	if(!strcmp(result->strings[0], "toggle"))
	{
	  sw->operation = LP_SWITCH_OP_ON_OFF;
	  sw->state = LP_SWITCH_DOWN;		// set to down
	}
	else
	if(!strcmp(result->strings[0], "mom_off_mom"))
	{
	  sw->operation = LP_SWITCH_OP_MOM_OFF_MOM;
	  sw->state = LP_SWITCH_CENTER;		// set to center
	}
	else
	if(!strcmp(result->strings[0], "off_mom"))
	{
	  sw->operation = LP_SWITCH_OP_OFF_MOM;
	  sw->state = LP_SWITCH_CENTER;		// set to center (down)
	}
	else
	{
	  printf("Invalid switch 'operation'. Should be one of 'toggle' or 'mom_off_mom' or 'off_mom'\n");
	  return result->var_pos;
	}
   }
   else if(!strcmp(switch_parse_rules[result->cmd_idx].cmd, "pos"))
   {
     for(i=0;i<result->num_args;i++)
	sw->parms->pos[i] =  result->floats[i];

   }
   else if(!strcmp(switch_parse_rules[result->cmd_idx].cmd, "size"))
   {
     for(i=0;i<result->num_args;i++)
	sw->parms->scale[i] =  result->floats[i];

   }
   else if(!strcmp(switch_parse_rules[result->cmd_idx].cmd, "type"))
   {
	if(!strcmp(result->strings[0], "toggle"))
	{
	  sw->type = LP_SWITCH_GFX_TOGGLE;
	}
	else
	if(!strcmp(result->strings[0], "paddle"))
	{
	  sw->type = LP_SWITCH_GFX_PADDLE;
	}
	else
	if(!strcmp(result->strings[0], "object"))
	{
	  sw->type = LP_SWITCH_GFX_OBJECT_REF;
	}
	else
	{
	  printf("Invalid switch 'type'. Should be one of 'toggle, paddle, object'\n");
	  return 0;
	}
   }
#endif
  }

  if( n == PARSER_DONE) break;
 }

  if( n >= 0) 
   { printf("n=%d\n",n);
     parser.printError();
     return n;
   }
   return(n);

}

int 
Lpanel::addSwitchCallback(const char *name, void (*cbfunc)(int state, int val), int userval)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  // int bitnum = 0, bit_inc = 1;
  lpSwitch *sw;

  num_names = xpand(name, &namelist);

  for(i=0;i<num_names;i++)
   {
     sw = findSwitchByName(namelist[i]);

     if(sw)
      {
        sw->addCallback( cbfunc, userval);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "addSwitchCallback: switch %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    // bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}


int
Lpanel::bindSwitch8(const char *name, void *loc_down, void *loc_up, int start_bit_number)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpSwitch *sw;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
    // printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

     if(bitnum <0)
      {
        fprintf(stderr, "bindSwitch8: switch %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     sw = findSwitchByName(namelist[i]);

     if(sw)
      {
        sw->bindData8( (uint8_t *) loc_down, (uint8_t *) loc_up, bitnum-1);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "bindSwitch8: switch %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}


int 
Lpanel::bindSwitch16(const char *name, void *loc_down, void *loc_up, int start_bit_number)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpSwitch *sw;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
    // printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

     if(bitnum <0)
      {
        fprintf(stderr, "bindSwitch16: switch %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     sw = findSwitchByName(namelist[i]);

     if(sw)
      {
        sw->bindData16( (uint16_t *) loc_down, (uint16_t *) loc_up, bitnum-1);
      }
     else
      {
       if(!ignore_bind_errors)  fprintf(stderr, "bindSwitch16: switch %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}


int
Lpanel::bindSwitch32(const char *name, void *loc_down, void *loc_up, int start_bit_number)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpSwitch *sw;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
    // printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

     if(bitnum <0)
      {
        fprintf(stderr, "bindSwitch32: switch %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     sw = findSwitchByName(namelist[i]);

     if(sw)
      {
        sw->bindData32( (uint32_t *) loc_down, (uint32_t *) loc_up, bitnum-1);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "bindSwitch32: switch %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

int
Lpanel::bindSwitch64(const char *name, void *loc_down, void *loc_up, int start_bit_number)
{
  char **namelist;
  int num_names;
  int i, status = 1;
  int bitnum, bit_inc;
  lpSwitch *sw;

  num_names = xpand(name, &namelist);

  bitnum = abs(start_bit_number);
  if(start_bit_number > 0) bit_inc = 1;
  else bit_inc = -1;

  for(i=0;i<num_names;i++)
   {
    // printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

     if(bitnum <0)
      {
        fprintf(stderr, "bindSwitch64: switch %s bad bitnum %d\n", namelist[i], bitnum);
        bitnum = 1;
      }

     sw = findSwitchByName(namelist[i]);

     if(sw)
      {
        sw->bindData64( (uint64_t *) loc_down, (uint64_t *) loc_up, bitnum-1);
      }
     else
      {
        if(!ignore_bind_errors) fprintf(stderr, "bindSwitch64: switch %s not found\n", namelist[i]);
        status = 0;
      }
    if(namelist[i]) delete[] namelist[i];
    bitnum += bit_inc;
   }
  delete[] namelist;

  return status;
}

void
lpSwitch::drawForPick(void)
{
 int i;

 glPushMatrix();

 glTranslatef(  parms->pos[0], 
		parms->pos[1], 
		parms->pos[2]);

 glScalef( parms->scale[0], 
	   parms->scale[1], 
	   parms->scale[2]);

  glColor3f(1.,0.,0.);

  glLoadName(select_up_name);
  glBegin(GL_POLYGON);
  for(i=0;i<4;i++)
    glVertex3fv(&up_target[i][0]);
  glEnd();

  glLoadName(select_dn_name);
  glBegin(GL_POLYGON);
  for(i=0;i<4;i++)
    glVertex3fv(&down_target[i][0]);
  glEnd();


 glPopMatrix();
}


//
// findSwitchByName
// ----------------

lpSwitch*
Lpanel::findSwitchByName(char *name)
{ int i;

 for(i=0;i<num_switches;i++)
  { if( !strcmp(switches[i]->name, name))
      return switches[i];
  }

return 0;
}


void
Lpanel::sampleSwitches(void)
{ int i;

  for(i=0;i<num_switches;i++)
    switches[i]->sampleData();

}
