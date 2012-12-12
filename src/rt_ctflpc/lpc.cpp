//-----------------------------------------------------------------------------
// name: lpc.cpp
// desc: linear predictive coding
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//
// based (heavily) on filtlpc.c and lpcresyn.c by Perry Cook
// matrix code based on fmatrix by George Tzanetakis
//
// date: today
//-----------------------------------------------------------------------------
#include "lpc.h"
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#define MAX_PITCH 500

// internal data structure
struct lpc_data_
{
    SAMPLE * corr;
    SAMPLE * Zs;
    SAMPLE * Zss;
    thematrix * R;
    thematrix * res;
    int order;
    int len;
    int ticker;
};




//-----------------------------------------------------------------------------
// name: lpc_create()
// desc: ...
//-----------------------------------------------------------------------------
lpc_data lpc_create( )
{
    lpc_data instance = new lpc_data_;
    memset( instance, 0, sizeof(lpc_data_) );
    return instance;
}




//-----------------------------------------------------------------------------
// name: lpc_destroy()
// desc: ...
//-----------------------------------------------------------------------------
void lpc_destroy( lpc_data & instance )
{
    if( instance )
    {
        if( instance->corr )
            delete [] instance->corr;
        if( instance->R )
            delete instance->R;
        if( instance->res )
            delete instance->res;

        delete instance;
        instance = NULL;
    }
}




//-----------------------------------------------------------------------------
// name: lpc_predict()
// desc: ...
//-----------------------------------------------------------------------------
float lpc_predict( lpc_data lpc, SAMPLE * x, int len, float * coefs, int order, 
                   SAMPLE * residue )
{
    int i,j;
    float power = 0.0f;
    float error, tmp;
    for( i = 0; i < order; i++ )
        lpc->Zs[i] = x[order - i - 1];

    // set the hope size
    int hope_size = len - order;
    
    // zero out the residue
    if( residue ) memset( residue, 0, len * sizeof(SAMPLE) );

    // find the MSE
    for( i = order; i < hope_size + order; i++ )
    {
        tmp = 0.0f;
        for( j = 0; j < order; j++ ) tmp += lpc->Zs[j] * coefs[j];
        for( j = order - 1; j > 0; j-- ) lpc->Zs[j] = lpc->Zs[j-1];
        lpc->Zs[0] = x[i];
        error = x[i] - tmp;
        power += error * error;

        if( residue ) residue[i] = error;
    }

    // power
    return (float)sqrt(power) / hope_size;
}




//-----------------------------------------------------------------------------
// name: lpc_analyze()
// desc: ...
//-----------------------------------------------------------------------------
void lpc_analyze( lpc_data lpc, SAMPLE * x, int len, float * coefs, int order,
                  float * power, float * pitch, SAMPLE * residue )
{
    int i, j;

    // allocate
    if( lpc->len != len )
    {
        if( lpc->corr ) delete [] lpc->corr;
        lpc->corr = new SAMPLE[len];
        lpc->len = len;
    }
    
    // allocate the matrix
    if( lpc->order != order )
    {
        if( lpc->R ) delete lpc->R;
        if( lpc->res ) delete lpc->res;
        if( lpc->Zs ) delete [] lpc->Zs;
        if( lpc->Zss ) delete [] lpc->Zss;
        lpc->R = new thematrix( order, order );
        lpc->res = new thematrix( order, order );
        lpc->Zs = new float[order];
        lpc->Zss = new float[order];
        memset( lpc->Zss, 0, order * sizeof(float) );
        lpc->order = order;
        lpc->ticker = 0;
    }

    // find the autocorrelation of the signal, with pitch
    *pitch = autocorrelate( x, len, lpc->corr );

    // construct the R matrix
    for( i = 1; i <= order; i++ )
        for( j = 1; j <= order; j++ )
             (*(lpc->R))[i-1][j-1] = lpc->corr[abs((int)(i-j))];

    // invert R
    lpc->R->invert( *(lpc->res) );

    // find the coefficients A = P*R^(-1)
    for( i = 0; i < order; i++ )
    {
        coefs[i] = 0.0f;
        for( j = 0; j < order; j++ )
            coefs[i] += (*(lpc->R))[i][j] * lpc->corr[1+j];
    }

    // do the linear prediction to find residue
    *power = lpc_predict( lpc, x, len, coefs, order, residue );
}




//-----------------------------------------------------------------------------
// name: lpc_synthesize()
// desc: ...
//-----------------------------------------------------------------------------
void lpc_synthesize( lpc_data lpc, SAMPLE * y, int len, float * coefs,
                     int order, float power, float pitch, SAMPLE * residue )
{
    SAMPLE output;
    int i, j;

    // for( i = 0; i < order; i++ )
    //     lpc->Zss[i] = 0.0f;
    
    for( i = 0; i < len; i++ ) {
        output = 0.0f;

        if( residue )
            output = residue[i];
        else
        {
            if( pitch == 0 )
            {
                output = power * 20.0f * ( 2.0f * rand() / RAND_MAX - 1.0f );
                lpc->ticker = 0;
                if( i == len - 1 )
                    memset( lpc->Zss, 0, lpc->order * sizeof(float) );
            }
            else {
                lpc->ticker--;
                if( lpc->ticker <= 0 ) {
                    output = power * pitch * 1.0f;
                    lpc->ticker = (int)(pitch + .5f);
                }
            }
        }
        
        for( j = 0; j < order; j++ )
            output += lpc->Zss[j] * coefs[j];

        for( j = order - 1; j > 0; j-- )
            lpc->Zss[j] = lpc->Zss[j-1];

        lpc->Zss[0] = output;

        y[i] = output;
    }
}




//-----------------------------------------------------------------------------
// name: thematrix()
// desc: ...
//-----------------------------------------------------------------------------
thematrix::thematrix( unsigned int rows, unsigned int cols )
{
    m_data = new float[rows * cols];
    memset( m_data, 0, rows * cols * sizeof(float) );
    m_size = rows * cols;
    m_rows = rows;
    m_cols = cols;
}




//-----------------------------------------------------------------------------
// name: ~thematrix()
// desc: ...
//-----------------------------------------------------------------------------
thematrix::~thematrix()
{
    if( m_data )
    {
        delete [] m_data;
        m_data = NULL;
    }
}




//-----------------------------------------------------------------------------
// name: invert()
// desc: ...
//-----------------------------------------------------------------------------
int thematrix::invert( thematrix & res )
{
    int rank = 0;
    unsigned int r,c,i;
    float temp;
    assert( m_rows == m_cols );

    // initialize res to identity
    for( r = 0; r < m_rows; r++ )
        for( c = 0; c < m_cols; c++ )
        {
            if( r == c )
                res[r][c] = 1.0f;
            else
                res[r][c] = 0.0f;
        }

    for( i = 0; i < m_rows; i++)
    {
        if( (*this)[i][i] == 0.0f )
        {
            for( r = i; r < m_rows; r++ )
                for( c = 0; c < m_cols; c++ )
                {
                    (*this)[i][c] += (*this)[r][c];
                    res[i][c] += res[r][c];
                }
        }

        for( r = i; r < m_rows; r++ )
        {
            temp = (*this)[r][i];
            if( temp != 0.0f )
                for( c = 0; c < m_cols; c++ )
                {
                    (*this)[r][c] /= temp;
                    res[r][c] /= temp;
                }
        }

        if( i != m_rows - 1 )
        {
            for( r = i + 1; r < m_rows; r++ )
            {
                temp = (*this)[r][i];
                if( temp != 0.0f )
                    for( c = 0; c < m_cols; c++ )
                    {
                        (*this)[r][c] -= (*this)[i][c];
                        res[r][c] -= res[i][c];
                    }
            }
        }
    }

    for( i = 1; i < m_rows; i++ )
        for( r = 0; r < i; r++ )
        {
            temp = (*this)[r][i];
            for( c = 0; c < m_cols; c++ )
            {
                (*this)[r][c] -= (temp * (*this)[i][c]);
                res[r][c] -= (temp * res[i][c]);
            }
        }

    for( r = 0; r < m_rows; r++ )
        for( c = 0; c < m_cols; c++ )
            (*this)[r][c] = res[r][c];

    return rank;
}




//-----------------------------------------------------------------------------
// name: autocorrelate()
// desc: ...
//-----------------------------------------------------------------------------
float autocorrelate( SAMPLE * x, int len, SAMPLE * y )
{
    float norm, temp;
    int n, i, j, k;

    // refer to pp. 89 for variable name consistency
    for( n = 0; n < len; n++ )
    {
        temp = 0.0;
        for ( i = 0; i < len - n; i++ ) // used to be len - n - 1 but didn't make sense
            temp += x[i] * x[i+n];
        y[n] = temp;
    }

    // set temp to the first element of y
    temp = y[0];
    // why?
    j = (unsigned int)(len * 0.02);
    // FLASH! y is the autocorrelation, so the point n where y(n) is highest could be the start
    // or end of a period. (You probably knew this.)
    // loop to the point y stops decreasing
    while( y[j] < temp && j < len )
    {
        temp = y[j];
        j++;
    }

    // yes
    temp = 0.0;
    // find the max between j and the end
    for( i = j; i < len; i++ )
    {
        if( y[i] > temp)
        {
            j = i;
            temp = y[i];
        }
    }

    // why are we doing this?
    norm = 1.0f / len;
    k = len;

    // normalize, we think
    for( i = 0; i < len; i++ )
        y[i] *= (k-i) * norm;

    // cerr << j << " "; // :-)
    if( (y[j] / y[0]) < 0.4 ) j = 0; 
    if( j > MAX_PITCH ) j = 0; // used to be j > len / 4

    // we return the pitch information
    return (float) j;
}
