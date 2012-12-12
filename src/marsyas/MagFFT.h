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

/* MagFFT:
   Calculate the FFT (Fast Fourier Transform) of the input vector.
   The actual fft code is scammed from the CARL software. 
*/

#if !defined(__MagFFT_h)
#define __MagFFT_h
	

/** 
    \class MagFFT:
    \brief FFT calculation.

   System for calculating the magnitude of the FFT (Fast Fourier
Transform) of the input fvec.  The actual fft code is scammed from
the CARL software and is very similar to the fft implementation in
Numerical Recipes. 
*/




#include "System.h"

class MagFFT: public System
{
private:
  void bitreverse(float x[], int N);

  fvec temp_;
public:
  MagFFT();
  MagFFT(unsigned int inSize);
  void rfft( float x[], int  N, int forward);
  void cfft(float x[], int N, int forward);  
  void process(fvec& in, fvec& out);
};


#endif
	
