#ifndef __PFFT_H__
#define __PFFT_H__

#include "chuck_fft.h"
#include <memory>
#include <stdlib.h>

#define SAMPLE float

struct peak {
	int bin;
	polar info;
};

struct pfft_data {
	peak * peaks;
	peak * last_peaks;
	int peaks_size;
	int last_peaks_size;
	int npeaks;
	int nlast_peaks;
	int * matching;
	int hop;
	int srate;
};

// create
pfft_data * pfft_create();

// destroy
void pfft_destroy( pfft_data * data );

// find npeaks and res out their bins
void pfft_analyze( pfft_data * data, polar * frame, int framesize, int npeaks, int lowbin, int highbin );

// synthesize
void pfft_synthesize( pfft_data * data, SAMPLE * buffer, int buffer_size, int fft_size, float freq_warp );

// match last_peaks to peaks in matching
void pfft_match( pfft_data * data, float error );

#endif