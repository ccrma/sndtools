//-----------------------------------------------------------------------------
// name: pvc.cpp
// desc: phase vocoder
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Ahmed Abdallah (aabdalla@princeton.edu)
//          Paul Botelho (pbotelho@princeton.edu)
// date: Spring 2004
//-----------------------------------------------------------------------------
#include "pvc.h"
#include <assert.h>
#include <memory.h>
#include <queue>
using namespace std;


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef BOOL__
#define BOOL__ unsigned int
#define UINT__ unsigned int
#define BYTE__ unsigned char
#endif



//-----------------------------------------------------------------------------
// name: class CBuffer
// desc: circular buffer
//-----------------------------------------------------------------------------
class CBuffer
{
public:
    CBuffer();
    ~CBuffer();

public:
    BOOL__ initialize( UINT__ num_elem, UINT__ width );
    void cleanup();

public:
    UINT__ get( void * data, UINT__ num_elem );
    void put( void * data, UINT__ num_elem );

protected:
    BYTE__ * m_data;
    UINT__   m_data_width;
    UINT__   m_read_offset;
    UINT__   m_write_offset;
    UINT__   m_max_elem;
};




// internal pvc data
struct pvc_data
{
    uint window_size;
    SAMPLE * window[2];
    uint which;
    SAMPLE * the_window;
    queue<polar_window *> windows;
    uint data_size;

    uint io_size;
    uint count;
    SAMPLE * ola[2];
    uint index;
    queue<SAMPLE *> ready;
    float K;

    polar * space;
    SAMPLE * space2;

    uint pool;
    CBuffer polar_pool;
    CBuffer win_pool;
};




//-----------------------------------------------------------------------------
// name: pv_create()
// desc: ...
//-----------------------------------------------------------------------------
pvc_data * pv_create( uint window_size, uint io_size, uint pool_size )
{
    pvc_data * data = new pvc_data;
    data->window[0] = new SAMPLE[(window_size + io_size)*2];
    data->window[1] = new SAMPLE[(window_size + io_size)*2];
    data->the_window = new SAMPLE[window_size];
    data->space = new polar[window_size/2];
    data->space2 = new SAMPLE[window_size];
    data->window_size = window_size;
    data->data_size = 0;
    data->which = 0;

    data->io_size = io_size;
    data->count = 0;
    data->ola[0] = new SAMPLE[(window_size + io_size)*2];
    data->ola[1] = new SAMPLE[(window_size + io_size)*2];
    data->index = 0;
    
    memset( data->ola[0], 0, (window_size + io_size)*2*sizeof(SAMPLE) );
    memset( data->ola[1], 0, (window_size + io_size)*2*sizeof(SAMPLE) );

    data->pool = pool_size;
    if( data->pool && data->pool < 256 )
        data->pool = 256;

    if( data->pool )
    {
        data->polar_pool.initialize( data->pool, sizeof(polar_window *) );
        data->win_pool.initialize( data->pool * 4, sizeof(SAMPLE *) );

        int i;
        polar_window * w;
        SAMPLE * a;
        for( i = 1; i < data->pool * 3 / 4; i++ )
        {
            w = new polar_window(window_size/2);
            data->polar_pool.put( &w, 1 );
        }
        for( i = 1; i < data->pool * 3; i++ )
        {
            a = new SAMPLE[window_size];
            data->win_pool.put( &a, 1 );
        }
    }

    // make the window
    pv_set_window( data, PV_HANNING );
    
    return data;
}




//-----------------------------------------------------------------------------
// name: pv_analyze()
// desc: ...
//-----------------------------------------------------------------------------
void pv_analyze( pvc_data * data, SAMPLE * buffer, uint hop_size )
{
    assert( hop_size <= data->window_size );
    
    uint to_copy = 0;
    complex * cmp = NULL;
    int i;
    uint len = data->window_size / 2;
    SAMPLE * w = data->window[data->which];
    data->which = !data->which;
    SAMPLE * w2 = data->window[data->which];

    // copy
    memcpy( w + data->data_size, buffer, data->io_size * sizeof(SAMPLE) );
    data->data_size += data->io_size;

    // one window
    while( data->data_size >= data->window_size )
    {
        polar_window * p = NULL;
        // analyze
        if( data->pool )
        {
            if( !data->polar_pool.get( &p, 1 ) )
            {
                fprintf( stderr, "pool exhausted!\n" );
                assert( FALSE );
            }
        }
        else
            p = new polar_window( data->window_size / 2 );

        memcpy( w2, w, data->window_size * sizeof(SAMPLE) );
        apply_window( w2, data->the_window, data->window_size );
        rfft( w2, data->window_size / 2, FFT_FORWARD );
        cmp = (complex *)w2;
        for( i = 0; i < len; i++ )
        {
            p->array[i].modulus = __modulus(cmp[i]);
            p->array[i].phase = __phase(cmp[i]);
        }
        // make a copy
        memcpy( p->old, p->array, p->len * sizeof(polar) );

        // queue
        data->windows.push( p );
        data->data_size -= hop_size;
        w += hop_size;
    }

    memcpy( w2, w, data->data_size * sizeof(SAMPLE) );
}




//-----------------------------------------------------------------------------
// name: pv_set_window()
// desc: ...
//-----------------------------------------------------------------------------
void pv_set_window( pvc_data * data, uint type )
{
    if( type == PV_HAMMING )
        hamming( data->the_window, data->window_size );
    else if( type == PV_BLACKMAN )
        blackman( data->the_window, data->window_size );
    else if( type == PV_HANNING )
        hanning( data->the_window, data->window_size );
    else
        rectangular( data->the_window, data->window_size );

    // find sum
    data->K = 0.0f;
    for( int i = 0; i < data->window_size; i++ )
        data->K += data->the_window[i] * data->the_window[i];
}




//-----------------------------------------------------------------------------
// name: pv_front()
// desc: ...
//-----------------------------------------------------------------------------
polar_window * pv_front( pvc_data * data )
{
    if( data->windows.size() > 0 )
        return data->windows.front();
    
    return NULL;
}




//-----------------------------------------------------------------------------
// name: pv_deque()
// desc: ...
//-----------------------------------------------------------------------------
polar_window * pv_deque( pvc_data * data )
{
    polar_window * p = NULL;

    if( data->windows.size() > 0 )
    {
        p = data->windows.front();
        data->windows.pop();
    }

    return p;
}




#define __PI 3.1415926f
//-----------------------------------------------------------------------------
// name: pv_unwrap_phase()
// desc: ...
//-----------------------------------------------------------------------------
void pv_unwrap_phase( polar_window * window )
{
    uint len = window->len;
    polar * p = (polar *)window->array;
    uint i;
    float x;

    for( i = 0; i < len; i++ )
    {
        x = floor( fabs( p[i].phase / __PI ) );
        if( p[i].phase < 0.0f ) x *= -1.0f;
        p[i].phase -= x * __PI;
    }
}




//-----------------------------------------------------------------------------
// name: pv_phase_fix()
// desc: ...
//-----------------------------------------------------------------------------
void pv_phase_fix( const polar_window * prev, polar_window * curr, float factor )
{
    // make sure not NULL
    if( !prev ) return;

    uint len = curr->len;
    const polar * p = prev->array;
    const polar * r = prev->old;
    polar * c = curr->array;

    int i;
    for (i = 0; i < len; i++, p++, r++, c++ )
        c->phase = factor * (c->phase - r->phase) + p->phase;
}




//-----------------------------------------------------------------------------
// name: pv_freq_shift()
// desc: ...
//-----------------------------------------------------------------------------
void pv_freq_shift( pvc_data * data, polar_window * window, float factor )
{
    uint len = window->len, n, floor, ceiling;
    polar * p = window->array;
    polar * q = data->space;
    float x, delta, alpha;

    // resample the frequency domain
    for( n = 0; n < len; n++ )
    {
        // index to sample
        x = (float)n/factor;
        // find the neighbors
        floor = (uint)x;
        ceiling = (uint)(x+.99);
        // interpolate
        if( ceiling < len )
        {
            if( floor == ceiling )
                q[n] = p[floor];
            else
            {
                q[n].modulus = p[floor].modulus +
                    ((float)x-floor)*(p[ceiling].modulus-p[floor].modulus);
                q[n].phase = p[floor].phase +
                    ((float)x-floor)*(p[ceiling].phase-p[floor].phase);
            }
        }
        else
        {
            q[n].modulus = 0.0f;
            q[n].phase = 0.0f;
        }

        q[n].modulus /= (float)sqrt(factor);
        q[n].phase *= factor;
    }

    // copy
    memcpy( p, q, len * sizeof(polar) );
}




//-----------------------------------------------------------------------------
// name: pv_cross_synth()
// desc: ...
//-----------------------------------------------------------------------------
void pv_cross_synth( polar_window * input, const polar_window * filter )
{
    const polar * p = (const polar *)filter->array;
    polar * q = input->array;
    int i;

    assert( input->len == filter->len );
    for( i = 0; i < input->len; i++ )
    {
        q[i].modulus *= p[i].modulus * input->len;
        q[i].phase += p[i].phase;
    }

    memcpy( input->old, input->array, input->len * sizeof(polar) );
}




//-----------------------------------------------------------------------------
// name: pv_ifft()
// desc: ...
//-----------------------------------------------------------------------------
void pv_ifft( const polar_window * window, SAMPLE * buffer )
{
    complex * cmp = (complex *)buffer;
    const polar * p = (const polar *)window->array;
    int i;

    for( i = 0; i < window->len; i++ )
    {
        cmp[i].re = p[i].modulus * cos( p[i].phase );
        cmp[i].im = p[i].modulus * sin( p[i].phase );
    }

    rfft( (float *)cmp, window->len, FFT_INVERSE );
}




//-----------------------------------------------------------------------------
// name: pv_overlap_add()
// desc: ...
//-----------------------------------------------------------------------------
void pv_overlap_add( pvc_data * data, polar_window * the_window, uint hop_size )
{
    assert( hop_size <= data->window_size );

    uint to_ola = data->window_size - hop_size;
    SAMPLE * window = data->space2;
    SAMPLE * w = data->ola[data->index];
    data->index = !data->index;
    SAMPLE * w2 = data->ola[data->index];
    int i;

    // ifft
    pv_ifft( the_window, window );
    
    // window
    apply_window( window, data->the_window, data->window_size );

    // scale
    float R = data->K / hop_size;
    for( i = 0; i < data->window_size; i++ )
        window[i] /= R;

    // overlap add
    SAMPLE * x = &w[data->count];
    for( i = 0; i < to_ola; i++ )
        x[i] += window[i];

    // copy
    memcpy( x+to_ola, window+to_ola, hop_size * sizeof(SAMPLE) );

    // queue
    data->count += hop_size;
    while( data->count >= data->io_size )
    {
        SAMPLE * buffer = NULL;
        if( data->pool )
        {
            if( !data->win_pool.get( &buffer, 1 ) )
            {
                fprintf( stderr, "pool exhausted!\n" );
                assert(FALSE);
            }
        }
        else
            buffer = new SAMPLE[data->io_size];
        memcpy( buffer, w, data->io_size * sizeof(SAMPLE) );
        data->ready.push( buffer );

        data->count -= data->io_size;
        w += data->io_size;
    }

    memset( w2, 0, (data->window_size + data->io_size)*2*sizeof(SAMPLE) );
    memcpy( w2, w, (data->window_size + data->count) * sizeof(SAMPLE) );
}




//-----------------------------------------------------------------------------
// name: pv_synthesize()
// desc: ...
//-----------------------------------------------------------------------------
uint pv_synthesize( pvc_data * data, SAMPLE * buffer )
{
    if( data->ready.size() == 0 )
        return FALSE;

    // deque
    memcpy( buffer, data->ready.front(), data->io_size * sizeof(SAMPLE) );
    pv_reclaim2( data, data->ready.front() );
    data->ready.pop();

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: pv_destroy()
// desc: ...
//-----------------------------------------------------------------------------
void pv_destroy( pvc_data * & data )
{
    delete [] data->window[0];
    delete [] data->window[1];
    delete [] data->ola[0];
    delete [] data->ola[1];
    delete [] data->space;
    delete [] data->space2;
    delete data;
    data = NULL;
}


uint pv_get_ready_len( pvc_data * data )
{
    return (uint)data->ready.size();
}


void pv_reclaim( pvc_data * data, polar_window * window )
{
    if( !window ) return;
    if( data->pool )
        data->polar_pool.put( &window, 1 );
    else
        delete window;
}


void pv_reclaim2( pvc_data * data, SAMPLE * window )
{
    if( !window ) return;
    if( data->pool )
        data->win_pool.put( &window, 1 );
    else
        delete []  window;
}


//-----------------------------------------------------------------------------
// name: Cbuffer()
// desc: constructor
//-----------------------------------------------------------------------------
CBuffer::CBuffer()
{
    m_data = NULL;
    m_data_width = m_read_offset = m_write_offset = m_max_elem = 0;
}




//-----------------------------------------------------------------------------
// name: ~CBuffer()
// desc: destructor
//-----------------------------------------------------------------------------
CBuffer::~CBuffer()
{
    this->cleanup();
}




//-----------------------------------------------------------------------------
// name: initialize()
// desc: initialize
//-----------------------------------------------------------------------------
BOOL__ CBuffer::initialize( UINT__ num_elem, UINT__ width )
{
    // cleanup
    cleanup();

    // allocate
    m_data = (BYTE__ *)malloc( num_elem * width );
    if( !m_data )
        return false;

    memset( m_data, 0, num_elem * width );
    m_data_width = width;
    m_read_offset = 0;
    m_write_offset = 0;
    m_max_elem = num_elem;

    return true;
}




//-----------------------------------------------------------------------------
// name: cleanup()
// desc: cleanup
//-----------------------------------------------------------------------------
void CBuffer::cleanup()
{
    if( !m_data )
        return;

    free( m_data );

    m_data = NULL;
    m_data_width = m_read_offset = m_write_offset = m_max_elem = 0;
}




//-----------------------------------------------------------------------------
// name: put()
// desc: put
//-----------------------------------------------------------------------------
void CBuffer::put( void * data, UINT__ num_elem )
{
    UINT__ i, j;
    BYTE__ * d = (BYTE__ *)data;

    // copy
    for( i = 0; i < num_elem; i++ )
    {
        for( j = 0; j < m_data_width; j++ )
        {
            m_data[m_write_offset*m_data_width+j] = d[i*m_data_width+j];
        }

        // move the write
        m_write_offset++;

        // wrap
        if( m_write_offset >= m_max_elem )
            m_write_offset = 0;
    }
}




//-----------------------------------------------------------------------------
// name: get()
// desc: get
//-----------------------------------------------------------------------------
UINT__ CBuffer::get( void * data, UINT__ num_elem )
{
    UINT__ i, j;
    BYTE__ * d = (BYTE__ *)data;

    // read catch up with write
    if( m_read_offset == m_write_offset )
        return 0;

    // copy
    for( i = 0; i < num_elem; i++ )
    {
        for( j = 0; j < m_data_width; j++ )
        {
            d[i*m_data_width+j] = m_data[m_read_offset*m_data_width+j];
            m_data[m_read_offset*m_data_width+j] = 0;
        }

        // move read
        m_read_offset++;

        // catch up
        if( m_read_offset == m_write_offset )
        {
            i++;
            break;
        }

        // wrap
        if( m_read_offset >= m_max_elem )
            m_read_offset = 0;
    }

    // return number of elems
    return 1;
}
