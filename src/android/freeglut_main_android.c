/*
 * freeglut_main_android.c
 *
 * The Android-specific windows message processing methods.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Copied for Platform code by Evan Felix <karcaw at gmail.com>
 * Copyright (C) 2012  Sylvain Beucler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <GL/freeglut.h>
#include "../Common/freeglut_internal.h"

#include <android/native_app_glue/android_native_app_glue.h>
 
/*
 * Handle a window configuration change. When no reshape
 * callback is hooked, the viewport size is updated to
 * match the new window size.
 */
void fgPlatformReshapeWindow ( SFG_Window *window, int width, int height )
{
  fprintf(stderr, "fgPlatformReshapeWindow: STUB\n");
}

/*
 * A static helper function to execute display callback for a window
 */
void fgPlatformDisplayWindow ( SFG_Window *window )
{
  fghRedrawWindow ( window ) ;
}

unsigned long fgPlatformSystemTime ( void )
{
  struct timeval now;
  gettimeofday( &now, NULL );
  return now.tv_usec/1000 + now.tv_sec*1000;
}

/*
 * Does the magic required to relinquish the CPU until something interesting
 * happens.
 */
void fgPlatformSleepForEvents( long msec )
{
  /* fprintf(stderr, "fgPlatformSleepForEvents: STUB\n"); */
}

void fgPlatformProcessSingleEvent ( void )
{
  /* Read pending event. */
  int ident;
  int events;
  struct android_poll_source* source;
    while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
    /* Process this event. */
    if (source != NULL) {
      source->process(source->app, source);
    }
  }
}

void fgPlatformMainLoopPreliminaryWork ( void )
{
  printf("fgPlatformMainLoopPreliminaryWork\n");

  /* Make sure glue isn't stripped. */
  /* JNI callbacks need to be bundled even when linking statically */
  app_dummy();

  glutPostRedisplay(); // TODO: necessary?
}

void fgPlatformDeinitialiseInputDevices ( void )
{
  fprintf(stderr, "fgPlatformDeinitialiseInputDevices: STUB\n");
}
