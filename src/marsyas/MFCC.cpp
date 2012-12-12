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



#include "MFCC.h"

MFCC::MFCC()
{
  inSize_ = DEFAULT_WIN_SIZE;
  zeroSize_ = 0;
  outSize_ = 13;
  init();
}

MFCC::MFCC(unsigned int inSize, unsigned int zeroSize)
{
  inSize_ = inSize;
  zeroSize_ = zeroSize;
  outSize_ = 13;
  init();
}

MFCC::~MFCC()
{
  delete hamming_;
  delete magfft_;
}


void 
MFCC::init()
{
  unsigned int i,j;
  unsigned int chan;
  
  if (inSize_ != DEFAULT_WIN_SIZE) 
    {
      cerr << "MFCC::init warning inSize_ != 512\n" << endl ;
    }
  
  // Initialize frequency boundaries for the filters 
  freqs_.create(42);
  lowestFrequency_ = 133.3333f;
  linearFilters_ = 13;
  linearSpacing_ = 66.66666f;
  logFilters_ = 27;
  logSpacing_ = 1.0711703f;
  
  totalFilters_ = linearFilters_ + logFilters_;
  
  lower_.create(totalFilters_);
  center_.create(totalFilters_);
  upper_.create(totalFilters_);
  triangle_heights_.create(totalFilters_);

  // Linear filter boundaries
  for (i=0; i< linearFilters_; i++)
    freqs_(i) = lowestFrequency_ + i * linearSpacing_;

  // Logarithmic filter boundaries  
  float first_log = freqs_(linearFilters_-1);
  for (i=1; i<=logFilters_+2; i++)
    {
      freqs_(linearFilters_-1+i) = first_log * pow(logSpacing_, (float)i);
    }  
 
  // Triangles information
  for (i=0; i<totalFilters_; i++)
    lower_(i) = freqs_(i);
  
  for (i=1; i<= totalFilters_; i++)
    center_(i-1) = freqs_(i);
  
  for (i=2; i<= totalFilters_+1; i++)
    upper_(i-2) = freqs_(i);
  
  for (i=0; i<totalFilters_; i++)
    triangle_heights_(i) = 2.0 / (upper_(i) - lower_(i));
 
  fftSize_ = 512;
  samplingRate_ = 22050;
  fftFreqs_.create(fftSize_);
  cepstralCoefs_ = 13;
  
  for (i=0; i< fftSize_; i++)
    fftFreqs_(i) = (float)i / (float)fftSize_ * (float)samplingRate_;
  
  mfccFilterWeights_.create(totalFilters_, fftSize_);
  mfccDCT_.create(cepstralCoefs_, totalFilters_);
  
  // Initialize mfccFilterWeights

  
  for (chan = 0; chan < totalFilters_; chan++)
    for (i=0; i< fftSize_; i++)
      {
	if ((fftFreqs_(i) > lower_(chan))&& (fftFreqs_(i) <= center_(chan)))
	  {
	    mfccFilterWeights_(chan, i) = triangle_heights_(chan) *
	      ((fftFreqs_(i) - lower_(chan))/(center_(chan) - lower_(chan)));
	  }
	if ((fftFreqs_(i) > center_(chan)) && (fftFreqs_(i) <= upper_(chan)))
	  {
	    mfccFilterWeights_(chan, i) = triangle_heights_(chan) *
	      ((upper_(chan) - fftFreqs_(i))/(upper_(chan) - center_(chan)));
	  }
      }

  // Initialize MFCC_DCT
  float scale_fac = 1.0/ sqrt((double)(totalFilters_/2));
  for (j = 0; j<cepstralCoefs_; j++)
    for (i=0; i< totalFilters_; i++)
      {
	mfccDCT_(j, i) = scale_fac * cos(j * (2*i +1) * PI/2/totalFilters_);
	if (i == 0)
	  mfccDCT_(j,i) *= sqrt(2.0)/2.0;
      }  

  // Prepare feature names 
  
  featSize_ = outSize_;
  featNames_.push_back("MFCC00");
  featNames_.push_back("MFCC01");
  featNames_.push_back("MFCC02");
  featNames_.push_back("MFCC03");
  featNames_.push_back("MFCC04");
  featNames_.push_back("MFCC05");
  featNames_.push_back("MFCC06");
  featNames_.push_back("MFCC07");
  featNames_.push_back("MFCC08");
  featNames_.push_back("MFCC09");
  featNames_.push_back("MFCC10");
  featNames_.push_back("MFCC11");
  featNames_.push_back("MFCC12");

  hamming_ = new Hamming(inSize_, zeroSize_);
  magfft_ = new MagFFT(inSize_);
  windowed.create(inSize_);
  magnitude.create(inSize_/2);
  fmagnitude.create(inSize_);
  earMagnitude_.create(totalFilters_);
}





void 
MFCC::process(fvec& in, fvec& out)
{
  unsigned int i,k;
  if ((in.size() != inSize_) || (out.size() != outSize_))
    {
      cerr << "Warnging: MFCC::process:  inSize_ and input window size do not agree" << endl;
      return;
    }  
  
  hamming_->process(in, windowed);
  magfft_->process(windowed, magnitude);

  for (i=0; i < inSize_/2; i++)
    fmagnitude(i) = magnitude(i);
  
  for (i=0; i< inSize_/2; i++)
    fmagnitude(i+ inSize_/2) = fmagnitude(inSize_/2 - i);
  
  float sum =0.0;
  // Calculate the filterbank responce
  for (i=0; i<totalFilters_; i++)
    { 
      sum = 0.0;
      for (k=0; k<fftSize_; k++)
	{
	  sum += (mfccFilterWeights_(i, k) * fmagnitude(k));
	}
      if (sum != 0.0)
	earMagnitude_(i) = log10(sum);
      else 
	earMagnitude_(i) = 0.0;
    }  

  // Take the DCT 
  for (i=0; i < cepstralCoefs_; i++)
    {
      sum =0.0;
      for (k=0; k < totalFilters_; k++)
	{
	  sum += (mfccDCT_(i,k) * earMagnitude_(k));
	}
      out(i) = sum;
    }  
  

}

