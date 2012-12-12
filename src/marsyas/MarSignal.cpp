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
   \class Signal
   \brief Abstract class for all signals

  Abstract base class for all signals. A signal must be able to get
and put windows (vectors of float point samples (fvec)).  Various classes
inherit from the Signal object implementing specific cases like: 
SoundFileIO, real time Audio, Instrument , PCM waveform, etc.
  winSize, hopSize, skipSize and zeroSize define the parameters 
that characterize the windows used by the get and put methods. 

*/



#include "MarSignal.h"


Signal::Signal()
{
  srate_    = DEFAULT_SRATE;
  winSize_  = DEFAULT_WIN_SIZE;
  hopSize_  = DEFAULT_HOP_SIZE;
  skipSize_ = DEFAULT_SKIP_SIZE;
  zeroSize_ = DEFAULT_ZERO_SIZE;
  channels_ = DEFAULT_CHANNELS;
}

Signal::~Signal()
{
}


void 
Signal::initSignal(unsigned int srate, unsigned int channels)
{
  srate_ = srate;
  channels_ = channels;
}


unsigned int 
Signal::srate()
{
  return srate_;
}

unsigned int 
Signal::hopSize()
{
  return hopSize_;
}


unsigned int 
Signal::winSize()
{
  return winSize_;
}

unsigned int 
Signal::zeroSize()
{
  return zeroSize_;
}

void 
Signal::initWindow(unsigned int winSize) 
{
  initWindow(winSize, winSize, 0,0);
}

void 
Signal::initWindow(unsigned int winSize, unsigned int hopSize, 
		   unsigned int skipSize_, unsigned int zeroSize_)
{
  cerr << "Signal::initWindow" << endl;
  winSize_ = winSize;
  hopSize_ = hopSize;
  skipSize_ = skipSize_;
  zeroSize_ = zeroSize_;
  if (hopSize_ > winSize_) 
    {
      cerr << "Hop size must be <= window size. Setting hopSize to winSize" << endl;
      hopSize_ = winSize_;
    }
  if (zeroSize_ > winSize_)
    {
      cerr << "Zeropad size must be <= window size. Setting zeroSize to winSize" << endl;
      zeroSize_ = winSize_;
    }
  
      
}


void 
Signal::debug_info()
{
  cerr << "------- Debug Information for Signal object -----" << endl;
  cerr << "Type              = " << type_  << endl;
  cerr << "Sample rate       = " << srate_ << endl;
  cerr << "Size (in samples) = " << sizeSamples_ << endl;
  cerr << "Size (in seconds) = " << sizeSeconds_ << endl;
  cerr << "Id                = " << id_ << endl;
  cerr << "Comments          = " << comments_ << endl;
  cerr << "------- Window characteristics -----" << endl;
  cerr << "Window size       = " << winSize_ << endl;
  cerr << "Hop    size       = " << hopSize_ << endl;
  cerr << "Skip   size       = " << skipSize_ << endl;
  cerr << "Zero   size       = " << zeroSize_ << endl;
}


