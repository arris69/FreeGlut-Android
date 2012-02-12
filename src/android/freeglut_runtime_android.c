/*
 * freeglut_runtime_android.c
 *
 * Android runtime
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

/* Parts taken from Android NDK's 'native-activity' sample : */
/*
 * Copyright (C) 2010 The Android Open Source Project
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/native_window.h>
#include "android/native_app_glue/android_native_app_glue.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "FreeGLUT", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "FreeGLUT", __VA_ARGS__))

#include <GL/freeglut.h>
#include "Common/freeglut_internal.h"

extern int main(int argc, char* argv[]);


static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
    LOGI("onNativeWindowResized: %p\n", activity);
    // Sent an event to the queue so it gets handled in the app thread
    // after other waiting events, rather than asynchronously in the
    // native_app_glue event thread:
    struct android_app* app = (struct android_app*)activity->instance;
    // TODO: too early?? the detected window size is the previous one
    android_app_write_cmd(app, APP_CMD_WINDOW_RESIZED);
}
static void onContentRectChanged(ANativeActivity* activity, const ARect* rect) {
    LOGI("onContentRectChanged: l=%d,t=%d,r=%d,b=%d", rect->left, rect->top, rect->right, rect->bottom);
    // Make Android realize the screen size changed, needed when the
    // GLUT app refreshes only on event rather than in loop.  Beware
    // that we're not in the GLUT thread here, but in the event one.
    struct android_app* app = (struct android_app*)activity->instance;
    // TODO: too early?? the detected window size is the previous one
    if (app->window)
      android_app_write_cmd(app, APP_CMD_WINDOW_RESIZED);
}

/**
 * Process the next input event.
 */
static int32_t handle_input(struct android_app* app, AInputEvent* event) {
  android_app_write_cmd(app, APP_CMD_WINDOW_RESIZED);
  return 0;  /* not handled */
}

/**
 * Process the next main command.
 */
static void handle_cmd(struct android_app* app, int32_t cmd) {
  printf("runtime: handle_cmd %d (wait for %d) tid=%d\n", cmd, APP_CMD_INIT_WINDOW, gettid());
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    /* The system has asked us to save our current state.  Do so. */
    break;
  case APP_CMD_INIT_WINDOW:
    /* The window is being shown, get it ready. */
    /* glPlatformOpenWindow is waiting for Handle to be defined: */
    fgDisplay.pDisplay.single_window->Window.Handle = app->window;
    break;
  case APP_CMD_TERM_WINDOW:
    /* The window is being hidden or closed, clean it up. */
    fgDestroyWindow(fgDisplay.pDisplay.single_window);
    break;
  case APP_CMD_DESTROY:
    /* Not reached because GLUT exit()s when last window is closed */
    break;
  case APP_CMD_GAINED_FOCUS:
    break;
  case APP_CMD_LOST_FOCUS:
    break;
  case APP_CMD_CONFIG_CHANGED:
    /* Handle rotation / orientation change */
    break;
  case APP_CMD_WINDOW_RESIZED:
    {
      SFG_Window* window = fgDisplay.pDisplay.single_window;
      int32_t width = ANativeWindow_getWidth(window->Window.Handle);
      int32_t height = ANativeWindow_getHeight(window->Window.Handle);
      LOGI("APP_CMD_WINDOW_RESIZED-engine: w=%d, h=%d", width, height);
      if( FETCH_WCB( *window, Reshape ) )
	INVOKE_WCB( *window, Reshape, ( width, height ) );
      else
	glViewport( 0, 0, width, height );
      glutPostRedisplay();
    }
  }
}

/**
 * Extract all .apk assets to the application directory so they can be
 * accessed using accessed.
 * TODO: parse directories recursively
 */
static void extract_assets(struct android_app* state_param) {
  /* Get usable JNI context */
  JNIEnv* env = state_param->activity->env;
  JavaVM* vm = state_param->activity->vm;
  (*vm)->AttachCurrentThread(vm, &env, NULL);
  
  /* Get a handle on our calling NativeActivity instance */
  {
    jclass activityClass = (*env)->GetObjectClass(env, state_param->activity->clazz);
    
    /* Get path to cache dir (/data/data/org.myapp/cache) */
    jmethodID getCacheDir = (*env)->GetMethodID(env, activityClass, "getCacheDir", "()Ljava/io/File;");
    jobject file = (*env)->CallObjectMethod(env, state_param->activity->clazz, getCacheDir);
    jclass fileClass = (*env)->FindClass(env, "java/io/File");
    jmethodID getAbsolutePath = (*env)->GetMethodID(env, fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring jpath = (jstring)(*env)->CallObjectMethod(env, file, getAbsolutePath);
    const char* app_dir = (*env)->GetStringUTFChars(env, jpath, NULL);
    
    /* chdir in the application cache directory */
    LOGI("app_dir: %s", app_dir);
    chdir(app_dir);
    (*env)->ReleaseStringUTFChars(env, jpath, app_dir);
    
    /* Pre-extract assets, to avoid Android-specific file opening */
    {
      AAssetManager* mgr = state_param->activity->assetManager;
      AAssetDir* assetDir = AAssetManager_openDir(mgr, "");
      const char* filename = (const char*)NULL;
      while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
	AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_STREAMING);
	char buf[BUFSIZ];
	int nb_read = 0;
	FILE* out = fopen(filename, "w");
	while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
	  fwrite(buf, nb_read, 1, out);
	fclose(out);
	AAsset_close(asset);
      }
      AAssetDir_close(assetDir);
    }
  }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state_param) {
  LOGI("android_main");

  // Register window resize callback
  state_param->activity->callbacks->onNativeWindowResized = onNativeWindowResized;
  state_param->activity->callbacks->onContentRectChanged = onContentRectChanged;
  
  state_param->onAppCmd = handle_cmd;
  state_param->onInputEvent = handle_input;

  extract_assets(state_param);

  /* Call user's main */
  {
    char progname[5] = "self";
    char* argv[] = {progname, NULL};
    main(1, argv);
  }

  LOGI("android_main: end");
  exit(0);
}
