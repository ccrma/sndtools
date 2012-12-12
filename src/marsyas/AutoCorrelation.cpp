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
    \class AutoCorrelation
    \brief Calculated time domain autocorrelation
    
*/



#include "AutoCorrelation.h"

AutoCorrelation::AutoCorrelation()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = DEFAULT_WIN_SIZE;
}

AutoCorrelation::AutoCorrelation(unsigned int inSize)
{
  inSize_ = inSize;
  outSize_ = inSize;
}

float 
AutoCorrelation::pitch()
{
  return pitch_;
}

AutoCorrelation::~AutoCorrelation()
{
}


void 
AutoCorrelation::process(fvec& in, fvec& out) 
{
  float norm;
  float temp;
  float v1, v2;
  assert((in.size() == inSize_) && (out.size() >= outSize_));
  unsigned int i,j,k;
  v1 = 0;
  v2 = 0;
  
  
  for (i=0; i< outSize_/2; i++)
    {
      temp = 0.0;
      for (j=0; j < outSize_ - i-1; j++)
	{
	  v1 = in(i+j);
	  v2 = in(j);
	  temp += (v1 * v2);
	}
      out(i) = temp;
    }
  temp = out(0);
  j = (unsigned int)(outSize_ * 0.02);
  while (out(j) < temp && j < outSize_/2)
    {
      temp = out(j);
      j++;
    }
  temp = 0.0;
  for (i=j; i < outSize_ * 0.5; i++)
    {
      if (out(i) > temp) 
	{
	  j = i;
	  temp = out(i);
	}
    }
  norm = 1.0 / outSize_;
  k = outSize_/2;
  for (i=0; i < outSize_/2; i++) 
    out(i) *= (k-i) * norm;
  
  
  
  if ((out(j) / out(0)) < 0.4) j=0;
  if (j > outSize_/4) j = 0;
  pitch_  = (float)j;
}
	
