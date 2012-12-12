#include "pfft.h"

/*********************
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
*********************/

#define NULL 0

// create
pfft_data * pfft_create()
{
	pfft_data * data = new pfft_data();
	data->peaks = NULL;
	data->last_peaks = NULL;
	data->peaks_size = data->last_peaks_size = 0;
	data->npeaks = data->nlast_peaks = 0;
	data->matching = NULL;
	data->hop = 0;
	data->srate = 0;
	return data;
}

// destroy
void pfft_destroy( pfft_data * data ) {
	if( data ) {
		if( data->peaks )
			delete [] data->peaks;
		if( data->last_peaks )
			delete [] data->last_peaks;
		if( data->matching )
			delete [] data->matching;
		delete [] data;
	}
}


// find npeaks and res out their bins
void pfft_analyze( pfft_data * data, polar * frame, int framesize, int npeaks, 
				  int lowbin, int highbin ) 
{
	// bookkeeping
	if( data->peaks_size > data->last_peaks_size ) {
		// last peaks
		delete [] data->last_peaks;
		data->last_peaks = new peak[data->peaks_size];
		data->last_peaks_size = data->peaks_size;
	}
	memcpy( data->last_peaks, data->peaks, data->last_peaks_size * sizeof(peak) );
	if(	npeaks > data->peaks_size ) {
		// peaks
		delete [] data->peaks;
		data->peaks = new peak[npeaks];
		data->peaks_size = npeaks;
		// matching
		delete [] data->matching;
		data->matching = new int[npeaks];
	}	
	memset( data->peaks, 0, data->peaks_size * sizeof(peak) );
	data->nlast_peaks = data->npeaks;
	data->npeaks = 0;

	// arbitrary dc blocking
	for( int s = 0; s < 5; s++ )
		frame[s].modulus = 0;

	// reset bin ranges
	if( lowbin < 0 ) lowbin = 0;
	if( lowbin >= framesize ) lowbin = framesize - 1;
	if( highbin <= 0 ) highbin = 1;
	if( highbin > framesize ) highbin = framesize;
	if( lowbin > highbin ) { // swap
		highbin = highbin + lowbin;
		lowbin = highbin - lowbin;
		highbin = highbin - lowbin;
	}
	//fprintf( stderr, "[pfft] bin ranges are now %d to %d\n", lowbin, highbin );

	// find top n
	double pi = 4.0 * atan( 1.0 );
	for( int n = 0; n < npeaks; n++ ) {
		// find max
		int max = 0;
		for( int s = lowbin; s < highbin; s++ ) {
			if( frame[s].modulus > frame[max].modulus )
				max = s;
		}
		polar maxinfo = frame[max]; 
		// res out bins
		int lo, hi;
		for( lo = max; lo > 0 && frame[lo].modulus > frame[lo-1].modulus; lo-- );
		for( hi = max; hi < framesize-1 && frame[hi].modulus > frame[hi+1].modulus; hi++ );
		if( max == lo || max == hi) { // boundary points
			frame[max].modulus = 0.0;
			frame[max].phase = rand() * 2 * pi;
		}
		float dy = frame[hi].modulus - frame[lo].modulus;
		float dx = hi - lo;
		for( int i = lo+1; i < hi; i++ ) {
			frame[i].modulus = frame[i-1].modulus + dy/dx;
			frame[i].phase = rand() * 2 * pi;
		}	
		// add peak if "unique"
		if( lo != hi ) {
			data->peaks[n].bin = max;
			data->peaks[n].info = maxinfo;
			data->npeaks++;
		}
		else 
			break;
	}
}

// synthesize
void pfft_synthesize( pfft_data * data, SAMPLE * buffer, int buffer_size, int fft_size, float freq_warp ) 
{
	// start with simple synthesis
//	double pi = 4.0 * atan( 1.0 );
//	memset(buffer, 0, buffer_size * sizeof(SAMPLE));
//	for( int i = 0; i < buffer_size; i++ ) {	
//		for( int n = 0; n < data->nlast_peaks; n++ ) {
//			float freq = data->last_peaks[n].bin /** data->srate*/ * 1.0 / fft_size;
//	//		buffer[i] += sin(i * 2 * pi * freq /*/ data->srate*/);
//			data->last_peaks[n].info.phase += (2 * pi * freq /*/ data->srate*/); 
//			buffer[i] += data->last_peaks[n].info.modulus * sin(data->last_peaks[n].info.phase);
//		}
//	} 
	// more complex
	double pi = 4.0 * atan( 1.0 );
	memset( buffer, 0, buffer_size * sizeof(SAMPLE) );
	// start-up new peaks now for smoothness
	for( int k = 0; k < data->npeaks; k++ ) {
		// first apply freq_warp
		data->peaks[k].bin *= freq_warp;
		// will this be a new peak in the next frame?
		bool newpeak = true;
		for( int n = 0; n < data->nlast_peaks; n++ ) {
			if( data->matching[n] == k ) {
				newpeak = false;
				break;
			}
		}
		if( !newpeak )
			continue;
		// if it is new, start it now
		float freq = data->peaks[k].bin * 1.0 / fft_size;
		float mag = 0;
		float deltamag = data->peaks[k].info.modulus / buffer_size;
		data->peaks[k].info.phase -= (2 * pi * freq * buffer_size);
		for( int i = 0; i < buffer_size; i++ ) {
			buffer[i] += mag * sin(data->peaks[k].info.phase);
			mag += deltamag;
			data->peaks[k].info.phase += (2 * pi * freq);	
		}
	}
	// continuing peaks
	for( int n = 0; n < data->nlast_peaks; n++ ) {
		float freq = data->last_peaks[n].bin * 1.0 / fft_size;
		float mag = data->last_peaks[n].info.modulus;
		float deltafreq = 0;
		float deltamag = mag / buffer_size;
		if( data->matching && data->matching[n] != -1 ) {
			deltafreq = (data->peaks[data->matching[n]].bin - data->last_peaks[n].bin) * 1.0 / fft_size;
			deltafreq = deltafreq / buffer_size;
			deltamag = (data->peaks[data->matching[n]].info.modulus - mag) / buffer_size;
		}
		for( int i = 0; i < buffer_size; i++ ) {
			buffer[i] += mag * sin(data->last_peaks[n].info.phase);
			freq += deltafreq;
			mag += deltamag;
			data->last_peaks[n].info.phase += (2 * pi * freq);
			if(data->last_peaks[n].info.phase > 2 * pi)
				data->last_peaks[n].info.phase -= 2 * pi;
		}
		if( data->matching && data->matching[n] != -1 ) {
			if( data->peaks[data->matching[n]].bin * 1.0 / fft_size != freq ) 
				fprintf( stderr, "%f, %f, %f\n", data->last_peaks[n].bin * 1.0 / fft_size, 
					data->peaks[data->matching[n]].bin * 1.0 / fft_size, freq );
		}
	}
}

// match last_peaks to peaks in matching
void pfft_match( pfft_data * data, float error ) 
{
	// assume matching is always as big as peaks_size	
	memset(data->matching, -1, data->last_peaks_size * sizeof(int));
	// nothing to match?
	if( data->nlast_peaks <= 0 || data->npeaks == 0 ) 
		return;
	// match
	for( int i = 0; i < data->nlast_peaks; i++ ) {
		float mindist = error * (data->last_peaks[i].bin + 1);
		for( int j = 0; j < data->npeaks; j++ ) {
			float dist = abs(data->last_peaks[i].bin - data->peaks[j].bin);
			if( dist < mindist || j == 0 ) {
				dist = mindist;
				data->matching[i] = j;
			}
		}
	}
}