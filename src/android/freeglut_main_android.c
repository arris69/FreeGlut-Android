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
#include "Common/freeglut_internal.h"

#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "FreeGLUT", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "FreeGLUT", __VA_ARGS__))
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

/**
 * Process the next input event.
 */
int32_t handle_input(struct android_app* app, AInputEvent* event) {
  return 0;  /* not handled */
}

/**
 * Process the next main command.
 */
void handle_cmd(struct android_app* app, int32_t cmd) {
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    /* The system has asked us to save our current state.  Do so. */
    LOGI("handle_cmd: APP_CMD_SAVE_STATE");
    break;
  case APP_CMD_INIT_WINDOW:
    /* The window is being shown, get it ready. */
    LOGI("handle_cmd: APP_CMD_INIT_WINDOW");
    fgDisplay.pDisplay.single_window->Window.Handle = app->window;
    /* glPlatformOpenWindow was waiting for Handle to be defined and
       will now return from fgPlatformProcessSingleEvent() */
    break;
  case APP_CMD_TERM_WINDOW:
    /* The window is being hidden or closed, clean it up. */
    LOGI("handle_cmd: APP_CMD_TERM_WINDOW");
    fgDestroyWindow(fgDisplay.pDisplay.single_window);
    break;
  case APP_CMD_DESTROY:
    /* Not reached because GLUT exit()s when last window is closed */
    LOGI("handle_cmd: APP_CMD_DESTROY");
    break;
  case APP_CMD_GAINED_FOCUS:
    LOGI("handle_cmd: APP_CMD_GAINED_FOCUS");
    break;
  case APP_CMD_LOST_FOCUS:
    LOGI("handle_cmd: APP_CMD_LOST_FOCUS");
    break;
  case APP_CMD_CONFIG_CHANGED:
    /* Handle rotation / orientation change */
    LOGI("handle_cmd: APP_CMD_CONFIG_CHANGED");
    break;
  case APP_CMD_WINDOW_RESIZED:
    LOGI("handle_cmd: APP_CMD_WINDOW_RESIZED");
    if (fgDisplay.pDisplay.single_window->Window.pContext.eglSurface != EGL_NO_SURFACE)
      /* Make ProcessSingleEvent detect the new size, only available
	 after the next SwapBuffer */
      glutPostRedisplay();
    break;
  default:
    LOGI("handle_cmd: unhandled cmd=%d", cmd);
  }
}

void fgPlatformProcessSingleEvent ( void )
{
  static int32_t last_width = -1;
  static int32_t last_height = -1;

  /* When the screen is resized, the window handle still points to the
     old window until the next SwapBuffer, while it's crucial to set
     the size (onShape) correctly before the next onDisplay callback.
     Plus we don't know if the next SwapBuffer already occurred at the
     time we process the event (e.g. during onDisplay). */
  /* So we do the check each time rather than on event. */
  /* Interestingly, on a Samsung Galaxy S/PowerVR SGX540 GPU/Android
     2.3, that next SwapBuffer is fake (but still necessary to get the
     new size). */
  SFG_Window* window = fgDisplay.pDisplay.single_window;
  if (window != NULL && window->Window.Handle != NULL) {
    int32_t width = ANativeWindow_getWidth(window->Window.Handle);
    int32_t height = ANativeWindow_getHeight(window->Window.Handle);
    if (width != last_width || height != last_height) {
      last_width = width;
      last_height = height;
      LOGI("width=%d, height=%d", width, height);
      if( FETCH_WCB( *window, Reshape ) )
	INVOKE_WCB( *window, Reshape, ( width, height ) );
      else
	glViewport( 0, 0, width, height );
      glutPostRedisplay();
    }
  }

  /* Read pending event. */
  int ident;
  int events;
  struct android_poll_source* source;
  /* This is called "ProcessSingleEvent" but this means we'd only
     process ~60 (screen Hz) mouse events per second, plus other ports
     are processing all events already.  So let's process all pending
     events. */
  /* if ((ident=ALooper_pollOnce(0, NULL, &events, (void**)&source)) >= 0) { */
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
  /* JNI entry points need to be bundled even when linking statically */
  app_dummy();
}

void fgPlatformDeinitialiseInputDevices ( void )
{
  fprintf(stderr, "fgPlatformDeinitialiseInputDevices: STUB\n");
}
