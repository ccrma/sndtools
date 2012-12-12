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

#if !defined(__Signal_h)
#define __Signal_h

#include "fvec.h"
#include <string> 
using namespace std;
#include "defs.h"



class Signal
{
protected:
  unsigned int srate_;			// Sample rate
  unsigned int winSize_;		// Window size
  unsigned int hopSize_;		// Hop size for advancing the window
  unsigned int skipSize_;		// Initial skip of samples
  unsigned int zeroSize_;		// Zeropad size for the window 
					// (i.e 0 no zeropadding) 
  unsigned int channels_;
  unsigned int id_;			// Serial number 
  unsigned long sizeSamples_;		// Size in samples if applicable
  float  sizeSeconds_;			// Size in seconds if applicable
  
  string comments_;
  string type_;				// Type of sound object
  
  
  
public:
  Signal();
  virtual ~Signal();
  virtual void get(fvec& win, unsigned long winIndex) = 0;
  virtual void put(fvec& win, unsigned long winIndex) = 0;
  void debug_info();
  virtual void initWindow(unsigned int winSize, unsigned int hopSize, 
			  unsigned int skipSize_, unsigned int zeroSize_);
  void initWindow(unsigned int winSize);
  void initSignal(unsigned int srate, unsigned int channels);
  unsigned int winSize();
  unsigned int zeroSize();
  unsigned int hopSize();
  unsigned int srate();
  /* iterations is the number of windows that can be returned by subsequent 
     calls to the get method based on the winSize, hopSize....
     Some signals like real time audio input can not return 
     a specific number of iterations. 
  */

  virtual unsigned long iterations()=0; 
};






#endif
