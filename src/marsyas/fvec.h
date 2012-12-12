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
    \class fvec
    \brief Vector of float values

    Array (vector in the numerical sense) of float values. Basic 
arithmetic operations and statistics are supported. 
*/

	

#if !defined(__fvec_h)
#define __fvec_h

#include <stdio.h>
#include <math.h>
#include "Communicator.h"
#include <assert.h>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;


class fvec 
{
// HACK: was private
public:
  float *data_;
  unsigned int size_;
  string name_;
  
public:
  fvec();
  ~fvec();
  fvec(unsigned int size);
  fvec(const fvec& a);
  fvec& operator=(const fvec& a);
  void create(unsigned long size);		// allocate(size) + fill zeros
  void create(float val, unsigned long size);	// allocate(size) + fill val
  void allocate(unsigned long size);	// just allocate(size) random values 
  void setval(unsigned int start, unsigned int end, float val);
  void setval(float val);			// set all entries to val 
  void setName(string name);
  unsigned int size();
  float *getData();			// dirty for easy integration 

  
  fvec& operator+=(const fvec& vec);
  fvec& operator-=(const fvec& vec);
  fvec& operator*=(const fvec& vec);
  fvec& operator/=(const fvec& vec);
  fvec& operator*=(const float val);
  fvec& operator/=(const float val);
  fvec& operator+=(const float val);
  fvec& operator-=(const float val);
  
  
  
  float& operator()(const unsigned int i);
  float operator()(const unsigned int i) const;
  void debug_info();
  void write(string filename);
  void read(string filename);
  friend ostream& operator<<(ostream&, const fvec&);
  friend istream& operator>>(istream&, fvec&);
  // HACK: was friend fvec operator
  static fvec plus(const fvec& vec1, const fvec& vec2);
  static fvec minus(const fvec& vec1, const fvec& vec2);
  float mean();
  float std();
  float var();
  void abs();
  void sqr();
  void sqroot();
  void renorm(float old_mean, float old_std, float new_mean, float new_std);
  void send(Communicator *com);
};


inline 
fvec&
fvec::operator/=(const float val)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] /= val;
  return *this;
}


inline 
fvec&
fvec::operator*=(const float val)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] *= val;
  return *this;
}




inline 
fvec&
fvec::operator-=(const float val)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] -= val;
  return *this;
}

inline 
fvec&
fvec::operator+=(const float val)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] += val;
  return *this;
}



  
inline
fvec& 
fvec::operator+=(const fvec& vec)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] += vec.data_[i];
  return *this;
}

inline
fvec& 
fvec::operator-=(const fvec& vec)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] -= vec.data_[i];
  return *this;
}







inline
fvec& 
fvec::operator*=(const fvec& vec)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] *= vec.data_[i];
  return *this;
}

inline
fvec& 
fvec::operator/=(const fvec& vec)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] /= vec.data_[i];
  return *this;
}
  

inline 
float fvec::operator()(const unsigned int i) const
{
  assert(i < size_);
  return data_[i];
}


inline 
float& fvec::operator()(const unsigned int i)
{
  assert(i < size_);
  return data_[i];
}



#endif
