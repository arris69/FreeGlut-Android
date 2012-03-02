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
#include <android/keycodes.h>

/* Cf. http://developer.android.com/reference/android/view/KeyEvent.html */
/* These codes are missing in <android/keycodes.h> */
/* Don't convert to enum, since it may conflict with future version of
   that <android/keycodes.h> */
#define AKEYCODE_FORWARD_DEL 112
#define AKEYCODE_CTRL_LEFT 113
#define AKEYCODE_CTRL_RIGHT 114
#define AKEYCODE_MOVE_HOME 122
#define AKEYCODE_MOVE_END 123
#define AKEYCODE_INSERT 124
#define AKEYCODE_ESCAPE 127
#define AKEYCODE_F1 131
#define AKEYCODE_F2 132
#define AKEYCODE_F3 133
#define AKEYCODE_F4 134
#define AKEYCODE_F5 135
#define AKEYCODE_F6 136
#define AKEYCODE_F7 137
#define AKEYCODE_F8 138
#define AKEYCODE_F9 139
#define AKEYCODE_F10 140
#define AKEYCODE_F11 141
#define AKEYCODE_F12 142

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
  SFG_Window* window = fgDisplay.pDisplay.single_window;

  int32_t keypress = -1;

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
    // Note: Android generates repeat events when key is left
    // pressed - just what like GLUT expects
    int32_t keyboard_metastate = AKeyEvent_getMetaState(event);

    if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
      int32_t code = AKeyEvent_getKeyCode(event);

      switch (code) {
      case AKEYCODE_F1:  keypress = GLUT_KEY_F1;  break;
      case AKEYCODE_F2:  keypress = GLUT_KEY_F2;  break;
      case AKEYCODE_F3:  keypress = GLUT_KEY_F3;  break;
      case AKEYCODE_F4:  keypress = GLUT_KEY_F4;  break;
      case AKEYCODE_F5:  keypress = GLUT_KEY_F5;  break;
      case AKEYCODE_F6:  keypress = GLUT_KEY_F6;  break;
      case AKEYCODE_F7:  keypress = GLUT_KEY_F7;  break;
      case AKEYCODE_F8:  keypress = GLUT_KEY_F8;  break;
      case AKEYCODE_F9:  keypress = GLUT_KEY_F9;  break;
      case AKEYCODE_F10: keypress = GLUT_KEY_F10; break;
      case AKEYCODE_F11: keypress = GLUT_KEY_F11; break;
      case AKEYCODE_F12: keypress = GLUT_KEY_F12; break;

      case AKEYCODE_PAGE_UP:   keypress = GLUT_KEY_PAGE_UP;   break;
      case AKEYCODE_PAGE_DOWN: keypress = GLUT_KEY_PAGE_DOWN; break;
      case AKEYCODE_MOVE_HOME: keypress = GLUT_KEY_HOME;      break;
      case AKEYCODE_MOVE_END:  keypress = GLUT_KEY_END;       break;
      case AKEYCODE_INSERT:    keypress = GLUT_KEY_INSERT;    break;

      case AKEYCODE_DPAD_UP:    keypress = GLUT_KEY_UP;    break;
      case AKEYCODE_DPAD_DOWN:  keypress = GLUT_KEY_DOWN;  break;
      case AKEYCODE_DPAD_LEFT:  keypress = GLUT_KEY_LEFT;  break;
      case AKEYCODE_DPAD_RIGHT: keypress = GLUT_KEY_RIGHT; break;

      case AKEYCODE_ALT_LEFT:    keypress = GLUT_KEY_ALT_L; break;
      case AKEYCODE_ALT_RIGHT:   keypress = GLUT_KEY_ALT_R; break;
      case AKEYCODE_SHIFT_LEFT:  keypress = GLUT_KEY_SHIFT_L; break;
      case AKEYCODE_SHIFT_RIGHT: keypress = GLUT_KEY_SHIFT_R; break;
      case AKEYCODE_CTRL_LEFT:   keypress = GLUT_KEY_CTRL_L; break;
      case AKEYCODE_CTRL_RIGHT:  keypress = GLUT_KEY_SHIFT_R; break;

      case AKEYCODE_DEL:
	/* The backspace key should be treated as an ASCII keypress: */
	/*
	  INVOKE_WCB( *window, Keyboard,
		    ( 8, window->State.MouseX, window->State.MouseY )
		    );
	*/
	break;
      case AKEYCODE_FORWARD_DEL:
	/* The delete key should be treated as an ASCII keypress: */
	/*
	  INVOKE_WCB( *window, Keyboard,
		    ( 127, window->State.MouseX, window->State.MouseY )
		    );
	*/
	break;
      case AKEYCODE_ESCAPE:
	/* The escape key should be treated as an ASCII keypress: */
	/*
	  INVOKE_WCB( *window, Keyboard,
		    ( 27, window->State.MouseX, window->State.MouseY )
		    );
	*/
	break;

      default:
	/* Let the system handle other keyevent (in particular the
	   Back button) */
	break;
      }

      if (keypress != -1 && FETCH_WCB(*window, Special)) {
	INVOKE_WCB( *window, Special,
		    ( keypress,
		      window->State.MouseX, window->State.MouseY )
		    );
	return 1;  /* handled */
      }
    }
  }
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
