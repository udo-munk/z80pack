// lp_window.c	lightpanel window functions

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WANT_SDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#else /* !WANT_SDL */
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
#include <windows.h>
#include <Commctrl.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#endif
#endif /* !WANT_SDL */

#include "lpanel.h"
#include "lp_materials.h"
#include "lp_font.h"

#define UNUSED(x) (void) (x)

#ifndef WANT_SDL
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
const char FPClassName[] = "FrontPanel 2.1";
#else
static int RGBA_DB_attributes[] = {
	GLX_RGBA,
	GLX_RED_SIZE, 1,
	GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1,
	GLX_DOUBLEBUFFER,
	GLX_DEPTH_SIZE, 1,
	None
};
#endif
#endif /* !WANT_SDL */

// OpenGL Light source

static GLfloat light_pos0[] = { 0., 0.5, 1., 0.};
// static GLfloat light_pos1[] = { 0.,-0.5,-1.,0.};
static GLfloat light_diffuse[] = { 1., 1., 1., 1.};
static GLfloat light_specular[] = { 1., 1., 1., 1.};

static GLfloat mtl_amb[] = { 0.2, 0.2, 0.2, 1.0 };
static GLfloat mtl_dif[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat mtl_spec[] = { 0.0, 0.0, 0.0, 1.0 };
static GLfloat mtl_shine[] = { 0.0 };
static GLfloat mtl_emission[] = { 0.0, 0.0, 0.0, 1.0 };

static int mousex, mousey, omx, omy, lmouse;

#ifdef WANT_SDL

void Lpanel_procEvent(Lpanel_t *p, SDL_Event *event)
{
	switch (event->type) {
	case SDL_WINDOWEVENT:
		if (event->window.windowID != SDL_GetWindowID(p->window))
			break;

		switch (event->window.event) {
		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED:
			Lpanel_resizeWindow(p);
			break;
		case SDL_WINDOWEVENT_CLOSE:
			// call user quit callback if exists
			if (p->quit_callbackfunc)
				(*p->quit_callbackfunc)();
			break;
		default:
			break;
		}
		break;

	case SDL_QUIT:
		// call user quit callback if exists
		if (p->quit_callbackfunc)
			(*p->quit_callbackfunc)();
		break;

	case SDL_MOUSEBUTTONDOWN:
		if (event->button.windowID != SDL_GetWindowID(p->window))
			break;

		SDL_GL_MakeCurrent(p->window, p->cx);
		if (!Lpanel_pick(p, event->button.button - 1, 1,
				 event->button.x, event->button.y)) {
			if (event->button.button == SDL_BUTTON_LEFT) {	// left mousebutton ?
				lmouse = 1;
				mousex = event->button.x;
				mousey = event->button.y;
			}
		}
		break;

	case SDL_MOUSEBUTTONUP:
		if (event->button.windowID != SDL_GetWindowID(p->window))
			break;

		SDL_GL_MakeCurrent(p->window, p->cx);
		if (!Lpanel_pick(p, event->button.button - 1, 0,
				 event->button.x, event->button.y)) {
			if (event->button.button == SDL_BUTTON_LEFT)
				lmouse = 0;
		}
		break;

	case SDL_KEYUP:
		if (event->key.windowID != SDL_GetWindowID(p->window))
			break;

		switch (event->key.keysym.sym) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			p->shift_key_pressed = false;
			break;

		}
		break;

	case SDL_KEYDOWN:
		if (event->key.windowID != SDL_GetWindowID(p->window))
			break;

		switch (event->key.keysym.sym) {
		case SDLK_ESCAPE:
			// exit(EXIT_SUCCESS);
			break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			p->shift_key_pressed = true;
			break;

		case SDLK_c:
			p->do_cursor = !p->do_cursor;
			break;

		case SDLK_d:
			p->view.pan[1] -= 0.1;
			p->view.redo_projections = true;
			break;

		case SDLK_s:
			p->do_stats = !p->do_stats;
			break;

		case SDLK_l:
			p->view.rot[1] += -1.;
			p->view.redo_projections = true;
			break;

		case SDLK_r:
			p->view.rot[1] -= -1.;
			p->view.redo_projections = true;
			break;

		case SDLK_u:
			p->view.pan[1] += 0.1;
			p->view.redo_projections = true;
			break;

		case SDLK_v:
			if (p->view.projection == LP_ORTHO)
				p->view.projection = LP_PERSPECTIVE;
			else
				p->view.projection = LP_ORTHO;

			p->view.redo_projections = true;
			break;

		case SDLK_z:
			if (p->shift_key_pressed) {
				p->view.pan[2] += .1;
				p->view.redo_projections = true;
			} else {
				p->view.pan[2] -= .1;
				p->view.redo_projections = true;
			}
			break;

		case SDLK_UP:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, 0., p->cursor_inc);
			else {
				if (p->shift_key_pressed)
					p->view.pan[1] += -0.1;
				else
					p->view.rot[0] += -1.;

				p->view.redo_projections = true;
			}
			break;

		case SDLK_DOWN:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, 0., -p->cursor_inc);
			else {
				if (p->shift_key_pressed)
					p->view.pan[1] += 0.1;
				else
					p->view.rot[0] += 1.;

				p->view.redo_projections = true;
			}
			break;

		case SDLK_RIGHT:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, p->cursor_inc, 0.);
			else {
				if (p->shift_key_pressed)
					p->view.pan[0] += -.1;
				else
					p->view.rot[1] += 1.;

				p->view.redo_projections = true;
			}
			break;

		case SDLK_LEFT:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, -p->cursor_inc, 0.);
			else {
				if (p->shift_key_pressed)
					p->view.pan[0] += .1;
				else
					p->view.rot[1] += -1.;

				p->view.redo_projections = true;
			}
			break;

		case SDLK_1:
		case SDLK_KP_1:
			break;

		default:
			break;
		}
		/* fallthrough */

	case SDL_MOUSEMOTION:
		if (event->motion.windowID != SDL_GetWindowID(p->window))
			break;

		if (lmouse) {
			omx = mousex;
			omy = mousey;

			if (p->shift_key_pressed) {
				p->view.pan[0] += ((float) event->motion.x - (float) omx) * .02;
				p->view.pan[1] -= ((float) event->motion.y - (float) omy) * .02;
			} else {
				p->view.rot[1] += ((float) event->motion.x - (float) omx) * .2;
				p->view.rot[0] += ((float) event->motion.y - (float) omy) * .2;
			}

			mousex = event->motion.x;
			mousey = event->motion.y;
			p->view.redo_projections = true;
		}
		break;

	default:
		break;
	}
} // end Lpanel_procEvent()

#else /* !WANT_SDL */

#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)

static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR user_data = GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (user_data)
		return Lpanel_WndProc((Lpanel_t *) user_data, Msg, wParam, lParam);
	else
		return DefWindowProc(hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK Lpanel_WndProc(Lpanel_t *p, UINT msg, WPARAM wParam, LPARAM lParam)
{
	unsigned int kcode;

	switch (msg) {
	case WM_KEYUP:
		if (LOWORD(wParam) == VK_SHIFT) {
			p->shift_key_pressed = false;
			return 0;
		}
		return 0;

	case WM_KEYDOWN:
		kcode = LOWORD(wParam);
		switch (kcode) {
		case VK_SHIFT:
			p->shift_key_pressed = true;
			return 0;

		case VK_UP:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, 0., p->cursor_inc);
			else {
				if (p->shift_key_pressed)
					p->view.pan[1] += -0.1;
				else
					p->view.rot[0] += -1.;

				p->view.redo_projections = true;
			}
			break;

		case VK_DOWN:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, 0., -cursor_inc);
			else {
				if (p->shift_key_pressed)
					p->view.pan[1] += 0.1;
				else
					p->view.rot[0] += 1.;

				p->view.redo_projections = true;
			}
			break;

		case VK_RIGHT:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, p->cursor_inc, 0.);
			else {
				if (p->shift_key_pressed)
					p->view.pan[0] += -.1;
				else
					p->view.rot[1] += 1.;

				p->view.redo_projections = true;
			}
			break;

		case VK_LEFT:
			if (p->do_cursor)
				Lpanel_inc_cursor(p, -p->cursor_inc, 0.);
			else {
				if (p->shift_key_pressed)
					p->view.pan[0] += .1;
				else
					p->view.rot[1] += -1.;

				p->view.redo_projections = true;
			}
			break;
		}
		return 0;

	case WM_CHAR:
		kcode = LOWORD(wParam);

		switch (kcode) {
		case VK_ESCAPE:
			// exit(EXIT_SUCCESS);
			break;

		case 'c':
		case 'C':
			p->do_cursor = !p->do_cursor;
			break;

		case 'd':
		case 'D':
			p->view.pan[1] -= 0.1;
			p->view.redo_projections = true;
			break;

		case 's':
		case 'S':
			p->do_stats = !p->do_stats;
			break;

		case 'l':
		case 'L':
			p->view.rot[1] += -1.;
			p->view.redo_projections = true;
			break;

		case 'r':
		case 'R':
			p->view.rot[1] -= -1.;
			p->view.redo_projections = true;
			break;

		case 'u':
		case 'U':
			p->view.pan[1] += 0.1;
			p->view.redo_projections = true;
			break;

		case 'v':
		case 'V':
			if (p->view.projection == LP_ORTHO)
				view.projection = LP_PERSPECTIVE;
			else
				view.projection = LP_ORTHO;

			view.redo_projections = true;
			break;

		case 'z':
			view.pan[2] -= .1;
			view.redo_projections = true;
			break;

		case 'Z':
			view.pan[2] += .1;
			view.redo_projections = true;
			break;

		case '1':
			break;
		}
		break;

	case WM_MOUSEWHEEL:
		p->view.pan[2] += (float) GET_WHEEL_DELTA_WPARAM(wParam) / 250.0;
		p->view.redo_projections = true;
		return 0;

	case WM_LBUTTONDOWN:
		if (!Lpanel_pick(p, 0, 1, LOWORD(lParam), HIWORD(lParam))) {
			mousex = LOWORD(lParam);
			mousey = HIWORD(lParam);
			lmouse = true;
		}
		return 0;

	case WM_LBUTTONUP:
		if (!Lpanel_pick(p, 0, 0, LOWORD(lParam), HIWORD(lParam)))
			lmouse = false;
		return 0;

	case WM_MOUSEMOVE:
		if (lmouse) {
			omx = mousex;
			omy = mousey;

			if (p->shift_key_pressed) {
				p->view.pan[0] += ((float) LOWORD(lParam) - (float) omx) * .02;
				p->view.pan[1] -= ((float) HIWORD(lParam) - (float) omy) * .02;
			} else {
				view.rot[1] += ((float) LOWORD(lParam) - (float) omx) * .2;
				view.rot[0] += ((float) HIWORD(lParam) - (float) omy) * .2;
			}

			mousex = LOWORD(lParam);
			mousey = HIWORD(lParam);
			p->view.redo_projections = true;
		}
		return 0;

	case WM_SIZE:
		p->window_xsize = (GLsizei) LOWORD(lParam);
		p->window_ysize = (GLsizei) HIWORD(lParam);
		p->view.aspect = (GLdouble) p->window_xsize / (GLdouble) p->window_ysize;

		glViewport(0, 0, p->window_xsize, p->window_ysize);
		glGetIntegerv(GL_VIEWPORT, p->viewport);
		setProjection(false);
		setModelview(false);

		return 0;

	case WM_QUIT:
	case WM_CLOSE:
		if (p->hRC) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(p->hRC);
		}
		if (p->hDC) {
			ReleaseDC(p->hWnd, p->hDC);
		}
		DestroyWindow(p->hWnd);
		return 0;

	case WM_DESTROY:
		UnregisterClass(FPClassName, p->hInstance);
		PostQuitMessage(0);
		if (quit_callbackfunc)
			(*quit_callbackfunc)();
		else
			exit(EXIT_SUCCESS);
		return 0;

	default:
		return DefWindowProc(p->hWnd, msg, wParam, lParam);
	}
}

void Lpanel_procEvents(Lpanel_t *p)
{
	MSG msg;

	if (PeekMessage(&msg, p->hWnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

#else /* !__MINGW32__ && !_WIN32 && !_WIN32_ && !__WIN32__ */

void Lpanel_procEvents(Lpanel_t *p)
{
	XEvent event;
	char buffer[5];
	int bufsize = 5;
	KeySym key;
	XComposeStatus compose;

	while (XPending(p->dpy)) {
		XNextEvent(p->dpy, &event);

		switch (event.type) {
		case ConfigureNotify:
			Lpanel_resizeWindow(p);
			break;

		case ClientMessage:
			if ((Atom) event.xclient.data.l[0] == p->wmDeleteMessage) {
				// call user quit callback if exists
				if (p->quit_callbackfunc) {
					(*p->quit_callbackfunc)();
				} else {
					XCloseDisplay(p->dpy);
					exit(EXIT_SUCCESS);
				}
			}
			break;

		case UnmapNotify:
			break;

		case DestroyNotify:
			break;

		case ButtonPress:
			if (!Lpanel_pick(p, event.xbutton.button - 1, 1,
					 event.xbutton.x, event.xbutton.y)) {
				if (event.xbutton.button == 1) {	// left mousebutton ?
					lmouse = 1;
					mousex = event.xbutton.x;
					mousey = event.xbutton.y;
				}
			}
			break;

		case ButtonRelease:
			if (!Lpanel_pick(p, event.xbutton.button - 1, 0,
					 event.xbutton.x, event.xbutton.y)) {
				if (event.xbutton.button == 1)
					lmouse = 0;
			}
			break;

		case KeyRelease:
			XLookupString(&event.xkey, buffer, bufsize, &key, &compose);

			switch (key) {
			case XK_Shift_L:
			case XK_Shift_R:
				p->shift_key_pressed = false;
				break;

			}
			break;

		case KeyPress:
			XLookupString(&event.xkey, buffer, bufsize, &key, &compose);

			switch (key) {
			case XK_Escape:
				// exit(EXIT_SUCCESS);
				break;

			case XK_Shift_L:
			case XK_Shift_R:
				p->shift_key_pressed = true;
				break;

			case XK_c:
			case XK_C:
				p->do_cursor = !p->do_cursor;
				break;

			case XK_d:
			case XK_D:
				p->view.pan[1] -= 0.1;
				p->view.redo_projections = true;
				break;

			case XK_s:
			case XK_S:
				p->do_stats = !p->do_stats;
				break;

			case XK_l:
			case XK_L:
				p->view.rot[1] += -1.;
				p->view.redo_projections = true;
				break;

			case XK_r:
			case XK_R:
				p->view.rot[1] -= -1.;
				p->view.redo_projections = true;
				break;

			case XK_u:
			case XK_U:
				p->view.pan[1] += 0.1;
				p->view.redo_projections = true;
				break;

			case XK_v:
			case XK_V:
				if (p->view.projection == LP_ORTHO)
					p->view.projection = LP_PERSPECTIVE;
				else
					p->view.projection = LP_ORTHO;

				p->view.redo_projections = true;
				break;

			case XK_z:
				p->view.pan[2] -= .1;
				p->view.redo_projections = true;
				break;

			case XK_Z:
				p->view.pan[2] += .1;
				p->view.redo_projections = true;
				break;

			case XK_Up:
				if (p->do_cursor)
					Lpanel_inc_cursor(p, 0., p->cursor_inc);
				else {
					if (p->shift_key_pressed)
						p->view.pan[1] += -0.1;
					else
						p->view.rot[0] += -1.;

					p->view.redo_projections = true;
				}
				break;

			case XK_Down:
				if (p->do_cursor)
					Lpanel_inc_cursor(p, 0., -p->cursor_inc);
				else {
					if (p->shift_key_pressed)
						p->view.pan[1] += 0.1;
					else
						p->view.rot[0] += 1.;

					p->view.redo_projections = true;
				}
				break;

			case XK_Right:
				if (p->do_cursor)
					Lpanel_inc_cursor(p, p->cursor_inc, 0.);
				else {
					if (p->shift_key_pressed)
						p->view.pan[0] += -.1;
					else
						p->view.rot[1] += 1.;

					p->view.redo_projections = true;
				}
				break;

			case XK_Left:
				if (p->do_cursor)
					Lpanel_inc_cursor(p, -p->cursor_inc, 0.);
				else {
					if (p->shift_key_pressed)
						p->view.pan[0] += .1;
					else
						p->view.rot[1] += -1.;

					p->view.redo_projections = true;
				}
				break;

			case XK_1:
			case XK_KP_1:
				break;

			default:
				break;
			}
			/* fallthrough */

		case MotionNotify:
			if (lmouse) {
				omx = mousex;
				omy = mousey;

				if (p->shift_key_pressed) {
					p->view.pan[0] += ((float) event.xmotion.x -
							   (float) omx) * .02;
					p->view.pan[1] -= ((float) event.xmotion.y -
							   (float) omy) * .02;
				} else {
					p->view.rot[1] += ((float) event.xmotion.x -
							   (float) omx) * .2;
					p->view.rot[0] += ((float) event.xmotion.y -
							   (float) omy) * .2;
				}

				mousex = event.xmotion.x;
				mousey = event.xmotion.y;
				p->view.redo_projections = true;
			}
			break;

		default:
			break;
		}
	} // end while
} // end Lpanel_procEvents()

static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg)
{
	UNUSED(d);

	if ((e->type == MapNotify) && (e->xmap.window == (Window) arg)) {
		return GL_TRUE;
	}
	return GL_FALSE;
}

#endif /* !__MINGW32__ && !_WIN32 && !_WIN32_ && !__WIN32__ */

#endif /* !WANT_SDL */

int Lpanel_openWindow(Lpanel_t *p, const char *title)
{
	float geom_aspect = (p->bbox.xyz_max[0] - p->bbox.xyz_min[0]) /
			    (p->bbox.xyz_max[1] - p->bbox.xyz_min[1]);
	p->window_ysize = (int) ((float) p->window_xsize / geom_aspect);
	p->view.aspect = (GLdouble) p->window_xsize / (GLdouble) p->window_ysize;

#ifdef WANT_SDL

	if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
		fprintf(stderr, "Can't initialize SDL_image: %s\n", IMG_GetError());
		return 0;
	}

	p->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				     p->window_xsize, p->window_ysize,
				     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (p->window == NULL) {
		fprintf(stderr, "Can't create window: %s\n", SDL_GetError());
		return 0;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
	if ((p->cx = SDL_GL_CreateContext(p->window)) == NULL) {
		fprintf(stderr, "Can't create context: %s\n", SDL_GetError());
		return 0;
	}
	if (SDL_GL_MakeCurrent(p->window, p->cx) < 0) {
		fprintf(stderr, "Can't make window current to context: %s\n", SDL_GetError());
		return 0;
	}
	/* First try adaptive vsync, than vsync */
	if (SDL_GL_SetSwapInterval(-1) < 0)
		SDL_GL_SetSwapInterval(1);

	Lpanel_resizeWindow(p);
	Lpanel_initGraphics(p);

#else /* !WANT_SDL */

#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)

	BOOL err;

	p->hInstance = GetModuleHandle(NULL);

	p->wc.cbSize        = sizeof(WNDCLASSEX);
	p->wc.style         = 0;
	p->wc.lpfnWndProc   = StaticWndProc;
	p->wc.cbClsExtra    = 0;
	p->wc.cbWndExtra    = 0;
	p->wc.hInstance     = p->hInstance;
	p->wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	p->wc.hCursor       = LoadCursor(NULL, IDC_HAND);
	// p->wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE);
	p->wc.hbrBackground = (HBRUSH) (COLOR_WINDOW);
	p->wc.lpszMenuName  = NULL;
	p->wc.lpszClassName = FPClassName;
	p->wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&p->wc)) {
		MessageBox(NULL, "Window registration failed!", "Error!",
			   MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	p->hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, FPClassName, title, WS_OVERLAPPEDWINDOW,
				 CW_USEDEFAULT, CW_USEDEFAULT, p->window_xsize, p->window_ysize,
				 NULL, NULL, p->hInstance, NULL);
	if (p->hWnd == NULL) {
		MessageBox(NULL, "Window creation failed!", "Error!",
			   MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// We put in the window user data area a pointer to the windowproc for its instance
	SetWindowLongPtr(p->hWnd, GWLP_USERDATA, (LONG_PTR) p);

	p->hDC = GetDC(p->hWnd);

	memset(&p->pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	p->pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	p->pfd.nVersion = 1;
	p->pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	p->pfd.iPixelType = PFD_TYPE_RGBA;
	p->pfd.cColorBits = 24;
	p->pfd.cDepthBits = 32;
	p->pfd.iLayerType = PFD_MAIN_PLANE;

	int pixelFormat = ChoosePixelFormat(p->hDC, &p->pfd);
	if (pixelFormat == 0) {
		return 0;
	}

	err = SetPixelFormat(p->hDC, pixelFormat, &p->pfd);
	if (!err) {
		return 0;
	}

	p->hRC = wglCreateContext(p->hDC);
	if (!p->hRC) {
		MessageBox(NULL, "lightpanel: Can't create window context", "Error!",
			   MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	err = wglMakeCurrent(p->hDC, p->hRC);
	if (!err) {
		MessageBox(NULL, "lightpanel: Can't make window current to context", "Error!",
			   MB_ICONEXCLAMATION | MB_OK);
		// ReleaseDC(p->hWnd, p->hDC);
		return 0;
	}

	ShowWindow(p->hWnd, SW_SHOWNORMAL);

	SetForegroundWindow(p->hWnd);
	SetFocus(p->hWnd);
	Lpanel_initGraphics(p);
	UpdateWindow(p->hWnd);

#else /* !__MINGW32__ && !_WIN32 && !_WIN32_ && !__WIN32__ */

	int status;
	XSetWindowAttributes swa;
	XSizeHints hints;
	XEvent ev;

	p->dpy = XOpenDisplay(0);
	if (p->dpy == NULL) {
		fprintf(stderr, "Can't connect to display \"%s\"\n", getenv("DISPLAY"));
		return 0;
	}

	p->vi = glXChooseVisual(p->dpy, DefaultScreen(p->dpy), RGBA_DB_attributes);
	if (p->vi == NULL) {
		fprintf(stderr, "Can't find visual\n");
		return 0;
	}

	glXGetConfig(p->dpy, p->vi, GLX_USE_GL, &status);
	if (!status) {
		printf("Your system must support OpenGL to run FrontPanel\n");
		return 0;
	}

	swa.border_pixel = 0;
	swa.colormap = XCreateColormap(p->dpy, RootWindow(p->dpy, p->vi->screen),
				       p->vi->visual, AllocNone);
	swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask |
			 ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

	p->window = XCreateWindow(p->dpy, RootWindow(p->dpy, p->vi->screen), 500, 500,
				  p->window_xsize, p->window_ysize,
				  0, p->vi->depth, InputOutput, p->vi->visual,
				  CWBorderPixel | CWColormap | CWEventMask, &swa);

	p->wmDeleteMessage = XInternAtom(p->dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(p->dpy, p->window, &p->wmDeleteMessage, 1);

	XStoreName(p->dpy, p->window, title);

	hints.width = 500;
	hints.height = 125;
	hints.min_aspect.x = p->window_xsize;
	hints.min_aspect.y = p->window_ysize;
	hints.max_aspect.x = p->window_xsize;
	hints.max_aspect.y = p->window_ysize;
	hints.flags = PSize | PAspect;
	XSetNormalHints(p->dpy, p->window, &hints);

	XMapWindow(p->dpy, p->window);
	XIfEvent(p->dpy, &ev, WaitForMapNotify, (char *) p->window);

	if ((p->cx = glXCreateContext(p->dpy, p->vi, 0, GL_TRUE)) == NULL) {
		printf("lightpanel: Can't create context\n");
		return 0;
	}
	if (!glXMakeCurrent(p->dpy, p->window, p->cx)) {
		printf("lightpanel: Can't make window current to context\n");
		return 0;
	}

	Lpanel_resizeWindow(p);
	Lpanel_initGraphics(p);

#endif /* !__MINGW32__ && !_WIN32 && !_WIN32_ && !__WIN32__ */

#endif /* !WANT_SDL */

	p->cursor[0] = (p->bbox.xyz_max[0] + p->bbox.xyz_min[0]) * .5;
	p->cursor[1] = (p->bbox.xyz_max[1] + p->bbox.xyz_min[1]) * .5;
	makeRasterFont();
	Lpanel_make_cursor_text(p);

	return 1;
}

void Lpanel_destroyWindow(Lpanel_t *p)
{
	glFlush();
	glFinish();

#ifdef WANT_SDL
	if (SDL_GL_MakeCurrent(NULL, NULL) < 0) {
		printf("lightpanel: destroyWindow: Can't release context\n");
	}
	SDL_GL_DeleteContext(p->cx);
	SDL_DestroyWindow(p->window);
	IMG_Quit();
	p->cx = NULL;
	p->window = NULL;
#else /* !WANT_SDL */
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(p->hWnd, p->hDC);
#else
	if (!glXMakeCurrent(p->dpy, None, None)) {
		printf("lightpanel: destroyWindow: Can't release context\n");
	}

	glXDestroyContext(p->dpy, p->cx);
	XDestroyWindow(p->dpy, p->window);
	XFree(p->vi);
	XCloseDisplay(p->dpy);
	p->dpy = NULL;
	p->cx = NULL;
	p->vi = NULL;
	p->window = 0;
#endif
#endif /* !WANT_SDL */
}

void Lpanel_setModelview(Lpanel_t *p, bool dopick)
{
	float x, y;

	if (!dopick)
		glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	switch (p->view.projection) {
	case LP_ORTHO:
		glTranslatef(0., 0., -10.);	// so objects at z=0 don't get clipped
		break;

	case LP_PERSPECTIVE:
		x = -((p->bbox.xyz_min[0] + p->bbox.xyz_max[0]) * 0.5 - p->view.pan[0]);
		y = -((p->bbox.xyz_min[1] + p->bbox.xyz_max[1]) * 0.5 - p->view.pan[1]);

		glTranslatef(x, y, p->view.pan[2]);

		glTranslatef(p->bbox.center[0], p->bbox.center[1], p->bbox.center[2]);
		glRotatef(p->view.rot[2], 0., 0., 1.);
		glRotatef(p->view.rot[0], 1., 0., 0.);
		glRotatef(p->view.rot[1], 0., 1., 0.);
		glTranslatef(-p->bbox.center[0], -p->bbox.center[1], -p->bbox.center[2]);
		break;
	}
}

void Lpanel_setProjection(Lpanel_t *p, bool dopick)
{
	switch (p->view.projection) {
	case LP_ORTHO:
		if (!dopick) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
		}

		glOrtho(p->bbox.xyz_min[0], p->bbox.xyz_max[0],
			p->bbox.xyz_min[1], p->bbox.xyz_max[1],
			.1, 1000.);

		if (!dopick)
			glMatrixMode(GL_MODELVIEW);
		break;

	case LP_PERSPECTIVE:
		if (!dopick) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
		}

		// gluPerspective(p->view.fovy, p->view.aspect, p->view.znear, p->view.zfar);
		double deltaz = p->view.zfar - p->view.znear;
		double cotangent = 1 / tan(p->view.fovy / 2 * M_PI / 180);
		GLdouble m[16] = {
			cotangent / p->view.aspect, 0, 0, 0,
			0, cotangent, 0, 0,
			0, 0, -(p->view.zfar + p->view.znear) / deltaz, -1,
			0, 0, -2 * p->view.znear *p->view.zfar / deltaz, 0
		};
		glMultMatrixd(m);

		if (!dopick)
			glMatrixMode(GL_MODELVIEW);
		break;
	}
}

#if !defined(__MINGW32__) && !defined(_WIN32) && !defined(_WIN32_) && !defined(__WIN32__)
void Lpanel_resizeWindow(Lpanel_t *p)
{
#ifdef WANT_SDL
	int width, height;

	SDL_GL_MakeCurrent(p->window, p->cx);
	SDL_GL_GetDrawableSize(p->window, &width, &height);
	p->window_xsize = width;
	p->window_ysize = height;
#else /* !WANT_SDL */
	XWindowAttributes windowattr;

	XGetWindowAttributes(p->dpy, p->window, &windowattr);
	p->window_xsize = windowattr.width;
	p->window_ysize = windowattr.height;
#endif /* !WANT_SDL */

	p->view.aspect = (double) p->window_xsize / (double) p->window_ysize;
	glViewport(0, 0, p->window_xsize, p->window_ysize);
	glGetIntegerv(GL_VIEWPORT, p->viewport);

	Lpanel_setProjection(p, false);

	Lpanel_setModelview(p, false);
}
#endif /* !__MINGW32__ && !_WIN32 && !_WIN32_ && !__WIN32__ */

void Lpanel_doPickProjection(Lpanel_t *p)
{
	// glOrtho(p->bbox.xyz_min[0], p->bbox.xyz_max[0],
	// 	p->bbox.xyz_min[1], p->bbox.xyz_max[1],
	// 	.1, 1000.);
	Lpanel_setProjection(p, 1);
}

void Lpanel_doPickModelview(Lpanel_t *p)
{
	Lpanel_setModelview(p, 1);
	// glTranslatef(0., 0., -10.);	// so objects at z=0 don't get clipped
}

void Lpanel_initGraphics(Lpanel_t *p)
{
	// initialize materials

	lp_init_materials_dlist();

	// define lights in case we use them

	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_specular);

	// glLightfv(GL_LIGHT1, GL_POSITION, light_pos1);
	// glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	// glLightfv(GL_LIGHT1, GL_DIFFUSE, light_specular);

	glEnable(GL_LIGHT0);
	if (p->view.do_depthtest)
		glEnable(GL_DEPTH_TEST);
	glPolygonOffset(0., -10.);
	// glEnable(GL_LIGHT1);

	glDisable(GL_LIGHTING);
	// glEnable(GL_LIGHTING);
	glClearColor(0., 0., 0., 1.);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mtl_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mtl_dif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mtl_spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mtl_shine);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mtl_emission);

	glEnable(GL_NORMALIZE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef WANT_SDL
	SDL_GL_SwapWindow(p->window);
#else
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
	SwapBuffers(p->hDC);
	// UpdateWindow(p->hWnd);
	// Sleep(100);
#else
	glXSwapBuffers(p->dpy, p->window);
#endif
#endif

	// download any textures that may have been read in

	lpTextures_downloadTextures(&p->textures);

	if (p->envmap_detected) {
		glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	}

	p->cursor[0] = (p->bbox.xyz_max[0] + p->bbox.xyz_min[0]) * .5;
	p->cursor[1] = (p->bbox.xyz_max[1] + p->bbox.xyz_min[1]) * .5;
	makeRasterFont();
	Lpanel_make_cursor_text(p);
}

void Lpanel_draw_stats(Lpanel_t *p)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0., p->window_xsize, 0., p->window_ysize, .1, 1000.);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0., 0., -10.);
	glDisable(GL_DEPTH_TEST);

	glColor3f(1., 1., 0.);
	snprintf(p->perf_txt, sizeof(p->perf_txt), "fps:%d sps:%d",
		 p->frames_per_second, p->samples_per_second);
	printStringAt(p->perf_txt, p->bbox.xyz_min[0] + .2, p->bbox.xyz_min[1] + .2);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void Lpanel_draw_cursor(Lpanel_t *p)
{
	float size = 0.1;

	glColor3f(1., 1., 0.);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	glVertex3f(p->cursor[0] - size, p->cursor[1] - size, p->cursor[2]);
	glVertex3f(p->cursor[0] + size, p->cursor[1] + size, p->cursor[2]);

	glVertex3f(p->cursor[0] - size, p->cursor[1] + size, p->cursor[2]);
	glVertex3f(p->cursor[0] + size, p->cursor[1] - size, p->cursor[2]);

	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0., p->window_xsize, 0., p->window_ysize, .1, 1000.);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0., 0., -10.);

	printStringAt(p->cursor_txt, p->cursor_textpos[0], p->cursor_textpos[1]);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
}

void Lpanel_inc_cursor(Lpanel_t *p, float x, float y)
{
	if (p->shift_key_pressed) {
		x *= 10.;
		y *= 10.;
	}
	p->cursor[0] += x;
	p->cursor[1] += y;
	Lpanel_make_cursor_text(p);
}

void Lpanel_make_cursor_text(Lpanel_t *p)
{
	p->cursor_textpos[0] = (p->bbox.xyz_max[0] + p->bbox.xyz_min[0]) * .5;
	p->cursor_textpos[1] = p->bbox.xyz_min[1] + .1;
	snprintf(p->cursor_txt, sizeof(p->cursor_txt), "cursor position=%7.3f,%7.3f",
		 p->cursor[0], p->cursor[1]);
}
