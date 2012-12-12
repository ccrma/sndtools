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



#include "fvec.h"


fvec::fvec()
{
  size_ = 0;
  data_ = NULL;
  name_ = "v";
}


fvec::~fvec()
{
  delete data_;
}

fvec::fvec(unsigned int size)
{
  size_ = size;
  data_ = new float[size_];
  name_ = "v";
}

fvec::fvec(const fvec& a):data_(new float[a.size_]), size_(a.size_), name_(a.name_)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] = a.data_[i];
}

      


fvec& 
fvec::operator=(const fvec& a)
{
  if (this != &a)
    {
      if (size_ != a.size_)
	{
	  cerr << "fvec::operator= : Different fvec sizes\n" << endl;
	  cerr << "fvec left unchanged\n";
	  cerr << "Left size = " << size_ << endl;
	  cerr << "Right size = " << a.size_ << endl;
	  return *this;
	}
      for (unsigned int i=0; i < size_; i++)
	data_[i] = a.data_[i];
      name_ = a.name_;
    }
  return *this;
}



float *
fvec::getData()
{
  return data_;
}

float 
fvec::mean()
{
  float sum = 0.0;
  for (unsigned int i=0; i < size_; i++)
    {
      sum += data_[i];
    }
  if (sum != 0.0) sum /= size_;
  return sum;
}

float 
fvec::var()
{
  float sum = 0.0;
  float sum_sq = 0.0;
  float val;
  float var;
  
  for (unsigned int i=0; i < size_; i++)
    {
      val = data_[i];
      sum += val;
      sum_sq += (val * val);
    }
  if (sum != 0.0) sum /= size_;
  if (sum_sq != 0.0) sum_sq /= size_;
  
  var = sum_sq - sum * sum;
  return var;
}



float 
fvec::std()
{
  return sqrt(var());
}

  

unsigned int
fvec::size()
{
  return size_;
}

void 
fvec::debug_info()
{
  cerr << "fvec information" << endl;
  cerr << "size = " << size_ << endl;
  cerr << "****************" << endl;
  for (unsigned int i=0; i<size_; i++)
    cerr << data_[i] << endl;
  cerr << "****************" << endl;
}


void 
fvec::create(unsigned long size)
{
  size_ = size;
  data_ = new float[size_];
  for (unsigned long i=0; i<size_; i++)
    data_[i] = 0.0;
}


void 
fvec::create(float val, unsigned long size)
{
  size_ = size;
  data_ = new float[size_];
  for (unsigned long i=0; i<size_; i++)
    data_[i] = val;
}



void 
fvec::allocate(unsigned long size)
{
  size_ = size;
  data_ = new float[size_];
}

void 
fvec::setName(string name)
{
  name_ = name;
}


void 
fvec::setval(unsigned int start, unsigned int end, float val)
{
  assert(start <= size_);
  assert(end <= size_);
  for (unsigned int i=start; i<end; i++)
    {
      data_[i] = val;
    }
}


void 
fvec::setval(float val)
{
  for (unsigned int i=0; i<size_; i++)
    {
      data_[i] = val;
    }
}

void 
fvec::abs()
{
  for (unsigned int i=0; i<size_; i++)
    {
      data_[i] = fabs(data_[i]);
    }
  
}

void 
fvec::renorm(float old_mean, float old_std, float new_mean, float new_std)
{
  unsigned int i;
  
  for(i=0; i < size_; i++)
    {
      data_[i] = (data_[i] - old_mean) / old_std;
      data_[i] *= new_std;
      data_[i] += new_mean;
    }
}


void
fvec::sqr()
{
  for (unsigned int i=0; i<size_; i++)
    {
      data_[i] *= data_[i];
    }
}


void
fvec::sqroot()
{
  for (unsigned int i=0; i<size_; i++)
    {
      data_[i] = sqrt(data_[i]);
    }
}


fvec 
fvec::plus(const fvec& vec1, const fvec& vec2)
{
  unsigned int size;
  unsigned int i;
  if (vec1.size_ != vec2.size_)
    cerr << "Size of fvecs does not match" << endl;
  if (vec1.size_ >= vec2.size_)
    size = vec1.size_;
  else 
    size = vec2.size_;
  fvec sum;
  sum.create(size);    
  
  for (i=0; i<vec1.size_; i++)
    {
      sum.data_[i] = vec1.data_[i];
    }
  for (i=0; i<vec2.size_; i++)
    {
      sum.data_[i] += vec2.data_[i];
    }
      
  return sum;
}


fvec 
fvec::minus(const fvec& vec1, const fvec& vec2)
{
  unsigned int size;
  unsigned int i;
  if (vec1.size_ != vec2.size_)
    cerr << "Size of fvecs does not match" << endl;
  if (vec1.size_ >= vec2.size_)
    size = vec1.size_;
  else 
    size = vec2.size_;
  fvec diff;
  diff.create(size);    
  
  for (i=0; i<vec1.size_; i++)
    {
      diff.data_[i] = vec1.data_[i];
    }
  for (i=0; i<vec2.size_; i++)
    {
      diff.data_[i] -= vec2.data_[i];
    }
      
  return diff;
}


void 
fvec::send(Communicator *com)
{ 
  unsigned int i;
  static char *buf = new char[256];
  string message;
  sprintf(buf, "%i\n", size_);
  message = buf;
  com->send_message(message);
  for (i=0; i<size_; i++)
    {
      sprintf(buf, "%f\n", data_[i]);
      message = buf;
      com->send_message(message);
    }
  cerr << "fvec::send " << size_ << " numbers were sent to the client\n";
}

void
fvec::read(string filename)
{
  ifstream from(filename.c_str());
  from >> (*this);  
}


void 
fvec::write(string filename)
{
  ofstream os(filename.c_str());
  os << (*this) << endl;
}



ostream& 
operator<< (ostream& o, const fvec& vec)
{
  o << "# MARSYAS fvec" << endl;
  o << "# Size = " << vec.size_ << endl << endl;
  o << endl;

  
  o << "# name: " << vec.name_ << endl;
  o << "# type: matrix" << endl;
  o << "# rows: " << vec.size_ << endl;
  o << "# columns: " << 1 << endl;


  for (unsigned int i=0; i<vec.size_; i++)
    o << vec.data_[i] << endl;
  o << endl;
  o << "# Size = " << vec.size_ << endl;
  o << "# MARSYAS fvec" << endl;  
  return o;
}

istream& 
operator>>(istream& is, fvec& vec)
{
  if (vec.size_ != 0)
    {
      cerr << "fvec::operator>>: fvec already allocated cannot read from istream\n" << endl;
      return is;
    }
  string str0,str1,str2;
  unsigned int size;
  unsigned int i;
  
  is >> str0;
  is >> str1;
  is >> str2;
  
  if ((str0 != "#") || (str1 != "MARSYAS") || (str2 != "fvec"))
    {
      cerr << "fvec::operator>>: Problem reading fvec object from istream" << endl;
      return is;
    }
  is >> str0;				
  is >> str1;
  is >> str2;
  if ((str0 != "#") || (str1 != "Size") || (str2 != "="))
    {
      cerr << "fvec::operator>>: Problem reading fvec object from istream" << endl;
      return is;
    }
  is >> size;
  
  for (i=0; i<12; i++)
    {
      is >> str0;
    }
  
  
  vec.allocate(size);
  for (i=0; i < size; i++)
    {
      is >> vec.data_[i];
    }
  is >> str0;				
  is >> str1;
  is >> str2;
  if ((str0 != "#") || (str1 != "Size") || (str2 != "="))
    {
      cerr << "fvec::operator>>: Problem reading fvec object from istream" << endl;
      return is;
    }
  is >> size;
  
  is >> str0;
  is >> str1;
  is >> str2;
  
  if ((str0 != "#") || (str1 != "MARSYAS") || (str2 != "fvec"))
    {
      cerr << "fvec::operator>>: Problem reading fvec object from istream" << endl;
      return is;
    }
  return is;
}
