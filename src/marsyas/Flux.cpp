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
    \class Flux:
    \brief Flux calculation.
    
    System for calculating the Flux (Fast Fourier Transform) of the
input fvec.  Calculate the magnitude of the difference of the
normalized input vector with the previous normalized input vector.
*/






#include "Flux.h"

Flux::Flux()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = 1;
  norm_ = new NormRMS(inSize_);
  prevWindow_.create(inSize_);
  diffWindow_.create(inSize_);
  normWindow_.create(inSize_);
}

Flux::Flux(unsigned int inSize)
{
  inSize_ = inSize;
  outSize_ = 1;
  norm_ = new NormRMS(inSize_);
  prevWindow_.create(inSize_);
  diffWindow_.create(inSize_);
  normWindow_.create(inSize_);
}

Flux::~Flux()
{
  delete norm_;
}


void 
Flux::process(fvec& in, fvec& out) 
{
  if ((in.size() != inSize_) || (out.size() != outSize_))
    {
      cerr << "Warning: Flux::process : inSize_ and input window size do not agree" << endl;
      return;
    } 
  norm_->process(in, normWindow_);
  diffWindow_ = fvec::minus( normWindow_, prevWindow_);
  diffWindow_.sqr();
  
  prevWindow_ = normWindow_;
  float flux = 0.0;
  for (unsigned int i=0; i<inSize_; i++)
    {
      flux += diffWindow_(i);
    }
  flux *= 1000.0;			// Scaling hack for display
  //cout << "FLUX = " << flux << endl;
  out(0) = flux;

  
}


	

	
