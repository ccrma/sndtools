//-----------------------------------------------------------------------------
// name: dct.c
// desc: diskrete cosinus transform
//
// authors: yugoslavian authors (see below)
//   copied and pasted by amisra and gewang
//   modified: fct changed to dct
// date: today
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "dct.h"

#define invroot2 0.7071067814

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

/* fast DCT based on IEEE signal proc, 1992 #8, yugoslavian authors. */

static int N=0;
static int m=0;
static float two_over_N=0;
static float root2_over_rootN=0;
static float *C=NULL;

static void bitrev(float *f, int len)
{
  int i,j,m,halflen;
  float temp;

  if (len<=2) return; /* No action necessary if n=1 or n=2 */
  halflen = len>>1;
  j=1;
  for(i=1; i<=len; i++){
    if(i<j){
      temp=f[j-1];
      f[j-1]=f[i-1];
      f[i-1]=temp;
    }
    m = halflen;
    while(j>m){
      j=j-m;
      m=(m+1)>>1;
    }
    j=j+m;
  }
}

static void inv_sums(float *f)
{
  int ii,stepsize,stage,curptr,nthreads,thread,step,nsteps;

  for(stage=1; stage <=m-1; stage++){
    nthreads = 1<<(stage-1);
    stepsize = nthreads<<1;
    nsteps   = (1<<(m-stage)) - 1;
    for(thread=1; thread<=nthreads; thread++){
      curptr=N-thread; 
      for(step=1; step<=nsteps; step++){
        f[curptr] += f[curptr-stepsize];
        curptr -= stepsize; 
      }
    }
  }
}

static void fwd_sums(float *f)
{
  int ii,stepsize,stage,curptr,nthreads,thread,step,nsteps;

  for(stage=m-1; stage >=1; stage--){
    nthreads = 1<<(stage-1);
    stepsize = nthreads<<1;
    nsteps   = (1<<(m-stage)) - 1;
    for(thread=1; thread<=nthreads; thread++){
      curptr=nthreads +thread-1;
      for(step=1; step<=nsteps; step++){
	f[curptr] += f[curptr+stepsize];
	curptr += stepsize;
      }
    }
  }
}

static void scramble(float *f,int len){
  float temp;
  int i,ii1,ii2,halflen,qtrlen;

  halflen = len >> 1;
  qtrlen = halflen >> 1;
  bitrev(f,len);
  bitrev(&f[0], halflen);
  bitrev(&f[halflen], halflen);
  ii1=len-1;
  ii2=halflen;
  for(i=0; i<=qtrlen-1; i++){
    temp = f[ii1];
    f[ii1] = f[ii2];
    f[ii2] = temp;
    ii1--;
    ii2++;
  }
}

static void unscramble(float *f,int len)
{
  float temp;
  int i,ii1,ii2,halflen,qtrlen;

  halflen = len >> 1;
  qtrlen = halflen >> 1;
  ii1 = len-1;
  ii2 = halflen;
  for(i=0; i<=qtrlen-1; i++){
    temp = f[ii1];
    f[ii1] = f[ii2];
    f[ii2] = temp;
    ii1--;
    ii2++;
  }
  bitrev(&f[0], halflen);
  bitrev(&f[halflen], halflen);
  bitrev(f,len);
}

static void initcosarray(int length)
{
  int i,group,base,item,nitems,halfN;
  float factor;

  printf("FCT-- new N=%d\n",length);
  m = -1;
  do{
    m++;
    N = 1<<m;
    if (N>length){
      printf("ERROR in FCT-- length %d not a power of 2\n",length);
      exit(1);
    }
  }while(N<length);
  if(C != NULL) free(C);
  C = (float *)calloc(N,sizeof(float));
  if(C == NULL){
    printf("Unable to allocate C array\n");
    exit(1);
  }
  halfN=N/2;
  two_over_N = 2.0/(float)N;
  root2_over_rootN = sqrt(2.0/(float)N);
  for(i=0;i<=halfN-1;i++) C[halfN+i]=4*i+1;
  for(group=1;group<=m-1;group++){
    base= 1<<(group-1);
    nitems=base;
    factor = 1.0*(1<<(m-group));
    for(item=1; item<=nitems;item++) C[base+item-1]=factor*C[halfN+item-1];
  }

  //printf("before taking cos, C array =\n"); rarrwrt(C,N);
  for(i=1;i<=N-1;i++) C[i] = 1.0/(2.0*cos(C[i]*M_PI/(2.0*N)));
  //printf("After taking cos, Carray = \n"); rarrwrt(C,N);
}

static void inv_butterflies(float *f)
{
  int stage,ii1,ii2,butterfly,ngroups,group,wingspan,increment,baseptr;
  float Cfac,T;

  for(stage=1; stage<=m;stage++){
    ngroups=1<<(m-stage);
    wingspan=1<<(stage-1);
    increment=wingspan<<1;
    for(butterfly=1; butterfly<=wingspan; butterfly++){
      Cfac = C[wingspan+butterfly-1];
      baseptr=0;
      for(group=1; group<=ngroups; group++){
	ii1=baseptr+butterfly-1;
	ii2=ii1+wingspan;
	T=Cfac * f[ii2];
	f[ii2]=f[ii1]-T;
	f[ii1]=f[ii1]+T;
	baseptr += increment;
      }
    }
  }
}

static void fwd_butterflies(float *f)
{
  int stage,ii1,ii2,butterfly,ngroups,group,wingspan,increment,baseptr;
  float Cfac,T;

  for(stage=m; stage>=1;stage--){
    ngroups=1<<(m-stage);
    wingspan=1<<(stage-1);
    increment=wingspan<<1;
    for(butterfly=1; butterfly<=wingspan; butterfly++){
      Cfac = C[wingspan+butterfly-1];
      baseptr=0;
      for(group=1; group<=ngroups; group++){
	ii1=baseptr+butterfly-1;
	ii2=ii1+wingspan;
	T= f[ii2];
	f[ii2]=Cfac *(f[ii1]-T);
	f[ii1]=f[ii1]+T;
	baseptr += increment;
      }
    }
  }
}

static void ifct_noscale(float *f, int length)
{
  if (length != N) initcosarray(length);
  f[0] *= invroot2;
  inv_sums(f);
  bitrev(f,N);
  inv_butterflies(f);
  unscramble(f,N);
}

static void fct_noscale(float *f, int length)
{
  if (length != N) initcosarray(length);
  scramble(f,N);
  fwd_butterflies(f);
  bitrev(f,N);
  fwd_sums(f);
  f[0] *= invroot2; 
}

static void ifct_defn_scaling(float *f, int length){
  ifct_noscale(f,length);
}

static void fct_defn_scaling(float *f, int length){
  int i;

  fct_noscale(f,length);
  for(i=0;i<=N-1;i++) f[i] *= two_over_N;
}

void idct(float *f, int length){
/* CALL THIS FOR INVERSE 1D DCT DON-MONRO PREFERRED SCALING */
  int i;

  if (length != N) initcosarray(length);  /* BGS patch June 1997 */
  for(i=0;i<=N-1;i++) f[i] *= root2_over_rootN;
  ifct_noscale(f,length);
}

void dct(float *f, int length){
/* CALL THIS FOR FORWARD 1D DCT DON-MONRO PREFERRED SCALING */
  int i;

  fct_noscale(f,length);
  for(i=0;i<=N-1;i++) f[i] *= root2_over_rootN;
}
