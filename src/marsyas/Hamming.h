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

#if !defined(__Hamming_h)
#define __Hamming_h
	

#include "System.h"
#include <math.h>

class Hamming: public System
{
private:
  fvec envelope_;
  int zeroSize_;
public:
  Hamming();
  ~Hamming();
  Hamming(unsigned int inSize, unsigned int zeroSize);
  void process(fvec& in, fvec& out);
};

#endif
