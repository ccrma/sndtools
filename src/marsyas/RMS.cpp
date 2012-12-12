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
    \class RMS
    \brief RMS energy of window

   Calculate the RMS energy of the input fvec window. 
RMS is defined as the square root of the sum of 
the squarred values of the signal. 
*/



#include "RMS.h"

RMS::RMS()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = 1;
}

RMS::RMS(unsigned int inSize)
{
  inSize_ = inSize;
  outSize_ = 1;
}

RMS::~RMS()
{
}


void 
RMS::process(fvec& in, fvec& out) 
{
  float rmsEnergy = 0.0;
  unsigned int i;
  
  if ((in.size() != inSize_) || (out.size() != outSize_))
    {
      cerr << "Warning: RMS::process: inSize_ and input window size do not agree" << endl;
      return;
    }
  float val;
  /* Compute centroid using moments */
  for (i=0; i < inSize_; i++)
    {
      val = in(i);
      rmsEnergy += (val * val);
    }
  rmsEnergy /= inSize_;
  rmsEnergy = sqrt(rmsEnergy);
  out(0) = rmsEnergy;
}


	

	
