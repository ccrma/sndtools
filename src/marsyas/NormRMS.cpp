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
   \class NormRMS
   \brief Normalize based on RMS 

   Normalized the energy of the input fvec window by dividing the values 
by the RMS energy. 
*/


/* NormRMS: 
   Compute NormRMS (the center of gravity of the input vector) 
*/


#include "NormRMS.h"

NormRMS::NormRMS()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = DEFAULT_WIN_SIZE;
}

NormRMS::NormRMS(unsigned int inSize)
{
  inSize_ = inSize;
  outSize_ = inSize;
}



NormRMS::~NormRMS()
{
}

void 
NormRMS::process(fvec& in, fvec& out) 
{
  
  assert((in.size() == inSize_) && (out.size() == outSize_));  
  unsigned int i;
  float energy =0.0;
  for (i=0; i< inSize_; i++)
    {
      energy += (in(i) * in(i));
    }
  if (energy == 0.0) 
    return;
  else 
    energy = sqrt(energy);
  for (i=0; i< inSize_; i++)
    {
      if (in(i) > 0.0) 
	out(i) = in(i) / energy;
    }
}


	

	
