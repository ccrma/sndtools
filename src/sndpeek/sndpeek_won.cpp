/*----------------------------------------------------------------------------
    sndpeek - real-time audio visualization tool

    Copyright (c) 2004 Ge Wang, Perry R. Cook, Ananya Misra. All rights reserved.
        http://soundlab.cs.princeton.edu/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: sndpeek.cpp
// desc: ...
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//         Ananya Misra (amisra@cs.princeton.edu)
// thanks to: Yilei Shao, Qin Lv, Tom Briggs
// library:
//         (STK) Perry R. Cook (prc@cs.princeton.edu)
//         (STK) Gary P. Scavone (gary@ccrma.stanford.edu)
//         (FFT) CARL CMusic Distribution
//         (Marsyas) George Tzanetakis (gtzan@cs.princeton.edu)
// date: 11.28.2003
//
// usage:
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <memory.h>

// STK
#include "RtAudio.h"
#include "WvIn.h"
#include "WvOut.h"
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

// FFT
#include "chuck_fft.h"

// Marsyas
#include "Centroid.h"
#include "DownSampler.h"
#include "Flux.h"
#include "LPC.h"
#include "MFCC.h"
#include "RMS.h"
#include "Rolloff.h"





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
void extract_buffer( );
double compute_log_spacing( int fft_size, double three_point_six, double factor );




//-----------------------------------------------------------------------------
// global variables and #defines
//-----------------------------------------------------------------------------
#define SAMPLE                  MY_FLOAT
#define SND_BUFFER_SIZE         1024
#define SND_ZERO_PADDING        2
#define SND_FFT_SIZE            ( SND_BUFFER_SIZE * SND_ZERO_PADDING )
#define SND_MARSYAS_SIZE        ( 512 )
#define INC_VAL                 1.0f


// width and height of the window
GLsizei g_width = 800;
GLsizei g_height = 600;

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
GLfloat g_eye_y = 0.0f; //0.2f; // just for stria, otherwise 0


// global audio buffer
SAMPLE g_fft_buffer[SND_FFT_SIZE];
SAMPLE g_audio_buffer[SND_BUFFER_SIZE];
SAMPLE g_stereo_buffer[SND_BUFFER_SIZE*2];
SAMPLE g_back_buffer[SND_BUFFER_SIZE];
GLfloat g_window[SND_BUFFER_SIZE];
long g_buffer_size = SND_BUFFER_SIZE;
long g_zero_padding = SND_ZERO_PADDING;
long g_fft_size = SND_FFT_SIZE;
GLboolean g_ready = FALSE;
RtAudio * g_audio = NULL;
WvIn * g_wvin = NULL;
WvOut * g_wvout = NULL;
Mutex g_mutex;

// sample rate
#if defined(__LINUX_ALSA__) || defined(__LINUX_OSS__) || defined(__LINUX_JACK__)
  unsigned int g_srate = 48000;
#else
  unsigned int g_srate = 44100;
#endif

// gain
GLfloat g_gain = 1.0f;
GLfloat g_time_scale = 1.0f;
GLfloat g_freq_scale = 1.0f;
GLfloat g_lissajous_scale = 0.45f;
GLint g_time_view = 1;
GLint g_freq_view = 2;

// for log scaling
double g_log_space = 0;
double g_log_factor = 1;

// delay for pseudo-Lissajous in mono stuff
int g_delay = SND_BUFFER_SIZE/2;

// number of channels
GLint g_sndout = 0;
GLint g_sndin = 2;

// input filename
const char * g_filename = NULL;

// marsyas analysis modules
Centroid * g_centroid = NULL;
DownSampler * g_down_sampler = NULL;
Flux * g_flux = NULL;
LPC * g_lpc = NULL;
MFCC * g_mfcc = NULL;
RMS * g_rms = NULL;
Rolloff * g_rolloff = NULL;
Rolloff * g_rolloff2 = NULL;

// flags with default...
//
// print features to stdout
GLboolean g_stdout = FALSE;
// opengl dislpay
GLboolean g_display = TRUE;
// fullscreen
GLboolean g_fullscreen = FALSE;
// plot waterfall
GLboolean g_wutrfall = TRUE;
// draw analysis features
GLboolean g_draw_features = FALSE;
// freeze display
GLboolean g_freeze = FALSE;
// mute audio
GLboolean g_mute = FALSE;
// use dB plot for spectrum
GLboolean g_usedb = FALSE;
// thing running
GLboolean g_running = TRUE;
// file input running
GLboolean g_file_running = FALSE;
// restart the file
GLboolean g_restart = FALSE;
// pause audio playback
GLboolean g_pause = FALSE;
// array of booleans for waterfall
GLboolean * g_draw = NULL;
// rainbowish coloring of waterfall?
GLboolean g_rainbow = FALSE;
// which way the waterfall moves
GLboolean g_backwards = FALSE;

// not sure anymore
GLint g_buffer_count_a = 0;
GLint g_buffer_count_b = 0;

// for waterfall
struct Pt2D { float x; float y; };
Pt2D ** g_spectrums = NULL;
unsigned int g_depth = 64;
float g_z = 0.0f;
float g_space = .1f;

// for time domain waterfall
SAMPLE ** g_waveforms = NULL;
unsigned int g_wf_delay = g_depth / 3;
unsigned int g_wf_index = 0;




//-----------------------------------------------------------------------------
// name: help()
// desc: ...
//-----------------------------------------------------------------------------
void help()
{
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "sndpeek + wutrfall (1.1)\n" );
    fprintf( stderr, "Ge Wang, Perry R. Cook, Ananya Misra\n" );
    fprintf( stderr, "http://soundlab.cs.princeton.edu/\n" );
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "'h' - print this help message\n" );
    fprintf( stderr, "'s' - toggle fullscreen\n" );
    fprintf( stderr, "'f' - freeze frame! (can still rotate/scale)\n" );
    fprintf( stderr, "'w' - toggle wutrfall plot\n" );
    fprintf( stderr, "'d' - toggle dB plot for spectrum\n" );
    fprintf( stderr, "'e' - toggle feature extraction\n" );
    fprintf( stderr, "'j' - move spectrum + z\n" );
    fprintf( stderr, "'k' - move spectrum - z\n" );
    fprintf( stderr, "'u' - spacing more!\n" );
    fprintf( stderr, "'i' - spacing less!\n" );
    fprintf( stderr, "L, R mouse button - rotate left or right\n" );
    fprintf( stderr, "'[', ']' - rotate up or down\n" );
    fprintf( stderr, "'-' - scale DOWN the waveform\n" );
    fprintf( stderr, "'=' - scale UP the waveform\n" );
    fprintf( stderr, "(shift)'-' - scale DOWN the spectrum\n" );
    fprintf( stderr, "(shift)'=' - scale UP the spectrum\n" );
    fprintf( stderr, "'c' = LOWER the # of samples in viewable waveform\n" );
    fprintf( stderr, "'v' = RAISE the # of samples in viewable waveform\n" );
    fprintf( stderr, "(shift)'c' - LOWER the max viewable frequency\n" );
    fprintf( stderr, "(shift)'v' - RAISE the max viewable frequency\n" );
    fprintf( stderr, "'x' - restart file playback (if applicable)\n" );
    fprintf( stderr, "'m' - mute\n" );
    fprintf( stderr, "'l' - scale DOWN the lissajous!\n" );
    fprintf( stderr, "'L' - scale UP the lissajous!\n" );
    fprintf( stderr, "'y' - decrease delay for lissajous plot\n" );
    fprintf( stderr, "'Y' - increase delay for lissajous plot\n" );
	fprintf( stderr, "'r' - toggle rainbow waterfall\n" );
	fprintf( stderr, "'b' - toggle waterfall moving backwards/forwards\n" );
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
    fprintf( stderr, "usage: sndpeek [--sndout] [--print] [--nodisplay] [filename]\n" );
}




//-----------------------------------------------------------------------------
// Name: main( )
// Desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    // command line arguments
    for( int i = 1; i < argc; i++ )
    {
        if( !strncmp( argv[i], "-", 1 ) )
        {
            if( !strcmp( argv[i], "--print" ) )
                g_stdout = TRUE;
            else if( !strcmp( argv[i], "--sndout" ) )
                g_sndout = 2;
            else if( !strcmp( argv[i], "--nodisplay" ) )
                g_display = FALSE;
            else
            {
                fprintf( stderr, "[sndpeek]: unrecognized option '%s'...\n", argv[i] );
                usage();
                return -1;
            }
        }
        else
        {
            if( g_filename )
            {
                fprintf( stderr, "[sndpeek]: multiple filenames specified...\n" );
                usage();
                return -2;
            }
            
            g_filename = argv[i];
            g_sndout = 2;
        }
    }
    
    // infer settings
    if( g_filename ) g_sndin = 0;
    if( !g_sndin && !g_sndout ) g_display = FALSE;

    if( g_display )
    {
#ifdef __OS_MACOSX__ //save working dir
        char * cwd = getcwd( NULL, 0 );
#endif

        // initialize GLUT
        glutInit( &argc, argv );

#ifdef __OS_MACOSX__ //restore working dir
        chdir( cwd );
        free( cwd );
#endif
        // double buffer, use rgb color, enable depth buffer
        glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
        // initialize the window size
        glutInitWindowSize( g_width, g_height );
        // set the window postion
        glutInitWindowPosition( 100, 100 );
        // create the window
        glutCreateWindow( "sndpeek" );
        
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
    }
    
    initialize_analysis( );
    if( !initialize_audio( ) )
    {
        fprintf( stderr, "[sndpeek]: exiting...\n" );
        return -3;
    }

    if( g_display )
    {
        // let GLUT handle the current thread from here
        glutMainLoop();
    }
    else
    {
        while( g_running )
        {
            if( g_filename && !g_sndout )
            {
                g_wvin->tick( g_audio_buffer, SND_BUFFER_SIZE );
                g_buffer_count_a++;
                g_ready = TRUE;
                if( g_wvin->isFinished() )
                {
                    g_file_running = FALSE;
                    fprintf( stderr, "[sndpeek]: file not running...\n" );
                }
            }

            extract_buffer();
        }
    }

    return 0;
}




//-----------------------------------------------------------------------------
// name: cb()
// desc: audio callback
//-----------------------------------------------------------------------------
int cb( char * buffer, int buffer_size, void * user_data )
{
    // freeze frame
    if( g_freeze ) {
        memset( buffer, 0, buffer_size * 2 * sizeof(SAMPLE) );
        g_ready = TRUE;
        return 0;
    }

    // g_mutex.lock();
    if( !g_filename )
    {
        memcpy( g_stereo_buffer, buffer, buffer_size * 2 * sizeof(SAMPLE) );
        // convert stereo to mono
        for( int i = 0; i < buffer_size; i++)
        {
            g_audio_buffer[i] = g_stereo_buffer[i*2] + g_stereo_buffer[i*2+1];
            g_audio_buffer[i] /= 2.0f;
        }
    }
    else
    {
        // check for restart
        if( g_restart )
        {
            g_wvin->reset();
            g_restart = FALSE;
        }

        if( !g_wvin->isFinished() && !g_pause )
        {
            // get the mono/stereo version
            g_wvin->tickFrame( g_stereo_buffer, buffer_size );
            // if stereo, convert to mono
            if( g_wvin->getChannels() == 2 )
            {
                // convert stereo to mono
                for( int i = 0; i < buffer_size; i++)
                {
                    g_audio_buffer[i] = g_stereo_buffer[i*2] + g_stereo_buffer[i*2+1];
                    g_audio_buffer[i] /= 2.0f;
                }
            }
            else
            {
                // actually mono
                memcpy( g_audio_buffer, g_stereo_buffer, buffer_size * sizeof(SAMPLE) );
                // convert mono to stereo
                for( int i = 0; i < buffer_size; i++ )
                {
                    g_stereo_buffer[i*2] = g_stereo_buffer[i*2+1] = g_audio_buffer[i];
                }
            }

            // time-domain waterfall delay
            if( g_waveforms != NULL )
            {
                // put current buffer in time-domain waterfall
                memcpy( g_waveforms[g_wf_index], g_stereo_buffer, buffer_size * 2 * sizeof(SAMPLE) );
                // incrment index (this is also the index to copy out of)
                g_wf_index = (g_wf_index + 1) % g_wf_delay;
                // copy delayed buffer out of time-domain waterfall
                memcpy( g_stereo_buffer, g_waveforms[g_wf_index], buffer_size * 2 * sizeof(SAMPLE) );
            }

            // play stereo
            memcpy( buffer, g_stereo_buffer, buffer_size * 2 * sizeof(SAMPLE) );

            g_buffer_count_a++;
        }
        else
        {
            g_running = FALSE;
            memset( g_audio_buffer, 0, buffer_size * sizeof(SAMPLE) );
            memset( buffer, 0, buffer_size * 2 * sizeof(SAMPLE) );
        }
    }
    g_ready = TRUE;
    // g_mutex.unlock();

    // mute the audio
    if( g_mute )
        memset( buffer, 0, buffer_size * 2 * sizeof(SAMPLE) );

    return 0;
}




//-----------------------------------------------------------------------------
// Name: initialize_audio( )
// Desc: set up audio capture and playback and initializes any application data
//-----------------------------------------------------------------------------
bool initialize_audio( )
{
    Stk::setSampleRate( g_srate );
    
    // read from file
    if( g_filename )
    {
        try
        {
            fprintf( stderr, "[sndpeek]: opening %s...\n", g_filename );
            g_wvin = new WvIn( g_filename, FALSE, FALSE );
        }
        catch( StkError & e )
        {
            // exception
            fprintf( stderr, "[sndpeek]: error: %s\n", e.getMessage() );
            fprintf( stderr, "[sndpeek]: error: cannot open '%s'...\n", g_filename );
            return false;
        }
        
        g_wvin->normalize();
        // g_wvin->setRate( 44100 );
        // g_wvin->setInterpolate( true );
        g_file_running = TRUE;

        // set srate from the WvIn
        g_srate = g_wvin->getFileRate();
        fprintf( stderr, "[sndpeek]: setting sample rate to %d\n", g_srate );
    }
    else
    {
        // no time-domain waterfall delay!
        g_wf_delay = 0;
    }

    // make sound
    if( !g_filename || g_sndout )
    {
        try
        {
            // buffer size
            int bufsize = g_buffer_size;
            // open the audio device for capture and playback
            g_audio = new RtAudio( 0, g_sndout, 0, g_sndin, RTAUDIO_FLOAT32,
                g_srate, &bufsize, 8 );
            // test
            if( bufsize != g_buffer_size )
            {
                // potential problem
                fprintf( stderr, "[sndpeek]: warning: using different buffer sizes: %i : %i\n",
                    bufsize, g_buffer_size );
            }
        }
        catch( StkError & e )
        {
            // exception
            fprintf( stderr, "[sndpeek](via RtAudio): %s\n", e.getMessage() );
            fprintf( stderr, "[sndpeek]: error: cannot open audio device for capture/playback...\n" );
            return false;
        }

        // set the audio callback
        g_audio->setStreamCallback( cb, NULL );
    
        // start the audio
        g_audio->startStream( );
    }

    // make the window
    make_window( g_window, SND_BUFFER_SIZE );
    // compute log spacing
    g_log_space = compute_log_spacing( g_fft_size / 2, 3.6, g_log_factor );

    // initialize
    if( g_wf_delay )
    {
        g_waveforms = new SAMPLE *[g_wf_delay];
        for( int i = 0; i < g_wf_delay; i++ )
        {
            // allocate memory (stereo)
            g_waveforms[i] = new SAMPLE[g_buffer_size * 2];
            // zero it
            memset( g_waveforms[i], 0, g_buffer_size * 2 * sizeof(SAMPLE) );
        }
    }

    return true;
}




//-----------------------------------------------------------------------------
// Name: initialize_analysis( )
// Desc: sets initial audio analysis parameters
//-----------------------------------------------------------------------------
void initialize_analysis( )
{
    // spectral centroid
    g_centroid = new Centroid( SND_MARSYAS_SIZE );
    // down sampler
    g_down_sampler = new DownSampler( SND_BUFFER_SIZE, 2 );
    // flux
    g_flux = new Flux( SND_MARSYAS_SIZE );
    // lpc
    g_lpc = new LPC( SND_MARSYAS_SIZE );
    g_lpc->init();
    // mfcc
    g_mfcc = new MFCC( SND_MARSYAS_SIZE, 0 );
    g_mfcc->init();
    // rms
    g_rms = new RMS( SND_MARSYAS_SIZE );
    // rolloff
    g_rolloff = new Rolloff( SND_MARSYAS_SIZE, 0.5f );
    g_rolloff2 = new Rolloff( SND_MARSYAS_SIZE, 0.8f );
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
    glLightModeli( GL_FRONT_AND_BACK, GL_TRUE );
    // material have diffuse and ambient lighting 
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
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
        g_spectrums[i] = new Pt2D[SND_FFT_SIZE];
        memset( g_spectrums[i], 0, sizeof(Pt2D)*SND_FFT_SIZE );
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
    case 'j':
        g_z += .1f;
    break;
    case 'k':
        g_z -= .1f;
    break;
    case 'u':
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
    case 'e':
        g_draw_features = !g_draw_features;
    break;
    case 'q':
        exit( 0 );
    break;
    case '_':
        g_time_scale *= .99f;
        fprintf( stderr, "[sndpeek]: time domain scale: %f\n", g_time_scale );
    break;
    case '+':
        g_time_scale *= 1.01f;
        fprintf( stderr, "[sndpeek]: time domain scale: %f\n", g_time_scale ); 
    break;
    case '-':
        g_freq_scale *= .99f;
        fprintf( stderr, "[sndpeek]: freq domain scale: %f\n", g_freq_scale );
    break;
    case '=':
        g_freq_scale *= 1.01f;
        fprintf( stderr, "[sndpeek]: freq domain scale: %f\n", g_freq_scale );
    break;
    
    case 'V':
        if( g_time_view > 1 )
            g_time_view--;

        fprintf( stderr, "[sndpeek]: time domain %i samples", SND_BUFFER_SIZE / g_time_view );
        fprintf( stderr, g_time_view == 1 ? " - (MAX)\n" : "\n" );
    break;
    case 'C':
        if( g_time_view < 32 )
            g_time_view++;

        fprintf( stderr, "[sndpeek]: time domain %i samples", SND_BUFFER_SIZE / g_time_view );
        fprintf( stderr, g_time_view == 32 ? " - (MIN)\n" : "\n" );
    break;
    case '[':
        g_eye_y -= .025f;
    break;
    case ']':
        g_eye_y += .025f;
    break;
    case 'h':
        help();
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
        g_mute = !g_mute;
        fprintf( stderr, "mu(te)!\n" );
    break;
    case 'x':
        if( g_wvin )
        {
            g_restart = TRUE;
            fprintf( stderr, "[sndpeek]: restarting file...\n" );
        }
    break;
    case 'l':
        g_lissajous_scale *= .95f;
    break;
    case 'L':
        g_lissajous_scale *= 1.05f;
    break;
    case 'y':
        g_delay -= 10;
        if( g_delay < 0 )
            g_delay = 0;
        fprintf( stderr, "[sndpeek]: delay = %i\n", g_delay );
    break;
    case 'Y':
        g_delay += 10;
        if( g_delay > SND_BUFFER_SIZE )
            g_delay = SND_BUFFER_SIZE;
        fprintf( stderr, "[sndpeek]: delay = %i\n", g_delay );
    break;
    case 'z':
    case 'f':
        g_freeze = g_pause = !g_pause;
        fprintf( stderr, "free(ze)!\n" );
    break;
    case 'v':
    // case 't':
        g_log_factor *= .99985;
        g_log_space = compute_log_spacing( g_fft_size / 2, 3.6, g_log_factor );
        fprintf( stderr, "[sndpeek]: log_factor = %f\n", g_log_factor );
    break;
    case 'c':
    // case 'r': taken but can be changed
        g_log_factor /= .99985;
        g_log_space = compute_log_spacing( g_fft_size / 2, 3.6, g_log_factor );
        fprintf( stderr, "[sndpeek]: log_factor = %f\n", g_log_factor );
    break;
	case 'r':
		g_rainbow = !g_rainbow;
		fprintf( stderr, "[sndpeek]: fall colors %s\n", g_rainbow ? "ON" : "OFF" );
	break;
	case 'b':
		g_backwards = !g_backwards;
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
// name: ...
// desc: ...
//-----------------------------------------------------------------------------
void drawLissajous( SAMPLE * buffer, int len, int channels)
{
    float x, y;

    assert( channels >= 1 && channels <= 2 );
    
    glColor3f( 1.0f, 1.0f, .5f );
    glPushMatrix();
    glTranslatef( 1.2f, 0.0f, 0.0f );
    glBegin( GL_LINE_STRIP );
    for( int i = 0; i < len * channels; i += channels )
    {
        x = buffer[i] * g_lissajous_scale;
        //y = buffer[i + channels-1] * g_lissajous_scale;
        if( channels == 1 )
        {
            y = (i - g_delay >= 0) ? buffer[i-g_delay] : g_back_buffer[len + i-g_delay]; 
            y *= g_lissajous_scale;
        }
        else
        {
            y = buffer[i + channels-1] * g_lissajous_scale;
        }

//        glVertex3f( x, y, sqrt( x*x + y*y ) * -g_lissajous_scale );
        glVertex3f( x, y, 0.0f );
    }
    glEnd();
    glPopMatrix();
    
    if( channels == 1 )
        memcpy( g_back_buffer, buffer, len * sizeof(SAMPLE) );
}




//-----------------------------------------------------------------------------
// Name: main( )
// Desc: ...
//-----------------------------------------------------------------------------
double compute_log_spacing( int fft_size, double three_point_six, double factor )
{
    if( factor == 1.0 )
        return three_point_six / (fft_size+1);

    double s = ( 1 - pow( factor, fft_size+1 ) ) / ( 1 - factor );
    s = three_point_six / s;

    /* double sum = s;
    for( int i = 0; i < fft_size; i++ )
    {
        s *= factor;
        sum += s;
    } */

    return s;
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
    static unsigned wf = 0;
	static int starting = 1;

    static fvec in(SND_MARSYAS_SIZE),
        centroid(1), flux(1), lpc(g_lpc->outSize()), mfcc(13), rms(1), rolloff(1),
        rolloff2(1), centroid_lp(LP), flux_lp(LP), rms_lp(LP), rolloff_lp(LP),
        rolloff2_lp(LP);

    static float centroid_val, flux_val, rms_val, rolloff_val, rolloff2_val;
    float ytemp, fval;
    SAMPLE * buffer = g_fft_buffer, * ptr = in.getData();
    GLint i;
    
    // clear the color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushMatrix( );

        // rotate the sphere about y axis
        glRotatef( g_angle_y += g_inc, 0.0f, 1.0f, 0.0f );

        // wait for data
        while( !g_ready )
#if !defined(__OS_WINDOWS__)
            usleep( 0 );
#else
            Sleep( 0 );
#endif
        
        //g_mutex.lock();
        // get the window
        memcpy( buffer, g_audio_buffer, SND_BUFFER_SIZE * sizeof(SAMPLE) );
        g_ready = FALSE;
        //g_mutex.unlock();

        // lissajous
        if( !g_filename ) {
            drawLissajous( buffer, SND_BUFFER_SIZE, 1 );
        } else {
            drawLissajous( g_stereo_buffer, SND_BUFFER_SIZE, g_wvin->getChannels() );
        }

        // apply the window
        GLfloat x = -1.8f, inc = 3.6f / g_buffer_size, y = .7f;
        apply_window( (float*)buffer, g_window, SND_BUFFER_SIZE );

        // draw the time domain waveform
        glPushMatrix();
        // color waveform
        glColor3f( 0.4f, 0.4f, 1.0f );
        // translate/scale the waveform
        glTranslatef( x, y, 0.0f );
        glScalef( inc * g_time_view , g_gain * g_time_scale * 2.0, 1.0 );
        // draw waveform
        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin( GL_LINE_STRIP );
          GLint ii = ( SND_BUFFER_SIZE - (SND_BUFFER_SIZE/g_time_view) ) / 2;
          GLfloat xcoord = 0.0f;
          // loop through samples
          for( i = ii; i < ii + g_buffer_size / g_time_view; i++ )
          {
              glVertex2f( xcoord++ , buffer[i] );
          }
        glEnd();
        glPopMatrix();

        // fft
        rfft( (float *)buffer, SND_FFT_SIZE/2, FFT_FORWARD );

        x = -1.8f;
        y = -1.0f;

        // cast to complex
        complex * cbuf = (complex *)buffer;

        // color the spectrum
        glColor3f( 0.4f, 1.0f, 0.4f );

        // copy the frequency domain representation into the waterfall memory
        glNormal3f( 0.0f, 1.0f, 0.0f );
        for( i = 0; i < g_fft_size/2; i++ )
        {
            g_spectrums[wf][i].x = x;
            if( !g_usedb )
                g_spectrums[wf][i].y = g_gain * g_freq_scale * 3.0f *
                    ::pow( 25 * cmp_abs( cbuf[i] ), .5 ) + y;
            else
                g_spectrums[wf][i].y = g_gain * g_freq_scale * 
                    ( 20.0f * log10( cmp_abs(cbuf[i])/8.0 ) + 80.0f ) / 80.0f + y + .5f;
            x += inc * g_freq_view;
        }

        // draw the right things
        g_draw[wf] = true;
		if( !g_wutrfall )
			g_draw[wf] = false;
		if( !starting )
			g_draw[(wf+g_wf_delay)%g_depth] = true;
		
        x = -1.8f;
        inc = 3.6f / g_fft_size;
        glPushMatrix();
        glTranslatef( x, 0.0, g_z );
        glScalef( inc*g_freq_view , 1.0 , -g_space );

        // log spacing
        float log_space = g_log_space;
        float log_factor = g_log_factor;
        float log_pos = 0;

        // loop through each layer of waterfall
        for( i = 0; i < g_depth; i++ )
        {
            // log spacing
            log_space = (float)g_log_space;
            log_factor = (float)g_log_factor;
            log_pos = 0;

            if( i == g_wf_delay || !g_freeze || g_wutrfall )
            {
                // if layer is flagged for draw
                if( g_draw[(wf+i)%g_depth] )
                {
                    Pt2D * pt = g_spectrums[(wf+i)%g_depth];
                    if( i < g_wf_delay )
                    {
                        //fval = 1 - (g_wf_delay-i)/(float)g_wf_delay;  
                        fval = (g_depth - g_wf_delay + i) / (float)(g_depth);
						if( !g_rainbow )
							glColor3f( 1.0 * fval, .7 * fval, .4 * fval ); // depth cue
							// interesting colors: (.7, 1, .2), (.4, .9. 1), (1.0, 0.7, 0.2)
						else
						{
							float cval = 1 - (g_wf_delay - i) / (float)(g_wf_delay);
							cval = 0.4f + cval * (1.0f - 0.4f);
							glColor3f( 1.0f * fval, cval * fval, .4f * fval );
						}
                    }
                    else if( i == g_wf_delay )
                    {
                        glLineWidth( 3.0f );
						glColor3f( .4f, 1.0f, 1.0f );
                    }
                    else
                    {
                        //fval = (g_depth - i)/(float)(g_depth - g_wf_delay );
						fval = (g_depth - i + g_wf_delay) / (float)(g_depth);
						if( !g_rainbow )
							glColor3f( .4f * fval, 1.0f * fval, .4f * fval ); //depth cue
						else
						{
							float cval = 1 - (i - g_wf_delay) / (float)(g_depth - g_wf_delay);
							cval = 0.4f + cval * (1.0f - 0.4f);
							glColor3f( cval * fval, 1.0f * fval, .4f * fval );
						}
					}
                    glBegin( GL_LINE_STRIP );
                    for( int j = 0; j < g_fft_size/g_freq_view; j++, pt++ )
                    {
                        // glVertex3f( (float) j, pt->y, (float) i );
                        // draw the vertex
						float d = g_backwards ? g_depth - (float) i : (float) i;
                        glVertex3f( log_pos * g_fft_size/g_freq_view/3.6, pt->y, d );
                        // compute the next pos
                        log_pos += log_space;
                        // compute the next space
                        log_space *= log_factor;
                    }
                    glEnd();

					glLineWidth(1.0f);
                }
            }
        }
        glPopMatrix();
        
        if( !g_wutrfall )
			g_draw[(wf+g_wf_delay)%g_depth] = false;

        // wtrfll
        if( !g_freeze )
        {
            wf--;
            wf %= g_depth;
			if( wf == 1 )
				starting = 0;
        }

        // draw features
        if( g_draw_features )
        {
            if( !g_freeze )
            {
                int ratio = g_fft_size / SND_MARSYAS_SIZE / 2;
                // analysis
                for( i = 0; i < SND_MARSYAS_SIZE; i++ )
                    ptr[i] = cmp_abs( cbuf[i*ratio] );
        
                // centroid
                g_centroid->process( in, centroid );
                // flux
                g_flux->process( in, flux );
                // rms
                g_rms->process( in, rms );
                // rolloff
                g_rolloff->process( in, rolloff );
                g_rolloff2->process( in, rolloff2 );
        
                // lp filter
                centroid_lp(count % LP) = centroid(0);
                flux_lp(count % LP) = flux(0);
                rms_lp(count % LP) = rms(0);
                rolloff_lp(count % LP) = rolloff(0);
                rolloff2_lp(count % LP) = rolloff2(0);
                count++;
        
                centroid_val = centroid_lp.mean();
                flux_val = flux_lp.mean();
                rms_val = rms_lp.mean();
                rolloff_val = rolloff_lp.mean();
                rolloff2_val = rolloff2_lp.mean();
            }

            inc = 3.6f / SND_MARSYAS_SIZE;

            // draw the centroid
            ytemp = y+.04f + 2 * (::pow( 30 * rms_val, .5 ) );
            glColor3f( 1.0f, .4f, .4f );
            glBegin( GL_LINE_STRIP );
            glVertex3f( -1.8f + (centroid_val * inc * g_freq_view), ytemp, 0.0f + g_z );
            glVertex3f( -1.8f + (centroid_val * inc * g_freq_view), y-.04f, 0.0f + g_z );
            glEnd();
    
            // centroid value
            glBegin( GL_LINE_STRIP );
            glVertex3f( -1.8f + (centroid_val * inc * g_freq_view), y-.04f, 0.0f + g_z );
            glVertex3f( -1.72f + (centroid_val * inc * g_freq_view), y-.15f, 0.0f + g_z );
            glVertex3f( -1.15f + (centroid_val * inc * g_freq_view), y-.15f, 0.0f + g_z );
            glEnd();
        
            sprintf( str, "centroid = %.0f Hz", centroid_val / g_buffer_size * g_srate / 2 );
            draw_string( -1.7f + (centroid_val * inc * g_freq_view), y-.14f, 0.0f + g_z, str, .4f );

            // rms value
            glBegin( GL_LINE_STRIP );
            glVertex3f( -1.8f + (centroid_val * inc * g_freq_view) - .23f, ytemp, 0.0f + g_z );
            glVertex3f( -1.8f + (centroid_val * inc * g_freq_view) + .23f, ytemp, 0.0f + g_z );
            glEnd();
        
            sprintf( str, "RMS = %f", 1000 * rms_val );
            draw_string( -1.8f + (centroid_val * inc * g_freq_view) - .23f, ytemp + .01f, 0.0f + g_z, str, 0.4f );
        
            // draw the rolloff
            glColor3f( 1.0f, 1.0f, .4f );
            glBegin( GL_LINE_STRIP );
            glVertex3f( -1.8f + (rolloff_val * inc * g_freq_view), y-.04f, 0.0f + g_z );
            glVertex3f( -1.8f + (rolloff_val * inc * g_freq_view), y+.04f, 0.0f + g_z );
            glEnd();

            glColor3f( 1.0f, 1.0f, 1.0f );
            glBegin( GL_LINE_STRIP );
            glVertex3f( -1.8f + (rolloff2_val * inc * g_freq_view), y-.04f, 0.0f + g_z );
            glVertex3f( -1.8f + (rolloff2_val * inc * g_freq_view), y+.04f, 0.0f + g_z );
            glEnd();
        
            // centroid
            sprintf( str, "centroid = %.0f", centroid_val / g_buffer_size * g_srate / 2 );
            draw_string( -1.7f, 0.4f, 0.0f, str, 0.4f );
            // flux
            sprintf( str, "flux = %.1f", flux_val );
            draw_string( -1.7f, 0.3f, 0.0f, str, 0.4f );
            // flux
            sprintf( str, "RMS = %.4f", 1000 * rms_val );
            draw_string( -1.7f, 0.2f, 0.0f, str, 0.4f );
            // flux
            sprintf( str, "50%% rolloff= %.0f", rolloff_val / g_buffer_size * g_srate / 2 );
            draw_string( -1.7f, 0.1f, 0.0f, str, 0.4f );
            // flux
            sprintf( str, "80%% rolloff = %.0f", rolloff2_val / g_buffer_size * g_srate / 2 );
            draw_string( -1.7f, 0.0f, 0.0f, str, 0.4f );
        }

        if( g_stdout )
        {
            fprintf( stdout, "%.2f  %.2f  %.8f  %.2f  %.2f  ", centroid(0), flux(0), rms(0), rolloff(0), rolloff2(0) );
            fprintf( stdout, "%.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f %.2f %.2f  ", 
                     mfcc(0), mfcc(1), mfcc(2), mfcc(3), mfcc(4), mfcc(5), mfcc(6),
                     mfcc(7), mfcc(8), mfcc(9), mfcc(10), mfcc(11), mfcc(12) );
            fprintf( stdout, "\n" );
        }

        glColor3f( 1, 1, 1 );

        // title
        draw_string( 0.0f, 0.2f, 0.0f, "sndpeek + wutrfall", .5f );

        // pause?
        if( g_pause )
            draw_string( 0.95f, 1.3f, 0.0f, "paused... (press f to resume)", .4f );

        // mute?
        if( g_mute )
            draw_string( 0.95f, 1.25f, 0.0f, "muted... (press m to unmute)", .4f );

    glPopMatrix( );

    // swap the buffers
    glFlush( );
    glutSwapBuffers( );
    
    g_buffer_count_b++;
    if( g_filename && !g_file_running && g_buffer_count_b == g_buffer_count_a )
        g_running = FALSE;
}




//-----------------------------------------------------------------------------
// Name: extract_buffer( )
// Desc: extract one buffer
//-----------------------------------------------------------------------------
void extract_buffer( )
{
    static fvec raw(SND_BUFFER_SIZE), in(SND_MARSYAS_SIZE),
        centroid(1), flux(1), lpc(g_lpc->outSize()), mfcc(13), rms(1), rolloff(1),
        rolloff2(1);
    
    SAMPLE buffer[SND_BUFFER_SIZE], * ptr = in.getData();
    GLint i;
    
    while( !g_ready )
#if defined( __OS_WINDOWS__ )
        Sleep( 0 );
#else
        usleep( 0 );
#endif

    if( !g_filename )
    {
        g_mutex.lock();
        memcpy( buffer, g_audio_buffer, SND_BUFFER_SIZE * sizeof(SAMPLE) );
        g_ready = FALSE;
        g_mutex.unlock();
    }
    else
    {
        memcpy( buffer, g_audio_buffer, SND_BUFFER_SIZE * sizeof(SAMPLE) );
        g_ready = FALSE;
    }

    // apply the window
    apply_window( (float*)buffer, g_window, SND_BUFFER_SIZE );

    // fft
    rfft( (float *)buffer, SND_BUFFER_SIZE/2, FFT_FORWARD );
    
    complex * cbuf = (complex *)buffer;

    // analysis
    for( i = 0; i < g_buffer_size/2; i++ )
        ptr[i] = cmp_abs( cbuf[i] );

    // centroid
    g_centroid->process( in, centroid );
    // flux
    g_flux->process( in, flux );
    // lpc
    g_lpc->process( in, lpc );
    // mfcc
    g_mfcc->process( in, mfcc );
    // rms
    g_rms->process( in, rms );
    // rolloff
    g_rolloff->process( in, rolloff );
    g_rolloff2->process( in, rolloff2 );

    if( g_stdout )
    {
        fprintf( stdout, "%.2f  %.2f  %.8f  %.2f  %.2f  ", centroid(0), flux(0), rms(0), rolloff(0), rolloff2(0) );
        fprintf( stdout, "%.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f %.2f %.2f  ", 
                 mfcc(0), mfcc(1), mfcc(2), mfcc(3), mfcc(4), mfcc(5), mfcc(6),
                 mfcc(7), mfcc(8), mfcc(9), mfcc(10), mfcc(11), mfcc(12) );
        fprintf( stdout, "\n" );
    }

    g_buffer_count_b++;
    if( g_filename && !g_file_running && g_buffer_count_a == g_buffer_count_b )
        g_running = FALSE;
}
