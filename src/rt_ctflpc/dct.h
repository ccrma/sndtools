//-----------------------------------------------------------------------------
// name: dct.h
// desc: diskrete cosinus transform
//
// authors: yugoslavian authors (see c file)
//   copied and pasted by amisra and gewang
//   modified: fct changed to dct
// date: today
//-----------------------------------------------------------------------------
#ifndef __DCT_H__
#define __DCT_H__


// dct
void dct( float * buffer, int length );
// idct
void idct( float * buffer, int length );


#endif