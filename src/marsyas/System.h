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
   \class System
   \brief System transforms a fvec

   Abstract base class for systems. Basically takes as input a 
vector of float numbers (fvec) and produces a new vector
(possibly with different dimensionality). Different 
types of computation can be used. System is a very basic 
class and includes transformations like FFT or Filter as well 
as feature extractors like Spectral Centroid.
*/



#if !defined(__System_h)
#define __System_h

#include <stdio.h>
#include "fvec.h"
#include "defs.h"
#include <string>

#pragma warning( disable : 4786 )

#include <vector>
#include <iostream>
using namespace std;


class System
{
protected:
  unsigned int inSize_;		// Input  vector size
  unsigned int outSize_;	// Output vector size
  string type_;			// Type of System

  vector<string> featNames_;	// Optional names of individual features
  unsigned int featSize_;	// size of featSize_ array 
public:
  System();
  System(unsigned int inSize);
  virtual ~System();
  unsigned int inSize();
  unsigned int outSize();
  vector<string> featNames();
  unsigned int featSize();
  virtual void process(fvec& in, fvec& out) = 0; 
};

#endif
