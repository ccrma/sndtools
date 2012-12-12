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
    \class Rolloff
    \brief Rolloff of input fvec

   Compute Rolloff (a percentile point) of the input fvec. 
For example if perc_ is 0.90 then Rolloff would be the 
index where the sum of values before that index are 
equal to 90% of the total energy. 
*/




#include "Rolloff.h"

Rolloff::Rolloff()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = 1;
  perc_ = 0.80f;
  sumWindow_.create(DEFAULT_WIN_SIZE);
}

Rolloff::Rolloff(unsigned int inSize, float perc)
{
  inSize_ = inSize;
  perc_ = perc;
  outSize_ = 1;
  sumWindow_.create(inSize_);
}


void 
Rolloff::process(fvec& in, fvec& out) 
{
  unsigned int i;
  if ((in.size() != inSize_) || (out.size() != outSize_))
    {
      cerr << "Warning: Rolloff::process: inSize_ and input window size do not agree" << endl;
      return;
    } 
  float sum = 0.0;
  for (i=0; i<inSize_; i++)
    {
      sum += in(i);
      sumWindow_(i) = sum;
    }
  float total = sumWindow_(inSize_-1);
  
  for (i=inSize_-1; i>1; i--)
    {
      if (sumWindow_(i) < perc_*total)
	{
	  out(0) = (float)i;
	  return;
	}
    }
  out(0) = (float)(inSize_ -1);
}


	

	
