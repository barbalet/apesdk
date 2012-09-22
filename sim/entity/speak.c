/****************************************************************
 
 speak.c
 
 =============================================================
 
 Copyright 1996-2012 Tom Barbalet. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.
 
 ****************************************************************/

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"

#else

#include "..\noble\noble.h"
#include "..\universe\universe.h"

#endif

#include "entity_internal.h"
#include "entity.h"

/* working on the real solution */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef	short	n_audio;
typedef	double	n_double;


/* macros */
#define TWO_PI (6.2831853071795864769252867665590057683943L)

/* the sound lengths vary from:
 
 (8k)  0.185 sec to
 (16k) 0.371 sec to
 (32k) 0.743 sec
 
 these can contain sounds or pauses
 */
#define	MAX_BUFFER    (16384*4)
#define ACT_BUFFER    (16384)

/* FFT recursion */
static void fft_rec(n_int N, n_int offset, n_int delta,
            n_double *x,
            n_double *y,
            n_double *X,
            n_double *Y,
            n_double *XX,
            n_double *YY
            )
{
  n_int N2 = N/2;            /* half the number of points in FFT */
  n_int k;                   /* generic index */

  if(N != 2)  /* Perform recursive step. */
    {
      /* Calculate two (N/2)-point DFT's. */
      fft_rec(N2, offset,       2*delta, x, y, XX, YY, X, Y);
      fft_rec(N2, offset+delta, 2*delta, x, y, XX, YY, X, Y);

      /* Combine the two (N/2)-point DFT's into one N-point DFT. */
      for(k=0; k<N2; k++)
        {
          n_int    k00 = offset + k*delta;
          n_int    k01 = k00 + N2*delta;
          n_int    k10 = offset + 2*k*delta;
          n_int    k11 = k10 + delta;
          n_double cs = cos(TWO_PI*k/(n_double)N);
          n_double sn = sin(TWO_PI*k/(n_double)N);
          n_double tmp0 = cs * XX[k11] + sn * YY[k11];
          n_double tmp1 = cs * YY[k11] - sn * XX[k11];
            
          X[k01] = XX[k10] - tmp0;
          Y[k01] = YY[k10] - tmp1;
          X[k00] = XX[k10] + tmp0;
          Y[k00] = YY[k10] + tmp1;
        }
    }
  else  /* Perform 2-point DFT. */
    {
      n_int    k00 = offset; 
      n_int    k01 = k00 + delta;
      X[k01] = x[k00] - x[k01];
      Y[k01] = y[k00] - y[k01];
      X[k00] = x[k00] + x[k01];
      Y[k00] = y[k00] + y[k01];
    }
}


/* FFT */
static void fft(n_int N, n_double *y, n_double *X)
{
    n_int loop = 0;
    /* Declare a pointer to scratch space. */
    n_double *XX = malloc(N * sizeof(n_double));
    n_double *YY = malloc(N * sizeof(n_double));
    
    n_double *x = malloc(N * sizeof(n_double));
    n_double *Y = malloc(N * sizeof(n_double));
    
    while (loop < N)
    {
        x[loop] = Y[loop] = 0;
        loop++;
    }
    
    /* Calculate FFT by a recursion. */
    fft_rec(N, 0, 1, x, y, X, Y, XX, YY);
    
    /* Free memory. */
    free(x);
    free(Y);

    free(XX);
    free(YY);
}



/* IFFT */
static void ifft(n_int N, n_double *x, n_double *Y)
{
    n_int N2 = N/2;       /* half the number of points in IFFT */
    n_int i;              /* generic index */

    /* Calculate IFFT via reciprocity property of DFT. */
    fft(N, Y, x);
    x[0] = x[0]/N;
    x[N2] = x[N2]/N;

    for(i=1; i<N2; i++)
    {
        n_double tmp0 = x[i]/N;
        x[i] = x[N-i]/N;
        x[N-i] = tmp0;
    }
}

void speak_aiff_header(FILE *fptr, n_audio *samples, n_uint nsamples)
{

	n_uint totalsize;
	
	/* Write the form chunk */
	fprintf(fptr,"FORM");
	totalsize = 4 + 8 + 18 + 8 + 2 * nsamples + 8;
	fputc((totalsize & 0xff000000) >> 24,fptr);
	fputc((totalsize & 0x00ff0000) >> 16,fptr);
	fputc((totalsize & 0x0000ff00) >> 8,fptr);
	fputc((totalsize & 0x000000ff),fptr);
	fprintf(fptr,"AIFF");
	
	/* Write the common chunk */
	fprintf(fptr,"COMM");
	fputc(0,fptr);                               /* Size */
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(18,fptr);
	fputc(0,fptr);                               /* Channels = 1 */
	fputc(1,fptr);
	fputc((nsamples & 0xff000000) >> 24,fptr);   /* Samples */
	fputc((nsamples & 0x00ff0000) >> 16,fptr);
	fputc((nsamples & 0x0000ff00) >> 8,fptr);
	fputc((nsamples & 0x000000ff),fptr);
	fputc(0,fptr);                               /* Size = 16 */
	fputc(16,fptr);
	fputc(0x40,fptr);                            /* 10 byte sampee rate */
	fputc(0x0e,fptr);
	fputc(0xac,fptr);
	fputc(0x44,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	
	/* Write the sound data chunk */
	fprintf(fptr,"SSND");
	fputc((2*nsamples+8 & 0xff000000) >> 24,fptr);/* Size      */
	fputc((2*nsamples+8 & 0x00ff0000) >> 16,fptr);
	fputc((2*nsamples+8 & 0x0000ff00) >> 8,fptr);
	fputc((2*nsamples+8 & 0x000000ff),fptr);
	fputc(0,fptr);                                /* Offset    */
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);                                /* Block     */
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	
	fwrite(samples,nsamples,2,fptr);
}

static void speak_make(n_string filename, n_int length)
{
    FILE     *out_file = 0L;
    n_double *frequency  = malloc(length * sizeof(n_double));
    n_double *timedomain = malloc(length * sizeof(n_double));
    n_audio  *output      = malloc(length * sizeof(n_audio));
    n_int     loop = 0;
    n_int     division = MAX_BUFFER/length;

    out_file = fopen(filename,"w");

    if (out_file == 0L)
    {
        (void)SHOW_ERROR("Failed create speak file!");
        return;
    }

    while (loop < length)
    {
        frequency[loop] = 0;
        timedomain[loop] = 0;
        loop++;
    }

    frequency[400/division] = 200000/division;
    frequency[600/division] = 800000/division;
    frequency[900/division] = 800000/division;
    frequency[700/division] = 800000/division;
    frequency[450/division] = 200000/division;
    frequency[460/division] = 900000/division;
    frequency[490/division] = 900000/division;

    ifft(length, timedomain, frequency);

    loop = 0;
    while (loop < length)
    {
        output[loop] = timedomain[loop];
        loop++;
    }

    loop = 0;
    while (loop < length)
    {
        frequency[loop] = 0;
        timedomain[loop] = 0;
        loop++;
    }

    frequency[0] = 30000/division;
    frequency[1] = 30000/division;
    frequency[2] = 30000/division;
    frequency[3] = 30000/division;

    ifft(length, timedomain, frequency);

    free(frequency);

    loop = 0;
    while (loop < length)
    {
        output[loop] *= timedomain[loop];
        loop++;
    }

    free(timedomain);

    speak_aiff_header(out_file,output,length);

    free(output);

    fclose(out_file);
}

void speak_out(n_string filename)
{
    speak_make(filename, ACT_BUFFER);
}