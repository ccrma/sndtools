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
   \class Centroid
   \brief Centroid of fvec

   Compute centroid (the center of gravity)  of the input fvec.
*/




#include "Centroid.h"

Centroid::Centroid()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = 1;
}

Centroid::Centroid(unsigned int inSize)
{
  inSize_ = inSize;
  outSize_ = 1;
}

Centroid::~Centroid()
{
}


void 
Centroid::process(fvec& in, fvec& out) 
{
  float m0 = 0.0;
  float m1 = 0.0;
  float centroid;
  unsigned int i;
  
  if ((in.size() != inSize_) || (out.size() != outSize_))
    {
      cerr << "Warning: Centroid::process: inSize_ and input window size do not agree" << endl;
      return;
    }
  
  
  /* Compute centroid using moments */
  for (i=0; i < in.size(); i++)
    {
      m1 += (i * in(i));
      m0 += in(i);
    }

  if (m0 != 0.0) 
    centroid = m1 / m0;
  else 
    centroid = in.size() /2;		// Perfectly balanced

  out(0) = centroid;
}


	

	
