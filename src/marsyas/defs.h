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

/* Written by George Tzanetakis, July 2000 */ 


#if HAVE_CONFIG_H
#include <config.h>
#endif


#if !defined(__Defs_h)
#define __Defs_h

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif


#define MRS_OK 1
#define MRS_ERROR -1


#ifndef M_PI
#define M_PI 3.14159265f 
#endif

#define MAXSHRT 32768 
#define FMAXSHRT 32768.0f
#ifndef PI
#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#endif
#define SQRT_TWO_PI 2.5066283f
#define ONE_OVER_TWO_PI 0.15915494309f
#define MAX_LINE_SIZE 256

#define MRS_SF_READ  0 
#define MRS_SF_WRITE 1 


#define MRS_AUDIO_PLAY 0
#define MRS_AUDIO_REC  1

#define FFT_FORWARD 1
#define FFT_INVERSE 0

#define WVL_FORWARD 1
#define WVL_INVERSE 0

#define MAX_ORDER 100
#define DEFAULT_ORDER 30

#define DEFAULT_SRATE      22050
#define DEFAULT_WIN_SIZE   512
#define DEFAULT_HOP_SIZE   512
#define DEFAULT_SKIP_SIZE  0
#define DEFAULT_ZERO_SIZE  0
#define DEFAULT_CHANNELS   1 
#define DEFAULT_MEM_SIZE   40

#define PEAK_FREQ2PITCH 1
#define PEAK_FREQ2BPM   2

#define UNFOLDED 0
#define FOLDED 1

#define ONE_OVER_RANDLIMIT 9.313225e-10f


#ifdef WIN32

#ifndef EOF
#define EOF (-1)
#endif

#define MARSYAS_MFDIR "../../mf"
#define MARSYAS_VERSION "0.1"
#define MAXHOSTNAMELEN 256

typedef int socklen_t;
#endif

 
#endif
	
