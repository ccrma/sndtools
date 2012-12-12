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
    \class Hamming
    \brief Hamming window

    
    Apply Hamming window to the input fvec. 
*/



#include "Hamming.h"


Hamming::Hamming()
{
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = DEFAULT_WIN_SIZE;

  zeroSize_ = 0;
  envelope_.allocate(inSize_ - zeroSize_);

  float A = 0.54f;
  float B = 0.46f;
  float ind;
  int envelopeSize = inSize_ - zeroSize_;
  
  for (int i=0; i< envelopeSize; i++)
    { 
      ind = i*2*PI / (envelopeSize-1);
      envelope_(i) = A - B*cos(ind);
    }        
  
}

Hamming::~Hamming()
{
  
}


Hamming::Hamming(unsigned int inSize, unsigned zeroSize)
{
  inSize_ = inSize;
  outSize_ = inSize_;
  zeroSize_ = zeroSize;
  envelope_.allocate(inSize_ - zeroSize_);
  int envelopeSize = inSize_ - zeroSize_;
  
  float A = 0.54f;
  float B = 0.46f;
  float ind;
  for (int i=0; i< envelopeSize; i++)
    { 
      ind = i*2*PI / (envelopeSize-1);
      envelope_(i) = A - B*cos(ind);
    }      
}



void 
Hamming::process(fvec& in, fvec& out)
{  
  unsigned int i;
  if ((in.size() != inSize_) || (out.size() != outSize_))
    {
      cerr << "Warning: Hamming::process: inSize_( " << inSize_ << ") and input window size(" << in.size() << ") do not agree. No feature calculation performed" << endl;
      return;
    }
  unsigned int hzeroSize = zeroSize_ /2 ;
  for (i=0; i < hzeroSize; i++)
    out(i) = 0.0;
  for (i=hzeroSize; i < inSize_ - hzeroSize; i++)
    out(i) = envelope_(i-hzeroSize) * in(i);
  for (i=inSize_ - hzeroSize; i < inSize_; i++)
    out(i) = 0.0;
}

	
