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


#include "System.h"

System::System()
{
  type_ = "Unspecified name";
  inSize_ = DEFAULT_WIN_SIZE;
  outSize_ = 1;
  featSize_ = 0;
  featNames_.reserve(100);
}


System::System(unsigned int inSize)
{
  inSize_ = inSize;
  outSize_ = 1;
  featSize_ = 0;
  featNames_.reserve(100);
}


System::~System()
{
  
}

vector <string>
System::featNames()
{
  return featNames_;
}

unsigned int 
System::featSize()
{
  return featSize_;
}

  

unsigned int
System::inSize()
{
  return inSize_;
}


unsigned int 
System::outSize()
{
  return outSize_;
}

    

	

