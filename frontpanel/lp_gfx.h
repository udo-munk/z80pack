// lp_gfx.h    lightpanel graphics classes and data structures

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

#ifndef _LP_GFX_DEFS
#define _LP_GFX_DEFS

#include <stdbool.h>
#ifdef WANT_SDL
#include <SDL.h>
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#endif

enum lp_el_types { LP_POLYGON, LP_TRISTRIP, LP_LINE };

typedef struct {
	float		xyz[3],
			norm[3],
			st[2];
} vertex_t;

// forward references

struct lpElement;
struct lpTextures;

typedef struct lpBBox {
	float		xyz_min[3],
			xyz_max[3],
			center[3];
} lpBBox_t;

extern lpBBox_t		*lpBBox_new(void);
extern void		lpBBox_delete(lpBBox_t *p);
extern void		lpBBox_init(lpBBox_t *p);
extern void		lpBBox_fini(lpBBox_t *p);

typedef struct lpObject {
	int		num_elements,
			max_elements;
	struct lpElement **elements;

	char		*name;

	char		*instance_name;
	struct lpObject	*instance_object;

	float		color[3];
	float		texture_scale[2],
			texture_translate[2];
	int		texture_num,
			material;
	bool		envmapped,
			referenced,	// object is only drawn by another object referencing it
			have_normals,
			is_alpha;	// object has transparent material or texture

	float		rotate[3],	// rotate hpr
			translate[3],	// translate
			origin[3];	// object origin

	struct lpTextures *textures;
	lpBBox_t	bbox;
} lpObject_t;

extern lpObject_t	*lpObject_new(void);
extern void		lpObject_delete(lpObject_t *p);
extern void		lpObject_init(lpObject_t *p);
extern void		lpObject_fini(lpObject_t *p);

// private functions

extern void		lpObject_growElements(lpObject_t *p);

// public functions

extern void		lpObject_setName(lpObject_t *p, char *s);
extern void		lpObject_setInstanceName(lpObject_t *p, char *s);
extern struct lpElement	*lpObject_addElement(lpObject_t *p, lpObject_t *obj);
extern void 		lpObject_draw(lpObject_t *p);
extern void		lpObject_draw_refoverride(lpObject_t *p, int refoverride);
extern void		lpObject_setTextureManager(lpObject_t *p, struct lpTextures *textures);
extern void		lpObject_genGraphicsData(lpObject_t *p);

typedef struct lpElement {
	int		type,		// LP_POLYGON, LP_LINE
			num_verts,
			max_verts;
	bool		have_tcoords,
			have_normals;

	lpObject_t	*parent;
	vertex_t	**verts;

	struct lpTextures *textures;

	lpBBox_t	bbox;		// element bounding box
} lpElement_t;

extern lpElement_t	*lpElement_new(void);
extern void		lpElement_delete(lpElement_t *p);
extern void		lpElement_init(lpElement_t *p);
extern void		lpElement_fini(lpElement_t *p);

// private functions

extern void		lpElement_growVerts(lpElement_t *p);

// public functions

extern vertex_t		*lpElement_addVertex(lpElement_t *p);
extern void		lpElement_draw(lpElement_t *p);
extern void		lpElement_genGraphicsData(lpElement_t *p);
extern void		lpElement_genTextureCoords(lpElement_t *p, lpObject_t *obj,
						   lpBBox_t *bbox);
extern void		lpElement_setTextureManager(lpElement_t *p, struct lpTextures *textures);

// texture class for managing graphics textures

typedef struct texture {
	// image specific attributes

	int		imgXsize,
			imgYsize,
			imgZsize;

#ifdef WANT_SDL
	SDL_Surface	*surface;
#else
	unsigned char	*pixels;
#endif

	// texture specific attributes

	GLuint		bind_id;
	GLuint		format,		// GL_RGB etc.
			type;		// GL_UNSIGNED_BYTE etc.

	unsigned char	*texels;

	int		texSsize,	// power of 2 texture size
			texTsize;

	float		texSmin,
			texSmax,
			texTmin,
			texTmax,

			texCropSmin,	// for cropping grab data
			texCropSmax,
			texCropTmin,
			texCropTmax;
} texture_t;

typedef struct lpTextures {
	int		num_textures,
			max_textures,
			last_accessed;
	texture_t 	**tex;
} lpTextures_t;

extern lpTextures_t	*lpTextures_new(void);
extern void		lpTextures_delete(lpTextures_t *p);
extern void		lpTextures_init(lpTextures_t *p);
extern void		lpTextures_fini(lpTextures_t *p);

// private functions

extern void		lpTextures_growTextures(lpTextures_t *p);

// public functions

extern int		lpTextures_addTexture(lpTextures_t *p, char *fname);
extern int		lpTextures_downloadTextures(lpTextures_t *p);
extern void		lpTextures_bindTexture(lpTextures_t *p, int n);
extern void		lpTextures_TexCoord2fv(lpTextures_t *p, float *st);

#endif /* !_LP_GFX_DEFS */
