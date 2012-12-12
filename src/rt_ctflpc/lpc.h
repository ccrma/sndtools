//-----------------------------------------------------------------------------
// name: lpc.h
// desc: linear predictive coding
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//
// based (heavily) on filtlpc.c and lpcresyn.c by Perry Cook
// matrix based on fmatrix by George Tzanetakis
//
// date: today
//-----------------------------------------------------------------------------
#ifndef __LPC_H__
#define __LPC_H__

#include <stdlib.h>

#ifndef SAMPLE
#define SAMPLE float
#endif


// forward reference
typedef struct lpc_data_ * lpc_data;


// init
lpc_data lpc_create( );
// analysis
void lpc_analyze( lpc_data instance, SAMPLE * x, int len, float * coefs, 
                  int order, float * power, float * pitch, 
                  SAMPLE * residue = NULL );
// synthesis
void lpc_synthesize( lpc_data instance, SAMPLE * y, int len, float * coefs,
                     int order, float power, float pitch,
                     SAMPLE * residue = NULL );
// done
void lpc_destroy( lpc_data & instance );


// helper -- autocorrelation
float autocorrelate( SAMPLE * x, int len, SAMPLE * y );
// helper -- lpc prediction 
float lpc_predict( lpc_data lpc, SAMPLE * x, int len, float * coefs, int order );


//-----------------------------------------------------------------------------
// name: class thematrix
// desc: ...
//-----------------------------------------------------------------------------
class thematrix
{
public:
    thematrix( unsigned int rows, unsigned int cols );
    ~thematrix();

public:
    int invert( thematrix & res );

public:
    float * operator[]( const long r );
    const float * operator[]( const long r ) const;

public:
    float * m_data;
    unsigned int m_size;
    unsigned int m_rows;
    unsigned int m_cols;
};


//-----------------------------------------------------------------------------
// name: operator[]
// desc: yes, you see what this does
//-----------------------------------------------------------------------------
inline float * thematrix::operator[]( const long row )
{ return m_data + row * m_cols; }


//-----------------------------------------------------------------------------
// name: the other operator[]
// desc: for the const cases if you know what i mean
//-----------------------------------------------------------------------------
inline const float * thematrix::operator[]( const long row ) const
{ return m_data + row * m_cols; }




#endif
