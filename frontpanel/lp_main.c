// lp_main.c	lightpanel main interface

/* Copyright (c) 2007-2015, John Kichury
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

/* Fixed portability problems, March 2014, Udo Munk */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef WANT_SDL
#include <SDL.h>
#include <SDL_opengl.h>
#else
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <pthread.h>
#include <unistd.h>
#endif

#include "frontpanel.h"
#include "lpanel.h"
#include "lp_main.h"
#include "lp_utils.h"

#define UNUSED(x) (void) (x)

#ifdef WANT_SDL
static SDL_mutex *data_sample_lock;
#else
static pthread_mutex_t data_lock;
static pthread_mutex_t data_sample_lock;
static thread_info_t thread_info;
#endif

static int framecount = 0;
static int samplecount = 0;

Lpanel_t *panel;
Parser_t parser;

#ifndef WANT_SDL

static void *lp_mainloop_thread(void *n)
{
	int quit = 0;
	double t1, t2;

	UNUSED(n);

	// printf("mainloop thread starting\n");
	thread_info.running = 1;
	if (!Lpanel_openWindow(panel, "FrontPanel")) {
		fprintf(stderr, "Can't open FrontPanel window\n");
		exit(EXIT_FAILURE);
	}

	t1 = frate_gettime();
	framerate_start_frame();

	while (!quit) {
		// lock
		pthread_mutex_lock(&data_lock);

		// proc events
		// Lpanel_sampleData(panel);
		Lpanel_procEvents(panel);

		// draw
		pthread_mutex_lock(&data_sample_lock);
		Lpanel_draw(panel);
		pthread_mutex_unlock(&data_sample_lock);

		// unlock
		pthread_mutex_unlock(&data_lock);

		// sleep remainder of fps time
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
		SwapBuffers(panel->hDC);
		// UpdateWindow(panel->hWnd);
#else
		glXSwapBuffers(panel->dpy, panel->window);
#endif
		glFinish();
		framerate_wait();
		t2 = frate_gettime();
		framecount++;
		framerate_start_frame();

		if (t2 - t1 > 1.0) {
			panel->frames_per_second = framecount;
			panel->samples_per_second = samplecount;
			t1 = frate_gettime();
			framecount = 0;
			samplecount = 0;
		}

		if (!thread_info.run)
			quit = 1;

	} // end while(!quit)

	pthread_mutex_lock(&data_lock);
	Lpanel_destroyWindow(panel);
	pthread_mutex_unlock(&data_lock);
	thread_info.running = 0;
	return NULL;
}

static int start_threads(void)
{
	int n;

	pthread_mutex_init(&data_lock, NULL);
	pthread_mutex_init(&data_sample_lock, NULL);
	thread_info.run = 1;
	n = pthread_create(&thread_info.thread_id, NULL, lp_mainloop_thread,
			   &thread_info.thread_no);
	if (n) {
		fprintf(stderr, "error %d starting mainloop thread\n", n);
		return 0;
	}

	// fprintf(stderr, "FrontPanel mainloop thread start (PID=%d)\n", thread_info.thread_id);

	return 1;
}

#endif /* !WANT_SDL */

// bind functions

int fp_bindLight64(const char *name, uint64_t *bits, int bitnum)
{
	// printf("fp_bindLight64: name=%s *bits=%lx bitnum=%d\n", name, *bits, bitnum);
	Lpanel_bindLight64(panel, name, bits, bitnum);
	return 1;
}

int fp_bindLight32(const char *name, uint32_t *bits, int bitnum)
{
	Lpanel_bindLight32(panel, name, bits, bitnum);
	return 1;
}

int fp_bindLight16(const char *name, uint16_t *bits, int bitnum)
{
	Lpanel_bindLight16(panel, name, bits, bitnum);
	return 1;
}

int fp_bindLightfv(const char *name, float *bits)
{
	Lpanel_bindLightfv(panel, name, bits);
	return 1;
}

int fp_bindLight8(const char *name, uint8_t *bits, int bitnum)
{
	Lpanel_bindLight8(panel, name, bits, bitnum);
	return 1;
}

int fp_bindLight8invert(const char *name, uint8_t *bits, int bitnum, uint8_t mask)
{
	Lpanel_bindLight8invert(panel, name, bits, bitnum, mask);
	return 1;
}

int fp_bindLight16invert(const char *name, uint16_t *bits, int bitnum, uint16_t mask)
{
	Lpanel_bindLight16invert(panel, name, bits, bitnum, mask);
	return 1;
}

int fp_bindLight32invert(const char *name, uint32_t *bits, int bitnum, uint32_t mask)
{
	Lpanel_bindLight32invert(panel, name, bits, bitnum, mask);
	return 1;
}

int fp_bindLight64invert(const char *name, uint64_t *bits, int bitnum, uint64_t mask)
{
	Lpanel_bindLight64invert(panel, name, bits, bitnum, mask);
	return 1;
}

int fp_smoothLight(const char *name, int nframes)
{
	Lpanel_smoothLight(panel, name, nframes);
	return 1;
}

void fp_bindPowerFlag(uint8_t *addr)
{
	Lpanel_bindPowerFlag(panel, addr);
}

void fp_bindRunFlag(uint8_t *addr)
{
	Lpanel_bindRunFlag(panel, addr);
}

void fp_bindSimclock(uint64_t *addr)
{
	Lpanel_bindSimclock(panel, addr);
}

int fp_bindSwitch8(const char *name, uint8_t *loc_down, uint8_t *loc_up, int bitnum)
{
	Lpanel_bindSwitch8(panel, name, loc_down, loc_up, bitnum);
	return 1;
}

int fp_bindSwitch16(const char *name, uint16_t *loc_down, uint16_t *loc_up, int bitnum)
{
	Lpanel_bindSwitch16(panel, name, loc_down, loc_up, bitnum);
	return 1;
}

int fp_bindSwitch32(const char *name, uint32_t *loc_down, uint32_t *loc_up, int bitnum)
{
	Lpanel_bindSwitch32(panel, name, loc_down, loc_up, bitnum);
	return 1;
}

int fp_bindSwitch64(const char *name, uint64_t *loc_down, uint64_t *loc_up, int bitnum)
{
	Lpanel_bindSwitch64(panel, name, loc_down, loc_up, bitnum);
	return 1;
}

void fp_ignoreBindErrors(int n)
{
	Lpanel_ignoreBindErrors(panel, n != 0);
}

void fp_framerate(float v)
{
	Lpanel_framerate_set(panel, v);
	framerate_set(v);
}

int fp_init(const char *cfg_fname)
{
	return fp_init2(NULL, cfg_fname, 800);
}

int fp_init2(const char *cfg_root_path, const char *cfg_fname, int size)
{
	printf("FrontPanel Simulator v2.1C Copyright (C) 2007-2015 by John Kichury\n");
#ifdef WANT_SDL
	printf("SDL2 version Copyright (C) 2024-2025 by Thomas Eberhardt\n");
#else
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
	printf("Windows version Copyright (C) 2014 by Stefano Bodrato\n");
#endif
#endif

	// allocate & initialize panel and parser instance
	panel = Lpanel_new();
	Parser_init(&parser);

	if (cfg_root_path)
		Lpanel_setConfigRootPath(panel, cfg_root_path);

	// set a default framerate
	fp_framerate(30.);

	// set initial window size
	panel->window_xsize = size;

	if (!Lpanel_readConfig(panel, cfg_fname)) {
		fprintf(stderr, "fp_init: error initializing the panel\n");
		return 0;
	}

#ifndef WANT_SDL
	start_threads();
#endif

	return 1;
}

#ifdef WANT_SDL

void fp_openWindow(void)
{
	data_sample_lock = SDL_CreateMutex();

	if (!Lpanel_openWindow(panel, "FrontPanel")) {
		fprintf(stderr, "Can't open FrontPanel window\n");
		exit(EXIT_FAILURE);
	}
}

void fp_procEvent(SDL_Event *event)
{
	Lpanel_procEvent(panel, event);
}

void fp_draw(bool tick)
{
	SDL_LockMutex(data_sample_lock);
	Lpanel_draw(panel);
	SDL_UnlockMutex(data_sample_lock);
	SDL_GL_SwapWindow(panel->window);
	glFinish();
	framecount++;
	if (tick) {
		panel->frames_per_second = framecount;
		panel->samples_per_second = samplecount;
		framecount = 0;
		samplecount = 0;
	}
}

#else /* !WANT_SDL */

void fp_openWindow(void)
{
	start_threads();
}

void fp_procEvents(void)
{
	Lpanel_procEvents(panel);
}

void fp_draw(void)
{
	Lpanel_draw(panel);
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
	SwapBuffers(panel->hDC);
	// UpdateWindow(panel->hWnd);
#else
	glXSwapBuffers(panel->dpy, panel->window);
#endif
}

#endif /* !WANT_SDL */

void fp_sampleData(void)
{
	// mutex lock
#ifdef WANT_SDL
	SDL_LockMutex(data_sample_lock);
#else
	pthread_mutex_lock(&data_sample_lock);
#endif

	Lpanel_sampleData(panel);

	// mutex unlock
#ifdef WANT_SDL
	SDL_UnlockMutex(data_sample_lock);
#else
	pthread_mutex_unlock(&data_sample_lock);
#endif

	samplecount++;
}

void fp_sampleLightGroup(int groupnum, int clockwarp)
{
	Lpanel_sampleLightGroup(panel, groupnum, clockwarp);
	samplecount++;
}

void fp_sampleSwitches(void)
{
	Lpanel_sampleSwitches(panel);
}

// callback functions

int fp_addSwitchCallback(const char *name, void (*cbfunc)(int state, int val),
			 int userval)
{
	Lpanel_addSwitchCallback(panel, name, cbfunc, userval);
	return 1;
}

void fp_addQuitCallback(void (*cbfunc)(void))
{
	Lpanel_addQuitCallback(panel, cbfunc);
}

void fp_quit(void)
{
#ifdef WANT_SDL
	Lpanel_destroyWindow(panel);

	SDL_DestroyMutex(data_sample_lock);
#else /* !WANT_SDL */
	int i;
	bool okay = false;

	pthread_mutex_lock(&data_lock);
	thread_info.run = 0;
	pthread_mutex_unlock(&data_lock);

	for (i = 0; i < 10; i++) {
		pthread_mutex_lock(&data_lock);
		if (thread_info.running == 0) {
			okay = true;
			break;
		}
		pthread_mutex_unlock(&data_lock);
#if defined(__MINGW32__) || defined(_WIN32) || defined(_WIN32_) || defined(__WIN32__)
		Sleep(1000);
#else
		sleep(1);
#endif
	}

	if (!okay) {
		Lpanel_destroyWindow(panel);
		fprintf(stderr, "Error. lightpanel draw thread did not terminate\n");
	}
#endif /* !WANT_SDL */

	// finalize & deallocate panel and parser instance
	Parser_fini(&parser);
	Lpanel_delete(panel);
}

int fp_test(int n)
{
	return Lpanel_test(panel, n);
}
