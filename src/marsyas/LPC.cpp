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
   \class LPC:
   \brief Linear Prediction Coefficients (LPC)

   Linear Prediction Coefficients (LPC). Features commonly used 
in Speech Recognition research. 
*/


#include "LPC.h"
#include <math.h>
#include <stdlib.h>

LPC::LPC( unsigned int inSize )
{
  inSize_ = inSize;
  outSize_ = DEFAULT_ORDER+1;
  order_ = DEFAULT_ORDER;
  autocorr_ = NULL;
  hopSize_ = inSize_ / 2;
  init();
}

LPC::LPC(Signal* src,unsigned int order)
{
  inSize_ = src->winSize();
  hopSize_ = src->hopSize();
  outSize_ = order+1;
  order_ = order;
  autocorr_ = NULL;
  init();
}

LPC::~LPC()
{
  delete autocorr_;
}

void
LPC::init()
{
  unsigned int i;
  
  // Prepare feature names
  featSize_ = outSize_;
  for (i=0; i < featSize_; i++)
    featNames_.push_back("LPC");
  rmat_.create(order_ - 1, order_ - 1);
  temp_.create(order_ - 1, order_ - 1);
  corr_.create(inSize_);
  autocorr_ = new AutoCorrelation(inSize_);
  Zs_.create(order_-1);
}

float 
LPC::pitch()
{
  return pitch_;
}

float 
LPC::power()
{
  return power_;
}


void 
LPC::predict(fvec& data, fvec& coeffs)
{
  unsigned int i,j;
  float power = 0.0;
  float error, tmp;
  for (i=0; i< order_-1; i++) 
    {
      Zs_(i) = data(order_-1-i-1);
    }
  for (i=order_-1; i<= hopSize_ + order_-1; i++)
    {
      tmp = 0.0;
      for (j=0; j< order_-1; j++) tmp += Zs_(j) * coeffs(j);
      for (j=order_-1-1; j>0; j--) Zs_(j) = Zs_(j-1);
      Zs_(0) = data(i);
      error = data(i) - tmp;
      power += error * error;
    }
  power_ = sqrt(power) / hopSize_;
}



void 
LPC::process(fvec& in, fvec& out)
{
//    fprintf( stderr, "%i %i %i %i\n", in.size(), inSize_, out.size(), outSize_ );
  assert((in.size() == inSize_) && (out.size() == outSize_));
  unsigned int i,j;
  autocorr_->process(in, corr_);
  pitch_ = autocorr_->pitch();
  for (i=1; i<order_; i++)
    for (j=1; j<order_; j++)
      {
		rmat_(i-1,j-1) = corr_(abs((int)(i-j)));
      }

  rmat_.invert(temp_);
  for (i=0; i < order_-1; i++)
    {
      out(i) = 0.0;
      for (j=0; j < order_-1; j++)
	    {
		  out(i) += (rmat_(i,j) * corr_(1+j));
	    }
	}

  predict(in, out);
  out(order_-1) = pitch_;
  out(order_) = power_;
}

