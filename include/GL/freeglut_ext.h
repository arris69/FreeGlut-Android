#ifndef  FREEGLUT_EXT_H
#define  FREEGLUT_EXT_H

/*
 * freeglut_ext.h
 *
 * The non-GLUT-compatible extensions to the freeglut library include file
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 2 1999
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

#ifdef __cplusplus
    extern "C" {
#endif

/*
 * GLUT API Extension macro definitions -- behaviour when the user clicks on an "x" to close a window
 */
#define GLUT_ACTION_EXIT                         0
#define GLUT_ACTION_GLUTMAINLOOP_RETURNS         1
#define GLUT_ACTION_CONTINUE_EXECUTION           2

/*
 * GLUT API macro definitions -- the glutGet parameters
 */
#define  GLUT_ACTION_ON_WINDOW_CLOSE        0x01F9

/*
 * Process loop function, see freeglut_main.c
 */
FGAPI void    FGAPIENTRY glutMainLoopEvent( void );
FGAPI void    FGAPIENTRY glutLeaveMainLoop( void );

/*
 * Window-specific callback functions, see freeglut_callbacks.c
 */
FGAPI void    FGAPIENTRY glutCloseFunc( void (* callback)( void ) );
FGAPI void    FGAPIENTRY glutWMCloseFunc( void (* callback)( void ) );

/*
 * State setting and retrieval functions, see freeglut_state.c
 */
FGAPI void    FGAPIENTRY glutSetOption ( GLenum option_flag, int value ) ;

/*
 * Font stuff, see freeglut_font.c
 */
FGAPI int     FGAPIENTRY glutBitmapHeight( void* font );
FGAPI GLfloat FGAPIENTRY glutStrokeHeight( void* font );
FGAPI void    FGAPIENTRY glutBitmapString( void* font, const char *string );
FGAPI void    FGAPIENTRY glutStrokeString( void* font, const char *string );

/*
 * Geometry functions, see freeglut_geometry.c
 */
FGAPI void    FGAPIENTRY glutWireRhombicDodecahedron( void );
FGAPI void    FGAPIENTRY glutSolidRhombicDodecahedron( void );
FGAPI void    FGAPIENTRY glutWireSierpinskiSponge ( int num_levels, GLfloat offset[3], GLfloat scale ) ;
FGAPI void    FGAPIENTRY glutSolidSierpinskiSponge ( int num_levels, GLfloat offset[3], GLfloat scale ) ;

#ifdef __cplusplus
    }
#endif

/*** END OF FILE ***/

#endif /* FREEGLUT_EXT_H */

