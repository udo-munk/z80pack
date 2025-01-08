// lpanel.c	lightpanel class

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

#define DEBUG 0

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WANT_SDL
#include <SDL.h>
#include <SDL_opengl.h>
#else /* !WANT_SDL */
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glx.h>
#endif
#endif /* !WANT_SDL */

#include "lp_utils.h"
#include "lp_materials.h"
#include "lpanel.h"

#include "lpanel_data.h"

#define UNUSED(x) (void) (x)

static parser_rules_t light_parse_rules[] = {
	{ "color",  3, 3, PARSER_FLOAT },
	{ "group",  1, 1, PARSER_INT },
	{ "object", 1, 1, PARSER_STRING },
	{ "pos",    2, 3, PARSER_FLOAT },
	{ "size",   2, 3, PARSER_FLOAT },
	{ NULL, 0, 0, 0 }
};

extern Parser_t parser;

static void drawLightObject(lpLight_t *p)
{
	lpObject_draw_refoverride(p->obj_ref, 2);
}

static void drawLightGraphics(lpLight_t *p)
{
	int i;
	int lod = 1;

	for (i = 0; i < 3; i++) {
		if (p->color[i] < 0. || p->color[i] > 1.)
			printf("drawLight: color out of range color[%d] = %f\n", i, p->color[i]);
	}

	// glColor3f(0., 0., 0.);
	glBegin(GL_POLYGON);

	for (i = 0; i < cir2d_nverts - 1; i += lod)
		glVertex2fv(&cir2d_data2[i][0]);

	glEnd();

	glBegin(GL_TRIANGLE_STRIP);

	for (i = 0; i < cir2d_nverts - 1; i += lod) {
		glColor3fv(&p->color[0]);
		glVertex2fv(&cir2d_data2[i][0]);
		glColor3f(0., 0., 0.);
		glVertex2fv(&cir2d_data[i][0]);
	}

	glColor3fv(&p->color[0]);
	glVertex2fv(&cir2d_data2[0][0]);
	glColor3f(0., 0., 0.);
	glVertex2fv(&cir2d_data[0][0]);
	glEnd();
}

static void sampleData8_error(lpLight_t *p)
{
	UNUSED(p);

#if 0
	static bool flag = false;

	if (!flag) {
		printf("sampleData8: light %s has no data bound to it.\n", p->name);
		flag = true;
	}
#endif
}

static void sampleData8(lpLight_t *p)
{
	unsigned char bit;
	uint8_t *ptr = (uint8_t *) p->dataptr;

	bit = (int) (*ptr >> p->bitnum) & 0x01;

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}
	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

static void sampleData8invert(lpLight_t *p)
{
	unsigned char bit;
	uint8_t *ptr = (uint8_t *) p->dataptr;

	bit = (int) ~(*ptr >> p->bitnum) & 0x01;

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}
	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

static void sampleData16(lpLight_t *p)
{
	unsigned char bit;
	uint16_t *ptr = (uint16_t *) p->dataptr;
#if 0
	uint64_t on_time_inc = 0;
#endif
	bit = (int) (*ptr >> p->bitnum) & 0x01;

#if 0
	if (bit) {
#if 0
		p->on_time += (*p->simclock - p->old_clock);
#endif
		on_time_inc = (*p->simclock - p->old_clock);
	}

	if (bit != p->state) {
		on_time_inc = on_time_inc >> 1;
	}
	p->on_time += on_time_inc;
#endif

	// if (p->old_clock > *p->simclock)
	// 	printf("sampleData16: clock stepped backward\n");

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}

	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

static void sampleDatafv(lpLight_t *p)
{
	float *ptr = (float *) p->dataptr;

	if (p->smoothing > 0) {
		p->intense_curr = p->intensity;
		p->intense_samples[p->intense_curr_idx] = ptr[p->bitnum];

		if (p->intense_samples[p->intense_curr_idx] > 1.0)
			p->intense_samples[p->intense_curr_idx] = 1.0;
		else if (p->intense_samples[p->intense_curr_idx] < 0.0)
			p->intense_samples[p->intense_curr_idx] = 0.0;

		p->intense_incr = (p->intense_samples[p->intense_curr_idx] -
				   p->intense_samples[!p->intense_curr_idx]) /
				  (float) p->smoothing;
	} else
		p->intensity = ptr[p->bitnum];
}

static void sampleData16invert(lpLight_t *p)
{
	unsigned char bit;
	uint16_t *ptr = (uint16_t *) p->dataptr;

	bit = (int) ~(*ptr >> p->bitnum) & 0x01;

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}
	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

static void sampleData32(lpLight_t *p)
{
	unsigned char bit;
	uint32_t *ptr = (uint32_t *) p->dataptr;

	bit = (int) (*ptr >> p->bitnum) & 0x01;

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}
	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

static void sampleData32invert(lpLight_t *p)
{
	unsigned char bit;
	uint32_t *ptr = (uint32_t *) p->dataptr;

	bit = (int) ~(*ptr >> p->bitnum) & 0x01;

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}
	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

static void sampleData64(lpLight_t *p)
{
	unsigned char bit;
	uint64_t *ptr = (uint64_t *) p->dataptr;

	bit = (int) (*ptr >> p->bitnum) & 0x01;

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}
	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

static void sampleData64invert(lpLight_t *p)
{
	unsigned char bit;
	uint64_t *ptr = (uint64_t *) p->dataptr;

	bit = (int) ~(*ptr >> p->bitnum) & 0x01;

	if (bit) {
		p->on_time += (*p->simclock - p->old_clock);
	}
	p->old_clock = *p->simclock;
	p->dirty = true;
	p->state = bit;
}

Lpanel_t *Lpanel_new(void)
{
	Lpanel_t *p = (Lpanel_t *) calloc(1, sizeof(Lpanel_t));

	if (p)
		Lpanel_init(p);

	return p;
}

void Lpanel_delete(Lpanel_t *p)
{
	if (p) {
		Lpanel_fini(p);
		free(p);
	}
}

void Lpanel_init(Lpanel_t *p)		// initializer
{
	int i;

#ifdef WANT_SDL
	p->window = NULL;
	p->cx = NULL;
#else
#if !defined(__MINGW32__) && !defined(_WIN32) && !defined(_WIN32_) && !defined(__WIN32__)
	p->window = 0;
	p->vi = NULL;
	p->cx = 0;
	p->dpy = 0;
#endif
#endif

	p->num_lights = 0;
	p->max_lights = 0;
	p->lights = NULL;

	p->num_switches = p->max_switches = 0;
	p->switches = NULL;
	p->mom_switch_pressed = NULL;

	p->default_clock = 0;
	p->old_clock = 0;
	p->simclock = &p->default_clock;
	p->clock_warp = 0;

	p->default_runflag = 0;
	p->runflag = &p->default_runflag;

	// init light groups

	for (i = 0; i < LP_MAX_LIGHT_GROUPS; i++) {
		p->light_groups[i].num_items = 0;
		p->light_groups[i].max_items = 0;
		p->light_groups[i].list = NULL;
	}

	// init light graphics

	p->lightcolor[0] = 1.;
	p->lightcolor[1] = 0.;
	p->lightcolor[2] = 0.;
	p->lightsize[0] = 0.1875;
	p->lightsize[1] = 0.1875;
	p->lightsize[2] = 0.1875;

	for (i = 0; i < cir2d_nverts; i++) {
		cir2d_data2[i][0] = cir2d_data[i][0] * .4;
		cir2d_data2[i][1] = cir2d_data[i][1] * .4;
	}

	// init config root path

	p->config_root_path = NULL;
	Lpanel_setConfigRootPath(p, ".");

	lpTextures_init(&p->textures);

	// init graphics objects

	p->envmap_detected = false;
	p->num_objects = 0;
	p->max_objects = 0;
	p->objects = NULL;

	p->num_alpha_objects = 0;
	p->max_alpha_objects = 0;
	p->alpha_objects = NULL;

	p->curr_object = NULL;
	p->curr_element = NULL;
	p->curr_vertex = NULL;

	lpBBox_init(&p->bbox);

	// use this for small displays like notebooks etc.
	p->window_xsize = 800;
	p->window_ysize = 325;

	// use this for Desktops/Workstations with good graphics
	// p->window_xsize = 1600;
	// p->window_ysize = 650;

	p->cursor[0] = 0.;
	p->cursor[1] = 0.;
	p->cursor[2] = 0.0;
	p->cursor_inc = .01;
	p->do_cursor = false;
	p->do_stats = false;
	p->shift_key_pressed = false;

	// graphics view parms

	p->view.rot[0] = 0.;
	p->view.rot[1] = 0.;
	p->view.rot[2] = 0.;

	p->view.pan[0] = 0.;
	p->view.pan[1] = -0.0;
	p->view.pan[2] = -16.0;

	p->view.znear = 0.01;
	p->view.zfar  = 1000.0;
	p->view.fovy  = 25.0;
	p->view.projection = LP_ORTHO;
	p->view.redo_projections = true;
	p->view.do_depthtest = false;	// default to no zbuffer

	p->quit_callbackfunc = NULL;
} // end initializer

void Lpanel_fini(Lpanel_t *p)		// finalizer
{
	int i;

	for (i = 0; i < p->num_lights; i++)
		if (p->lights[i])
			lpLight_delete(p->lights[i]);
	p->num_lights = p->max_lights = 0;

	for (i = 0; i < p->num_objects; i++)
		if (p->objects[i])
			lpObject_delete(p->objects[i]);
	p->num_objects = p->max_objects = 0;

	for (i = 0; i < p->num_switches; i++)
		if (p->switches[i])
			lpSwitch_delete(p->switches[i]);
	p->num_switches = p->max_switches = 0;

	for (i = 0; i < LP_MAX_LIGHT_GROUPS; i++) {
		if (p->light_groups[i].list) {
			free(p->light_groups[i].list);
			p->light_groups[i].list = NULL;
		}
		p->light_groups[i].num_items = 0;
		p->light_groups[i].max_items = 0;
	}

	lpTextures_fini(&p->textures);
	lpBBox_fini(&p->bbox);
} // end finalizer

int Lpanel_test(Lpanel_t *p, int n)
{
	UNUSED(p);

	printf("panel test %d\n", n);

	switch (n) {
	case 0:
		break;
	case 1:
		break;
	default:
		break;
	}

	return 1;
}

void Lpanel_addQuitCallback(Lpanel_t *p, lp_quit_cbf_t cbfunc)
{
	p->quit_callbackfunc = cbfunc;
}

int Lpanel_addLight(Lpanel_t *p, const char *name, lp_obj_parm_t *obj, const char *buff)
{
	int i, n;
	parser_result_t *result;
	lpLight_t *light;

	if (p->num_lights + 1 > p->max_lights)
		Lpanel_growLights(p);

	p->lights[p->num_lights] = light = lpLight_new();
	light->parms = obj;
	lpLight_setName(light, name);
	lpLight_bindSimclock(light, p->simclock, &p->clock_warp);
	light->panel = p;

	Parser_setRules(&parser, light_parse_rules);
	Parser_setParseString(&parser, buff);

	// parse config file line values such has position, color etc.
	// if n >= 0 it contains the char position in the line where an error
	// ocurred

	while ((n = Parser_parse(&parser, &result)) < 0) {
		if (n != PARSER_DONE) {

			if (!strcmp(light_parse_rules[result->cmd_idx].cmd, "color")) {
				for (i = 0; i < result->num_args; i++)
					light->parms->color[i] = result->floats[i];
			} else if (!strcmp(light_parse_rules[result->cmd_idx].cmd, "group")) {
				light->parms->group = result->ints[0];
			} else if (!strcmp(light_parse_rules[result->cmd_idx].cmd, "object")) {
				light->obj_refname =
					(char *) malloc(strlen(result->strings[0]) + 1);
				strcpy(light->obj_refname, result->strings[0]);
			} else if (!strcmp(light_parse_rules[result->cmd_idx].cmd, "pos")) {
				for (i = 0; i < result->num_args; i++)
					light->parms->pos[i] = result->floats[i];
			} else if (!strcmp(light_parse_rules[result->cmd_idx].cmd, "size")) {
				for (i = 0; i < result->num_args; i++)
					light->parms->scale[i] = result->floats[i];
			}
		} // end if (n != PARSER_DONE)

		if (n == PARSER_DONE)
			break;
	}

	if (n >= 0) {
		printf("n=%d\n", n);
		Parser_printError(&parser);
		return n;
	}

	if (obj->group >= 0) {
		Lpanel_addLightToGroup(p, p->num_lights, obj->group);
	}

	p->num_lights++;

	return -1;
}

int Lpanel_addLightToGroup(Lpanel_t *p, int lightnum, int groupnum)
{
	if (groupnum >= LP_MAX_LIGHT_GROUPS) {
		fprintf(stderr, "error: light %s invalid group number (%d)\n",
			p->lights[lightnum]->name, groupnum);
		return 0;
	}

	if (p->light_groups[groupnum].num_items + 1 > p->light_groups[groupnum].max_items) {
		int *new_list;

		new_list = (int *) realloc(p->light_groups[groupnum].list,
					   sizeof(int) *
					   (p->light_groups[groupnum].max_items + 1));
		p->light_groups[groupnum].max_items += 1;
		p->light_groups[groupnum].list = new_list;
	}
	p->light_groups[groupnum].list[p->light_groups[groupnum].num_items] = lightnum;
	p->light_groups[groupnum].num_items++;
	return 1;
}

bool Lpanel_bindLight8(Lpanel_t *p, const char *name, void *loc, int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight8: light %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			lpLight_bindData8(light, (uint8_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight8: light %s not found\n",
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

// bind light and invert logic according to mask

bool Lpanel_bindLight8invert(Lpanel_t *p, const char *name, void *loc, int start_bit_number,
			     uint8_t mask)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight8invert: light %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			if (mask & (0x1 << (bitnum - 1)))
				lpLight_bindData8invert(light, (uint8_t *) loc);
			else
				lpLight_bindData8(light, (uint8_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight8invert: light %s not found\n",
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

bool Lpanel_bindLight16(Lpanel_t *p, const char *name, void *loc, int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight16: light %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			lpLight_bindData16(light, (uint16_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight16: light %s not found\n",
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

// bindLightfv
// bind to an array of float values
// subsequent sampling will use the bit number as an index into the array of float values
// instead of a single bit

bool Lpanel_bindLightfv(Lpanel_t *p, const char *name, void *loc)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	for (i = 0; i < num_names; i++) {

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			lpLight_bindDatafv(light, (float *) loc);
			lpLight_setBitNumber(light, i);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLightfv: light %s not found\n",
					namelist[i]);
			status = false;
		}
		if (namelist[i])
			free(namelist[i]);
	}
	free(namelist);

	return status;
}

bool Lpanel_bindLight16invert(Lpanel_t *p, const char *name, void *loc, int start_bit_number,
			      uint16_t mask)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight16invert: light %s bad bitnum %d\n",
				namelist[i],
				bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			if (mask & (0x1 << (bitnum - 1)))
				lpLight_bindData16invert(light, (uint16_t *) loc);
			else
				lpLight_bindData16(light, (uint16_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight16invert: light %s not found\n",
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

bool Lpanel_bindLight32(Lpanel_t *p, const char *name, void *loc, int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight16: light %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			lpLight_bindData32(light, (uint32_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight32: light %s not found\n",
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

bool Lpanel_bindLight32invert(Lpanel_t *p, const char *name, void *loc, int start_bit_number,
			      uint32_t mask)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight32invert: light %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			if (mask & (0x1 << (bitnum - 1)))
				lpLight_bindData32invert(light, (uint32_t *) loc);
			else
				lpLight_bindData32(light, (uint32_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight32invert: light %s not found\n",
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

bool Lpanel_bindLight64(Lpanel_t *p, const char *name, void *loc, int start_bit_number)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight64: light %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			lpLight_bindData64(light, (uint64_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight64: light %s not found\n",
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

bool Lpanel_bindLight64invert(Lpanel_t *p, const char *name, void *loc, int start_bit_number,
			      uint64_t mask)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	int bitnum, bit_inc;
	lpLight_t *light;

	num_names = xpand(name, &namelist);

	bitnum = abs(start_bit_number);
	if (start_bit_number > 0)
		bit_inc = 1;
	else
		bit_inc = -1;

	for (i = 0; i < num_names; i++) {
		if (bitnum <= 0) {
			fprintf(stderr, "bindLight64invert: light %s bad bitnum %d\n",
				namelist[i], bitnum);
			bitnum = 1;
		}

		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			if (mask & (0x1 << (bitnum - 1)))
				lpLight_bindData64invert(light, (uint64_t *) loc);
			else
				lpLight_bindData64(light, (uint64_t *) loc);
			lpLight_setBitNumber(light, bitnum - 1);
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "bindLight64invert: light %s not found\n",
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

void Lpanel_bindRunFlag(Lpanel_t *p, uint8_t *addr)
{
	int i;

	p->runflag = (uint8_t *) addr;

	for (i = 0; i < p->num_lights; i++)
		lpLight_bindRunFlag(p->lights[i], (uint8_t *) addr);
}

void Lpanel_bindSimclock(Lpanel_t *p, uint64_t *addr)
{
	int i;

	p->simclock = (uint64_t *) addr;

	for (i = 0; i < p->num_lights; i++)
		lpLight_bindSimclock(p->lights[i], (uint64_t *) addr, &p->clock_warp);

}

void Lpanel_draw(Lpanel_t *p)
{
	int i;

#ifdef WANT_SDL
	SDL_GL_MakeCurrent(p->window, p->cx);
#endif

	if (p->view.redo_projections) {
		Lpanel_setProjection(p, false);
		Lpanel_setModelview(p, false);
		p->view.redo_projections = false;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw graphics objects

	for (i = 0; i < p->num_objects; i++)
		if (!p->objects[i]->is_alpha)
			lpObject_draw(p->objects[i]);

	// draw lights

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0., -10.);

	for (i = 0; i < p->num_lights; i++)
		lpLight_draw(p->lights[i]);

	// draw switches

	for (i = 0; i < p->num_switches; i++)
		p->switches[i]->drawFunc(p->switches[i]);

	if (p->alpha_objects) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		for (i = 0; i < p->num_alpha_objects; i++)
			lpObject_draw(p->alpha_objects[i]);

		glDisable(GL_BLEND);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);

	glEnable(GL_POLYGON_OFFSET_LINE);
	if (p->do_cursor)
		Lpanel_draw_cursor(p);
	if (p->do_stats)
		Lpanel_draw_stats(p);
	glDisable(GL_POLYGON_OFFSET_LINE);

#ifdef WANT_SDL
	SDL_GL_SwapWindow(p->window);
#else
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
	SwapBuffers(p->hDC);
	// UpdateWindow(p->hWnd);
#else
	glXSwapBuffers(p->dpy, p->window);
#endif
#endif
}

void Lpanel_growLights(Lpanel_t *p)
{
	lpLight_t **new_lights;

	new_lights = (lpLight_t **) realloc(p->lights,
					    sizeof(lpLight_t *) * (p->num_lights + 8));
	p->max_lights += 8;
	p->lights = new_lights;
}

void Lpanel_growSwitches(Lpanel_t *p)
{
	lpSwitch_t **new_switches;

	new_switches = (lpSwitch_t **) realloc(p->switches,
					       sizeof(lpSwitch_t *) * (p->num_switches + 1));
	p->max_switches += 1;
	p->switches = new_switches;
}

int Lpanel_pick(Lpanel_t *p, int button, int state, int x, int y)
{
	GLuint namebuf[500], *ptr;
	int i,
	    num_picked = 0,
	    switch_dir;
	uint32_t switch_num;

	UNUSED(button);

	if (state == 0) {
		if (p->mom_switch_pressed)
			lpSwitch_action(p->mom_switch_pressed, 2);
		return num_picked;
	}

#ifdef WANT_SDL
	SDL_GL_MakeCurrent(p->window, p->cx);
#endif

	namebuf[0] = 0;
	glSelectBuffer(500, &namebuf[0]);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	glGetIntegerv(GL_VIEWPORT, p->viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	// gluPickMatrix ((GLdouble) x, (GLdouble) (window_ysize - y), 1.0, 1.0, viewport);
	glTranslatef(p->viewport[2] - 2 * (x - p->viewport[0]),
		     p->viewport[3] - 2 * (p->window_ysize - y - p->viewport[1]), 0);
	glScalef(p->viewport[2], p->viewport[3], 1.0);
	Lpanel_doPickProjection(p);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	Lpanel_doPickModelview(p);

	// draw switches

	for (i = 0; i < p->num_switches; i++)
		lpSwitch_drawForPick(p->switches[i]);

	glPopName();
	num_picked = glRenderMode(GL_RENDER);

	if (num_picked) {
		uint32_t n;

		ptr = (GLuint *) namebuf;
		n = ptr[3];

		switch_num = n & LP_SW_PICK_IDMASK;		// decode the switch number
		switch_dir = ((n & LP_SW_PICK_UP_BIT) != 0);
		// printf("pick: switch_num=%d dir=%d\n", switch_num, switch_dir);

		lpSwitch_action(p->switches[switch_num], switch_dir);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glRenderMode(GL_RENDER);

	return num_picked;
}

#define BUFSIZE 256
#define TOKENSIZE 80

bool Lpanel_readConfig(Lpanel_t *p, const char *_fname)
{
	FILE *fd;
	int i, n;
	char buffer[BUFSIZE], token[TOKENSIZE];
	int pos, lineno = 0;
	bool bailout = false;
	lp_obj_parm_t *obj;
	char *fname;

	fname = (char *) malloc(strlen(p->config_root_path) + 1 + strlen(_fname) + 1);
	strcpy(fname, p->config_root_path);
	strcat(fname, "/");
	strcat(fname, _fname);

	if ((fd = fopen(fname, "r")) == 0) {
		fprintf(stderr, "readFile: could not open file %s\n", fname);
		free(fname);
		return 0;
	}

	lp_init_materials();

	while (!feof(fd) && !bailout) {
		lineno++;
		if (!freadlin(fd, buffer, BUFSIZE))
			continue;
		pos = 0;
		if (!gtoken(buffer, token, TOKENSIZE, &pos))	// blank line
			continue;
		if (token[0] == '#')		// comment
			continue;

		if (!strcmp(token, "color")) {	// color
			int n;

			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("color defined outside of an object\n");
				bailout = true;
			} else {
				n = sscanf(&buffer[pos], "%f %f %f",
					   &p->curr_object->color[0],
					   &p->curr_object->color[1],
					   &p->curr_object->color[2]);

				if (n < 3) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("color must have 3 (r g b) values in the "
					       "range of 0.0 - 1.0.\n");
					bailout = true;
				}
			}
		} else if (!strcmp(token, "zbuffer")) {
			p->view.do_depthtest = true;
		} else if (!strcmp(token, "envmap")) {	// environment mapped reflection
			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("envmap defined outside of an object\n");
				bailout = true;
			} else {
				p->envmap_detected = true;
				p->curr_object->envmapped = true;
			}
		} else if (!strcmp(token, "instance")) {
			char s[100];

			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("instance defined outside of an object\n");
				bailout = true;
			} else {
				if (!gtoken(buffer, s, 100, &pos)) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("instance with no object name.\n");
					bailout = true;
				} else
					lpObject_setInstanceName(p->curr_object, s);
			}
		} else if (!strcmp(token, "light")) {
			obj = (lp_obj_parm_t *) malloc(sizeof(lp_obj_parm_t));
			obj->type = LP_LED;
			obj->subtype = LP_LED_3D;
			obj->group = -1;		// does not belong to group
			obj->pos[0] = 0.;
			obj->pos[1] = 0.;
			obj->pos[2] = 0.;
			obj->color[0] = p->lightcolor[0];
			obj->color[1] = p->lightcolor[1];
			obj->color[2] = p->lightcolor[2];
			obj->scale[0] = p->lightsize[0];
			obj->scale[1] = p->lightsize[1];
			obj->scale[2] = p->lightsize[2];

			gtoken(buffer, token, TOKENSIZE, &pos);		// get name

			n = Lpanel_addLight(p, token, obj, &buffer[pos]);
			if (n >= 0) {
				bailout = true;
				n += pos;
				printf("Error on line %d of config file %s\n", lineno, fname);

				printf("%s\n", buffer);
				for (i = 0; i < n; i++)
					putchar(' ');
				printf("^\n");
			}
		} else if (!strcmp(token, "lightcolor")) {	// default light size
			int n;

			n = sscanf(&buffer[pos], "%f %f %f",
				   &p->lightcolor[0],
				   &p->lightcolor[1],
				   &p->lightcolor[2]);

			if (n < 3) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("lightcolor must have 3 (r g b) values in the "
				       "range of 0.0 - 1.0.\n");
				bailout = true;
			}
		} else if (!strcmp(token, "lightsize")) {	// default light size
			int n;

			n = sscanf(&buffer[pos], "%f %f %f",
				   &p->lightsize[0],
				   &p->lightsize[1],
				   &p->lightsize[2]);

			if (n < 1) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("lightsize must have 1, 2, or 3 values (x y z).\n");
				bailout = true;
			}

			if (n == 1)
				p->lightsize[1] = p->lightsize[2] = p->lightsize[0];
			else if (n == 2)
				p->lightsize[2] = p->lightsize[0];
		} else if (!strcmp(token, "line")) {
			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("line defined outside of an object\n");
				bailout = true;
			} else {
				if (!(p->curr_element = lpObject_addElement(p->curr_object,
									    p->curr_object))) {
					printf("could not allocate memory for line.\n");
					bailout = true;
				} else {
					p->curr_element->type = LP_LINE;
					lpElement_setTextureManager(p->curr_element, &p->textures);
				}
			}
		} else if (!strcmp(token, "material")) {	// material reference or definition
			int n, matnum;
			float Ar, Ag, Ab, Aa,	// Ambient rgba
			      Dr, Dg, Db, Da,	// Diffuse rgba
			      Sr, Sg, Sb, Sa,	// Specular rgba
			      shine,		// shinyness
			      Er, Eg, Eb, Ea;	// emission rgba

			if (p->curr_object) {	// material reference
				n = sscanf(&buffer[pos], "%d", &p->curr_object->material);

				if (n != 1) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("material within an object definition must have "
					       "one integer value.\n");
					bailout = true;
				}
				p->curr_object->is_alpha =
					lp_is_material_alpha(p->curr_object->material);
			} else {	// material definition

				n = sscanf(&buffer[pos],
					   "%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
					   &matnum, &Ar, &Ag, &Ab, &Aa, &Dr, &Dg, &Db, &Da,
					   &Sr, &Sg, &Sb, &Sa, &shine, &Er, &Eg, &Eb, &Ea);

				if (n < 18) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("material definition must have 18 values.\n");
					bailout = true;
				} else
					lp_set_material_params(matnum, Ar, Ag, Ab, Aa,
							       Dr, Dg, Db, Da,
							       Sr, Sg, Sb, Sa,
							       shine, Er, Eg, Eb, Ea);
			}
		} else if (!strcmp(token, "message")) {	// message
			printf("%s\n", &buffer[pos]);
		} else if (!strcmp(token, "n")) {	// vertex normal vector
			int n;

			if (!p->curr_element || !p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("normal defined outside of a polygon or line\n");
				bailout = true;
			} else {
				if (!p->curr_vertex) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("normal defined before a vertex definition\n");
					bailout = true;
				}

				n = sscanf(&buffer[pos], "%f %f %f",
					   &p->curr_vertex->norm[0],
					   &p->curr_vertex->norm[1],
					   &p->curr_vertex->norm[2]);

				if (n < 3) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("normal must have 3 values.\n");
					bailout = true;
				} else
					p->curr_object->have_normals = true;
			}
		} else if (!strcmp(token, "object")) {
			if (!(p->curr_object = Lpanel_addObject(p))) {
				printf("could not allocate memory for object.\n");
				bailout = true;
			} else {
				lpObject_setTextureManager(p->curr_object, &p->textures);
				if (gtoken(buffer, token, TOKENSIZE, &pos))
					lpObject_setName(p->curr_object, token);
				p->curr_element = NULL;
				p->curr_vertex = NULL;
			}
		} else if (!strcmp(token, "perspective")) {
			p->view.projection = LP_PERSPECTIVE;
		} else if (!strcmp(token, "polygon")) {
			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("polygon defined outside of an object\n");
				bailout = true;
			} else {
				if (!(p->curr_element = lpObject_addElement(p->curr_object,
									    p->curr_object))) {
					printf("could not allocate memory for polygon.\n");
					bailout = true;
				} else {
					p->curr_element->type = LP_POLYGON;
					lpElement_setTextureManager(p->curr_element, &p->textures);
				}
			}
		} else if (!strcmp(token, "referenced")) {	// referenced
			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("color defined outside of an object\n");
				bailout = true;
			} else
				p->curr_object->referenced = true;
		} else if (!strcmp(token, "rotate")) {	// rotate hpr
			int n;

			if (!p->curr_object) {
				n = sscanf(&buffer[pos], "%f %f",
					   &p->view.rot[0],
					   &p->view.rot[1]);

				if (n < 2) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("scene rotate must have 2 values.\n");
					bailout = true;
				}
			} else {
				n = sscanf(&buffer[pos], "%f %f %f",
					   &p->curr_object->rotate[0],
					   &p->curr_object->rotate[1],
					   &p->curr_object->rotate[2]);

				if (n < 3) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("object rotate must have 3 values.\n");
					bailout = true;
				}
			}
		} else if (!strcmp(token, "switch")) {
			obj = (lp_obj_parm_t *) malloc(sizeof(lp_obj_parm_t));
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

			gtoken(buffer, token, TOKENSIZE, &pos);		// get name

			n = Lpanel_addSwitch(p, token, obj, &buffer[pos]);
			if (n >= 0) {
				bailout = true;
				n += pos;
				printf("Error on line %d of config file %s\n", lineno, fname);

				printf("%s\n", buffer);
				for (i = 0; i < n; i++)
					putchar(' ');
				printf("^\n");
			}
		} else if (!strcmp(token, "t")) {	// texture coordinate
			int n;

			if (!p->curr_element || !p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("texture coord defined outside of a polygon or line\n");
				bailout = true;
				break;
			} else {
				if (!p->curr_vertex) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("texture coord defined before a vertex coordinate "
					       "was defined.\n");
					bailout = true;
					break;
				}

				n = sscanf(&buffer[pos], "%f %f",
					   &p->curr_vertex->st[0],
					   &p->curr_vertex->st[1]);

				if (n < 2) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("texture coordinate must have 2 coordinates "
					       "(e.g. t <s> <t>).\n");
					bailout = true;
					break;
				}
				p->curr_element->have_tcoords = true;
			}
		} else if (!strcmp(token, "texture")) {	// texture
			char *texture_path = NULL;
			int len;

			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("texture defined outside of an object.\n");
				bailout = true;
				break;
			}

			if (!gtoken(buffer, token, TOKENSIZE, &pos)) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("texture with no path to jpeg file defined.\n");
				bailout = true;
				break;
			}

			len = strlen(p->config_root_path) + strlen(token) + 1;

			texture_path = (char *) malloc(len + 1);
			strcpy(texture_path, p->config_root_path);
			strcat(texture_path, "/");
			strcat(texture_path, token);
			texture_path[len] = 0;

			if (!(p->curr_object->texture_num = lpTextures_addTexture(&p->textures,
										  texture_path))) {
				printf("Error on line %d of config file %s\n",
				       lineno, texture_path);
				printf("could not load texture '%s'.\n", texture_path);
				bailout = true;
				break;
			}

			free(texture_path);
		} else if (!strcmp(token, "texture_scale")) {
			int n;

			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("texture_scale defined outside of an object\n");
				bailout = true;
			} else {
				n = sscanf(&buffer[pos], "%f %f",
					   &p->curr_object->texture_scale[0],
					   &p->curr_object->texture_scale[1]);

				if (n < 2) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("texture_scale must have 2 (x y) values.\n");
					bailout = true;
				}
			}
		} else if (!strcmp(token, "texture_translate")) {
			int n;

			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("texture_translate defined outside of an object\n");
				bailout = true;
			} else {
				n = sscanf(&buffer[pos], "%f %f",
					   &p->curr_object->texture_translate[0],
					   &p->curr_object->texture_translate[1]);

				if (n < 2) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("texture_translate must have 2 (x y) values.\n");
					bailout = true;
				}
			}
		} else if (!strcmp(token, "translate")) {
			int n;

			if (!p->curr_object) {
				n = sscanf(&buffer[pos], "%f %f %f",
					   &p->view.pan[0],
					   &p->view.pan[1],
					   &p->view.pan[2]);

				if (n < 3) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("scene translate must have 3 values.\n");
					bailout = true;
				}
			}
		} else if (!strcmp(token, "tristrip")) {
			if (!p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("tristrip defined outside of an object\n");
				bailout = true;
			} else {
				if (!(p->curr_element = lpObject_addElement(p->curr_object,
									    p->curr_object))) {
					printf("could not allocate memory for tristrip.\n");
					bailout = true;
				} else {
					p->curr_element->type = LP_TRISTRIP;
					lpElement_setTextureManager(p->curr_element, &p->textures);
				}
			}
		} else if (!strcmp(token, "v")) {	// vertex
			int n;

			if (!p->curr_element || !p->curr_object) {
				printf("Error on line %d of config file %s\n", lineno, fname);
				printf("vertex defined outside of a polygon or line\n");
				bailout = true;
			} else {
				if (!(p->curr_vertex = lpElement_addVertex(p->curr_element))) {
					printf("could not allocate memory for vertex.\n");
					bailout = true;
				}

				n = sscanf(&buffer[pos], "%f %f %f",
					   &p->curr_vertex->xyz[0],
					   &p->curr_vertex->xyz[1],
					   &p->curr_vertex->xyz[2]);

				if (n < 2) {
					printf("Error on line %d of config file %s\n",
					       lineno, fname);
					printf("vertex must have 2 or 3 coordinates.\n");
					bailout = true;
				}
			}
		} else {
			printf("Error: unknown statement on line %d of config file %s\n",
			       lineno, fname);
			bailout = true;

		}
	} // end while(!feof)

	free(fname);
	fclose(fd);

	if (!bailout) {
		Lpanel_genGraphicsData(p);
		Lpanel_resolveObjectInstances(p);

		for (i = 0; i < p->num_lights; i++)
			lpLight_setupData(p->lights[i]);

		for (i = 0; i < p->num_switches; i++)
			lpSwitch_setupData(p->switches[i], i);
	}
	return !bailout;
} // end Lpanel_readConfig()

lpLight_t *Lpanel_findLightByName(Lpanel_t *p, char *name)
{
	int i;

	for (i = 0; i < p->num_lights; i++) {
		if (!strcmp(p->lights[i]->name, name))
			return p->lights[i];
	}

	return NULL;
}

void Lpanel_printLights(Lpanel_t *p)
{
	int i;

	printf("lights:\n");
	for (i = 0; i < p->num_lights; i++)
		lpLight_print(p->lights[i]);
}

void Lpanel_resolveObjectInstances(Lpanel_t *p)
{
	int i, j;
	lpObject_t *obj;

	for (i = 0; i < p->num_objects; i++) {
		if (p->objects[i]->instance_name) {
			if ((obj = Lpanel_findObjectByName(p, p->objects[i]->instance_name)))
				p->objects[i]->instance_object = obj;
			else
				fprintf(stderr, "Error: object %s instances object %s which "
					"cannot be found.\n",
					p->objects[i]->name, p->objects[i]->instance_name);
		}
	}

	for (i = 0; i < p->num_objects; i++) {
		obj = p->objects[i]->instance_object;
		while (obj) {
			for (j = 0; j < 3; j++) {
				p->objects[i]->bbox.xyz_min[j] =
					min(p->objects[i]->bbox.xyz_min[j], obj->bbox.xyz_min[j]);
				p->objects[i]->bbox.xyz_max[j] =
					max(p->objects[i]->bbox.xyz_max[j], obj->bbox.xyz_max[j]);
			}
			obj = obj->instance_object;
		}
	}
}

void Lpanel_sampleData(Lpanel_t *p)
{
	int i;

	if (*p->simclock < p->old_clock) {
		fprintf(stderr, "libfrontpanel: Warning clock went backwards (current=%" PRIu64
			" previous=%" PRIu64 ".\n", *p->simclock, p->old_clock);

	}
	p->old_clock = *p->simclock;

	for (i = 0; i < p->num_lights; i++)
		lpLight_sampleData(p->lights[i]);
}

void Lpanel_sampleDataWarp(Lpanel_t *p, int clockwarp)
{
	int i;

	p->clock_warp = clockwarp;

	for (i = 0; i < p->num_lights; i++)
		lpLight_sampleData(p->lights[i]);

	p->clock_warp = 0;
}

void Lpanel_sampleLightGroup(Lpanel_t *p, int groupnum, int clockval)
{
	int i;

	if (groupnum < 0 || groupnum >= LP_MAX_LIGHT_GROUPS) {
		fprintf(stderr, "sampleLightGroup: groupnum (%d) must be in the "
			"range of (0-%d).\n", groupnum, LP_MAX_LIGHT_GROUPS - 1);
	}

	p->clock_warp = clockval;

	for (i = 0; i < p->light_groups[groupnum].num_items; i++)
		lpLight_sampleData(p->lights[p->light_groups[groupnum].list[i]]);

	p->clock_warp = 0;
}

void Lpanel_setConfigRootPath(Lpanel_t *p, const char *path)
{
	if (p->config_root_path)
		free(p->config_root_path);
	p->config_root_path = (char *) malloc(strlen(path) + 1);
	strcpy(p->config_root_path, path);
}

void Lpanel_framerate_set(Lpanel_t *p, float n)
{
	p->framerate = n;
	p->frametime = 1. / n;
};

void Lpanel_ignoreBindErrors(Lpanel_t *p, bool f)
{
	p->ignore_bind_errors = f;
};

// -------------
// lpLight class
// -------------

lpLight_t *lpLight_new(void)
{
	lpLight_t *p = (lpLight_t *) calloc(1, sizeof(lpLight_t));

	if (p)
		lpLight_init(p);

	return p;
}

void lpLight_delete(lpLight_t *p)
{
	if (p) {
		lpLight_fini(p);
		free(p);
	}
}

void lpLight_init(lpLight_t *p)
{
	p->bindtype = LBINDTYPE_BIT;	// default to bind to bit
	p->parms = NULL;
	p->name  = NULL;
	p->obj_refname = NULL;
	p->obj_ref = NULL;
	p->sampleDataFunc = sampleData8_error;
	p->drawFunc = drawLightGraphics;
	p->t1 = p->t2 = p->on_time = 1;
	p->start_clock = 0;
	p->old_clock = 0;
	p->dirty = false;
	p->state = 0;
	p->old_state = 0;
	p->intensity = 1.;

	p->smoothing = 0;		// no intensity smoothing for bindtype of FLOATV
	p->intense_curr_idx = 0;
	p->intense_samples[0] = 0.;
	p->intense_samples[1] = 0.;
	p->color[0] = 0.2;
	p->color[1] = 0.;
	p->color[2] = 0.;
	p->default_runflag = 0;
	p->runflag = &p->default_runflag;
}

void lpLight_fini(lpLight_t *p)
{
	if (p->name)
		free(p->name);
	if (p->obj_refname)
		free(p->obj_refname);
	if (p->parms)
		free(p->parms);
}

void lpLight_bindRunFlag(lpLight_t *p, uint8_t *addr)
{
	p->runflag = (uint8_t *) addr;
}

void lpLight_bindSimclock(lpLight_t *p, uint64_t *addr, int *clockwarp)
{
	p->simclock = addr;
	p->clock_warp = clockwarp;
}

void lpLight_draw(lpLight_t *p)
{
	int i;
	// float *fp;

	switch (p->bindtype) {

	case LBINDTYPE_BIT:
		if (*p->runflag) {
			if (p->dirty)
				lpLight_calcIntensity(p);
		} else {
			for (i = 0; i < 3; i++) {
				p->color[i] = p->parms->color[i] * (float) p->state +
					      (p->parms->color[i] * .2);
				p->color[i] = min(p->color[i], 1.0);
			}
		}
		break;

	case LBINDTYPE_FLOATV:
		// fp = (float *) p->dataptr;
		// p->intensity = fp[p->bitnum];
		// if (p->intensity > 1.0)
		// 	p->intensity = 1.0;

		if (p->smoothing) {
			p->intense_curr += p->intense_incr;
			if (p->intense_curr > 1.0)
				p->intense_curr = 1.0;
			else if (p->intense_curr < 0.0)
				p->intense_curr = 0.0;
			p->intensity = p->intense_curr;
		}

		for (i = 0; i < 3; i++) {
			p->color[i] = p->parms->color[i] * p->intensity +
				      (p->parms->color[i] * .2);
			p->color[i] = min(p->color[i], 1.0);
		}
		// printf("xyzzy: intense=%f bitnum=%d\n", p->intensity, p->bitnum);
		break;
	}

#if DEBUG
#if 0
	if (!strcmp(p->name, "LED_ADDR_00") ||
	    !strcmp(p->name, "LED_ADDR_01") ||
	    !strcmp(p->name, "LED_ADDR_10"))
#endif
		if (!strcmp(p->name, "LED_ADDR_15")) {
			fprintf(stderr, "draw: %s %f\n", p->name, p->intensity);
		}
#endif

	glPushMatrix();
	glTranslatef(p->parms->pos[0], p->parms->pos[1], p->parms->pos[2]);
	glScalef(p->parms->scale[0], p->parms->scale[1], p->parms->scale[2]);

	glColor3fv(&p->color[0]);

	(*p->drawFunc)(p);

	glPopMatrix();
}

void lpLight_print(lpLight_t *p)
{
	printf("light: name=%s\n", p->name);

	printf("obj: pos=%f %f %f size=%f %f %f color=%f %f %f\n",
	       p->parms->pos[0],
	       p->parms->pos[1],
	       p->parms->pos[2],
	       p->parms->scale[0],
	       p->parms->scale[1],
	       p->parms->scale[2],
	       p->parms->color[0],
	       p->parms->color[1],
	       p->parms->color[2]);
}

void lpLight_bindData8(lpLight_t *p, uint8_t *ptr)
{
	p->sampleDataFunc = sampleData8;
	p->dataptr = (uint8_t *) ptr;
}

void lpLight_bindData8invert(lpLight_t *p, uint8_t *ptr)
{
	p->sampleDataFunc = sampleData8invert;
	p->dataptr = (uint8_t *) ptr;
}

void lpLight_bindData16(lpLight_t *p, uint16_t *ptr)
{
	// xyzzy
	p->sampleDataFunc = sampleData16;
	p->dataptr = (uint16_t *) ptr;
}

void lpLight_bindDatafv(lpLight_t *p, float *ptr)
{
	p->sampleDataFunc = sampleDatafv;
	p->dataptr = (float *) ptr;
	p->bindtype = LBINDTYPE_FLOATV;
}

void lpLight_bindData16invert(lpLight_t *p, uint16_t *ptr)
{
	p->sampleDataFunc = sampleData16invert;
	p->dataptr = (uint16_t *) ptr;
}

void lpLight_bindData32(lpLight_t *p, uint32_t *ptr)
{
	p->sampleDataFunc = sampleData32;
	p->dataptr = (uint32_t *) ptr;
}

void lpLight_bindData32invert(lpLight_t *p, uint32_t *ptr)
{
	p->sampleDataFunc = sampleData32invert;
	p->dataptr = (uint32_t *) ptr;
}

void lpLight_bindData64(lpLight_t *p, uint64_t *ptr)
{
	p->sampleDataFunc = sampleData64;
	p->dataptr = (uint64_t *) ptr;
}

void lpLight_bindData64invert(lpLight_t *p, uint64_t *ptr)
{
	p->sampleDataFunc = sampleData64invert;
	p->dataptr = (uint64_t *) ptr;
}

void lpLight_calcIntensity(lpLight_t *p)
{
	int i;
	// unsigned int dt;
	uint64_t clock_delta;

	clock_delta = p->old_clock - p->start_clock;
	if (clock_delta == 0) {
		// p->intensity = 0.;
		return;
	} else
		p->intensity = (float) ((double) p->on_time / (double) clock_delta * 2.0);
	if (p->intensity > 1.0)
		p->intensity = 1.0;

#if 0
	// printf("calcIntensity runflag = %d\n", *p->runflag);
	if (*p->runflag)
		// if (!strcmp(p->name, "LED_ADDR_15") && p->intensity > .2)
		if (!strcmp(p->name, "LED_ADDR_15")) {
			fprintf(stderr, "dbg: draw: %s intense=%f on_time=%d old_clock=%d "
				"start_clock=%d old-start=%d\n",
				p->name, p->intensity, p->on_time, p->old_clock,
				p->start_clock, p->old_clock - p->start_clock);
		}
#endif

	p->start_clock = *p->simclock;
	p->on_time = 0;
	p->dirty = false;

	for (i = 0; i < 3; i++) {
		p->color[i] = p->parms->color[i] * p->intensity + p->parms->color[i] * .2;
		// p->color[i] = min(p->color[i], 1.0);
		if (p->color[i] > 1.0)
			p->color[i] = 1.0;
	}
}

void lpLight_sampleData(lpLight_t *p)
{
	(*p->sampleDataFunc)(p);
}

void lpLight_setupData(lpLight_t *p)
{
	if (p->obj_refname) {
		if (!(p->obj_ref = Lpanel_findObjectByName(p->panel, p->obj_refname))) {
			fprintf(stderr, "error: light %s references object %s which "
				"cannot be found.\n", p->name, p->obj_refname);
			return;
		}

		p->drawFunc = drawLightObject;
	}
}

void lpLight_setName(lpLight_t *p, const char *name)
{
	int n;

	n = strlen(name);
	p->name = (char *) malloc(n + 1);
	strcpy(p->name, name);
}

void lpLight_setBitNumber(lpLight_t *p, int bitnum)
{
	p->bitnum = bitnum;
};

bool Lpanel_smoothLight(Lpanel_t *p, const char *name, int nframes)
{
	char **namelist;
	int num_names;
	int i;
	bool status = true;
	lpLight_t *light;

	if (nframes < 0) {
		fprintf(stderr, "smoothLight: light %s error. nframes = %d must be >=0 \n",
			name, nframes);
		return false;
	}

	num_names = xpand(name, &namelist);

	for (i = 0; i < num_names; i++) {
		light = Lpanel_findLightByName(p, namelist[i]);

		if (light) {
			light->smoothing = nframes;
		} else {
			if (!p->ignore_bind_errors)
				fprintf(stderr, "smoothLight: light %s not found\n", namelist[i]);
			status = false;
		}
		if (namelist[i])
			free(namelist[i]);
	}
	free(namelist);

	return status;
}
