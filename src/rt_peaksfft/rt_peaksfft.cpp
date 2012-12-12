//-----------------------------------------------------------------------------
// name: rt_peaksfft.cpp
// desc: real-time spectral peaks synthesizer - from Perry's peaksfft.c
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry Cook (prc@cs.princeton.edu)
// library:
//         (STK) Synthesis ToolKit - Perry Cook and Gary Scavone
//         (FFT) CARL CMusic Distribution
// date: today
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

// STK
#include "RtAudio.h"
#include "Thread.h"

// OpenGL
#if defined(__OS_MACOSX__)
  #include <GLUT/glut.h>
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/glut.h>
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

#if defined(__OS_WINDOWS__)
  #include <process.h>
#else
  #include <unistd.h>
#endif

#ifdef __USE_LIBSNDFILE__
#include "sndfile.h"
#else
#include "util_sndfile.h"
#endif

#include "chuck_fft.h"
#include "pfft.h"



//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
void idleFunc( );
void displayFunc( );
void reshapeFunc( int width, int height );
void keyboardFunc( unsigned char, int, int );
void mouseFunc( int button, int state, int x, int y );
void initialize_graphics( );
bool initialize_audio( );
void initialize_analysis( );




//-----------------------------------------------------------------------------
// global variables and #defines
//-----------------------------------------------------------------------------
#define INC_VAL                 1.0f
#define THE__PI                 3.14159265359
#define THE_BUFFER_SIZE         ( RT_BUFFER_SIZE )
//#define SAMPLE                  float

// width and height of the window
GLsizei g_width = 800;
GLsizei g_height = 600;

// whether to animate
GLboolean g_rotate = GL_TRUE;
// fill mode
GLenum g_fillmode = GL_FILL;

// light 0 position
GLfloat g_light0_pos[4] = { 2.0f, 1.2f, 4.0f, 1.0f };

// light 1 parameters
GLfloat g_light1_ambient[] = { .2f, .2f, .2f, 1.0f };
GLfloat g_light1_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat g_light1_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat g_light1_pos[4] = { -2.0f, 0.0f, -4.0f, 1.0f };

// modelview stuff
GLfloat g_angle_y = 0.0f;
GLfloat g_inc = 0.0f;
GLfloat g_eye_y = 0;

// global audio buffer
polar g_polar_buffer[THE_BUFFER_SIZE/2];	polar g_polar_buffer_hop[THE_BUFFER_SIZE/2];
SAMPLE g_another_buffer[THE_BUFFER_SIZE];	SAMPLE g_another_buffer_hop[THE_BUFFER_SIZE];
SAMPLE g_syn_buffer[THE_BUFFER_SIZE];		
GLboolean g_ready = FALSE;
GLfloat g_window[THE_BUFFER_SIZE];
int g_buffer_size = THE_BUFFER_SIZE;
RtAudio * g_audio = NULL;
Mutex g_mutex;
float g_order = 10.0f;
#if defined(__LINUX_ALSA__)
unsigned int g_srate = 48000;
#else
unsigned int g_srate = 44100;
#endif

// stuff
pfft_data * g_pfft;
int g_npeaks = 1;
int g_hop;
float g_freq_warp = 1.0;
int g_low_bin = 0;
int g_high_bin = THE_BUFFER_SIZE / 2;

// gain
GLfloat g_gain = 1.0f;
GLfloat g_time_scale = 1.0f;
GLfloat g_freq_scale = 1.0f;
GLint g_time_view = 1;
GLint g_freq_view = 2;

// fullscreen
GLboolean g_fullscreen = FALSE;

// flags
GLint g_sndout = 1;
GLint g_sndin = 1;
GLboolean g_display = TRUE;
GLboolean g_raw = FALSE;
const char * g_filename = NULL;
GLboolean g_ctf = TRUE;

GLboolean g_running = TRUE;
GLboolean g_file_running = FALSE;
GLint g_buffer_count_a = 0;
GLint g_buffer_count_b = 0;

struct Pt2D { float x; float y; };
Pt2D ** g_spectrums = NULL;
unsigned int g_depth = 32;
GLboolean g_usedb = FALSE;
GLboolean g_wutrfall = TRUE;
float g_z = 0.0f;
float g_space = .15f;
GLboolean * g_draw = NULL;




//-----------------------------------------------------------------------------
// name: help()
// desc: ...
//-----------------------------------------------------------------------------
void help()
{
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "RT_PEAKSFFT\n" );
    fprintf( stderr, "Perry Cook, Ge Wang\n" );
    fprintf( stderr, "http://soundlab.cs.princeton.edu/\n" );
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "'s' - toggle fullscreen\n" );
    fprintf( stderr, "'=' - increase freq warp\n" );
    fprintf( stderr, "'-' - decrease freq warp\n" );
    fprintf( stderr, "'p', 'P' - increase high bin\n" );
    fprintf( stderr, "'o', 'O' - decrease high bin\n" );
	fprintf( stderr, "'t', 'T' - increase low bin\n" );
    fprintf( stderr, "'r', 'R' - decrease low bin\n" );
	fprintf( stderr, "'m' - increase # peaks\n" );
    fprintf( stderr, "'n' - decrease # peaks\n" );
    fprintf( stderr, "'w' - toggle wutrfall plot\n" );
    fprintf( stderr, "'d' - toggle dB plot for spectrum\n" );
    fprintf( stderr, "'f' - move spectrum + z\n" );
    fprintf( stderr, "'j' - move spectrum - z\n" );
    fprintf( stderr, "'e' - spacing more!\n" );
    fprintf( stderr, "'i' - spacing less!\n" );
    fprintf( stderr, "L, R mouse button - rotate left or right\n" );
    fprintf( stderr, "'h' - print this help message\n" );
    fprintf( stderr, "'q' - quit\n" );
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "\n" );
}




//-----------------------------------------------------------------------------
// name: usage()
// desc: ...
//-----------------------------------------------------------------------------
void usage()
{
    fprintf( stderr, "usage: rt_peaksfft --peaks# [filename]\n" );
}




//-----------------------------------------------------------------------------
// Name: main( )
// Desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    // initialize GLUT
    glutInit( &argc, argv );
    // double buffer, use rgb color, enable depth buffer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    // initialize the window size
    glutInitWindowSize( g_width, g_height );
    // set the window postion
    glutInitWindowPosition( 100, 100 );
    // create the window
    glutCreateWindow( "rt_peaksfft" );

    // set the idle function - called when idle
    glutIdleFunc( idleFunc );
    // set the display function - called when redrawing
    glutDisplayFunc( displayFunc );
    // set the reshape function - called when client area changes
    glutReshapeFunc( reshapeFunc );
    // set the keyboard function - called on keyboard events
    glutKeyboardFunc( keyboardFunc );
    // set the mouse function - called on mouse stuff
    glutMouseFunc( mouseFunc );

	// parse command line arguments
    int n = 1;
    while( n < argc )
    {
        if( strncmp( argv[n], "--peaks", 7 ) == 0 )
        {
            g_npeaks = atoi( argv[n]+7 );
            if( g_npeaks <= 0 )
            {
                fprintf( stderr, "rt_peaksfft: invalid # of peaks '%i'...\n", g_npeaks );
                usage();
                exit( 1 );
            }
        }
        n++;
    }

    // do our own initialization
    if( argc > 1 ) g_filename = argv[argc - 1];
    initialize_graphics( );
	fprintf(stderr, "done initing graphics\n");
    if( !initialize_audio( ) )
    {
        fprintf( stderr, "rt_peaksfft: error initializing audio, exiting...\n" );
        return -3;
    }
	fprintf(stderr, "done initing audio\n");
    initialize_analysis( );
	fprintf(stderr, "done initing analysis\n");

    // let GLUT handle the current thread from here
    glutMainLoop();
    

    return 0;
}




SNDFILE * g_fin = NULL;
SF_INFO g_finfo;
//-----------------------------------------------------------------------------
// name: cb()
// desc: audio callback
//-----------------------------------------------------------------------------
int cb( char * buffer, int buffer_size, void * user_data )
{
    // play
    if( !g_ready ) memcpy( buffer, (g_npeaks ? g_syn_buffer : g_another_buffer), buffer_size * sizeof(SAMPLE) );
    else memset( buffer, 0, buffer_size * sizeof(SAMPLE) );

    g_ready = TRUE;

    return 0;
}




//-----------------------------------------------------------------------------
// Name: initialize_audio( )
// Desc: set up audio capture and playback and initializes any application data
//-----------------------------------------------------------------------------
bool initialize_audio( )
{
	// open the file
    if( g_filename )
    {
        g_fin = sf_open( g_filename, SFM_READ, &g_finfo );
        if( !g_fin )
        {
            fprintf( stderr, "rt_peaksfft: cannot open file '%s' for input...\n", g_filename );
            return false;
        }

        g_srate = g_finfo.samplerate;
	}
	Stk::setSampleRate( g_srate );
	try
    {
        // open the audio device for capture and playback
        g_audio = new RtAudio( 0, g_sndout, 0, g_filename ? 0 : g_sndin,
            RTAUDIO_FLOAT32, g_srate, &g_buffer_size, 8 );
    }
    catch( StkError & e )
    {
        // exception
        fprintf( stderr, "%s\n", e.getMessage() );
        fprintf( stderr, "rt_peaksfft: cannot open audio device for capture/playback...\n" );
        return false;
    }

	// set the audio callback
    g_audio->setStreamCallback( cb, NULL );
	// start the audio
    g_audio->startStream( );
	// make the window
    hamming( g_window, g_buffer_size );
	
	return true;
}




//-----------------------------------------------------------------------------
// Name: initialize_analysis( )
// Desc: sets initial audio analysis parameters
//-----------------------------------------------------------------------------
void initialize_analysis( )
{
	g_pfft = pfft_create();
	g_pfft->srate = g_srate;
	g_hop = THE_BUFFER_SIZE;///4;
	g_pfft->hop = g_hop;
}




//-----------------------------------------------------------------------------
// Name: initialize_graphics( )
// Desc: sets initial OpenGL states and initializes any application data
//-----------------------------------------------------------------------------
void initialize_graphics()
{
    // set the GL clear color - use when the color buffer is cleared
    glClearColor( 0.0f, 0.0f,0.0f, 1.0f );
    // set the shading model to 'smooth'
    glShadeModel( GL_SMOOTH );
    // enable depth
    glEnable( GL_DEPTH_TEST );
    // set the front faces of polygons
    glFrontFace( GL_CCW );
    // set fill mode
    glPolygonMode( GL_FRONT_AND_BACK, g_fillmode );
    // enable lighting
    glEnable( GL_LIGHTING );
    // enable lighting for front
    glLightModeli( GL_FRONT, GL_TRUE );
    // material have diffuse and ambient lighting 
    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    // enable color
    glEnable( GL_COLOR_MATERIAL );

    // enable light 0
    glEnable( GL_LIGHT0 );

    // setup and enable light 1
    glLightfv( GL_LIGHT1, GL_AMBIENT, g_light1_ambient );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, g_light1_diffuse );
    glLightfv( GL_LIGHT1, GL_SPECULAR, g_light1_specular );
    glEnable( GL_LIGHT1 );

    // initialize
    g_spectrums = new Pt2D *[g_depth];
    for( int i = 0; i < g_depth; i++ )
    {
        g_spectrums[i] = new Pt2D[g_buffer_size];
        memset( g_spectrums[i], 0, sizeof(Pt2D)*g_buffer_size );
    }
    g_draw = new GLboolean[g_depth];
    memset( g_draw, 0, sizeof(GLboolean)*g_depth );

    help();
}




//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void reshapeFunc( int w, int h )
{
    // save the new window size
    g_width = w; g_height = h;
    // map the view port to the client area
    glViewport( 0, 0, w, h );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum
    gluPerspective( 45.0, (GLfloat) w / (GLfloat) h, 1.0, 300.0 );
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 3.5f * sin( g_eye_y ), 3.5f * cos( g_eye_y ), 
               0.0f, 0.0f, 0.0f, 
               0.0f, ( cos( g_eye_y ) < 0 ? -1.0f : 1.0f ), 0.0f );

    // set the position of the lights
    glLightfv( GL_LIGHT0, GL_POSITION, g_light0_pos );
    glLightfv( GL_LIGHT1, GL_POSITION, g_light1_pos );
}




//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc( unsigned char key, int x, int y )
{
    switch( key )
    {
    case '[':
        g_eye_y -= .01f;
    break;
    case ']':
        g_eye_y += .01f;
    break;
    case 'h':
        help();
    break;
    case '=':
    case '+':
        g_freq_warp += .02f;
        fprintf( stderr, "frequency warp: %.4f\n", g_freq_warp );
        break;
    case '-':
    case '_':
        if( g_freq_warp > .1f )
            g_freq_warp -= .02f;
        else if( g_freq_warp > .01f )
            g_freq_warp -= .002f;
        fprintf( stderr, "frequency warp: %.4f\n", g_freq_warp );
        break;
    case 'o':
        g_high_bin--;
        if( g_high_bin < 1 )
            g_high_bin = 1;
		if( g_high_bin <= g_low_bin )
			g_high_bin = g_low_bin + 1;
        fprintf( stderr, "high bin: %i\n", g_high_bin );
        break;
	case 'O':
        g_high_bin -= 5;
        if( g_high_bin < 1 )
            g_high_bin = 1;
		if( g_high_bin <= g_low_bin )
			g_high_bin = g_low_bin + 1;
        fprintf( stderr, "high bin: %i\n", g_high_bin );
        break;
    case 'p':
        g_high_bin++;
        if( g_high_bin > g_buffer_size / 2)
            g_high_bin = g_buffer_size / 2;
        fprintf( stderr, "high bin: %i\n", g_high_bin );
        break;
	case 'P':
        g_high_bin += 5;
        if( g_high_bin > g_buffer_size / 2)
            g_high_bin = g_buffer_size / 2;
        fprintf( stderr, "high bin: %i\n", g_high_bin );
        break;
	case 'r':
        g_low_bin--;
        if( g_low_bin < 0 )
            g_low_bin = 0;
        fprintf( stderr, "low bin: %i\n", g_low_bin );
        break;
	case 'R':
        g_low_bin -= 5;
        if( g_low_bin < 0 )
            g_low_bin = 0;
        fprintf( stderr, "low bin: %i\n", g_low_bin );
        break;
    case 't':
        g_low_bin++;
        if( g_low_bin >= g_buffer_size / 2)
            g_low_bin = g_buffer_size / 2 - 1;
		if( g_high_bin <= g_low_bin ) 
			g_low_bin = g_high_bin - 1;
		fprintf( stderr, "low bin: %i\n", g_low_bin );
        break;
	case 'T':
        g_low_bin += 5;
        if( g_low_bin >= g_buffer_size / 2)
            g_low_bin = g_buffer_size / 2 - 1;
		if( g_high_bin <= g_low_bin ) 
			g_low_bin = g_high_bin - 1;
		fprintf( stderr, "low bin: %i\n", g_low_bin );
        break;
	case 'm':
		g_npeaks++;
		fprintf( stderr, "# peaks: %i\n", g_npeaks );
		break;
	case 'n':
		g_npeaks--;
		if( g_npeaks < 0 )
			g_npeaks = 0;
		fprintf( stderr, "# peaks: %i\n", g_npeaks );
		break;
    case 'f':
        g_z += .1f;
        break;
    case 'j':
        g_z -= .1f;
        break;
    case 'e':
        g_space *= 1.02f;
        break;
    case 'i':
        g_space *= .98f;
        break;
    case 'w':
        g_wutrfall = !g_wutrfall;
        break;
    case 'd':
        g_usedb = !g_usedb;
        break;
    case 'q':
        exit( 0 );
        break;
    case 's':
    {
        static GLuint w, h;

        if( !g_fullscreen )
        {
            w = g_width;
            h = g_height;
            glutFullScreen();
        }
        else
            glutReshapeWindow( w, h );

        g_fullscreen = !g_fullscreen;
    }
    break;
    case 'x':
        g_ctf = !g_ctf;
        sf_seek( g_fin, 0, SEEK_SET );
		break;
    }

    // do a reshape since g_eye_y might have changed
    reshapeFunc( g_width, g_height );
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON )
    {
        // rotate
        if( state == GLUT_DOWN )
            g_inc -= INC_VAL;
        else
            g_inc += INC_VAL;
    }
    else if ( button == GLUT_RIGHT_BUTTON )
    {
        if( state == GLUT_DOWN )
            g_inc += INC_VAL;
        else
            g_inc -= INC_VAL;
    }
    else
        g_inc = 0.0f;

    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc( )
{
    // render the scene
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// name: draw_string()
// desc: ...
//-----------------------------------------------------------------------------
void draw_string( GLfloat x, GLfloat y, GLfloat z, const char * str, GLfloat scale = 1.0f )
{
    GLint len = strlen( str ), i;

    glPushMatrix();
    glTranslatef( x, y, z );
    glScalef( .001f * scale, .001f * scale, .001f * scale );

    for( i = 0; i < len; i++ )
        glutStrokeCharacter( GLUT_STROKE_ROMAN, str[i] );
    
    glPopMatrix();
}




//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc( )
{
    static const int LP = 4;
    static long int count = 0;
    static char str[1024];
    static unsigned int wf = 0;
    static SAMPLE buffer[THE_BUFFER_SIZE*4] = { 0.0f };

    int i;
    float fval;

    // clear the color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushMatrix( );

        // rotate the sphere about y axis
        glRotatef( g_angle_y += g_inc, 0.0f, 1.0f, 0.0f );

        // color waveform
        glColor3f( 0.4f, 0.4f, 1.0f );

        // wait for data
        while( !g_ready )
#if !defined(__OS_WINDOWS__)
            usleep( 0 );
#else
            Sleep( 0 );
#endif
	
		// read here?
        sf_readf_float( g_fin, buffer, THE_BUFFER_SIZE );
		memcpy( g_another_buffer, buffer, THE_BUFFER_SIZE * sizeof(SAMPLE) );
		
        apply_window( (float*)buffer, g_window, g_buffer_size );
		
        g_ready = FALSE;

        // apply the window
        GLfloat x = -1.8f, inc = 3.6f / g_buffer_size, y = 1.0f;
        
        // draw the time domain waveform
        glBegin( GL_LINE_STRIP );
        GLint ii = ( g_buffer_size - (g_buffer_size/g_time_view) ) / 2;
        for( i = ii; i < ii + g_buffer_size / g_time_view; i++ )
        {
            glVertex3f( x, g_gain * g_time_scale * .75f * buffer[i] + y, 0.0f );
            x += inc * g_time_view;
        }
        glEnd();

        // fft
        // memcpy( buffer, g_another_buffer, THE_BUFFER_SIZE * sizeof(SAMPLE) );
        rfft( (float *)buffer, g_buffer_size/2, FFT_FORWARD );
		complex * cbuf = (complex *)buffer;  
		
		// copy into polar buffer
		for( i = 0; i < g_buffer_size/2; i++ ) {
			g_polar_buffer[i].modulus = __modulus(cbuf[i]);
			g_polar_buffer[i].phase = __phase(cbuf[i]);
		}
		pfft_analyze( g_pfft, g_polar_buffer, g_buffer_size / 2, g_npeaks, g_low_bin, g_high_bin );
		pfft_match( g_pfft, 1 );
		if( g_hop >= g_buffer_size )
			pfft_synthesize( g_pfft, g_syn_buffer, g_buffer_size, g_buffer_size, g_freq_warp );
		else {
			// hops
			bool filedone = false;
			if( sf_seek( g_fin, 0, SEEK_CUR ) != g_finfo.frames ) {
				pfft_synthesize( g_pfft, g_syn_buffer, g_hop, g_buffer_size, g_freq_warp );
				sf_seek( g_fin, g_hop - THE_BUFFER_SIZE, SEEK_CUR );
			}
			else {
				filedone = true;
				memset( g_syn_buffer, 0, sizeof(SAMPLE) * g_buffer_size );
			}
			for( int h = g_hop; h < THE_BUFFER_SIZE && !filedone; h += g_hop ) {
				// read
				sf_count_t read = sf_readf_float( g_fin, g_another_buffer_hop, THE_BUFFER_SIZE );
				int	where = sf_seek( g_fin, 0, SEEK_CUR );
				if( where != g_finfo.frames ) {
					sf_seek( g_fin, g_hop - THE_BUFFER_SIZE, SEEK_CUR );
				}
				else
					filedone = true;
				// window
				apply_window( (float *)g_another_buffer_hop, g_window, g_buffer_size );
				// fft 
				rfft( (float *)g_another_buffer_hop, g_buffer_size/2, FFT_FORWARD );
				complex * cbuf_hop = (complex *)g_another_buffer_hop;
				// polar
				for( int p = 0; p < g_buffer_size/2; p++ ) {
					g_polar_buffer_hop[p].modulus = __modulus(cbuf_hop[p]);
					g_polar_buffer_hop[p].phase = __phase(cbuf_hop[p]);
				}
				// pfft
				pfft_analyze( g_pfft, g_polar_buffer_hop, g_buffer_size / 2, g_npeaks, g_low_bin, g_high_bin );
				pfft_match( g_pfft, 1 ); 
				if( !filedone ) {
					pfft_synthesize( g_pfft, g_another_buffer_hop, g_hop, g_buffer_size, g_freq_warp ); 
					memcpy( g_syn_buffer + h, g_another_buffer_hop, sizeof(SAMPLE) * g_hop );
				}
				else {
					pfft_synthesize( g_pfft, g_another_buffer_hop, read, g_buffer_size, g_freq_warp );
					memcpy( g_syn_buffer + h, g_another_buffer_hop, sizeof(SAMPLE) * read );
				}
				//fprintf(stderr, "%d %d %d %d, ", THE_BUFFER_SIZE, h, g_hop, h + g_hop );
			}
		}

		// draw synthesized part
        x = -1.8f; inc = 3.6f / g_buffer_size; y = 0.5f;
        glColor3f( 1.0f, 0.4f, 0.4f );
		glBegin( GL_LINE_STRIP );
        ii = ( g_buffer_size - (g_buffer_size/g_time_view) ) / 2;
        for( i = ii; i < ii + g_buffer_size / g_time_view; i++ )
        {
            glVertex3f( x, g_gain * g_time_scale * g_syn_buffer[i] + y, 0.0f );
            x += inc * g_time_view;
        }
        glEnd();

        // color the spectrum
        glColor3f( 0.4f, 1.0f, 0.4f );
        x = -1.8f;
        y = -1.2f;
        inc = 3.6f / g_buffer_size;

        // start to draw the (newest frame) frequency domain representation
        for( i = 0; i < g_buffer_size/g_freq_view; i++ )
        {
            g_spectrums[wf][i].x = x;
            if( !g_usedb )
                g_spectrums[wf][i].y = g_gain * g_freq_scale * .7f *
                    ::pow( 25 * cmp_abs( cbuf[i] ), .5 ) + y;
            else
                g_spectrums[wf][i].y = g_gain * g_freq_scale * .8f *
                    ( 20.0f * log10( cmp_abs(cbuf[i])/8.0 ) + 80.0f ) / 80.0f + y + .73f;
            x += inc * g_freq_view;
        }
        
		// draw waterfall
        g_draw[wf] = true;
        
		x = -1.8f; 
        for( i = 0; i < g_depth; i++ )
        {
            if( g_draw[(wf+i)%g_depth] )
            {
                Pt2D * pt = g_spectrums[(wf+i)%g_depth];
                fval = (g_depth-i)/(float)g_depth;
				float end = x + g_buffer_size * inc; // new for freq_warp
				float xp = x; // new for freq_warp
                glColor3f( .4f * fval, 1.0f * fval, .4f * fval );
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < g_buffer_size/g_freq_view; j++, pt++ ) {
					// glVertex3f( pt->x, pt->y, -i * g_space + g_z );
					xp = x + j * inc * g_freq_view * g_freq_warp;  // new for freq_warp
					if( xp < end )										// new for freq_warp
						glVertex3f( xp , pt->y, -i * g_space + g_z );	// new for freq_warp
				}
				if( xp < end )	// new for freq_warp
					glVertex3f( end, y, -i * g_space + g_z );	// new for freq_warp
                glEnd();
            }
        }

		// draw frequency range
		x = -1.8f;
		glLineWidth(2.0f);
		glColor3f( 2 * g_low_bin * 1.0f / g_buffer_size, .4f, 1.0f - 2 * g_low_bin * 1.0f / g_buffer_size );
		glBegin( GL_LINE_STRIP );
		glVertex3f( x + g_low_bin * inc * g_freq_view * g_freq_warp, y - .05, g_z );
		glVertex3f( x + g_low_bin * inc * g_freq_view * g_freq_warp, y + .05, g_z );
		glEnd();
		glColor3f( 2 * g_high_bin * 1.0f / g_buffer_size, .4f, 1.0f - 2 * g_high_bin * 1.0f / g_buffer_size );
		glBegin( GL_LINE_STRIP );
		glVertex3f( x + g_high_bin * inc * g_freq_view * g_freq_warp, y - .05, g_z );
		glVertex3f( x + g_high_bin * inc * g_freq_view * g_freq_warp, y + .05, g_z );
		glEnd();
		glLineWidth(1.0f);
        
		// draw peaks
		x = -1.8f; 
		for( i = 0; i < g_pfft->npeaks; i++ )
		{
			peak p = g_pfft->peaks[i];
			float xpos = x + p.bin * inc * g_freq_view; // = g_spectrums[wf][p.bin].x
			float ypos = p.info.modulus; // eventually = g_spectrums[wf][p.bin].y
			if( !g_usedb )
				ypos = g_gain * g_freq_scale * .7f * ::pow( 25 * ypos, .5 ) + y;
			else
				ypos = g_gain * g_freq_scale * .8f * (20.0 * log10( ypos / 8.0 ) + 80.0f) / 80.0f + y + .73f;
			float ptradius = .02;
			glColor3f( .4f, .4f, 1.0f); // scaled by fval depending on depth
			glLineWidth( 2.0 );
			glBegin( GL_LINE_STRIP );
			glVertex3f( xpos, ypos - ptradius, g_z ); // multiplied by g_space * depth
			glVertex3f( xpos, ypos + ptradius, g_z ); // for nonzero depths
			glEnd();
			glBegin( GL_LINE_STRIP );
			glVertex3f( xpos - ptradius, ypos, g_z );
			glVertex3f( xpos + ptradius, ypos, g_z );
			glEnd();
			glLineWidth( 1.0 );
			//fprintf( stderr, "(%i, %f) : (%f, %f)\n", p.bin, p.info.modulus, xpos, ypos );
		}
		//fprintf( stderr, "\n" );

        if( !g_wutrfall )
            g_draw[wf] = false;
        
        wf--;
        wf %= g_depth;
        
        /*
        for( int i = 0; i < 9; i++ )
            fprintf( stderr, "%.4f ", coefs[i] );
        fprintf( stderr, "power: %.8f  pitch: %.4f\n", power, pitch );
        for( int i = 0; i < 9; i++ )
            fprintf( stderr, "%.4f ", lpc(i) );
        fprintf( stderr, "power: %.8f  pitch: %.4f\n", lpc(10), lpc(9) );
        fprintf( stderr, "\n" );
         */

		glColor3f( 1.0f, 1.0f, 1.0f );
        draw_string( 0.4f, 0.8f, 0.0f, "original", .5f );
        draw_string( 0.4f, 0.3f, 0.0f, "synthesized", .5f );
        draw_string( 0.4f, -.2f, 0.0f, "waterfall", .5f );
        //draw_string( -1.8f, -.7f, 0.0f, "coefficients", .5f );

        sprintf( str, "frequency warp : %.4f", g_freq_warp );
        glColor3f( 0.8f, .4f, .4f );
        draw_string( 1.2f, -.25f, 0.0f, str, .35f );
        sprintf( str, "# peaks : %i", g_npeaks );
        draw_string( 1.2f, -.35f, 0.0f, str, .35f );
        
    glPopMatrix( );

    // swap the buffers
    glFlush( );
    glutSwapBuffers( );
    
    g_buffer_count_b++;
}
