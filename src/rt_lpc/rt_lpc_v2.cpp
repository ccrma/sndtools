//-----------------------------------------------------------------------------
// name: rt_lpc.cpp
// desc: real-time LPC analysis/synthesis/visualization, formerly iannith
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
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
#include <queue>
#include <assert.h>
using namespace std;

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

#if defined(__OS_WINDOWS__) && !defined(__WINDOWS_PTHREAD__)
  #include <process.h>
#else
  #include <unistd.h>
#endif

#if defined(__MACOSX_CORE__)
  #include "midiio_osx.h"
#elif defined(__LINUX_ALSA__)
  #include "midiio_alsa.h"
#elif defined(__LINUX_OSS__)
  #include "midiio_alsa.h"
#else
  #include "midiio_win32.h"
#endif

#include "lpc.h"
#include "chuck_fft.h"




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
void down_sample_4( float * data, int length );
void real_peel( float * coef, int order, float * radii );
void lintube( float * radii, int order );
void sectube( float * radii, int order );
void moretube( float * radii, int order );




//-----------------------------------------------------------------------------
// global variables and #defines
//-----------------------------------------------------------------------------
#define INC_VAL                 1.0f
#define LPC__PI                 3.14159265359
#define LPC_BUFFER_SIZE         ( RT_BUFFER_SIZE * 2 )

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
GLUquadricObj * g_quad = NULL;

// global audio buffer
SAMPLE g_audio_buffer[LPC_BUFFER_SIZE];
SAMPLE g_another_buffer[LPC_BUFFER_SIZE];
SAMPLE g_buffest[LPC_BUFFER_SIZE];
SAMPLE g_buffet[LPC_BUFFER_SIZE];
GLboolean g_ready = FALSE;
GLfloat g_window[LPC_BUFFER_SIZE];
int g_buffer_size = LPC_BUFFER_SIZE;
RtAudio * g_audio = NULL;
Mutex g_mutex;
#if defined(__LINUX_ALSA__) || defined(__LINUX_OSS__)
unsigned int g_srate = 24000;
#elif defined(__MACOSX_CORE__)
unsigned int g_srate = 44100;
#else
unsigned int g_srate = 20000;
#endif

// gain
GLfloat g_gain = 1.0f;
GLfloat g_time_scale = 1.0f;
GLfloat g_freq_scale = 1.0f;
GLint g_time_view = 1;
GLint g_freq_view = 2;
MidiIn * g_min = NULL;

lpc_data g_lpc = NULL;
float g_speed = 1.0f;
int g_order = 30;

// fullscreen
GLboolean g_fullscreen = FALSE;

// flags
GLint g_sndout = 1;
GLint g_sndin = 1;
GLboolean g_display = TRUE;
GLboolean g_raw = FALSE;
const char * g_filename = NULL;

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
GLboolean g_midi =  FALSE;
GLboolean g_draw_vocal = TRUE;
int g_which = 0;

static const float midi2pitch[129] = {
8.18,8.66,9.18,9.72,10.30,10.91,11.56,12.25,
12.98,13.75,14.57,15.43,16.35,17.32,18.35,19.45,
20.60,21.83,23.12,24.50,25.96,27.50,29.14,30.87,
32.70,34.65,36.71,38.89,41.20,43.65,46.25,49.00,
51.91,55.00,58.27,61.74,65.41,69.30,73.42,77.78,
82.41,87.31,92.50,98.00,103.83,110.00,116.54,123.47,
130.81,138.59,146.83,155.56,164.81,174.61,185.00,196.00,
207.65,220.00,233.08,246.94,261.63,277.18,293.66,311.13,
329.63,349.23,369.99,392.00,415.30,440.00,466.16,493.88,
523.25,554.37,587.33,622.25,659.26,698.46,739.99,783.99,
830.61,880.00,932.33,987.77,1046.50,1108.73,1174.66,1244.51,
1318.51,1396.91,1479.98,1567.98,1661.22,1760.00,1864.66,1975.53,
2093.00,2217.46,2349.32,2489.02,2637.02,2793.83,2959.96,3135.96,
3322.44,3520.00,3729.31,3951.07,4186.01,4434.92,4698.64,4978.03,
5274.04,5587.65,5919.91,6271.93,6644.88,7040.00,7458.62,7902.13,
8372.02,8869.84,9397.27,9956.06,10548.08,11175.30,11839.82,12543.85,
13289.75
};

typedef unsigned long uint__;

struct the_queue
{
    queue<SAMPLE *> q;
    CBuffer p;
    uint__ bs;
    
    the_queue() { bs = 0; }
    
    void be( uint__ pool_size, uint__ buffer_size )
    {
         p.initialize( pool_size, sizeof(SAMPLE *) );
         uint__ i;
         SAMPLE * w;
         for( i = 1; i < pool_size * 3 / 4; i++ )
         {
             w = new SAMPLE[buffer_size];
             p.put( &w, 1 );
         }
         bs = buffer_size;
    }
    
    uint__ size() { return q.size(); }
    
    void enqueue( SAMPLE * buffer )
    {
        SAMPLE * w;
        
        // get from pool
        if( !p.get( &w, 1 ) )
        {
            fprintf( stderr, "[rt_lpc]: pool exhausted!\n" );
            assert( FALSE );
        }
        
        // copy
        memcpy( w, buffer, bs * sizeof(SAMPLE) );
        
        q.push( w );
    }
    
    void dequeue( SAMPLE * buffer )
    {
        if( q.size() == 0 )
        {
            fprintf( stderr, "[rt_lpc]: nothing to dequeue! now look what you have done...\n" );
            assert( FALSE );
        }
        SAMPLE * w = q.front();
        q.pop();
        
        // copy
        memcpy( buffer, w, bs * sizeof(SAMPLE) );
        memset( w, 0, bs * sizeof(SAMPLE) );
        
        // throw back in to pool
        p.put( &w, 1 );
    }
};

the_queue g_q;

//-----------------------------------------------------------------------------
// name: help()
// desc: ...
//-----------------------------------------------------------------------------
void help()
{
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "RT_LPC\n" );
    fprintf( stderr, "Ananya Misra, Ge Wang, Perry Cook\n" );
    fprintf( stderr, "http://soundlab.cs.princeton.edu/\n" );
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "'s' - toggle fullscreen\n" );
    fprintf( stderr, "'=' - increase pitch factor\n" );
    fprintf( stderr, "'-' - decrease pitch factor\n" );
    fprintf( stderr, "'p' - increase order\n" );
    fprintf( stderr, "'o' - decrease order\n" );
    fprintf( stderr, "'t' - toggle vocal tract\n" );
    fprintf( stderr, "'v' - select vocal tract rendering mode\n" );
    fprintf( stderr, "'m' - use MIDI input as pitch\n" );
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
    fprintf( stderr, "usage: rt_lpc\n" );
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
    glutCreateWindow( "rt_lpc" );

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
    
    // do our own initialization
    initialize_graphics( );
    if( !initialize_audio( ) )
    {
        fprintf( stderr, "rt_lpc: error initializing audio, exiting...\n" );
        return -3;
    }
    initialize_analysis( );
    
    // let GLUT handle the current thread from here
    glutMainLoop();


    return 0;
}




//-----------------------------------------------------------------------------
// name: cb()
// desc: audio callback
//-----------------------------------------------------------------------------
int cb( char * buffer, int buffer_size, void * user_data )
{
    // g_mutex.lock();

    // copy in and out
    memcpy( g_audio_buffer, buffer, buffer_size * sizeof(SAMPLE) );
    g_q.enqueue( (SAMPLE *)buffer );
    if( !g_ready ) memcpy( buffer, g_another_buffer, buffer_size * sizeof(SAMPLE) );
    else memset( buffer, 0, buffer_size * sizeof(SAMPLE) );

    g_ready = TRUE;
    // g_mutex.unlock();

    return 0;
}




//-----------------------------------------------------------------------------
// Name: initialize_audio( )
// Desc: set up audio capture and playback and initializes any application data
//-----------------------------------------------------------------------------
bool initialize_audio( )
{
    Stk::setSampleRate( g_srate );
    g_buffer_size /= 2;

    try
    {
        // open the audio device for capture and playback
        g_audio = new RtAudio( 0, g_sndout, 0, g_sndin, RTAUDIO_FLOAT32,
            g_srate, &g_buffer_size, 2 );
    }
    catch( StkError & e )
    {
        // exception
        fprintf( stderr, "%s\n", e.getMessage() );
        fprintf( stderr, "error: cannot open audio device for capture/playback...\n" );
        return false;
    }
    
    g_q.be( 512, g_buffer_size );
    g_buffer_size *= 2;

    // set the audio callback
    g_audio->setStreamCallback( cb, NULL );
    
    // start the audio
    g_audio->startStream( );
    
    // make the window
    make_window( g_window, g_buffer_size );
    
    // take a guess
    if( g_midi && (g_min = new MidiIn) && !g_min->open( 0 ) )
    {
        fprintf( stderr, "rt_lpc: cannot open MIDI input..." );
        delete g_min;
        g_min = NULL;
        g_midi = FALSE;
    }

    return true;
}




//-----------------------------------------------------------------------------
// Name: initialize_analysis( )
// Desc: sets initial audio analysis parameters
//-----------------------------------------------------------------------------
void initialize_analysis( )
{
    g_lpc = lpc_create( );
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
    
    // glu
    g_quad = gluNewQuadric( );
    gluQuadricDrawStyle( g_quad, GLU_FILL );
    gluQuadricNormals( g_quad, GLU_FLAT );

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
        reshapeFunc( g_width, g_height );
    break;
    case ']':
        g_eye_y += .01f;
        reshapeFunc( g_width, g_height );
    break;
    case 'h':
        help();
    break;
    case '=':
    case '+':
        g_speed += .02f;
        fprintf( stderr, "pitch factor: %.4f\n", g_speed );
        break;
    case '-':
    case '_':
        if( g_speed > .1f )
            g_speed -= .02f;
        else if( g_speed > .01f )
            g_speed -= .002f;
        fprintf( stderr, "pitch factor: %.4f\n", g_speed );
        break;
    case 'o':
        g_order--;
        if( g_order < 1 )
            g_order = 1;
        fprintf( stderr, "order: %i\n", g_order );
        break;
    case 'p':
        g_order++;
        if( g_order > 100 )
            g_order = 100;
        fprintf( stderr, "order: %i\n", g_order );
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
    case 'm':
        g_midi = !g_midi;
        if( !g_min )
        {
            g_min = new MidiIn;
            if( !g_min->open( 0 ) )
            {
                fprintf( stderr, "cannot open MIDI input 0 (default)\n" );
                delete g_min;
                g_min = NULL;
                g_midi = FALSE;
            }
        }
        fprintf( stderr, "%susing MIDI input as pitch\n", g_midi ? "" : "not " );
    break;
    case 't':
        g_draw_vocal = !g_draw_vocal;
    break;
    case 'v':
    {
        g_which = (g_which + 1) % 4;
    }
    break;
    }

    // do a reshape since g_eye_y might have changed
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
    static SAMPLE buffer[LPC_BUFFER_SIZE], residue[LPC_BUFFER_SIZE],
           coefs[1024], radii[1024];
    static MidiMsg ananya, ge;
    static int keys = 0;
    static float bend = 0;

    int i;
    float pitch, power, fval;

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

        // midi
        while( g_midi && g_min && g_min->recv( &ge ) )
        {
            // note on
            if( (ge.data[0] & 0xf0) == 0x90 )
            {
                if( ge.data[2] )
                {
                    keys++;
                    memcpy( &ananya, &ge, sizeof(MidiMsg) );
                }
                else
                {
                    keys--;
                    if( keys <= 0 ) memcpy( &ananya, &ge, sizeof(MidiMsg) );
                }
            }
            // pitch bend
            else if( (ge.data[0] & 0xf0) == 0xe0 )
                bend = ge.data[1] / 128.0f + ge.data[2] - 64;
        }

        while( 0 == g_q.size() )
#ifndef __OS_WINDOWS__
            usleep( 50 );
#else
		    Sleep( 0 );
#endif

        // get the window
        memcpy( buffer, g_buffest+g_buffer_size/2, g_buffer_size/2*sizeof(SAMPLE) );
        g_q.dequeue( buffer+g_buffer_size/2 );
        memcpy( g_buffest+g_buffer_size/2, buffer+g_buffer_size/2, g_buffer_size/2*sizeof(SAMPLE) );
        memset( g_another_buffer, 0, g_buffer_size * sizeof(SAMPLE) );

		lpc_preemphasis(buffer, g_buffer_size, 0.5); // No
        lpc_analyze( g_lpc, buffer, g_buffer_size, coefs, g_order, &power, &pitch, residue );
        if( g_midi ) {
            pitch = (int)( Stk::sampleRate() / midi2pitch[ananya.data[1]] ) /
                    pow( 1.0653f, bend/64.0f*11.0f );
            power *= ananya.data[2] / 64.0f;
        }
        lpc_synthesize( g_lpc, g_another_buffer, g_buffer_size, coefs, g_order, power, pitch / g_speed, 1 );
		lpc_deemphasis(g_another_buffer, g_buffer_size, 0.5); // Go away

        // ugly code
        for( i = 0; i < g_buffer_size/2; i++ )
        {
             g_another_buffer[i] = g_another_buffer[i] * g_window[i] + g_buffest[i] * (g_window[i + g_buffer_size/2]);
             g_buffest[i] = g_another_buffer[i+g_buffer_size/2];
        }

        if( g_draw_vocal )
        {
            // vocal tract model
            real_peel( coefs, g_order, radii );
            // draw it
            glColor3f( 1.0f, 0.4f, 0.4f );
            if( g_which == 0 )
                lintube( radii, g_order );
            else if( g_which == 1 )
                sectube( radii, g_order );
            else if( g_which == 2 )
            {
                gluQuadricDrawStyle( g_quad, GLU_FILL );
                moretube( radii, g_order );
            }
            else
            {
                gluQuadricDrawStyle( g_quad, GLU_SILHOUETTE );
                moretube( radii, g_order );
            }
            glColor3f( 0.4f, 0.4f, 1.0f );
        }
        g_ready = FALSE;

        // apply the window
        GLfloat x = -1.8f, inc = 3.6f / g_buffer_size, y = 1.0f;
        apply_window( (float*)buffer, g_window, g_buffer_size );
        
        // draw the time domain waveform
        glBegin( GL_LINE_STRIP );
        GLint ii = ( g_buffer_size - (g_buffer_size/g_time_view) ) / 2;
        for( i = ii; i < ii + g_buffer_size / g_time_view; i++ )
        {
            glVertex3f( x, g_gain * g_time_scale * .75f * buffer[i] + y, 0.0f );
            x += inc * g_time_view;
        }
        glEnd();

        // apply the window
        x = -1.8f, inc = 3.6f / g_buffer_size, y = .5f;

        glColor3f( 0.4f, 0.8f, 1.0f );
        // draw the prediction
        glBegin( GL_LINE_STRIP );
        for( i = ii; i < ii + g_buffer_size / g_time_view; i++ )
        {
            glVertex3f( x, g_gain * g_time_scale * .75f * (buffer[i]-residue[i]) + y, 0.0f );
            x += inc * g_time_view;
        }
        glEnd();

        // apply the window
        x = -1.8f, inc = 3.6f / g_buffer_size, y = .0f;

        // draw the residue
        glColor3f( 0.8f, 0.8f, 0.4f );
        glBegin( GL_LINE_STRIP );
        for( i = ii; i < ii + g_buffer_size / g_time_view; i++ )
        {
            glVertex3f( x, g_gain * g_time_scale * 5.0f * residue[i] + y, 0.0f );
            x += inc * g_time_view;
        }
        glEnd();        
        
        // fft
        rfft( (float *)buffer, g_buffer_size/2, FFT_FORWARD );
        
        x = -1.8f;
        y = -1.0f;
        complex * cbuf = (complex *)buffer;
        
        // color the spectrum
        glColor3f( 0.4f, 1.0f, 0.4f );

        // draw the frequency domain representation
        glBegin( GL_LINE_STRIP );
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
        glEnd();

        g_draw[wf] = true;

        for( i = 0; i < g_depth; i++ )
        {
            if( g_draw[(wf+i)%g_depth] )
            {
                Pt2D * pt = g_spectrums[(wf+i)%g_depth];
                fval = (g_depth-i)/(float)g_depth;
                glColor3f( .4f * fval, 1.0f * fval, .4f * fval );
                x = -1.8f;
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < g_buffer_size/g_freq_view; j++, pt++ )
                    glVertex3f( x + j*(inc*g_freq_view), pt->y, -i * g_space + g_z );
                glEnd();
            }
        }
        
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

        draw_string( 0.4f, 0.8f, 0.0f, "original", .5f );
        draw_string( 0.4f, 0.3f, 0.0f, "predicted", .5f );
        draw_string( 0.4f, -.2f, 0.0f, "residue (error)", .5f );

        sprintf( str, "pitch factor: %.4f", g_speed );
        glColor3f( 0.8f, .4f, .4f );
        draw_string( 1.2f, -.25f, 0.0f, str, .35f );
        sprintf( str, "LPC order: %i", g_order );
        draw_string( 1.2f, -.35f, 0.0f, str, .35f );


        SAMPLE sum = 0.0f;
        for( i = 0; i < g_buffer_size; i++ )
            sum += fabs(buffer[i]);

        glPushMatrix();
        if( pitch == 0 )
        {
            glColor3f( 1.0f, .4f, .4f );
            glTranslatef( 1.28f, -.45f, 0.0f );
        }
        else
        {
            glColor3f( 1.0f, 1.2f - pitch/g_buffer_size*4.0f, .4f );
            glTranslatef( 1.55f, -.45f, 0.0f );
        }
        glutSolidSphere( .001f + sum/g_buffer_size * 120.0f, 10, 10 );
        glPopMatrix();

        draw_string( 1.2f, -.55f, 0.0f, "non-pitch | pitched", .35f );
        
    glPopMatrix( );
    
    g_ready = FALSE;

    // swap the buffers
    glFlush( );
    glutSwapBuffers( );
    
    g_buffer_count_b++;
}




// interpolation for down sampler
#define FILT_ORDER    12
#define FILT_CENTER   (FILT_ORDER/2)
SAMPLE g_filt_buff[FILT_ORDER+1];
SAMPLE g_filt_coef[FILT_ORDER+1] = { 0.0f };
int g_filt_ready = FALSE;

//------------------------------------------------------------------------------
// name: down_sample_4()
// desc: ...
//------------------------------------------------------------------------------
void down_sample_4( float * data, int size )
{
    int i, j, k = -FILT_CENTER;
    float output, filtPhase= -6.0 * LPC__PI / 4.0;

    if( !g_filt_ready )
    {
        for( j = 0; j < FILT_ORDER+1; j++ )
        {
            g_filt_coef[j] = sin(filtPhase) / (float)k;
            filtPhase += LPC__PI / 4.0;
            k++;
        }
        g_filt_coef[FILT_CENTER] = 1.0;
        g_ready = TRUE;
    }

    for( i = 0; i < size; i++ )
    {
        output = 0.0;
        for( j = 0; j < FILT_ORDER; j++ )
        {
            output += g_filt_coef[j] * g_filt_buff[j];
            g_filt_buff[j] = g_filt_buff[j+1];
        }

        output += g_filt_coef[j] * g_filt_buff[j];
        g_filt_buff[j] = data[i];

        if( i % 4 == 0 )
            data[i/4] = output;
    }
}




//------------------------------------------------------------------------------
// name: real_peel()
// desc: ...
//------------------------------------------------------------------------------
void real_peel( float * coef, int order, float * radii )
{
    int i,j;
    float rs, temp, tempcoeffs1[1024], tempcoeffs2[1024], reflections[1024];

    tempcoeffs1[0] = 1.0f;

    // initialize
    for( i = 0; i < order; i++ )
        tempcoeffs1[i+1] = -coef[i];

    for( i = 0; i < order; i++ )
    {
        for( j = 0; j <= order-i; j++ ) tempcoeffs2[order-i-j] = tempcoeffs1[j];
        reflections[i] = tempcoeffs2[0];
        rs = 1.0f - reflections[i]*reflections[i];
        if( rs <= 0.0f ) rs = 10000.0f; else rs = 1.0f / rs;
        for( j = 0; j < order-i; j++ )
            tempcoeffs1[j] = (tempcoeffs1[j] - reflections[i] * tempcoeffs2[j]) * rs;
    }

    radii[0] = 1.0f;
    for( i = 0; i <= order; i++ )
        radii[i+1] = radii[i] * sqrt((1-reflections[i])/(1.0f + reflections[i]));
    for( i = 0; i <= order; i++ )
        radii[i] = radii[i] / radii[order];
    for( i = 0; i <= order/2; i++ )
    {
        temp = radii[i];
        radii[i] = radii[order-i];
        radii[order-i] = temp;
    }
    radii[0] = .5f;
    //for( i = 0; i < order; i++ )
    //    printf("%f ",radii[i]);

    //printf("\n");
    //fflush(stdout);
}




//------------------------------------------------------------------------------
// name:
// desc:
//------------------------------------------------------------------------------
void moretube( float * r, int order )
{
    int i;
    float xdel, x;

    xdel = 1.0f / (order + 2);
    x = xdel;

    // draw the cylinders
    glPushMatrix();
    glTranslatef( -1.5f, 0.0f, 0.0f );
    glRotatef( 90.0f, 0.0f, 1.0f, 0.0f );
    for( i = 0; i < order; i++ )
    {
        gluCylinder( g_quad, r[i]*.33, r[i]*.33, xdel*2.0f, 10, 10 );
        glTranslatef( 0.0f, 0.0f, 2.0f*xdel );
    }
    glPopMatrix();
}




//------------------------------------------------------------------------------
// name:
// desc:
//------------------------------------------------------------------------------
void lintube( float * r, int order )
{
    int i;
    float xdel, x;

    xdel = 1.0f / (order + 2);
    x = xdel;

    // line
    glBegin(GL_LINE_STRIP);
    for( i = 0; i < order; i++ )
    {
        glVertex2f(-1.5 + x*2.0, r[i]*0.33);
        x += xdel;
    }
    glEnd();

    x = xdel;

    // line
    glBegin(GL_LINE_STRIP);
    for( i = 0; i < order; i++ )
    {
        glVertex2f(-1.5 + x*2.0, -r[i]*0.33);
        x += xdel;
    }
    glEnd();
}




//------------------------------------------------------------------------------
// name: sectube()
// desc: ...
//------------------------------------------------------------------------------
void sectube( float * r, int order )
{
    int i;
    float xdel, x;

    xdel = 1.0f / (order + 3);
    x = xdel;

    glBegin(GL_LINE_STRIP);
    for( i = 0; i < order; i++ )
    {
        glVertex2f(-1.5 + x*2.0, r[i]*0.33);
        x += xdel;
        glVertex2f(-1.5 + x*2.0, r[i]*0.33);
    }
    glEnd();

    x = xdel;

    // line
    glBegin(GL_LINE_STRIP);
    for( i = 0; i < order; i++ )
    {
        glVertex2f(-1.5 + x*2.0, -r[i]*0.33);
        x += xdel;
        glVertex2f(-1.5 + x*2.0, -r[i]*0.33);
    }
    glEnd();
}




//------------------------------------------------------------------------------
// name: line()
// desc: ...
//------------------------------------------------------------------------------
void line(float x, float y, float x2, float y2)
{
    glBegin(GL_LINES);
    glVertex2f(-1.0 + x*2.0, y*0.33);
    glVertex2f(-1.0 + x2*2.0, y2*0.33);
    glEnd();
}
