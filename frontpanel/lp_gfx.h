// lp_gfx.h    light panel graphics classes and data structures

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


#ifndef _LP_GFX_DEFS
#define _LP_GFX_DEFS

enum lp_el_types { LP_POLYGON, LP_TRISTRIP, LP_LINE };

typedef struct
{
  float xyz[3],
        norm[3],
        st[2]; 
} vertex_t;


// forward references

class lpElement;
class lpTextures;

class lpBBox
{
  private:

  public:
	lpBBox();
	~lpBBox();

  float	xyz_min[3],
	xyz_max[3],
	center[3];

};

class lpObject
{

  private:


	void		growElements(void);

  public:

	lpObject();
	~lpObject();

	int		num_elements,
			max_elements;
	lpElement	**elements;

	char 		*name;
	void		setName(char *s);

	char 		*instance_name;
	void		setInstanceName(char *s);
	lpObject	*instance_object;

	lpElement	*addElement(lpObject *p);
	void 		draw(void);
	void 		draw(int referenced); // draw with reference override

	float		color[3];
	float		texture_scale[2],
			texture_translate[2];
	int		texture_num,
			envmapped;
	int		referenced,	// object is only drawn by another object referencing it
			have_normals,
			material,
			is_alpha;	// object has transparent material or texture

	float		rotate[3],	// rotate hpr
			translate[3],	// translate
			origin[3];	// object origin

	lpTextures	*textures;
	void		setTextureManager(lpTextures *p) { textures = p; };

	void		genGraphicsData(void);
	lpBBox		bbox;
};


class lpElement
{
  private:

	void		growVerts(void);

  public:

	lpElement();
	~lpElement();

	int	type,		// LP_POLYGON, LP_LINE
		num_verts,
		max_verts,
		have_tcoords,
		have_normals;

	lpObject *parent;
	vertex_t **verts;

	vertex_t *addVertex(void);
	void	draw(void);
	void	genGraphicsData(void);
	void	genTextureCoords(lpObject *obj, lpBBox *bbox);

	lpTextures	*textures;
	void		setTextureManager(lpTextures *p) { textures = p; };

	lpBBox		bbox;	// element bounding box
};


#endif


// texture class for managing graphics textures

typedef struct 
{

 // image specific attributes

 int imgXsize,
     imgYsize,
     imgZsize;


 unsigned char *pixels;

 // texture specific attributes

 GLuint bind_id;
 GLuint format,                 // GL_RGB etc.
        type;                   // GL_UNSIGNED_BYTE etc.

 unsigned char *texels;

 int    texSsize,       // power of 2 texture size
        texTsize;

 float  texSmin,
        texSmax,
        texTmin,
        texTmax,

        texCropSmin,  // for cropping grab data
        texCropSmax,
        texCropTmin,
        texCropTmax;

} texture_t;


class lpTextures
{
 private:

	void		growTextures(void);

 public:
	lpTextures();
	~lpTextures();

	int		num_textures,
	    		max_textures,
			last_accessed;
	texture_t 	**tex;

	int		addTexture(char *fname);
	int		downloadTextures(void);
	void		bindTexture(int n);
	void		TexCoord2fv(float *st);
};

