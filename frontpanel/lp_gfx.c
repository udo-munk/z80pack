// lp_gfx.c	lightpanel graphics

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WANT_SDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include "jpeg.h"
#endif

#include "lpanel.h"
#include "lp_utils.h"
#include "lp_gfx.h"
#include "lp_materials.h"

#define UNUSED(x) (void) (x)

// static GLfloat mtl2_amb[] = { 0.2, 0.2, 0.2, 1.0 };
// static GLfloat mtl2_dif[] = { 1.0, 1.0, 1.0, 1.0 };
// static GLfloat mtl2_spec[] = { 0.0, 0.0, 0.0, 1.0 };
// static GLfloat mtl2_shine[] = { 0.0 };
// static GLfloat mtl2_emission[] = { 0.0, 0.0, 0.0, 1.0 };

lpObject_t *Lpanel_addObject(Lpanel_t *p)
{
	lpObject_t *obj;

	if (p->num_objects + 1 > p->max_objects)
		Lpanel_growObjects(p);
	obj = lpObject_new();

	if (obj) {
		obj->color[0] = .1;
		obj->color[1] = .1;
		obj->color[2] = .1;
		obj->referenced = false;
		p->objects[p->num_objects] = obj;
		p->num_objects++;
	}
	return obj;
}

void Lpanel_addAlphaObject(Lpanel_t *p, lpObject_t *obj)
{
	if (p->num_alpha_objects + 1 > p->max_alpha_objects)
		Lpanel_growAlphaObjects(p);

	if (obj) {
		p->alpha_objects[p->num_alpha_objects] = obj;
		p->num_alpha_objects++;
	}
}

lpObject_t *Lpanel_findObjectByName(Lpanel_t *p, char *name)
{
	int i;

	for (i = 0; i < p->num_objects; i++)
		if (p->objects[i]->name)
			if (!strcmp(name, p->objects[i]->name))
				return p->objects[i];

	return NULL;
}

void Lpanel_genGraphicsData(Lpanel_t *p)
{
	int i, j, first;

	// generate data such as geometric extents, texture coords etc.
	// for all graphics objects

	for (i = 0; i < p->num_objects; i++)
		lpObject_genGraphicsData(p->objects[i]);

	// get bounding box for all geometry and put any alpha objects that are
	// not referenced on the alpha list

	first = 1;

	for (i = 0; i < p->num_objects; i++) {
		if (p->objects[i]->referenced)
			continue;

		if (p->objects[i]->is_alpha)
			Lpanel_addAlphaObject(p, p->objects[i]);

		if (first) {
			for (j = 0; j < 3; j++) {
				p->bbox.xyz_min[j] = p->objects[i]->bbox.xyz_min[j];
				p->bbox.xyz_max[j] = p->objects[i]->bbox.xyz_max[j];
			}
			first = 0;
		} else {
			for (j = 0; j < 3; j++) {
				p->bbox.xyz_min[j] = min(p->bbox.xyz_min[j],
							 p->objects[i]->bbox.xyz_min[j]);
				p->bbox.xyz_max[j] = max(p->bbox.xyz_max[j],
							 p->objects[i]->bbox.xyz_max[j]);
			}
		}
	}

	for (j = 0; j < 3; j++)
		p->bbox.center[j] = (p->bbox.xyz_min[j] + p->bbox.xyz_max[j]) / 2.0f;
}

void Lpanel_growObjects(Lpanel_t *p)
{
	lpObject_t **new_objects;

	new_objects = (lpObject_t **) realloc(p->objects,
					      sizeof(lpObject_t *) * (p->num_objects + 1));
	p->max_objects += 1;
	p->objects = new_objects;
}

void Lpanel_growAlphaObjects(Lpanel_t *p)
{
	lpObject_t **new_alpha_objects;

	new_alpha_objects = (lpObject_t **) realloc(p->alpha_objects,
						    sizeof(lpObject_t *) *
						    (p->num_alpha_objects + 1));
	p->max_alpha_objects += 1;
	p->alpha_objects = new_alpha_objects;
}

// ---------------------
// object class

lpObject_t *lpObject_new(void)
{
	lpObject_t *p = (lpObject_t *) calloc(1, sizeof(lpObject_t));

	if (p)
		lpObject_init(p);

	return p;
}

void lpObject_delete(lpObject_t *p)
{
	if (p) {
		lpObject_fini(p);
		free(p);
	}
}

void lpObject_init(lpObject_t *p)		// initializer
{
	int i;

	p->num_elements = p->max_elements = 0;
	p->elements = NULL;
	p->name = NULL;
	p->instance_name = NULL;
	p->instance_object = NULL;
	p->have_normals = false;
	p->material = 0;
	p->is_alpha = false;
	p->texture_num = 0;
	p->textures = NULL;
	p->envmapped = false;
	p->texture_scale[0] = p->texture_scale[1] = 1.0;
	p->texture_translate[0] = p->texture_translate[1] = 0.0;

	for (i = 0; i < 3; i++) {
		p->rotate[i] = 0.;
		p->translate[i] = 0.;
		p->origin[i] = 0.;
	}

	lpBBox_init(&p->bbox);
}

void lpObject_fini(lpObject_t *p)
{
	int i;

	if (p->elements) {
		for (i = 0; i < p->num_elements; i++)
			if (p->elements[i])
				lpElement_delete(p->elements[i]);

		free(p->elements);
	}
	if (p->name)
		free(p->name);

	lpBBox_fini(&p->bbox);
}

lpElement_t *lpObject_addElement(lpObject_t *p, lpObject_t *obj)
{
	lpElement_t *element;

	if (p->num_elements + 1 > p->max_elements)
		lpObject_growElements(obj);

	element = lpElement_new();
	if (element) {
		p->elements[p->num_elements] = element;
		p->num_elements++;
		element->parent = obj;
	}

	return element;
}

void lpObject_draw(lpObject_t *p)
{
	int i;
	lpObject_t *obj;

	if (p->referenced)
		return;

	// glDisable(GL_DEPTH_TEST);

	if (p->texture_num) {
		glEnable(GL_TEXTURE_2D);
		lpTextures_bindTexture(p->textures, p->texture_num);

		if (p->envmapped) {
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
		}
	}

	if (p->have_normals) {
		glEnable(GL_LIGHTING);
		if (p->material)
			lp_bind_material(p->material);
	} else
		glColor3fv(p->color);

	glPushMatrix();

	glRotatef(p->rotate[2], 1., 0., 0.);
	glRotatef(p->rotate[0], 1., 0., 0.);
	glRotatef(p->rotate[1], 1., 0., 0.);

	for (i = 0; i < p->num_elements; i++)
		lpElement_draw(p->elements[i]);

	if (p->have_normals)
		glDisable(GL_LIGHTING);

	obj = p->instance_object;

	while (obj) {
		if (!obj->referenced) {
			glEnable(GL_LIGHTING);

			if (obj->have_normals) {
				if (obj->material)
					lp_bind_material(obj->material);
			} else
				glColor3fv(obj->color);

			for (i = 0; i < obj->num_elements; i++)
				lpElement_draw(obj->elements[i]);

			glDisable(GL_LIGHTING);
		}
		obj = obj->instance_object;
	}

	glPopMatrix();

	if (p->texture_num) {
		if (p->envmapped) {
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
		glDisable(GL_TEXTURE_2D);
	}
}

void lpObject_draw_refoverride(lpObject_t *p, int refoverride)
{
	int i;
	lpObject_t *obj;

	if (p->texture_num) {
		glEnable(GL_TEXTURE_2D);
		lpTextures_bindTexture(p->textures, p->texture_num);

		if (p->envmapped) {
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
		}
	}

	if (p->have_normals) {
		glEnable(GL_LIGHTING);
		if (p->material)
			lp_bind_material(p->material);
	} else {
		if (refoverride != 2)
			glColor3fv(p->color);
	}

	glPushMatrix();

	glRotatef(p->rotate[2], 1., 0., 0.);
	glRotatef(p->rotate[0], 1., 0., 0.);
	glRotatef(p->rotate[1], 1., 0., 0.);

	for (i = 0; i < p->num_elements; i++)
		lpElement_draw(p->elements[i]);

	obj = p->instance_object;

	while (obj) {
		glEnable(GL_LIGHTING);

		if (obj->have_normals) {
			if (p->material)
				lp_bind_material(p->material);
		} else
			glColor3fv(obj->color);

		for (i = 0; i < obj->num_elements; i++)
			lpElement_draw(obj->elements[i]);

		glDisable(GL_LIGHTING);

		obj = obj->instance_object;

	}
	glPopMatrix();

	if (p->texture_num) {
		if (p->envmapped) {
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
		glDisable(GL_TEXTURE_2D);
	}
}

void lpObject_genGraphicsData(lpObject_t *p)
{
	int i, j, first = 1;
	lpElement_t *ep;

	// generate data such as geometric extents, texture coords etc.
	// for this object

	for (i = 0; i < p->num_elements; i++) {
		ep = p->elements[i];

		lpElement_genGraphicsData(ep);

		if (first) {
			for (j = 0; j < 3; j++) {
				p->bbox.xyz_min[j] = ep->bbox.xyz_min[j];
				p->bbox.xyz_max[j] = ep->bbox.xyz_max[j];
			}
			first = 0;
		} else {
			for (j = 0; j < 3; j++) {
				p->bbox.xyz_min[j] = min(p->bbox.xyz_min[j], ep->bbox.xyz_min[j]);
				p->bbox.xyz_max[j] = max(p->bbox.xyz_max[j], ep->bbox.xyz_max[j]);
			}
		}
	}

	for (j = 0; j < 3; j++)
		p->bbox.center[j] = (p->bbox.xyz_min[j] + p->bbox.xyz_max[j]) / 2.0f;

	// if this object has a texture defined,
	// generate texture coords for elements that need them

	if (p->texture_num != 0)
		for (i = 0; i < p->num_elements; i++) {
			lpElement_genTextureCoords(p->elements[i], p, &p->bbox);
		}
}

void lpObject_growElements(lpObject_t *p)
{
	lpElement_t **new_elements;

	new_elements = (lpElement_t **) realloc(p->elements,
						sizeof(lpElement_t *) * (p->num_elements + 1));
	p->max_elements += 1;
	p->elements = new_elements;
}

void lpObject_setName(lpObject_t *p, char *s)
{
	if (p->name)
		free(p->name);
	p->name = (char *) malloc(strlen(s) + 1);
	strcpy(p->name, s);
}

void lpObject_setInstanceName(lpObject_t *p, char *s)
{
	if (p->instance_name)
		free(p->instance_name);
	p->instance_name = (char *) malloc(strlen(s) + 1);
	strcpy(p->instance_name, s);
}

void lpObject_setTextureManager(lpObject_t *p, lpTextures_t *textures)
{
	p->textures = textures;
};

lpElement_t *lpElement_new(void)
{
	lpElement_t *p = (lpElement_t *) calloc(1, sizeof(lpElement_t));

	if (p)
		lpElement_init(p);

	return p;
}

void lpElement_delete(lpElement_t *p)
{
	if (p) {
		lpElement_fini(p);
		free(p);
	}
}

void lpElement_init(lpElement_t *p)
{
	p->parent = NULL;
	p->num_verts = p->max_verts = 0;
	p->have_tcoords = false;
	p->have_normals = false;
	p->verts = NULL;

	lpBBox_init(&p->bbox);
}

void lpElement_fini(lpElement_t *p)
{
	int i;

	if (p->verts) {
		for (i = 0; i < p->num_verts; i++)
			if (p->verts[i])
				free(p->verts[i]);

		free(p->verts);
	}

	lpBBox_fini(&p->bbox);
}

vertex_t *lpElement_addVertex(lpElement_t *p)
{
	int i;
	vertex_t *vert;

	if (p->num_verts + 1 > p->max_verts)
		lpElement_growVerts(p);

	vert = (vertex_t *) malloc(sizeof(vertex_t));
	if (vert) {
		for (i = 0; i < 3; i++) {
			vert->xyz[i] = 0.;
			vert->norm[i] = 0.;
		}

		for (i = 0; i < 2; i++)
			vert->st[i] = 0.;

		p->verts[p->num_verts] = vert;
		p->num_verts++;
	}

	return vert;
}

void lpElement_draw(lpElement_t *p)
{
	int i;

#if 0
	if (p->have_tcoords)
		glEnable(GL_TEXTURE_2D);
#endif

	switch (p->type) {
	case LP_LINE:
		glBegin(GL_LINE_LOOP);
		break;
	case LP_POLYGON:
		glBegin(GL_POLYGON);
		break;
	case LP_TRISTRIP:
		glBegin(GL_TRIANGLE_STRIP);
		break;
	default:
		break;
	}

	if (p->parent->have_normals) {
		if (p->have_tcoords) {
			for (i = 0; i < p->num_verts; i++) {
				lpTextures_TexCoord2fv(p->textures, p->verts[i]->st);
				glNormal3fv(p->verts[i]->norm);
				glVertex3fv(p->verts[i]->xyz);
			}
		} else {
			for (i = 0; i < p->num_verts; i++) {
				glNormal3fv(p->verts[i]->norm);
				glVertex3fv(p->verts[i]->xyz);
			}
		}
	} else {
		if (p->have_tcoords) {
			for (i = 0; i < p->num_verts; i++) {
				lpTextures_TexCoord2fv(p->textures, p->verts[i]->st);
				glVertex3fv(p->verts[i]->xyz);
			}
		} else {
			for (i = 0; i < p->num_verts; i++) {
				glVertex3fv(p->verts[i]->xyz);
			}
		}
	}
	glEnd();

#if 0
	if (p->have_tcoords)
		glDisable(GL_TEXTURE_2D);
#endif
}

void lpElement_genGraphicsData(lpElement_t *p)
{
	int first = 1, i, j;
	vertex_t *vp;

	// calc bbox for this element

	for (i = 0; i < p->num_verts; i++) {
		vp = p->verts[i];

		if (first) {
			for (j = 0; j < 3; j++)
				p->bbox.xyz_min[j] = p->bbox.xyz_max[j] = vp->xyz[j];

			first = 0;
		} else {
			for (j = 0; j < 3; j++) {
				p->bbox.xyz_min[j] = min(p->bbox.xyz_min[j], vp->xyz[j]);
				p->bbox.xyz_max[j] = max(p->bbox.xyz_max[j], vp->xyz[j]);
			}
		}
	}
}

void lpElement_genTextureCoords(lpElement_t *p, lpObject_t *obj, lpBBox_t *bbox)
{
	int i, j;
	vertex_t *vp;

	if (p->have_tcoords)
		return;		// already have them from config file

	// generate texture coordinates

	for (i = 0; i < p->num_verts; i++) {
		vp = p->verts[i];

		for (j = 0; j < 2; j++) {
			vp->st[j] = (1.0 / (bbox->xyz_max[j] - bbox->xyz_min[j])) *
				    (vp->xyz[j] - bbox->xyz_min[j]);
			vp->st[j] *= 1.0 / obj->texture_scale[j];
			vp->st[j] -= obj->texture_translate[j];
		}
	}
	// vp->st[j] = (1.0 / (bbox->xyz_max[j] - bbox->xyz_min[j])) *
	// 	    (vp->xyz[j] - bbox->xyz_min[j]);

	p->have_tcoords = true;
}

void lpElement_growVerts(lpElement_t *p)
{
	vertex_t **new_verts;

	new_verts = (vertex_t **) realloc(p->verts,
					  sizeof(vertex_t *) * (p->num_verts + 4));
	p->max_verts += 4;
	p->verts = new_verts;
}

void lpElement_setTextureManager(lpElement_t *p, lpTextures_t *textures)
{
	p->textures = textures;
};

// texture class

lpTextures_t *lpTextures_new(void)
{
	lpTextures_t *p = (lpTextures_t *) calloc(1, sizeof(lpTextures_t));

	if (p)
		lpTextures_init(p);

	return p;
}

void lpTextures_delete(lpTextures_t *p)
{
	if (p) {
		lpTextures_fini(p);
		free(p);
	}
}

void lpTextures_init(lpTextures_t *p)
{
	p->tex = (texture_t **) malloc(sizeof(texture_t *));
	p->tex[0] = (texture_t *) calloc(1, sizeof(texture_t));
	p->num_textures = 1;	// start at 1, zero reserved for unbinding
	p->max_textures = 1;
	p->last_accessed = 0;
}

void lpTextures_fini(lpTextures_t *p)
{
	int i;

	if (p->tex) {
		for (i = 0; i < p->num_textures; i++)
			if (p->tex[i]) {
				if (p->tex[i]->texels)
					free(p->tex[i]->texels);
				free(p->tex[i]);
			}

		free(p->tex);
	}
}

int lpTextures_addTexture(lpTextures_t *p, char *fname)
{
	int texnum;
	texture_t *tp;
#ifdef WANT_SDL
	SDL_Surface *surface, *temp_surface;
#else
	unsigned char *pixels;
#endif

	if (p->num_textures + 1 > p->max_textures)
		lpTextures_growTextures(p);
	texnum = p->num_textures;

	p->tex[texnum] = tp = (texture_t *) calloc(1, sizeof(texture_t));

#ifdef WANT_SDL
	temp_surface = IMG_Load(fname);
	if (!temp_surface)
		return 0;
	/* convert loaded image to RGBA32 pixels */
	surface = SDL_ConvertSurfaceFormat(temp_surface, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(temp_surface);
	if (!surface)
		return 0;
	tp->imgXsize = surface->w;
	tp->imgYsize = surface->h;
	tp->imgZsize = surface->format->BytesPerPixel;
	tp->surface = surface;
#else /* !WANT_SDL */
	pixels = read_jpeg(fname, &tp->imgXsize, &tp->imgYsize, &tp->imgZsize);
	if (!pixels)
		return 0;
	tp->pixels = pixels;
#endif /* !WANT_SDL */

	p->num_textures++;

	// printf("\n\nAddTextureFile: added %s %dx%dx%d as texnum %d \n", fname,
	//        tp->imgXsize, tp->imgYsize, tp->imgZsize, texnum);

	p->last_accessed = texnum;

	return texnum;
}

/* get power of 2 */

static int GetPowerOf2i(int n)
{
	int result = 0x2;

	while (result < n)
		result = result << 1;
	return result;
}

int lpTextures_downloadTextures(lpTextures_t *p)
{
	int texnum, n;
	texture_t *tp;

	for (texnum = 1; texnum < p->num_textures; texnum++) {
		tp = p->tex[texnum];

#ifdef WANT_SDL
		if (tp->surface) {
#else
		if (tp->pixels) {
#endif
			// calc power of two S and T dimensions

			tp->texSsize = GetPowerOf2i(tp->imgXsize);
			tp->texTsize = GetPowerOf2i(tp->imgYsize);

			// set gl parms

			switch (tp->imgZsize) {
			case 1:
				tp->format = GL_LUMINANCE;
				break;
			case 2:
				tp->format = GL_LUMINANCE_ALPHA;
				break;
			case 3:
				tp->format = GL_RGB;
				break;
			case 4:
				tp->format = GL_RGBA;
				break;
			default:
				fprintf(stderr, "addTexture: Invalid # texture components.\n");
				return 0;
			}

			// set type

			tp->type = GL_UNSIGNED_BYTE;

			// calc S and T limits

			tp->texSmin = 0;
			tp->texTmin = 0;
			tp->texSmax = (float) tp->imgXsize / (float) tp->texSsize;

			tp->texTmax = (float) tp->imgYsize / (float) tp->texTsize;

#ifdef WANT_SDL
			// copy SDL surface pixel data to texel data bottom to top

			unsigned char *src, *rsrc, *dst;
			int x, y, z;

			// lock SDL surface for direct access
			if (SDL_MUSTLOCK(tp->surface) && SDL_LockSurface(tp->surface) < 0) {
				fprintf(stderr, "addTexture: Can't lock SDL surface.\n");
				return 0;
			}

			// position after last row of pixels
			rsrc = (unsigned char *) tp->surface->pixels +
			       tp->surface->pitch * tp->imgYsize;

			tp->texels = (unsigned char *) malloc(tp->texSsize * tp->texTsize *
							      tp->imgZsize);

			dst = tp->texels;

			for (y = 0; y < tp->texTsize; y++) {
				// move up one row of pixels
				rsrc -= tp->surface->pitch;
				src = rsrc;
				for (x = 0; x < tp->texSsize; x++)
					for (z = 0; z < tp->imgZsize; z++)
						if (y < tp->imgYsize && x < tp->imgXsize) {
							*dst++ = *src++;
						} else
							*dst++ = 0;
			}

			// unlock SDL surface and free it
			if (SDL_MUSTLOCK(tp->surface))
				SDL_UnlockSurface(tp->surface);
			SDL_FreeSurface(tp->surface);
			tp->surface = NULL;
#else /* !WANT_SDL */
			// copy image pixel data to texel data

			unsigned char *src, *dst;
			int x, y, z;

			src = tp->pixels;

			tp->texels = (unsigned char *) malloc(tp->texSsize * tp->texTsize *
							      tp->imgZsize);

			dst = tp->texels;

			for (y = 0; y < tp->texTsize; y++)
				for (x = 0; x < tp->texSsize; x++)
					for (z = 0; z < tp->imgZsize; z++)
						if (y < tp->imgYsize && x < tp->imgXsize)
							*dst++ = *src++;
						else
							*dst++ = 0;

			// free pixels
			free(tp->pixels);
			tp->pixels = NULL;
#endif /* !WANT_SDL */

			// get a bind id from OpenGL

			(void) glGetError();	/* clear any gl errors */

			glGenTextures(1, (GLuint *) &tp->bind_id);
			glBindTexture(GL_TEXTURE_2D, tp->bind_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, tp->imgZsize, tp->texSsize, tp->texTsize,
				     0, tp->format, GL_UNSIGNED_BYTE, tp->texels);

			glBindTexture(GL_TEXTURE_2D, 0);

			n = glGetError();
			if (n)
				fprintf(stderr, "addTexture: glError %d\n", n);
		}
	} // end for(texnum...

	return 1;
}

void lpTextures_bindTexture(lpTextures_t *p, int n)
{
	if (n == 0)
		glBindTexture(GL_TEXTURE_2D, 0);
	else {
		if (n > p->num_textures) {
			fprintf(stderr, "bindTexture: Invalid texture number (%d)\n", n);
			return;
		} else {
			// printf("bind %d %d\n", n, tex[n]->bind_id);
			glBindTexture(GL_TEXTURE_2D, p->tex[n]->bind_id);
		}
	}
	p->last_accessed = n;
}

void lpTextures_growTextures(lpTextures_t *p)
{
	int n = 20;
	texture_t **new_tex;

	new_tex = (texture_t **) realloc(p->tex,
					 sizeof(texture_t *) * (p->num_textures + n));
	p->max_textures += n;
	p->tex = new_tex;
}

void lpTextures_TexCoord2fv(lpTextures_t *p, float *st)
{
	glTexCoord2f(st[0] * p->tex[p->last_accessed]->texSmax,
		     st[1] * p->tex[p->last_accessed]->texTmax);
}

// Bbox class

lpBBox_t *lpBBox_new(void)
{
	lpBBox_t *p = (lpBBox_t *) calloc(1, sizeof(lpBBox_t));

	if (p)
		lpBBox_init(p);

	return p;
}

void lpBBox_delete(lpBBox_t *p)
{
	if (p) {
		lpBBox_fini(p);
		free(p);
	}
}

void lpBBox_init(lpBBox_t *p)
{
	int i;

	for (i = 0; i < 3; i++)
		p->xyz_min[i] = p->xyz_max[i] = p->center[i] = 0.0f;
}

void lpBBox_fini(lpBBox_t *p)
{
	UNUSED(p);
}
