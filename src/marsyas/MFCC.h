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
   \class MFCC:
   \brief Mel-frequency cepstral coefficients

   Mel-frequency cepstral coefficients. Features commonly used 
in Speech Recognition research. Code based on the Matlab 
implementation of Malcolm Slaney. 
*/

#if !defined(__MFCC_h)
#define __MFCC_h

#include "System.h"
#include "MagFFT.h"
#include "fvec.h"
#include "fmatrix.h"
#include <math.h>
#include "Hamming.h"

class MFCC: public System
{
private:

  Hamming* hamming_;
  MagFFT* magfft_;
  fvec windowed;
  fvec magnitude;
  fvec fmagnitude;
  

  fvec freqs_;
  fvec lower_;
  fvec center_;
  fvec upper_;
  fvec triangle_heights_;

  unsigned int zeroSize_;
  float lowestFrequency_;
  unsigned int linearFilters_;
  float linearSpacing_;
  unsigned int logFilters_;
  float logSpacing_;
  unsigned int totalFilters_;

  unsigned int fftSize_;
  unsigned int samplingRate_;
  unsigned int cepstralCoefs_;
  
  fvec fftFreqs_;
  
  fmatrix mfccFilterWeights_;
  fmatrix mfccDCT_;
  fvec earMagnitude_;
  

  
public:
  MFCC();
  MFCC(unsigned int inSize, unsigned int zeroSize);
  ~MFCC();
  void init();
  void process(fvec& in, fvec& out);
};


#endif 
