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

/* Flux: 
   Compute Flux (the center of gravity of the input vector) 
*/

#if !defined(__Flux_h)
#define __Flux_h

#include "System.h"	
#include "NormRMS.h"

/** 
    \class Flux:
    \brief Flux calculation.

   System for calculating the Flux (Fast Fourier Transform) of the
input fvec.  Calculate the magnitude of the difference of the
normalized input vector with the previous normalized input vector.
 */



class Flux: public System
{
private:
  fvec prevWindow_;
  fvec diffWindow_;
  fvec normWindow_;
  NormRMS* norm_;
public:
  Flux();
  Flux(unsigned int inSize);
  ~Flux();
  void process(fvec& in, fvec& out);
  
};

#endif

	
