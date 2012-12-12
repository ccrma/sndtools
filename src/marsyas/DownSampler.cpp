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
    \class DownSampler
    \brief System for downsampling

*/


#include "DownSampler.h"


DownSampler::DownSampler()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = DEFAULT_WIN_SIZE /2;
  factor_ = 2;
}

DownSampler::DownSampler(unsigned int inSize, unsigned int factor)
{
  inSize_ = inSize;
  outSize_ = inSize_ / factor;
  factor_ = factor;
}


DownSampler::~DownSampler()
{
}



void
DownSampler::process(fvec& in, fvec& out)
{
  assert((in.size() == inSize_) && (out.size() == outSize_));  
  unsigned int size = in.size();
  unsigned int i;
  
  for (i=0; i< size / factor_; i++)
    {
      out(i) = in(i * factor_);
    }
}

	
