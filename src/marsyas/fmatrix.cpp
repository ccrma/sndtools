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
    \brief Matrix of double values

    Matrix of doubleing point values. Similar to fvec. Basic 
arithmetic operations and statistics are supported. 
*/


#include "fmatrix.h"

fmatrix::fmatrix(): data_(NULL), size_(0), rows_(0), cols_(0), name_("m"), printHeader_(true)
{
}

fmatrix::~fmatrix()
{
  delete data_;
}

fmatrix::fmatrix(unsigned int rows): data_(new double[rows * rows]), size_(rows*rows), rows_(rows), cols_(rows), name_("m"), printHeader_(true)
{
  row_.create(cols_);
  col_.create(rows_);
  meanRow_.create(cols_);
  stdRow_.create(cols_);
  varRow_.create(cols_);
}


fmatrix::fmatrix(unsigned int rows, unsigned int cols): data_(new double[rows * cols]), size_(rows*cols), rows_(rows), cols_(cols), name_("m"), printHeader_(true)
{
  row_.create(cols_);
  col_.create(rows_);  
  meanRow_.create(cols_);
  stdRow_.create(cols_);
  varRow_.create(cols_);
}


fmatrix::fmatrix(const fmatrix& a):data_(new double[a.size_]), size_(a.size_), rows_(a.rows_), cols_(a.cols_), name_(a.name_), printHeader_(a.printHeader_)
{
  for (unsigned int i=0; i<size_; i++)
    data_[i] = a.data_[i];
  row_.create(cols_);
  col_.create(rows_);  
  meanRow_.create(cols_);
  stdRow_.create(cols_);
  varRow_.create(cols_);
} 

					 



fmatrix&
fmatrix::operator=(const fmatrix& a)
{
  if (this != &a)
    {
      if ((rows_ != a.rows_)|| (cols_ != a.cols_))
	{
	  cerr << "fmatrix::operator=: Different fmatrix dimensions\n" << endl;
	  cerr << "fmatrix left unchanged\n";
	  cerr << "Left rows = " << rows_ << endl;
	  cerr << "Left cols = " << cols_ << endl;
	  cerr << "Right rows = " << rows_ << endl;
	  cerr << "Right cols = " << cols_ << endl;
	  return *this;
	}
      for (unsigned int i=0; i< size_; i++)
	{
	  data_[i] = a.data_[i];
	}
      name_ = a.name_;
      printHeader_ = a.printHeader_;
    }
  return *this;
}


void 
fmatrix::create(unsigned int rows)
{
  rows_ = rows;
  cols_ = rows;
  size_ = rows_ * cols_;
  data_ = new double[size_];
  for (unsigned int i=0; i<size_; i++)
    data_[i] = 0.0;
  row_.create(cols_);
  col_.create(rows_);
  meanRow_.create(cols_);
  stdRow_.create(cols_);  
  varRow_.create(cols_);
}

void 
fmatrix::create(unsigned int rows, unsigned int cols)
{
  unsigned int i;
  rows_ = rows;
  cols_ = cols;
  size_ = rows_ * cols_;
  
  data_ = new double[size_];
  for (i=0; i<size_; i++)
    {
      data_[i] = 0.0;
    }
  row_.create(cols_);
  col_.create(rows_);
  meanRow_.create(cols_);
  stdRow_.create(cols_);  
  varRow_.create(cols_);
}


void 
fmatrix::setval(double val)
{
  unsigned int r,c;
  for (r=0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      data_[r*cols_ + c] = val;
  
}

unsigned int 
fmatrix::rows()
{
  return rows_;
}

unsigned int 
fmatrix::cols()
{
  return cols_;
}


void 
fmatrix::col(const unsigned int c, fvec& vec)
{
  cerr << "fmatrix::col c = " << c << endl;
  if (c >= cols_)
    {
      cerr <<"fmatrix::col Column out of range\n" << endl;
      return;
    }
  if (vec.size() != rows_)
    {
      cerr << "fmatrix::col Incompatible vector size\n" << endl;
      return;
    }
  
  for (unsigned int r=0; r<rows_; r++)
    {
      data_[r*cols_ +c] = vec(r);
    }
}

void fmatrix::standarize(fvec mrow, fvec srow)
{
  unsigned int r,c;
  for (r=0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      data_[r*cols_ + c] = (data_[r*cols_ +c] - mrow(c))/srow(c);  
}

void fmatrix::flround()
{
  unsigned int r,c;
  for (r=0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      data_[r*cols_ + c] = (int)(data_[r*cols_ +c]);
}


void fmatrix::clip(double min, double max)
{
  unsigned int r,c;
  for (r=0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      {
	if (data_[r*cols_ + c] < min) 
	  data_[r*cols_ +c] = min;
	if (data_[r*cols_ + c] > max) 
	  data_[r*cols_ +c] = max;
      }
}



fmatrix
fmatrix::correlation()
{
  fmatrix temp(rows_, cols_);
  unsigned int r,c;
  fvec mrow = meanRow();     // Row with means of each column
  fvec srow = stdRow();      // Row with standard deviations of each column
  
  // copy to temporary and standarize data
  // standarize (subtract mean, divide by standard deviation)

  for (r=0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      temp(r,c) = (data_[r*cols_ +c] - mrow(c))/srow(c);
  
  unsigned int c1,c2;
  fmatrix res(cols_, cols_);
  double sum = 0.0;
  for (c1=0; c1< cols_; c1++)
    {
      for (c2=0; c2 < cols_; c2++)
	{
	  sum = 0.0;
	  for (r=0; r < rows_; r++)
	    sum += (temp(r,c1) * temp(r,c2));
	  sum /= rows_;
	  res(c1,c2) = sum;
	}
    }
  return res;
}


fmatrix 
fmatrix::covariance()
{
  unsigned int c1,c2, r;
  fmatrix res(cols_, cols_);
  double sum = 0.0;
  for (c1=0; c1< cols_; c1++)
    {
      for (c2=0; c2 < cols_; c2++)
	{
	  sum = 0.0;
	  for (r=0; r < rows_; r++)
	    sum += (data_[r * cols_ + c1] * data_[r * cols_ + c2]);
	  sum /= rows_;
	  res(c1,c2) = sum;
	}
    }
  return res;
}

int 
fmatrix::invert(fmatrix& res)
{
  int rank;
  assert(rows_ == cols_);
  unsigned int r,c,i;
  double temp;
  
  rank = 0;
  for (r = 0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      {
	if (r == c) 
	  res(r,c) = 1.0;
	else 
	  res(r,c) = 0.0;
      }
  for (i = 0; i < rows_; i++)
    {
      if ((*this)(i,i) == 0)
	{
	  for (r = i; r < rows_; r++)
	    for (c = 0; c < cols_; c++)
	      {
		(*this)(i,c) += (*this)(r,c);
		res(i,c) += res(r,c);
	      }
	}
      for (r = i; r < rows_; r++) 
	{
	  temp = (*this)(r,i);
	  if (temp != 0) 
	    for (c =0; c < cols_; c++)
	      {
		(*this)(r,c) /= temp;
		res(r,c) /= temp;
	      }
	}
      if (i != rows_-1)
	{
	  for (r = i+1; r < rows_; r++)
	    {
	      temp = (*this)(r,i);
	      if (temp != 0.0) 
		for (c=0; c < cols_; c++)
		  {
		    (*this)(r,c) -= (*this)(i,c);
		    res(r,c) -= res(i,c);
		  }
	    }
	}
    }
  for (i=1; i < rows_; i++)
    for (r=0; r < i; r++)
      {
	temp = (*this)(r,i);
	for (c=0; c < cols_; c++)
	  {
	    (*this)(r,c) -= (temp * (*this)(i,c));
	    res(r,c) -= (temp * res(i,c));
	  }
      }
  for (r =0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      (*this)(r,c) = res(r,c);
  return rank;
}



fvec
fmatrix::meanRow()
{
  meanRow_.setval(0.0);
  for (unsigned int r=0; r<rows_; r++)
    {
      for (unsigned int c=0; c<cols_; c++)
	{
	  meanRow_(c) += data_[r*cols_ +c];
	}
    }
  meanRow_ /= rows_;
  return meanRow_;
}

fvec
fmatrix::stdRow()
{
  stdRow_.setval(0.0);
  for (unsigned int c = 0; c < cols_; c++) {
    col_ = col(c);
    stdRow_(c) = col_.std();
  }
  return stdRow_;
  
  /* 
  unsigned int c;
  meanRow_.setval(0.0);
  stdRow_.setval(0.0);
  double val;
  for (unsigned int r=0; r<rows_; r++)
    {
      for (unsigned int c=0; c<cols_; c++)
	{
	  val = data_[r * cols_ + c];
	  meanRow_(c) += val;
	  stdRow_(c) += val * val;
	}
    }

  for (c =0; c < cols_; c++)
    {
      if (stdRow_(c) != 0.0)
	stdRow_(c) /= rows_;
	if (meanRow_(c) != 0.0)
	meanRow_(c) /= rows_;  
    }


  
  meanRow_.sqr();
  stdRow_ -= meanRow_;  
  
  for (c =0; c < cols_; c++)
    {
      if (stdRow_(c) < 0.0)
	stdRow_(c) = 0.0;
    }
  return stdRow_;
  */
}


fvec
fmatrix::varRow()
{
  meanRow_.setval(0.0);
  varRow_.setval(0.0);
  double val;
  for (unsigned int r=0; r<rows_; r++)
    {
      for (unsigned int c=0; c<cols_; c++)
	{
	  val = data_[r * cols_ + c];
	  meanRow_(c) += val;
	  varRow_(c) += val * val;
	}
    }
  varRow_ /= rows_;
  meanRow_ /= rows_;  
  meanRow_.sqr();
  varRow_ -= meanRow_;  
  return varRow_;
}


void 
fmatrix::row(const unsigned int r, fvec& vec)
{
  if (r >= rows_)
    {
      cerr <<"fmatrix::row(const int r, fvec& vec)  Row out of range ( " << r << ") \n" << endl;
      return;
    }
  if (vec.size() != cols_)
    {
      cerr << "fmatrix::row Incompatible vector size\n" << endl;
      return;
    }
  for (unsigned int c=0; c<cols_; c++)
    {
      data_[r*cols_ +c] = vec(c);
    }
}


fmatrix& fmatrix::operator+=(const double val)
{
  unsigned int r, c;
  
  
  for (r=0; r < rows_; r++)
    {
      for (c=0; c < cols_; c++)
	{
	  data_[r * cols_ + c] += val;
	}
    }
  return *this;
}


fmatrix& fmatrix::operator*=(const double val)
{
  unsigned int r, c;
  
  
  for (r=0; r < rows_; r++)
    {
      for (c=0; c < cols_; c++)
	{
	  data_[r * cols_ + c] *= val;
	}
    }
  return *this;
}



fmatrix& fmatrix::operator/=(const double val)
{
  unsigned int r, c;
  
  
  for (r=0; r < rows_; r++)
    {
      for (c=0; c < cols_; c++)
	{
	  data_[r * cols_ + c] /= val;
	}
    }
  return *this;
}



fmatrix& fmatrix::operator+=(const fmatrix& matrix)
{
  unsigned int r, c;
  
  if ((matrix.rows_ != cols_)||(matrix.cols_ != cols_))
    {
      cerr << "fmatrix::operator+=: Wrong # of columns" << endl;
      cerr << "fmatrix left unchanged" << endl;
      return *this;
    }
  for (r=0; r < rows_; r++)
    {
      for (c=0; c < cols_; c++)
	{
	  data_[r * cols_ + c] += matrix(r,c);
	}
    }
  return *this;
}


fmatrix& fmatrix::operator*=(const fmatrix& matrix)
{
  unsigned int r, c, j;
  double sum;
  fmatrix temp(rows_, cols_);
  for (r=0; r < rows_; r++)
    for (c=0; c < cols_; c++)
      temp(r,c) = data_[r * cols_ + c];
  
  if ((matrix.rows_ != cols_)||(matrix.cols_ != cols_))
    {
      cerr << "fmatrix::operator*=: Wrong # of columns" << endl;
      cerr << "fmatrix left unchanged" << endl;
      return *this;
    }
  for (r=0; r < rows_; r++)
    {
      for (c=0; c < cols_; c++)
	{
	  sum = 0.0;
	  for (j=0; j < cols_; j++)
	    sum += ((temp(r ,j))  * matrix(j,c));
	  data_[r*cols_ + c] = sum;
	}
    }
  return *this;
}




fvec
fmatrix::col(const unsigned int c) 
{
  if (c >= cols_)
    {
      cerr <<"fmatrix::col Column out of range\n" << endl;
      col_.setval(0.0);
      return col_;
    }

  for (unsigned int r=0; r<rows_; r++)
    {
      col_(r) = data_[r*cols_ +c];
    }
  return col_;
}

fvec
fmatrix::row(const unsigned int r) 
{
  if (r >= rows_)
    {
      cerr << "fmatrix::row(const long r) Row out of range (" << r << ") rows_ =  " << rows_ << "\n" << endl;
      row_.setval(0.0);
      return row_;
    }      
  for (unsigned long c=0; c<cols_; c++)
    {
      row_(c) = data_[r*cols_ +c];
    }
  return row_;
}



void 
fmatrix::read(string filename)
{
  ifstream from(filename.c_str());
  from >> (*this);
} 






istream& operator>> (istream& is, fmatrix& fm)
{
  unsigned int r,c;
  unsigned int rows, cols;
  string str0, str1, str2;
  is >> str0 >> str1;
  
  if ((str0 != "#") || (str1 != "name:"))
    {
      cerr << "fmatrix:::operator>>: Problem reading fmatrix object from istream" << endl;
      cerr << "str0 = " << str0 << endl;
      fm.create(0,0);
      return is;
    }
  

  is >> fm.name_;
  
  
  is >> str0 >> str1 >> str2;
  is >> str0 >> str1;
  is >> rows;
  is >> str0 >> str1;
  is >> cols;
  
  fm.create(rows, cols);
  for (r=0; r< rows; r++)
    {
      for (c=0; c < cols; c++)
	{
	  is >> fm(r,c);
	}
    }
  return is;
}


void 
fmatrix::printHeader(bool flag)
{
  printHeader_ = flag;
}



ostream& operator<< (ostream& o, const fmatrix& m)
{
  int rows, cols;
  rows = m.rows_;
  cols = m.cols_;
  int r,c;
  if (m.printHeader_)
    {
      o << "# name: " << m.name_ << endl;
      o << "# type: matrix" << endl;
      o << "# rows: " << rows << endl;
      o << "# columns: " << cols << endl << endl;
    }
      
  for (r=0; r < rows; r++)
    {
      for (c=0; c < cols; c++)
	{
	  o << m(r,c) << "\t";
	}
      o << endl;
    }
  return o;
}
 
