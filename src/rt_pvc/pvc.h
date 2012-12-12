//-----------------------------------------------------------------------------
// name: pvc.h
// desc: phase vocoder
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Ahmed Abdallah (aabdalla@princeton.edu)
//          Paul Botelho (pbotelho@princeton.edu)
// date: Spring 2004
//-----------------------------------------------------------------------------
#ifndef __PVC_H__
#define __PVC_H__

#include <stdlib.h>
#include "chuck_fft.h"

#ifndef SAMPLE
#define SAMPLE float
#endif

#define uint   unsigned int
#define PV_WINDOW_DEFAULT	1024
#define PV_HOPSIZE_DEFAULT  ( PV_WINDOW_DEFAULT / 8 )




// polar_array
struct polar_window
{
    polar * array;
    polar * old;
    uint len;

    // constructor
    polar_window( uint size )
    {
        array = new polar[size];
        old = new polar[size];
        len = size;
    }
    
    // destructor
    ~polar_window()
    {
        if( array )
        {
            delete [] array;
            delete [] old;
            array = NULL;
            old = NULL;
            len = 0;
        }
    }
};

struct pvc_data;




#define PV_HAMMING          0
#define PV_HANNING          1
#define PV_BLACKMAN         2
#define PV_RECTANGULAR      3




// interface
pvc_data * pv_create( uint window_size, uint io_size, uint pool_size );
void pv_set_window( pvc_data * data, uint type );
void pv_analyze( pvc_data * data, SAMPLE * in, uint hop_size );
polar_window * pv_front( pvc_data * data );
polar_window * pv_deque( pvc_data * data );
void pv_unwrap_phase( polar_window * window );
void pv_phase_fix( const polar_window * prev, polar_window * curr, float factor );
void pv_freq_shift( pvc_data * data, polar_window * window, float factor );
void pv_cross_synth( polar_window * input, const polar_window * filter );
void pv_overlap_add( pvc_data * data, polar_window * window, uint hop_size );
uint pv_synthesize( pvc_data * data, SAMPLE * out );
void pv_destroy( pvc_data * & data );

void pv_reclaim( pvc_data * data, polar_window * window );
void pv_reclaim2( pvc_data * data, SAMPLE * window );
uint pv_get_ready_len( pvc_data * data );
void pv_ifft( const polar_window * window, SAMPLE * buffer );




#endif
