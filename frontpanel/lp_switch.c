// lp_switch.c	lightpanel switch class

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WANT_SDL
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#endif

#include "lpanel.h"
#include "lp_switch.h"
#include "lp_utils.h"

#define UNUSED(x) (void) (x)

// parse rules for switches defined in the user configuration file

static parser_rules_t switch_parse_rules[] = {
	{ "type",    1, 1, PARSER_STRING },
	{ "operate", 1, 1, PARSER_STRING },
	{ "pos",     2, 3, PARSER_FLOAT },
	{ "size",    2, 3, PARSER_FLOAT },
	{ "objects", 1, 3, PARSER_STRING },
	{ NULL, 0, 0, 0 }
};

extern Parser_t parser;	// parser class for parsing config arguments

// supporting render functions

static void lp_drawSwitchObjectDummy(lpSwitch_t *p)
{
	UNUSED(p);

	// a stub function for switches that do not have a render object defined
}

static void lp_drawSwitchObject(lpSwitch_t *p)
{
	int state = p->state;

	glPushMatrix();

	glTranslatef(p->parms->pos[0], p->parms->pos[1], p->parms->pos[2]);

	glScalef(p->parms->scale[0], p->parms->scale[1], p->parms->scale[2]);

#if 1
	lpObject_draw_refoverride(p->object_refs[state], 1);
#else
	int i;
	int flag = 0;

	if (!strcmp(p->name, "SW_11"))
		flag = 1;

	glColor3f(1., 0., 0.);
	glBegin(GL_POLYGON);
	if (flag)
		printf("sw_11 targ=\n");
	for (i = 0; i < 4; i++) {
		if (flag)
			printf("= %f %f\n",
			       p->up_target[i][0],
			       p->up_target[i][1]);
		glVertex3fv(&p->up_target[i][0]);
	}
	glEnd();

	glColor3f(0., 1., 0.);
	glBegin(GL_POLYGON);
	for (i = 0; i < 4; i++)
		glVertex3fv(&p->down_target[i][0]);
	glEnd();
#endif

	glPopMatrix();
}

#if 0
static void lp_drawSwitchObject_rotated(lpSwitch_t *p)
{
	int state = p->state;

	glPushMatrix();

	glTranslatef(p->parms->pos[0], p->parms->pos[1], p->parms->pos[2]);

	glScalef(p->parms->scale[0], p->parms->scale[1], p->parms->scale[2]);

	glRotatef(p->rotate[state][0],
		  p->rotate[state][1],
		  p->rotate[state][2],
		  p->rotate[state][3]);

	lpObject_draw_refoverride(p->object_refs[state], 1);

	glPopMatrix();
}

static void lp_drawSwitchPaddle(lpSwitch_t *p)
{
	int state = p->state;

	glPushMatrix();

	glTranslatef(p->parms->pos[0], p->parms->pos[1], p->parms->pos[2]);

	glScalef(p->parms->scale[0], p->parms->scale[1], p->parms->scale[2]);

	glRotatef(p->rotate[state][0],
		  p->rotate[state][1],
		  p->rotate[state][2],
		  p->rotate[state][3]);

	// draw_paddle();

	glPopMatrix();
}

static void lp_drawSwitchToggle(lpSwitch_t *p)
{
	int state = p->state;

	glPushMatrix();

	glTranslatef(p->parms->pos[0], p->parms->pos[1], p->parms->pos[2]);

	glScalef(p->parms->scale[0], p->parms->scale[1], p->parms->scale[2]);

	glRotatef(p->rotate[state][0],
		  p->rotate[state][1],
		  p->rotate[state][2],
		  p->rotate[state][3]);

	// draw_toggle();

	glPopMatrix();
}
#endif

// lpSwitch class
// --------------

lpSwitch_t *lpSwitch_new(void)
{
	lpSwitch_t *p = (lpSwitch_t *) calloc(1, sizeof(lpSwitch_t));

	if (p)
		lpSwitch_init(p);

	return p;
}

void lpSwitch_delete(lpSwitch_t *p)
{
	if (p) {
		lpSwitch_fini(p);
		free(p);
	}
}

void lpSwitch_init(lpSwitch_t *p)
{
	p->name = NULL;
	p->parms = NULL;
	p->gfx_mode = LP_SWITCH_GFX_TOGGLE;
	p->state = LP_SWITCH_DOWN;
	p->operation = LP_SWITCH_OP_ON_OFF;
	p->num_object_refs = 0;
	p->drawFunc = NULL;
	p->dataptr[0] = p->dataptr[1] = NULL;
	p->object_ref_names = NULL;
	p->select_up_name = p->select_dn_name = 0;
	p->callback = NULL;
	p->userdata = 0;
}

void lpSwitch_fini(lpSwitch_t *p)
{
	int i;

	if (p->name)
		free(p->name);
	if (p->parms)
		free(p->parms);

	if (p->object_ref_names) {
		for (i = 0; i < p->num_object_refs; i++)
			free(p->object_ref_names[i]);
		free(p->object_ref_names);
	}
}

//
// switch action
// -------------
// val 0=down 1=up 2=release (momentary )

void lpSwitch_action(lpSwitch_t *p, int val)
{
	switch (val) {
	case 0:		// down

		if (p->operation == LP_SWITCH_OP_OFF_MOM)
			break;

		p->state = LP_SWITCH_DOWN;

		switch (p->operation) {
		case LP_SWITCH_OP_MOM_OFF_MOM:

			p->panel->mom_switch_pressed = p;

			if (p->dataptr[p->state])
				switch (p->datatype) {
				case LP_SWITCH_DATATYPE_UINT8: {
					uint8_t *ptr, val;

					ptr = (uint8_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr | val;
					}
					break;

				case LP_SWITCH_DATATYPE_UINT16: {
					uint16_t *ptr, val;

					ptr = (uint16_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr | val;
					}
					break;
				case LP_SWITCH_DATATYPE_UINT32: {
					uint32_t *ptr, val;

					ptr = (uint32_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr | val;
					}
					break;
				}
			break;

		case LP_SWITCH_OP_ON_OFF:
			if (p->dataptr[p->state])
				switch (p->datatype) {
				case LP_SWITCH_DATATYPE_UINT8: {
					uint8_t *ptr, val;

					ptr = (uint8_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr & (~val);
					}
					break;
				case LP_SWITCH_DATATYPE_UINT16: {
					uint16_t *ptr, val;

					ptr = (uint16_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr & (~val);
					}
					break;
				case LP_SWITCH_DATATYPE_UINT32: {
					uint32_t *ptr, val;

					ptr = (uint32_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr & (~val);
					}
					break;
				}
			break;

		}
		break;

	case 1:		// up

		p->state = LP_SWITCH_UP;

		switch (p->operation) {
		case LP_SWITCH_OP_MOM_OFF_MOM:
		case LP_SWITCH_OP_OFF_MOM:
			p->panel->mom_switch_pressed = p;
		/* fallthrough */

		case LP_SWITCH_OP_ON_OFF:
			if (p->dataptr[p->state])
				switch (p->datatype) {
				case LP_SWITCH_DATATYPE_UINT8: {
					uint8_t *ptr, val;

					ptr = (uint8_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr | val;
					}
					break;
				case LP_SWITCH_DATATYPE_UINT16: {
					uint16_t *ptr, val;

					ptr = (uint16_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr | val;
					}
					break;
				case LP_SWITCH_DATATYPE_UINT32: {
					uint32_t *ptr, val;

					ptr = (uint32_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr | val;
					}
					break;
				}
			break;

		}
		break;

	case 2:		// release mom switch

		switch (p->operation) {
		case LP_SWITCH_OP_MOM_OFF_MOM:
		case LP_SWITCH_OP_OFF_MOM:
			if (p->dataptr[p->state])
				switch (p->datatype) {
				case LP_SWITCH_DATATYPE_UINT8: {
					uint8_t *ptr, val;

					ptr = (uint8_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr & (~val);
					}
					break;
				case LP_SWITCH_DATATYPE_UINT16: {
					uint16_t *ptr, val;

					ptr = (uint16_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr & (~val);
					}
					break;
				case LP_SWITCH_DATATYPE_UINT32: {
					uint32_t *ptr, val;

					ptr = (uint32_t *) p->dataptr[p->state];
					val = 0x1 << p->bitnum;
					*ptr = *ptr & (~val);
					}
					break;
				}
			p->panel->mom_switch_pressed = NULL;
			p->state = LP_SWITCH_CENTER;
			break;

		case LP_SWITCH_OP_ON_OFF:
			break;
		}
		break;

	}
	if (p->callback)
		(*p->callback)(p->state, p->userdata);
}

void lpSwitch_addCallback(lpSwitch_t *p, lp_switch_cbf_t cbfunc, int userval)
{
	p->callback = cbfunc;
	p->userdata = userval;
}

void lpSwitch_bindData8(lpSwitch_t *p, uint8_t *ptr_down, uint8_t *ptr_up, int bit_number)
{
	p->dataptr[LP_SWITCH_DOWN] = ptr_down;
	p->dataptr[LP_SWITCH_UP] = ptr_up;
	p->datatype = LP_SWITCH_DATATYPE_UINT8;
	p->bitnum = bit_number;
}

void lpSwitch_bindData16(lpSwitch_t *p, uint16_t *ptr_down, uint16_t *ptr_up, int bit_number)
{
	p->dataptr[LP_SWITCH_DOWN] = ptr_down;
	p->dataptr[LP_SWITCH_UP] = ptr_up;
	p->datatype = LP_SWITCH_DATATYPE_UINT16;
	p->bitnum = bit_number;
}

void lpSwitch_bindData32(lpSwitch_t *p, uint32_t *ptr_down, uint32_t *ptr_up, int bit_number)
{
	p->dataptr[LP_SWITCH_DOWN] = ptr_down;
	p->dataptr[LP_SWITCH_UP] = ptr_up;
	p->datatype = LP_SWITCH_DATATYPE_UINT32;
	p->bitnum = bit_number;
}

void lpSwitch_bindData64(lpSwitch_t *p, uint64_t *ptr_down, uint64_t *ptr_up, int bit_number)
{
	p->dataptr[LP_SWITCH_DOWN] = ptr_down;
	p->dataptr[LP_SWITCH_UP] = ptr_up;
	p->datatype = LP_SWITCH_DATATYPE_UINT64;
	p->bitnum = bit_number;
}

void lpSwitch_setName(lpSwitch_t *p, const char *name)
{
	int n;

	n = strlen(name);
	p->name = (char *) malloc(n + 1);
	strcpy(p->name, name);
}

void lpSwitch_sampleData(lpSwitch_t *p)
{
	if (!p->dataptr[0])
		return;
	if (p->operation != LP_SWITCH_OP_ON_OFF)
		return;

	switch (p->datatype) {
	case LP_SWITCH_DATATYPE_UINT8: {
		uint8_t *ptr, bit;

		ptr = (uint8_t *) p->dataptr[0];
		bit = (1 << p->bitnum) & *ptr;
		if (bit)
			p->state = 1;
		else
			p->state = 0;
		}
		break;

	case LP_SWITCH_DATATYPE_UINT16: {
		uint16_t *ptr, bit;

		ptr = (uint16_t *) p->dataptr[0];
		bit = (1 << p->bitnum) & *ptr;
		if (bit)
			p->state = 1;
		else
			p->state = 0;
		}
		break;

	case LP_SWITCH_DATATYPE_UINT32: {
		uint32_t *ptr, bit;

		ptr = (uint32_t *) p->dataptr[0];
		bit = (1 << p->bitnum) & *ptr;
		if (bit)
			p->state = 1;
		else
			p->state = 0;
		}
		break;
	}
}

void lpSwitch_setupData(lpSwitch_t *p, int sw_num)
{
	int i;

	// resolve referenced graphics objects

	for (i = 0; i < p->num_object_refs; i++) {
		if (!(p->object_refs[i] = Lpanel_findObjectByName(p->panel,
								  p->object_ref_names[i]))) {
			fprintf(stderr, "error: switch %s references object %s which "
				"cannot be found.\n", p->name, p->object_ref_names[i]);
		}
	}

	// set the drawfunc and pick targets

	switch (p->type) {
	case LP_SWITCH_GFX_TOGGLE:
		printf("setupData: toggle\n");
		break;

	case LP_SWITCH_GFX_PADDLE:
		printf("setupData: paddle\n");
		break;

	case LP_SWITCH_GFX_OBJECT_REF:
		p->drawFunc = lp_drawSwitchObject;

		if (p->object_refs[0]) {
			p->up_target[0][0] = p->object_refs[0]->bbox.xyz_min[0];
			p->up_target[0][1] = p->object_refs[0]->bbox.center[1];
			p->up_target[0][2] = p->object_refs[0]->bbox.xyz_min[2];

			p->up_target[1][0] = p->object_refs[0]->bbox.xyz_max[0];
			p->up_target[1][1] = p->object_refs[0]->bbox.center[1];
			p->up_target[1][2] = p->object_refs[0]->bbox.xyz_min[2];

			p->up_target[2][0] = p->object_refs[0]->bbox.xyz_max[0];
			p->up_target[2][1] = p->object_refs[0]->bbox.xyz_max[1];
			p->up_target[2][2] = p->object_refs[0]->bbox.xyz_min[2];

			p->up_target[3][0] = p->object_refs[0]->bbox.xyz_min[0];
			p->up_target[3][1] = p->object_refs[0]->bbox.xyz_max[1];
			p->up_target[3][2] = p->object_refs[0]->bbox.xyz_min[2];

			p->down_target[0][0] = p->object_refs[0]->bbox.xyz_min[0];
			p->down_target[0][1] = p->object_refs[0]->bbox.xyz_min[1];
			p->down_target[0][2] = p->object_refs[0]->bbox.xyz_min[2];

			p->down_target[1][0] = p->object_refs[0]->bbox.xyz_max[0];
			p->down_target[1][1] = p->object_refs[0]->bbox.xyz_min[1];
			p->down_target[1][2] = p->object_refs[0]->bbox.xyz_min[2];

			p->down_target[2][0] = p->object_refs[0]->bbox.xyz_max[0];
			p->down_target[2][1] = p->object_refs[0]->bbox.center[1];
			p->down_target[2][2] = p->object_refs[0]->bbox.xyz_min[2];

			p->down_target[3][0] = p->object_refs[0]->bbox.xyz_min[0];
			p->down_target[3][1] = p->object_refs[0]->bbox.center[1];
			p->down_target[3][2] = p->object_refs[0]->bbox.xyz_min[2];
		} else
			p->drawFunc = lp_drawSwitchObjectDummy;

		break;
	default:
		printf("setupData: invalid switch type %d\n", p->type);
		break;
	}

	// set opengl name for mouse select
	// the low order 31 bits are used to store the switch number (index within the array of
	// switches). The high order bit indicates the target (1=up 0=down).

	p->select_up_name = ((uint32_t) sw_num & LP_SW_PICK_IDMASK) | LP_SW_PICK_UP_BIT;
	p->select_dn_name = (uint32_t) sw_num & LP_SW_PICK_IDMASK;

	switch (p->type) {
	case LP_SWITCH_OP_MOM_OFF_MOM:
	case LP_SWITCH_OP_OFF_MOM:
		break;
	}
}

int Lpanel_addSwitch(Lpanel_t *p, const char *name, lp_obj_parm_t *obj, const char *buff)
{
	int i, n;
	lpSwitch_t *sw;
	parser_result_t *result;

	// parse config file line values such as position, type etc.
	// if n >= 0 it contains the char position in the line where an error
	// ocurred

	if (p->num_switches + 1 > p->max_switches)
		Lpanel_growSwitches(p);

	p->switches[p->num_switches] = sw = lpSwitch_new();
	sw->parms = obj;
	lpSwitch_setName(p->switches[p->num_switches], name);
	sw->panel = p;
	p->num_switches++;

	Parser_setRules(&parser, switch_parse_rules);
	Parser_setParseString(&parser, buff);

	// set reasonable defaults

	obj->scale[0] = 1.;
	obj->scale[1] = 1.;
	obj->scale[2] = 1.;

	obj->pos[0] = 0.;
	obj->pos[1] = 0.;
	obj->pos[2] = 0.;

	while ((n = Parser_parse(&parser, &result)) < 0) {
		if (n != PARSER_DONE) {
			// printf("\nresult %s\n", switch_parse_rules[result->cmd_idx]);

#if 1
			if (!strcmp(switch_parse_rules[result->cmd_idx].cmd, "objects")) {
				sw->object_ref_names = (char **) malloc(sizeof(char *) *
									result->num_args);

				for (i = 0; i < result->num_args; i++) {
					sw->object_ref_names[i] =
						(char *) malloc(strlen(result->strings[i]) + 1);
					strcpy(sw->object_ref_names[i], result->strings[i]);
				}
				sw->num_object_refs = result->num_args;
			} else if (!strcmp(switch_parse_rules[result->cmd_idx].cmd, "operate")) {
				if (!strcmp(result->strings[0], "toggle")) {
					sw->operation = LP_SWITCH_OP_ON_OFF;
					sw->state = LP_SWITCH_DOWN;		// set to down
				} else if (!strcmp(result->strings[0], "mom_off_mom")) {
					sw->operation = LP_SWITCH_OP_MOM_OFF_MOM;
					sw->state = LP_SWITCH_CENTER;		// set to center
				} else if (!strcmp(result->strings[0], "off_mom")) {
					sw->operation = LP_SWITCH_OP_OFF_MOM;
					sw->state = LP_SWITCH_CENTER;		// to center (down)
				} else {
					printf("Invalid switch 'operation'. Should be one "
					       "of 'toggle' or 'mom_off_mom' or 'off_mom'\n");
					return result->var_pos;
				}
			} else if (!strcmp(switch_parse_rules[result->cmd_idx].cmd, "pos")) {
				for (i = 0; i < result->num_args; i++)
					sw->parms->pos[i] = result->floats[i];

			} else if (!strcmp(switch_parse_rules[result->cmd_idx].cmd, "size")) {
				for (i = 0; i < result->num_args; i++)
					sw->parms->scale[i] = result->floats[i];

			} else if (!strcmp(switch_parse_rules[result->cmd_idx].cmd, "type")) {
				if (!strcmp(result->strings[0], "toggle")) {
					sw->type = LP_SWITCH_GFX_TOGGLE;
				} else if (!strcmp(result->strings[0], "paddle")) {
					sw->type = LP_SWITCH_GFX_PADDLE;
				} else if (!strcmp(result->strings[0], "object")) {
					sw->type = LP_SWITCH_GFX_OBJECT_REF;
				} else {
					printf("Invalid switch 'type'. Should be one "
					       "of 'toggle, paddle, object'\n");
					return 0;
				}
			}
#endif
		}

		if (n == PARSER_DONE)
			break;
	}

	if (n >= 0) {
		printf("n=%d\n", n);
		Parser_printError(&parser);
		return n;
	}

	return n;
}

bool Lpanel_addSwitchCallback(Lpanel_t *p, const char *name, lp_switch_cbf_t cbfunc, int userval)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	lpSwitch_t *sw;

	num_names = xpand(name, &namelist);

	for (i = 0; i < num_names; i++) {
		sw = Lpanel_findSwitchByName(p, namelist[i]);

		if (sw) {
			lpSwitch_addCallback(sw, cbfunc, userval);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "addSwitchCallback: switch %s not found\n",
					namelist[i]);
			status = false;
		}
		if (namelist[i])
			free(namelist[i]);
	}
	free(namelist);

	return status;
}

bool Lpanel_bindSwitch8(Lpanel_t *p, const char *name, void *loc_down, void *loc_up,
			int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpSwitch_t *sw;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		// printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

		if (bitnum < 0) {
			fprintf(stderr, "bindSwitch8: switch %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		sw = Lpanel_findSwitchByName(p, namelist[i]);

		if (sw) {
			lpSwitch_bindData8(sw, (uint8_t *) loc_down, (uint8_t *) loc_up,
					   bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindSwitch8: switch %s not found\n",
					namelist[i]);
			status = false;
		}
		if (namelist[i])
			free(namelist[i]);
		bitnum += bit_inc;
	}
	free(namelist);

	return status;
}

bool Lpanel_bindSwitch16(Lpanel_t *p, const char *name, void *loc_down, void *loc_up,
			 int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpSwitch_t *sw;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		// printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

		if (bitnum < 0) {
			fprintf(stderr, "bindSwitch16: switch %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		sw = Lpanel_findSwitchByName(p, namelist[i]);

		if (sw) {
			lpSwitch_bindData16(sw, (uint16_t *) loc_down, (uint16_t *) loc_up,
					    bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindSwitch16: switch %s not found\n",
					namelist[i]);
			status = false;
		}
		if (namelist[i])
			free(namelist[i]);
		bitnum += bit_inc;
	}
	free(namelist);

	return status;
}

bool Lpanel_bindSwitch32(Lpanel_t *p, const char *name, void *loc_down, void *loc_up,
			 int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpSwitch_t *sw;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		// printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

		if (bitnum < 0) {
			fprintf(stderr, "bindSwitch32: switch %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		sw = Lpanel_findSwitchByName(p, namelist[i]);

		if (sw) {
			lpSwitch_bindData32(sw, (uint32_t *) loc_down, (uint32_t *) loc_up,
					    bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindSwitch32: switch %s not found\n",
					namelist[i]);
			status = false;
		}
		if (namelist[i])
			free(namelist[i]);
		bitnum += bit_inc;
	}
	free(namelist);

	return status;
}

bool Lpanel_bindSwitch64(Lpanel_t *p, const char *name, void *loc_down, void *loc_up,
			 int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpSwitch_t *sw;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		// printf("name %d = %s bitnum=%d\n", i, namelist[i], bitnum);

		if (bitnum < 0) {
			fprintf(stderr, "bindSwitch64: switch %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		sw = Lpanel_findSwitchByName(p, namelist[i]);

		if (sw) {
			lpSwitch_bindData64(sw, (uint64_t *) loc_down, (uint64_t *) loc_up,
					    bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindSwitch64: switch %s not found\n",
					namelist[i]);
			status = false;
		}
		if (namelist[i])
			free(namelist[i]);
		bitnum += bit_inc;
	}
	free(namelist);

	return status;
}

void lpSwitch_drawForPick(lpSwitch_t *p)
{
	int i;

	glPushMatrix();

	glTranslatef(p->parms->pos[0],
		     p->parms->pos[1],
		     p->parms->pos[2]);

	glScalef(p->parms->scale[0],
		 p->parms->scale[1],
		 p->parms->scale[2]);

	glColor3f(1., 0., 0.);

	glLoadName(p->select_up_name);
	glBegin(GL_POLYGON);
	for (i = 0; i < 4; i++)
		glVertex3fv(&p->up_target[i][0]);
	glEnd();

	glLoadName(p->select_dn_name);
	glBegin(GL_POLYGON);
	for (i = 0; i < 4; i++)
		glVertex3fv(&p->down_target[i][0]);
	glEnd();

	glPopMatrix();
}

//
// findSwitchByName
// ----------------

lpSwitch_t *Lpanel_findSwitchByName(Lpanel_t *p, char *name)
{
	int i;

	for (i = 0; i < p->num_switches; i++) {
		if (!strcmp(p->switches[i]->name, name))
			return p->switches[i];
	}

	return 0;
}

void Lpanel_sampleSwitches(Lpanel_t *p)
{
	int i;

	for (i = 0; i < p->num_switches; i++)
		lpSwitch_sampleData(p->switches[i]);
}
