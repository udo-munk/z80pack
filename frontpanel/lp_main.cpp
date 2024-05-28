// lp_main.cpp   light panel main interface

/* Copyright (c) 2007-2015, John Kichury

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


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/types.h>
#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
#include <windows.h>
#else
#include <sys/signal.h>
#endif
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include <sys/stat.h>

#include "lpanel.h"
#include "lp_main.h"
#include "lp_utils.h"

#define UNUSED(x) (void) (x)


//static pthread_mutex_t data_lock;
pthread_mutex_t data_lock;
pthread_mutex_t data_sample_lock;
extern pthread_mutex_t data_sample_lock;

static thread_info_t thread_info;
static Lpanel *panel = new Lpanel;
static int samplecount = 0;

void *
lp_mainloop_thread(void *n)
{
  int quit = 0;
  double t1, t2;
  int framecount = 0;

 UNUSED(n);

 //printf("mainloop thread starting\n");
 thread_info.running = 1;
 panel->openWindow("FrontPanel");

 t1 = frate_gettime();
 framerate_start_frame();

 while(!quit)
 {
  // lock
  pthread_mutex_lock(&data_lock);

  //  proc events

//  panel->sampleData();
  panel->procEvents();

  // draw

  pthread_mutex_lock(&data_sample_lock);
  panel->draw();
  pthread_mutex_unlock(&data_sample_lock);

  // unlock
  pthread_mutex_unlock(&data_lock);

  // sleep remainder of fps time
  glFinish();
  framerate_wait();
  t2 = frate_gettime();
  framecount++;
  framerate_start_frame();

  if(t2 - t1 > 1.0)
   {
     panel->frames_per_second = framecount;
     panel->samples_per_second = samplecount;
     t1 = frate_gettime();
     framecount = 0;
     samplecount = 0;
   }

  if( !thread_info.run ) quit = 1;

 } // end while(!quit)
  pthread_mutex_lock(&data_lock);
  panel->Quit();
  panel->destroyWindow();
  pthread_mutex_unlock(&data_lock);
  thread_info.running = 0;
  return NULL;
}

int start_threads(void)
{
  // don't use unitialised attr, use NULL pointer instead. **UM**
  //static pthread_attr_t attr;
  int n;

  pthread_mutex_init(&data_lock,NULL);
  pthread_mutex_init(&data_sample_lock,NULL);
  thread_info.run = 1;
  //n = pthread_create(&thread_info.thread_id, &attr, lp_mainloop_thread, &thread_info.thread_no);
  n = pthread_create(&thread_info.thread_id, NULL, lp_mainloop_thread, &thread_info.thread_no);
  if(n)
   { fprintf(stderr,"error %d starting mainloop thread\n",n);
     return 0;
   }

  //fprintf(stderr,"FrontPanel mainloop thread start (PID=%d)\n",thread_info.thread_id);

  return 1;
}

// bind functions

int fp_bindLight64(const char *name, uint64_t *bits, int bitnum)
{
//printf("fp_bindLight64: name=%s *bits=%lx  bitnum=%d\n",name,*bits,bitnum);
 panel->bindLight64(name,bits,bitnum);
 return 1;
}

int fp_bindLight32(const char *name, uint32_t *bits, int bitnum)
{
 panel->bindLight32(name,bits,bitnum);
 return 1;
}

int fp_bindLight16(const char *name, uint16_t *bits, int bitnum)
{
 panel->bindLight16(name,bits,bitnum);
 return 1;
}

int fp_bindLightfv(const char *name, float *bits)
{
 panel->bindLightfv(name,bits);
 return 1;
}

int fp_bindLight8(const char *name, uint8_t *bits, int bitnum)
{
 panel->bindLight8(name,bits,bitnum);
 return 1;
}

int fp_bindLight8invert(const char *name, uint8_t *bits, int bitnum, uint8_t mask)
{
 panel->bindLight8invert(name,bits,bitnum, mask);
 return 1;
}

int fp_bindLight16invert(const char *name, uint16_t *bits, int bitnum, uint16_t mask)
{
 panel->bindLight16invert(name,bits,bitnum, mask);
 return 1;
}

int fp_bindLight32invert(const char *name, uint32_t *bits, int bitnum, uint32_t mask)
{
 panel->bindLight32invert(name,bits,bitnum, mask);
 return 1;
}

int fp_bindLight64invert(const char *name, uint64_t *bits, int bitnum, uint64_t mask)
{
 panel->bindLight64invert(name,bits,bitnum, mask);
 return 1;
}

int fp_smoothLight(const char *name, int nframes)
{
 panel->smoothLight(name,nframes);
 return 1;
}

void fp_bindRunFlag(uint8_t *addr)
{
 panel->bindRunFlag(addr);
}

void fp_bindSimclock( uint64_t *addr)
{
 panel->bindSimclock(addr);
}

int fp_bindSwitch8(const char *name, uint8_t *loc_down, uint8_t *loc_up, int bitnum)
{
 panel->bindSwitch8(name, loc_down, loc_up, bitnum);
 return 1;
}

int fp_bindSwitch16(const char *name, uint16_t *loc_down, uint16_t *loc_up, int bitnum)
{
 panel->bindSwitch16(name, loc_down, loc_up, bitnum);
 return 1;
}

int fp_bindSwitch32(const char *name, uint32_t *loc_down, uint32_t *loc_up, int bitnum)
{
 panel->bindSwitch32(name, loc_down, loc_up, bitnum);
 return 1;
}
int fp_bindSwitch64(const char *name, uint64_t *loc_down, uint64_t *loc_up, int bitnum)
{
 panel->bindSwitch64(name, loc_down, loc_up, bitnum);
 return 1;
}

void fp_ignoreBindErrors(int n)
{ panel->ignoreBindErrors(n);
}

void fp_framerate(float v)
{
 if(panel) panel->framerate_set(v);
 framerate_set(v);
}

int fp_init(const char *cfg_fname)
{
  return (fp_init2(NULL, cfg_fname, 800));
}


int fp_init2(const char *cfg_root_path, const char *cfg_fname, int size)
{
 printf("FrontPanel Simulator v2.1 Copyright (C) 2007-2015 by John Kichury\n");
#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
 printf("Windows version Copyright (C) 2014 by Stefano Bodrato\n");
#endif

 if(panel == NULL) panel = new Lpanel;

  // set a default framerate

  if(cfg_root_path) panel->setConfigRootPath(cfg_root_path);

  fp_framerate(30.);

  // set initial window size
  panel->window_xsize = size;

  if(!panel->readConfig(cfg_fname))
   { fprintf(stderr, "fp_init: error initializing the panel\n");
     return 0;
   }

  start_threads();
  return 1;
}

int  fp_openWindow(const char *title)
{
   UNUSED(title);

   start_threads();
   return 1;
}

void fp_procEvents(void)
{
 panel->procEvents();
}
void fp_draw(void)
{
 panel->draw();
}

void fp_sampleData(void)
{ panel->sampleData();
  samplecount++;
}

void fp_sampleLightGroup(int groupnum, int clockwarp)
{ panel->sampleLightGroup(groupnum, clockwarp);
  samplecount++;
}

void fp_sampleSwitches(void)
{ panel->sampleSwitches();
}

// callback functions

int fp_addSwitchCallback(const char *name, void (*cbfunc)(int state, int val), int userval)
{
 panel->addSwitchCallback(name, cbfunc, userval);
 return 1;
}

void fp_addQuitCallback(void (*cbfunc)(void))
{
 panel->addQuitCallback(cbfunc);
}

void fp_quit(void)
{
 int i, okay=0;

  pthread_mutex_lock(&data_lock);
 thread_info.run = 0;
  pthread_mutex_unlock(&data_lock);

 for(i=0;i<10;i++)
  {
  pthread_mutex_lock(&data_lock);
    if(thread_info.running == 0) 
     {  okay = 1;
	break;
     }
  pthread_mutex_unlock(&data_lock);
#if defined (__MINGW32__) || defined (_WIN32) || defined (_WIN32_) || defined (__WIN32__)
    Sleep(1000);
#else
    sleep(1);
#endif
  }

 if(!okay) fprintf(stderr, "Error. lightpanel draw thread did not terminate\n");

}


int 
fp_test(int n)
{
 return(panel->test(n));
}

