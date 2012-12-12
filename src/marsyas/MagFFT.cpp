/*
** Copyright (C) 2000 George Tzanetakis <gtzan@cs.princeton.edu>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/** 
    \class MagFFT:
    \brief FFT calculation.

   System for calculating the magnitude of the FFT (Fast Fourier
Transform) of the input fvec.  The actual fft code is scammed from
the CARL software and is very similar to the fft implementation in
Numerical Recipes. 
*/




#include <stdio.h>
#include <math.h>
#include "MagFFT.h"

typedef struct { float re ; float im ; } complex ;

#define CABS(x) hypot( (x).re, (x).im )

complex cadd(), csub(), cmult(), smult(), cdiv(), conjg(), csqrt() ;

extern complex zero ;
extern complex one ;
extern float synt ;



MagFFT::MagFFT()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = DEFAULT_WIN_SIZE/2;
  temp_.allocate(inSize_);
}


MagFFT::MagFFT(unsigned int inSize)
{
  inSize_ = inSize;
  outSize_ = inSize/2;
  temp_.allocate(inSize_);
}

void
MagFFT::process(fvec& in, fvec& out)
{
  unsigned int i;
  assert((in.size() == inSize_) && (out.size() == outSize_));  

  for (i=0; i<inSize_; i++)
    temp_(i) = in(i);
  float *temp = temp_.getData();
  rfft(temp, inSize_/2, FFT_FORWARD);
  
  temp[0] = 0.0;
  temp[1] = 0.0;
  out(0) = 0.0;				// DC
  
  /* Computer magnitude */
  for (i=1; i<inSize_/2; i++)
    {
      out(i) = sqrt(temp[2*i]*temp[2*i] + temp[2*i+1]*temp[2*i+1]);
    }
  
  


}




/*
 * bitreverse places float array x containing N/2 complex values
 * into bit-reversed order
 */

void
MagFFT::bitreverse(float x[], int N ) 
{
  float rtemp, itemp ;
  int i, j, m ;
  for ( i = j = 0 ; i < N ; i += 2, j += m ) {
    if ( j > i ) {
      rtemp = x[j] ; itemp = x[j+1] ; /* complex exchange */
      x[j] = x[i] ; x[j+1] = x[i+1] ;
      x[i] = rtemp ; x[i+1] = itemp ;
    }
    for ( m = N>>1 ; m >= 2 && j >= m ; m >>= 1 )
      j -= m ;
  }
}



/*
 * If forward is true, rfft replaces 2*N real data points in x with
 * N complex values representing the positive frequency half of their
 * Fourier spectrum, with x[1] replaced with the real part of the Nyquist
 * frequency value.  If forward is false, rfft expects x to contain a
 * positive frequency spectrum arranged as before, and replaces it with
 * 2*N real values.  N MUST be a power of 2.
 */
void
MagFFT::rfft( float x[], int  N, int forward ) 
{
  float c1, c2, h1r, h1i, h2r, h2i, wr, wi, wpr, wpi, temp, theta ;
  float xr, xi ;
  int i, i1, i2, i3, i4, N2p1 ;
  static int first = 1 ;
  theta = PI/N ;
  wr = 1. ;
  wi = 0. ;
  c1 = 0.5 ;
    if ( forward ) {
      c2 = -0.5 ;
      cfft( x, N, forward ) ;
      xr = x[0] ;
      xi = x[1] ;
    } 
    else 
      {
	c2 = 0.5 ;
	theta = -theta ;
	xr = x[1] ;
	xi = 0. ;
        x[1] = 0. ;
      }
    wpr = -2.*pow( sin( 0.5*theta ), 2. ) ;
    wpi = sin( theta ) ;
    N2p1 = (N<<1) + 1 ;
    for ( i = 0 ; i <= N>>1 ; i++ ) 
      {
	i1 = i<<1 ;
	i2 = i1 + 1 ;
	i3 = N2p1 - i2 ;
	i4 = i3 + 1 ;
	if ( i == 0 ) {
	  h1r =  c1*(x[i1] + xr ) ;
	  h1i =  c1*(x[i2] - xi ) ;
	  h2r = -c2*(x[i2] + xi ) ;
	  h2i =  c2*(x[i1] - xr ) ;
	  x[i1] =  h1r + wr*h2r - wi*h2i ;
	  x[i2] =  h1i + wr*h2i + wi*h2r ;
	  xr =  h1r - wr*h2r + wi*h2i ;
	  xi = -h1i + wr*h2i + wi*h2r ;
        } 
	else {
	  h1r =  c1*(x[i1] + x[i3] ) ;
	  h1i =  c1*(x[i2] - x[i4] ) ;
	  h2r = -c2*(x[i2] + x[i4] ) ;
	  h2i =  c2*(x[i1] - x[i3] ) ;
	  x[i1] =  h1r + wr*h2r - wi*h2i ;
	  x[i2] =  h1i + wr*h2i + wi*h2r ;
	  x[i3] =  h1r - wr*h2r + wi*h2i ;
	  x[i4] = -h1i + wr*h2i + wi*h2r ;
        }
        wr = (temp = wr)*wpr - wi*wpi + wr ;
        wi = wi*wpr + temp*wpi + wi ;
      }
    if ( forward )
      x[1] = xr ;
    else
      cfft( x, N, forward ) ;
}

/*
 * cfft replaces float array x containing NC complex values
 * (2*NC float values alternating real, imagininary, etc.)
 * by its Fourier transform if forward is true, or by its
 * inverse Fourier transform if forward is false, using a
 * recursive Fast Fourier transform method due to Danielson
 * and Lanczos.  NC MUST be a power of 2.
 */


void
MagFFT::cfft(float x[], int NC, int forward ) 
{
  float wr, wi, wpr, wpi, theta, scale ;
  int mmax, ND, m, i, j, delta ;
  ND = NC<<1 ;
  bitreverse( x, ND ) ;
  for ( mmax = 2 ; mmax < ND ; mmax = delta ) {
    delta = mmax<<1 ;
    theta = TWO_PI/( forward? mmax : -mmax ) ;
    wpr = -2.*pow( sin( 0.5*theta ), 2. ) ;
    wpi = sin( theta ) ;
    wr = 1. ;
    wi = 0. ;
    for ( m = 0 ; m < mmax ; m += 2 ) {
      register float rtemp, itemp ;
            for ( i = m ; i < ND ; i += delta ) 
	      {
		j = i + mmax ;
		rtemp = wr*x[j] - wi*x[j+1] ;
		itemp = wr*x[j+1] + wi*x[j] ;
		x[j] = x[i] - rtemp ;
		x[j+1] = x[i+1] - itemp ;
		x[i] += rtemp ;
		x[i+1] += itemp ;
	      }
            wr = (rtemp = wr)*wpr - wi*wpi + wr ;
            wi = wi*wpr + rtemp*wpi + wi ;
    }
  }
  /*
   * scale output
   */
  scale = forward ? 1./ND : 2. ;
  { register float *xi=x, *xe=x+ND ;
  while ( xi < xe )
    *xi++ *= scale ;
  }

}









