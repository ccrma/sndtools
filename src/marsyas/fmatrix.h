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
    \class fmatrix
    \brief Matrix of float values

    Matrix of floating point values. Similar to fvec. Basic 
arithmetic operations and statistics are supported. 
*/


#if !defined(__fmatrix_h)
#define __fmatrix_h


#include <fstream>
#include <iostream>
#include <string>
using namespace std;



#include "fvec.h"

class fmatrix
{
public:
  double *data_;
  unsigned int size_;
  unsigned int rows_;
  unsigned int cols_;
  string name_;
  
  
  fvec row_;
  fvec col_;
  fvec meanRow_;
  fvec stdRow_;
  fvec varRow_;
  bool printHeader_;

  
public: 
  fmatrix();
  ~fmatrix();
  fmatrix(unsigned int rows);
  fmatrix(unsigned int rows, unsigned int cols);
  fmatrix(const fmatrix& a);
  fmatrix& operator=(const fmatrix& a);
  
  // Initialization functions
  void create(unsigned int rows, unsigned int cols);
  void create(unsigned int rows);
  void identity(unsigned int rows);
  void setval(double val);
  fmatrix covariance();
  fmatrix correlation();
  // Statistics 
  fvec  row(const unsigned int r);
  fvec meanRow();
  fvec stdRow();
  fvec varRow();
  fvec  col(const unsigned int c);
  void row(const unsigned int r, fvec& vec);
  void col(const unsigned int r, fvec& vec);
  int invert(fmatrix& res);
  void standarize(fvec meanRow, fvec stdRow);
  void flround();
  void clip(double min, double max);
  
  
  
  unsigned int rows();
  unsigned int cols();
  friend istream& operator>>(istream&, fmatrix&);
  friend ostream& operator<<(ostream&, const fmatrix&);
  fmatrix& operator*=(const fmatrix& matrix);
  fmatrix& operator+=(const fmatrix& matrix);
  fmatrix& operator/=(const double val);
  fmatrix& operator*=(const double val);
  fmatrix& operator+=(const double val);
  
  
  // Misc
  void setName();
  void printHeader(bool flag);
  
  /* row referencing operator */
  
  double& operator()(const long r, const long c);
  double operator()(const long r, const long c) const;

  void read(string filename);
  void write(string filename);


  
};




inline 
double fmatrix::operator()(const long r, const long c) const
{
  return data_[r * cols_ + c];
}


inline 
double& fmatrix::operator()(const long r, const long c)
{
  return data_[r * cols_ + c];
}




#endif


