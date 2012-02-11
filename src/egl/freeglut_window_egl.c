/*
 * freeglut_display_android.c
 *
 * Window management methods for EGL
 *
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

/**
 * Initialize an EGL context for the current display.
 */
void fghCreateContext( ) {
  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  /* Ensure OpenGLES 2.0 context */
  printf("DisplayMode: %d (DEPTH %d)\n", fgState.DisplayMode, (fgState.DisplayMode & GLUT_DEPTH));
  const EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_DEPTH_SIZE, (fgState.DisplayMode & GLUT_DEPTH) ? 24 : 0,
    EGL_NONE
  };
  EGLint format;
  EGLint numConfigs;
  EGLConfig config;
  EGLContext context;

  EGLDisplay display = fgDisplay.pDisplay.eglDisplay;

  /* TODO : apply DisplayMode */
  /*        (GLUT_DEPTH already applied in attribs[] above) */

  /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria */
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

  /* Ensure OpenGLES 2.0 context */
  static const EGLint ctx_attribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attribs);

  fgDisplay.pDisplay.eglContext = context;
  fgDisplay.pDisplay.eglContextConfig = config;
  fgDisplay.pDisplay.eglContextFormat = format;
}

/*
 * Really opens a window when handle is available
 */
EGLSurface fghEGLPlatformOpenWindow( EGLNativeWindowType handle )
{
  EGLDisplay display = fgDisplay.pDisplay.eglDisplay;
  EGLContext context = fgDisplay.pDisplay.eglContext;
  EGLConfig  config  = fgDisplay.pDisplay.eglContextConfig;

  EGLSurface surface = eglCreateWindowSurface(display, config, handle, NULL);
  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    fprintf(stderr, "Unable to eglMakeCurrent");
    return;
  }

  //EGLint w, h;
  //eglQuerySurface(display, surface, EGL_WIDTH, &w);
  //eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  return surface;
}

/*
 * Closes a window, destroying the frame and OpenGL context
 */
void fgPlatformCloseWindow( SFG_Window* window )
{
  if (window->Window.pContext.eglSurface != EGL_NO_SURFACE) {
    eglDestroySurface(fgDisplay.pDisplay.eglDisplay, window->Window.pContext.eglSurface);
    window->Window.pContext.eglSurface = EGL_NO_SURFACE;
  }
}
