/*
 * freeglut_init.c
 *
 * Various freeglut initialization functions.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../include/GL/freeglut.h"
#include "freeglut_internal.h"

/*
 * TODO BEFORE THE STABLE RELEASE:
 *
 *  fgDeinitialize()        -- Win32's OK, X11 needs the OS-specific
 *                             deinitialization done
 *  glutInitDisplayString() -- display mode string parsing
 *
 * Wouldn't it be cool to use gettext() for error messages? I just love
 * bash saying  "nie znaleziono pliku" instead of "file not found" :)
 * Is gettext easily portable?
 */

/* -- GLOBAL VARIABLES ----------------------------------------------------- */

/*
 * A structure pointed by g_pDisplay holds all information
 * regarding the display, screen, root window etc.
 */
SFG_Display fgDisplay;

/*
 * The settings for the current freeglut session
 */
SFG_State fgState = { { -1, -1, GL_FALSE },  /* Position */
                      { 300, 300, GL_TRUE }, /* Size */
                      GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH,  /* DisplayMode */
                      GL_FALSE,              /* Initalized */
                      GL_FALSE,              /* ForceDirectContext */
                      GL_TRUE,               /* TryDirectContext */
                      GL_FALSE,              /* ForceIconic */
                      GL_FALSE,              /* UseCurrentContext */
                      GL_FALSE,              /* GLDebugSwitch */
                      GL_FALSE,              /* XSyncSwitch */
                      GL_TRUE,               /* IgnoreKeyRepeat */
                      0,                     /* FPSInterval */
                      0,                     /* SwapCount */
                      0,                     /* SwapTime */
#if TARGET_HOST_WIN32
                      { 0, GL_FALSE },       /* Time */
#else
                      { { 0, 0 }, GL_FALSE },
#endif
                      { NULL, NULL },         /* Timers */
                      NULL,                   /* IdleCallback */
                      0,                      /* ActiveMenus */
                      NULL,                   /* MenuStateCallback */
                      NULL,                   /* MenuStatusCallback */
                      { 640, 480, GL_TRUE },  /* GameModeSize */
                      16,                     /* GameModeDepth */
                      72,                     /* GameModeRefresh */
                      GLUT_ACTION_EXIT,       /* ActionOnWindowClose */
                      GLUT_EXEC_STATE_INIT    /* ExecState */
} ;


/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

/*
 * A call to this function should initialize all the display stuff...
 */
void fgInitialize( const char* displayName )
{
#if TARGET_HOST_UNIX_X11
    fgDisplay.Display = XOpenDisplay( displayName );

    if( fgDisplay.Display == NULL )
        fgError( "failed to open display '%s'", XDisplayName( displayName ) );

    if( !glXQueryExtension( fgDisplay.Display, NULL, NULL ) )
        fgError( "OpenGL GLX extension not supported by display '%s'",
            XDisplayName( displayName ) );

    fgDisplay.Screen = DefaultScreen( fgDisplay.Display );
    fgDisplay.RootWindow = RootWindow(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.ScreenWidth  = DisplayWidth(
        fgDisplay.Display,
        fgDisplay.Screen
    );
    fgDisplay.ScreenHeight = DisplayHeight(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.ScreenWidthMM = DisplayWidthMM(
        fgDisplay.Display,
        fgDisplay.Screen
    );
    fgDisplay.ScreenHeightMM = DisplayHeightMM(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.Connection = ConnectionNumber( fgDisplay.Display );

    /*
     * Create the window deletion atom
     */
    fgDisplay.DeleteWindow = XInternAtom(
        fgDisplay.Display,
        "WM_DELETE_WINDOW",
        FALSE
    );

#elif TARGET_HOST_WIN32

    WNDCLASS wc;
    ATOM atom;

    /*
     * What we need to do is to initialize the fgDisplay global structure here...
     */
    fgDisplay.Instance = GetModuleHandle( NULL );

    atom = GetClassInfo( fgDisplay.Instance, "FREEGLUT", &wc );
    if( atom == 0 )
    {
        ZeroMemory( &wc, sizeof(WNDCLASS) );

        /*
         * Each of the windows should have its own device context...
         */
        wc.style          = CS_OWNDC;
        wc.lpfnWndProc    = fgWindowProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;
        wc.hInstance      = fgDisplay.Instance;
        wc.hIcon          = LoadIcon( fgDisplay.Instance, "GLUT_ICON" );
        if (!wc.hIcon)
          wc.hIcon        = LoadIcon( NULL, IDI_WINLOGO );

        wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
        wc.hbrBackground  = NULL;
        wc.lpszMenuName   = NULL;
        wc.lpszClassName  = "FREEGLUT";

        /*
         * Register the window class
         */
        atom = RegisterClass( &wc );
        assert( atom );
    }

    /*
     * The screen dimensions can be obtained via GetSystemMetrics() calls
     */
    fgDisplay.ScreenWidth  = GetSystemMetrics( SM_CXSCREEN );
    fgDisplay.ScreenHeight = GetSystemMetrics( SM_CYSCREEN );

    {
        HWND desktop = GetDesktopWindow( );
        HDC  context = GetDC( desktop );

        fgDisplay.ScreenWidthMM  = GetDeviceCaps( context, HORZSIZE );
        fgDisplay.ScreenHeightMM = GetDeviceCaps( context, VERTSIZE );

        ReleaseDC( desktop, context );
    }

#endif

    fgJoystickInit( 0 );

    fgState.Initalized = GL_TRUE;
}

/*
 * Perform the freeglut deinitialization...
 */
void fgDeinitialize( void )
{
    SFG_Timer *timer;

    if( !fgState.Initalized )
    {
        fgWarning( "fgDeinitialize(): "
                   "no valid initialization has been performed" );
        return;
    }

    /* fgState.Initalized = GL_FALSE; */

    /*
     * If there was a menu created, destroy the rendering context
     */
    if( fgStructure.MenuContext )
    {
        free( fgStructure.MenuContext );
        fgStructure.MenuContext = NULL;
    }

    fgDestroyStructure( );

    while( timer = ( SFG_Timer * )fgState.Timers.First )
    {
        fgListRemove ( &fgState.Timers, &timer->Node );
        free( timer );
    }

    fgJoystickClose( );

    fgState.Initalized = GL_FALSE;

    fgState.Position.X = -1;
    fgState.Position.Y = -1;
    fgState.Position.Use = GL_FALSE;

    fgState.Size.X = 300;
    fgState.Size.Y = 300;
    fgState.Size.Use = GL_TRUE;

    fgState.DisplayMode = GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH;

    fgState.ForceDirectContext  = GL_FALSE;
    fgState.TryDirectContext    = GL_TRUE;
    fgState.ForceIconic         = GL_FALSE;
    fgState.UseCurrentContext   = GL_FALSE;
    fgState.GLDebugSwitch       = GL_FALSE;
    fgState.XSyncSwitch         = GL_FALSE;
    fgState.ActionOnWindowClose = GLUT_ACTION_EXIT ;
    fgState.ExecState           = GLUT_EXEC_STATE_INIT ;

    fgState.IgnoreKeyRepeat = GL_TRUE;

    fgState.GameModeSize.X  = 640;
    fgState.GameModeSize.Y  = 480;
    fgState.GameModeDepth   =  16;
    fgState.GameModeRefresh =  72;

    fgState.Time.Set = GL_FALSE;

    fgState.Timers.First = fgState.Timers.Last = NULL;
    fgState.IdleCallback = NULL;
    fgState.MenuStateCallback = ( FGCBMenuState )NULL;
    fgState.MenuStatusCallback = ( FGCBMenuStatus )NULL;

    fgState.SwapCount   = 0;
    fgState.SwapTime    = 0;
    fgState.FPSInterval = 0;

    if( fgState.ProgramName )
    {
        free( fgState.ProgramName );
        fgState.ProgramName = NULL;
    }
    

#if TARGET_HOST_UNIX_X11

    /*
     * Make sure all X-client data we have created will be destroyed on
     * display closing
     */
    XSetCloseDownMode( fgDisplay.Display, DestroyAll );

    /*
     * Close the display connection, destroying all windows we have
     * created so far
     */
    XCloseDisplay( fgDisplay.Display );

#endif
}

/*
 * Everything inside the following #ifndef is copied from the X sources.
 */

#ifndef TARGET_HOST_UNIX_X11

#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */

static int
ReadInteger(char *string, char **NextString)
{
    register int Result = 0;
    int Sign = 1;
    
    if (*string == '+')
	string++;
    else if (*string == '-')
    {
	string++;
	Sign = -1;
    }
    for (; (*string >= '0') && (*string <= '9'); string++)
    {
	Result = (Result * 10) + (*string - '0');
    }
    *NextString = string;
    if (Sign >= 0)
	return (Result);
    else
	return (-Result);
}

static int XParseGeometry (
_Xconst char *string,
int *x,
int *y,
unsigned int *width,    /* RETURN */
unsigned int *height)    /* RETURN */
{
	int mask = NoValue;
	register char *strind;
	unsigned int tempWidth = 0, tempHeight = 0;
	int tempX = 0, tempY = 0;
	char *nextCharacter;

	if ( (string == NULL) || (*string == '\0')) return(mask);
	if (*string == '=')
		string++;  /* ignore possible '=' at beg of geometry spec */

	strind = (char *)string;
	if (*strind != '+' && *strind != '-' && *strind != 'x') {
		tempWidth = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter) 
		    return (0);
		strind = nextCharacter;
		mask |= WidthValue;
	}

	if (*strind == 'x' || *strind == 'X') {	
		strind++;
		tempHeight = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= HeightValue;
	}

	if ((*strind == '+') || (*strind == '-')) {
		if (*strind == '-') {
  			strind++;
			tempX = -ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return (0);
			strind = nextCharacter;
			mask |= XNegative;

		}
		else
		{	strind++;
			tempX = ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return(0);
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-')) {
			if (*strind == '-') {
				strind++;
				tempY = -ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
				mask |= YNegative;

			}
			else
			{
				strind++;
				tempY = ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
			}
			mask |= YValue;
		}
	}
	
	/* If strind isn't at the end of the string the it's an invalid
		geometry specification. */

	if (*strind != '\0') return (0);

	if (mask & XValue)
	    *x = tempX;
 	if (mask & YValue)
	    *y = tempY;
	if (mask & WidthValue)
            *width = tempWidth;
	if (mask & HeightValue)
            *height = tempHeight;
	return (mask);
}
#endif

/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Perform initialization. This usually happens on the program startup
 * and restarting after glutMainLoop termination...
 */
void FGAPIENTRY glutInit( int* pargc, char** argv )
{
    char* displayName = NULL;
    char* geometry = NULL;
    int i, j, argc = *pargc;

    if (pargc && *pargc && argv && *argv && **argv)
        fgState.ProgramName = strdup (*argv);
    else
        fgState.ProgramName = strdup ("");
    if( !fgState.ProgramName )
        fgError ("Could not allocate space for the program's name.");

    if( fgState.Initalized )
        fgError( "illegal glutInit() reinitialization attemp" );

    fgCreateStructure( );

    fgElapsedTime( );

    /* check if GLUT_FPS env var is set */
    {
        const char *fps = getenv( "GLUT_FPS" );
        if( fps )
        {
            sscanf( fps, "%d", &fgState.FPSInterval );
            if( fgState.FPSInterval <= 0 )
                fgState.FPSInterval = 5000;  /* 5000 milliseconds */
        }
    }

    displayName = getenv( "DISPLAY");

    for( i = 1; i < argc; i++ )
    {
        if( strcmp( argv[ i ], "-display" ) == 0 )
        {
            if( ++i >= argc )
                fgError( "-display parameter must be followed by display name" );

            displayName = argv[ i ];

            argv[ i - 1 ] = NULL;
            argv[ i     ] = NULL;
            ( *pargc ) -= 2;
        }
        else if( strcmp( argv[ i ], "-geometry" ) == 0 )
        {
            if( ++i >= argc )
                fgError( "-geometry parameter must be followed by window "
                         "geometry settings" );

            geometry = argv[ i ];

            argv[ i - 1 ] = NULL;
            argv[ i     ] = NULL;
            ( *pargc ) -= 2;
        }
        else if( strcmp( argv[ i ], "-direct" ) == 0)
        {
            if( ! fgState.TryDirectContext )
                fgError( "parameters ambiguity, -direct and -indirect "
                    "cannot be both specified" );

            fgState.ForceDirectContext = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-indirect" ) == 0 )
        {
            if( fgState.ForceDirectContext )
                fgError( "parameters ambiguity, -direct and -indirect "
                    "cannot be both specified" );

            fgState.TryDirectContext = GL_FALSE;
            argv[ i ] = NULL;
            (*pargc)--;
        }
        else if( strcmp( argv[ i ], "-iconic" ) == 0 )
        {
            fgState.ForceIconic = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-gldebug" ) == 0 )
        {
            fgState.GLDebugSwitch = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-sync" ) == 0 )
        {
            fgState.XSyncSwitch = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
    }

    /*
     * Compact {argv}.
     */
    j = 2 ;
    for( i = 1; i < *pargc; i++, j++ )
    {
        if( argv[ i ] == NULL )
        {
            /* Guaranteed to end because there are "*pargc" arguments left */
            while ( argv[ j ] == NULL )
                j++;
            argv[ i ] = argv[ j ] ;
        }
    }

    /*
     * Have the display created now. As I am too lazy to implement
     * the program arguments parsing, we will have the DISPLAY
     * environment variable used for opening the X display:
     *
     * XXX The above comment is rather unclear.  We have just
     * XXX completed parsing of the program arguments for GLUT
     * XXX parameters.  We obviously canNOT parse the application-
     * XXX specific parameters.  Can someone re-write the above
     * XXX more clearly?
     */
    fgInitialize( displayName );

    /*
     * Geometry parsing deffered until here because we may need the screen
     * size.
     */

    if (geometry )
    {
        int mask = XParseGeometry( geometry,
                                   &fgState.Position.X, &fgState.Position.Y,
                                   &fgState.Size.X, &fgState.Size.Y );

        if( (mask & (WidthValue|HeightValue)) == (WidthValue|HeightValue) )
            fgState.Size.Use = GL_TRUE;

        if( mask & XNegative )
            fgState.Position.X += fgDisplay.ScreenWidth - fgState.Size.X;

        if( mask & YNegative )
            fgState.Position.Y += fgDisplay.ScreenHeight - fgState.Size.Y;

        if( (mask & (XValue|YValue)) == (XValue|YValue) )
            fgState.Position.Use = GL_TRUE;
    }
}

/*
 * Sets the default initial window position for new windows
 */
void FGAPIENTRY glutInitWindowPosition( int x, int y )
{
    fgState.Position.X = x;
    fgState.Position.Y = y;

    if( ( x >= 0 ) && ( y >= 0 ) )
        fgState.Position.Use = GL_TRUE;
    else
        fgState.Position.Use = GL_FALSE;
}

/*
 * Sets the default initial window size for new windows
 */
void FGAPIENTRY glutInitWindowSize( int width, int height )
{
    fgState.Size.X = width;
    fgState.Size.Y = height;

    if( ( width > 0 ) && ( height > 0 ) )
        fgState.Size.Use = GL_TRUE;
    else
        fgState.Size.Use = GL_FALSE;
}

/*
 * Sets the default display mode for all new windows
 */
void FGAPIENTRY glutInitDisplayMode( unsigned int displayMode )
{
    /*
     * We will make use of this value when creating a new OpenGL context...
     */
    fgState.DisplayMode = displayMode;
}


/* -- INIT DISPLAY STRING PARSING ------------------------------------------ */

#if 0 /* FIXME: CJP */
/*
 * There is a discrete number of comparison operators we can encounter:
 *
 *     comparison ::= "=" | "!=" | "<" | ">" | "<=" | ">=" | "~"
 */
#define  FG_NONE           0x0000
#define  FG_EQUAL          0x0001
#define  FG_NOT_EQUAL      0x0002
#define  FG_LESS           0x0003
#define  FG_MORE           0x0004
#define  FG_LESS_OR_EQUAL  0x0005
#define  FG_MORE_OR_EQUAL  0x0006
#define  FG_CLOSEST        0x0007

/*
 * The caller can feed us with a number of capability tokens:
 *
 * capability ::= "alpha" | "acca" | "acc" | "blue" | "buffer" | "conformant" | "depth" | "double" |
 *                "green" | "index" | "num" | "red" | "rgba" | "rgb" | "luminance" | "stencil" |
 *                "single" | "stereo" | "samples" | "slow" | "win32pdf" | "xvisual" | "xstaticgray" |
 *                "xgrayscale" | "xstaticcolor" | "xpseudocolor" | "xtruecolor" | "xdirectcolor"
 */
static gchar* g_Tokens[] =
{
    "none", "alpha", "acca", "acc", "blue", "buffer", "conformant", "depth", "double", "green",
    "index", "num", "red", "rgba", "rgb", "luminance", "stencil", "single", "stereo", "samples",
    "slow", "win32pdf", "xvisual", "xstaticgray", "xgrayscale", "xstaticcolor", "xpseudocolor",
    "xtruecolor", "xdirectcolor", NULL
};

/*
 * The structure to hold the parsed display string tokens
 */
typedef struct tagSFG_Capability SFG_Capability;
struct tagSFG_Capability
{
    gint capability;        /* the capability token enumerator */
    gint comparison;        /* the comparison operator used    */
    gint value;             /* the value we're comparing to    */
};

/*
 * The scanner configuration for the init display string
 */
static GScannerConfig fgInitDisplayStringScannerConfig =
{
    ( " \t\r\n" )               /* cset_skip_characters     */,
    (
        G_CSET_a_2_z
        "_"
        G_CSET_A_2_Z
    )                                        /* cset_identifier_first    */,
    (
        G_CSET_a_2_z
        "_0123456789"
        G_CSET_A_2_Z
        G_CSET_LATINS
        G_CSET_LATINC
        "<>!=~"
    )                                        /* cset_identifier_nth      */,
    ( "#\n" )                            /* cpair_comment_single     */,
    FALSE                                    /* case_sensitive           */,
    TRUE                                    /* skip_comment_multi       */,
    TRUE                                    /* skip_comment_single      */,
    TRUE                                    /* scan_comment_multi       */,
    TRUE                                    /* scan_identifier          */,
    FALSE                                    /* scan_identifier_1char    */,
    FALSE                                    /* scan_identifier_NULL     */,
    TRUE                                    /* scan_symbols             */,
    FALSE                                    /* scan_binary              */,
    TRUE                                    /* scan_octal               */,
    TRUE                                    /* scan_float               */,
    TRUE                                    /* scan_hex                 */,
    FALSE                                    /* scan_hex_dollar          */,
    TRUE                                    /* scan_string_sq           */,
    TRUE                                    /* scan_string_dq           */,
    TRUE                                    /* numbers_2_int            */,
    FALSE                                    /* int_2_float              */,
    FALSE                                    /* identifier_2_string      */,
    TRUE                                    /* char_2_token             */,
    FALSE                                    /* symbol_2_token           */,
    FALSE                                    /* scope_0_fallback         */,
};

/*
 * Sets the default display mode for all new windows using a string
 */
void FGAPIENTRY glutInitDisplayString( char* displayMode )
{
    /*
     * display_string ::= (switch)
     * switch         ::= capability [comparison value]
     * comparison     ::= "=" | "!=" | "<" | ">" | "<=" | ">=" | "~"
     * capability     ::= "alpha" | "acca" | "acc" | "blue" | "buffer" | "conformant" |
     *                    "depth" | "double" | "green" | "index" | "num" | "red" | "rgba" |
     *                    "rgb" | "luminance" | "stencil" | "single" | "stereo" |
     *                    "samples" | "slow" | "win32pdf" | "xvisual" | "xstaticgray" |
     *                    "xgrayscale" | "xstaticcolor" | "xpseudocolor" |
     *                    "xtruecolor" | "xdirectcolor"
     * value          ::= 0..9 [value]
     *
     * The display string grammar. This should be EBNF, but I couldn't find the definitions so, to
     * clarify: (expr) means 0 or more times the expression, [expr] means 0 or 1 times expr.
     *
     * Create a new GLib lexical analyzer to process the display mode string
     */
    GScanner* scanner = g_scanner_new( &fgInitDisplayStringScannerConfig );
    GList* caps = NULL;
    gint i;

    /*
     * Fail if the display mode string is empty or the scanner failed to initialize
     */
    freeglut_return_if_fail( (scanner != NULL) && (strlen( displayMode ) > 0) );

    /*
     * Set the scanner's input name (for debugging)
     */
    scanner->input_name = "glutInitDisplayString";

    /*
     * Start the lexical analysis of the extensions string
     */
    g_scanner_input_text( scanner, displayMode, strlen( displayMode ) );

    /*
     * While there are any more tokens to be checked...
     */
    while( !g_scanner_eof( scanner ) )
    {
        /*
         * Actually we're expecting only string tokens
         */
        GTokenType tokenType = g_scanner_get_next_token( scanner );

        /*
         * We are looking for identifiers
         */
        if( tokenType == G_TOKEN_IDENTIFIER )
        {
            gchar* capability  = NULL;  /* the capability identifier string (always present) */
            gint   capID       =    0;  /* the capability identifier value (from g_Tokens)   */
            gint   comparison  =    0;  /* the comparison operator value, see definitions    */
            gchar* valueString = NULL;  /* if the previous one is present, this is needed    */
            gint   value       =    0;  /* the integer value converted using a strtol call   */
            SFG_Capability* capStruct;  /* the capability description structure              */

            /*
             * OK. The general rule of thumb that we always should be getting a capability identifier
             * string (see the grammar description). If it is followed by a comparison identifier, then
             * there must follow an integer value we're comparing the capability to...
             *
             * Have the current token analyzed with that in mind...
             */
            for( i=0; i<(gint) strlen( scanner->value.v_identifier ); i++ )
            {
                gchar c = scanner->value.v_identifier[ i ];

                if( (c == '=') || (c == '!') || (c == '<') || (c == '>') || (c == '~') )
                    break;
            }

            /*
             * Here we go with the length of the capability identifier string.
             * In the worst of cases, it is as long as the token identifier.
             */
            capability = g_strndup( scanner->value.v_identifier, i );

            /*
             * OK. Is there a chance for comparison and value identifiers to follow?
             * Note: checking against i+1 as this handles two cases: single character
             * comparison operator and first of value's digits, which must always be
             * there, or the two-character comparison operators.
             */
            if( (i + 1) < (gint) strlen( scanner->value.v_identifier ) )
            {
                /*
                 * Yeah, indeed, it is the i-th character to start the identifier, then.
                 */
                gchar c1 = scanner->value.v_identifier[ i + 0 ];
                gchar c2 = scanner->value.v_identifier[ i + 1 ];

                if( (c1 == '=')                ) { i += 1; comparison = FG_EQUAL;         } else
                if( (c1 == '!') && (c2 == '=') ) { i += 2; comparison = FG_NOT_EQUAL;     } else
                if( (c1 == '<') && (c2 == '=') ) { i += 2; comparison = FG_LESS_OR_EQUAL; } else
                if( (c1 == '>') && (c2 == '=') ) { i += 2; comparison = FG_MORE_OR_EQUAL; } else
                if( (c1 == '<')                ) { i += 1; comparison = FG_LESS;          } else
                if( (c1 == '>')                ) { i += 1; comparison = FG_MORE;          } else
                if( (c1 == '~')                ) { i += 1; comparison = FG_CLOSEST;       } else
                g_warning( "invalid comparison operator in token `%s'", scanner->value.v_identifier );
            }

            /*
             * Grab the value string that must follow the comparison operator...
             */
            if( comparison != FG_NONE && i < (gint) strlen( scanner->value.v_identifier ) )
            {
                valueString = strdup( scanner->value.v_identifier + i );
                if (!valueString)
                    fgError ("Could not allocate an internal string.");
            }                

            /*
             * If there was a value string, convert it to integer...
             */
            if( comparison != FG_NONE && strlen( valueString ) > 0 )
                value = strtol( valueString, NULL, 0 );

            /*
             * Now we need to match the capability string and its ID
             */
            for( i=0; g_Tokens[ i ]!=NULL; i++ )
            {
                if( strcmp( capability, g_Tokens[ i ] ) == 0 )
                {
                    /*
                     * Looks like we've found the one we were looking for
                     */
                    capID = i;
                    break;
                }
            }

            /*
             * Create a new capability description structure
             */
            capStruct = g_new0( SFG_Capability, 1 );

            /*
             * Fill in the cap's values, as we have parsed it:
             */
            capStruct->capability =      capID;
            capStruct->comparison = comparison;
            capStruct->value      =      value;

            /*
             * Add the new capabaility to the caps list
             */
            caps = g_list_append( caps, capStruct );

            /*
             * Clean up the local mess and keep rolling on
             */
            g_free( valueString );
            g_free( capability );
        }
    }

    /*
     * Now that we have converted the string into somewhat more machine-friendly
     * form, proceed with matching the frame buffer configuration...
     *
     * The caps list could be passed to a function that would try finding the closest 
     * matching pixel format, visual, frame buffer configuration or whatever. It would 
     * be good to convert the glutInitDisplayMode() to use the same method.
     */
#if 0
    g_message( "found %i capability preferences", g_list_length( caps ) );

    g_message( "token `%s': cap: %i, com: %i, val: %i",
        scanner->value.v_identifier,
        capStruct->capability,
        capStruct->comparison,
        capStruct->value
    );
#endif

    /*
     * Free the capabilities we have parsed
     */
    for( i=0; i<(gint) g_list_length( caps ); i++ )
        g_free( g_list_nth( caps, i )->data );

    /*
     * Destroy the capabilities list itself
     */
    g_list_free( caps );

    /*
     * Free the lexical scanner now...
     */
    g_scanner_destroy( scanner );
}
#endif

#define NUM_TOKENS             28
static char* Tokens[] =
{
    "alpha", "acca", "acc", "blue", "buffer", "conformant", "depth", "double", "green",
    "index", "num", "red", "rgba", "rgb", "luminance", "stencil", "single", "stereo", "samples",
    "slow", "win32pdf", "xvisual", "xstaticgray", "xgrayscale", "xstaticcolor", "xpseudocolor",
    "xtruecolor", "xdirectcolor"
};

static int TokenLengths[] =
{
         5,      4,     3,      4,        6,           10,       5,        6,       5,
         5,     3,     3,      4,     3,           9,         7,        6,        6,         7,
        4,          8,         7,            11,           10,             12,             12,
             10,             12
};

void FGAPIENTRY glutInitDisplayString( const char* displayMode )
{
  int glut_state_flag = 0 ;
  /*
   * Unpack a lot of options from a character string.  The options are delimited by blanks or tabs.
   */
  char *token ;
  int len = strlen ( displayMode ) ;
  char *buffer = (char *)malloc ( (len+1) * sizeof(char) ) ;
  memcpy ( buffer, displayMode, len ) ;
  buffer[len] = '\0' ;

  token = strtok ( buffer, " \t" ) ;
  while ( token )
  {
    /*
     * Process this token
     */
    int i ;
    for ( i = 0; i < NUM_TOKENS; i++ )
    {
      if ( strncmp ( token, Tokens[i], TokenLengths[i] ) == 0 ) break ;
    }

    switch ( i )
    {
    case 0 :  /* "alpha":  Alpha color buffer precision in bits */
      glut_state_flag |= GLUT_ALPHA ;  /* Somebody fix this for me! */
      break ;

    case 1 :  /* "acca":  Red, green, blue, and alpha accumulation buffer precision in bits */
      break ;

    case 2 :  /* "acc":  Red, green, and blue accumulation buffer precision in bits with zero bits alpha */
      glut_state_flag |= GLUT_ACCUM ;  /* Somebody fix this for me! */
      break ;

    case 3 :  /* "blue":  Blue color buffer precision in bits */
      break ;

    case 4 :  /* "buffer":  Number of bits in the color index color buffer */
      break ;

    case 5 :  /* "conformant":  Boolean indicating if the frame buffer configuration is conformant or not */
      break ;

    case 6 :  /* "depth":  Number of bits of precsion in the depth buffer */
      glut_state_flag |= GLUT_DEPTH ;  /* Somebody fix this for me! */
      break ;

    case 7 :  /* "double":  Boolean indicating if the color buffer is double buffered */
      glut_state_flag |= GLUT_DOUBLE ;
      break ;

    case 8 :  /* "green":  Green color buffer precision in bits */
      break ;

    case 9 :  /* "index":  Boolean if the color model is color index or not */
      glut_state_flag |= GLUT_INDEX ;
      break ;

    case 10 :  /* "num":  A special capability  name indicating where the value represents the Nth frame buffer configuration matching the description string */
      break ;

    case 11 :  /* "red":  Red color buffer precision in bits */
      break ;

    case 12 :  /* "rgba":  Number of bits of red, green, blue, and alpha in the RGBA color buffer */
      glut_state_flag |= GLUT_RGBA ;  /* Somebody fix this for me! */
      break ;

    case 13 :  /* "rgb":  Number of bits of red, green, and blue in the RGBA color buffer with zero bits alpha */
      glut_state_flag |= GLUT_RGB ;  /* Somebody fix this for me! */
      break ;

    case 14 :  /* "luminance":  Number of bits of red in the RGBA and zero bits of green, blue (alpha not specified) of color buffer precision */
      glut_state_flag |= GLUT_LUMINANCE ;  /* Somebody fix this for me! */
      break ;

    case 15 :  /* "stencil":  Number of bits in the stencil buffer */
      glut_state_flag |= GLUT_STENCIL ;  /* Somebody fix this for me! */
      break ;

    case 16 :  /* "single":  Boolean indicate the color buffer is single buffered */
      glut_state_flag |= GLUT_SINGLE ;
      break ;

    case 17 :  /* "stereo":  Boolean indicating the color buffer supports OpenGL-style stereo */
      glut_state_flag |= GLUT_STEREO ;
      break ;

    case 18 :  /* "samples":  Indicates the number of multisamples to use based on GLX's SGIS_multisample extension (for antialiasing) */
      glut_state_flag |= GLUT_MULTISAMPLE ;  /* Somebody fix this for me! */
      break ;

    case 19 :  /* "slow":  Boolean indicating if the frame buffer configuration is slow or not */
      break ;

    case 20 :  /* "win32pdf":  matches the Win32 Pixel Format Descriptor by number */
#if TARGET_HOST_WIN32
#endif
      break ;

    case 21 :  /* "xvisual":  matches the X visual ID by number */
#if TARGET_HOST_UNIX_X11
#endif
      break ;

    case 22 :  /* "xstaticgray":  boolean indicating if the frame buffer configuration's X visual is of type StaticGray */
#if TARGET_HOST_UNIX_X11
#endif
      break ;

    case 23 :  /* "xgrayscale":  boolean indicating if the frame buffer configuration's X visual is of type GrayScale */
#if TARGET_HOST_UNIX_X11
#endif
      break ;

    case 24 :  /* "xstaticcolor":  boolean indicating if the frame buffer configuration's X visual is of type StaticColor */
#if TARGET_HOST_UNIX_X11
#endif
      break ;

    case 25 :  /* "xpseudocolor":  boolean indicating if the frame buffer configuration's X visual is of type PseudoColor */
#if TARGET_HOST_UNIX_X11
#endif
      break ;

    case 26 :  /* "xtruecolor":  boolean indicating if the frame buffer configuration's X visual is of type TrueColor */
#if TARGET_HOST_UNIX_X11
#endif
      break ;

    case 27 :  /* "xdirectcolor":  boolean indicating if the frame buffer configuration's X visual is of type DirectColor */
#if TARGET_HOST_UNIX_X11
#endif
      break ;

    case 28 :  /* Unrecognized */
      printf ( "WARNING - Display string token not recognized:  %s\n", token ) ;
      break ;
    }

    token = strtok ( NULL, " \t" ) ;
  }

  free ( buffer ) ;

  /*
   * We will make use of this value when creating a new OpenGL context...
   */
  fgState.DisplayMode = glut_state_flag;
}

/*** END OF FILE ***/
