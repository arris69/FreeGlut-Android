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
void fghCreateEGLContext( EGLNativeWindowType handle ) {
  printf("fghCreateEGLContext %p\n", (void*)handle);
  //SFG_Window* window = fgWindowByID(glutGetWindow());
  SFG_Window* window = fgDisplay.pDisplay.single_window;
  window->Window.Handle = handle;
  window->Window.pContext.init = GL_TRUE;

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
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  /* TODO : apply DisplayMode */
  /*        (GLUT_DEPTH already applied in attribs[] above) */

  eglInitialize(display, 0, 0);

  /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria */
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

  ANativeWindow_setBuffersGeometry(window->Window.Handle, 0, 0, format);

  surface = eglCreateWindowSurface(display, config, window->Window.Handle, NULL);
  /* Ensure OpenGLES 2.0 context */
  static const EGLint ctx_attribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attribs);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    fprintf(stderr, "Unable to eglMakeCurrent");
    window->Window.pContext.eglSurface = EGL_NO_SURFACE;
    return;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  fgDisplay.pDisplay.eglDisplay = display;
  fgDisplay.pDisplay.eglContext = context;
  window->Window.pContext.eglSurface = surface;

  return;
}
