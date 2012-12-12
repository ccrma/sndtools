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

#if !defined(__LPC_h)
#define __LPC_h



#include "System.h"
#include "MagFFT.h"
#include "fvec.h"
#include "fmatrix.h"
#include <math.h>
#include "AutoCorrelation.h"
#include "MarSignal.h"

class LPC: public System
{
private:
  unsigned int order_;
  fmatrix rmat_;
  fvec corr_;
  fvec pres_;
  fvec Zs_;
  fmatrix temp_;
  float pitch_;
  float power_;
  AutoCorrelation* autocorr_;
  unsigned int hopSize_;

public:
  LPC( unsigned int size = DEFAULT_WIN_SIZE );
  LPC(Signal *src, unsigned int order);
  ~LPC();
  void init();
  float power();
  float pitch();
  void predict(fvec& data, fvec& coeffs);
  void process(fvec& in, fvec& out);
};


#endif 
