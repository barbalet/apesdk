/****************************************************************

 audio.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

/*! \file   audio.c
 *  \brief  Handles audio input and output for the Noble Toolkit

 */

/*NOBLEMAKE DEL=""*/
#include "noble.h"
/*NOBLEMAKE END=""*/
/*NOBLEMAKE VAR=""*/
#include "math.h"
/*NOBLEMAKE END=""*/

typedef	double	n_double;
typedef	double	n_float;

#define TWO_PI (6.2831853071795864769252867665590057683943L)

static n_float frequency[AUDIO_FFT_MAX_BUFFER];
static n_float timedomain[AUDIO_FFT_MAX_BUFFER];
static n_float frequencyi[AUDIO_FFT_MAX_BUFFER];
static n_float timedomaini[AUDIO_FFT_MAX_BUFFER];

static n_uint	audio_reverse_bits (n_uint index, n_uint power_sample)
{
    n_uint i = 0;
    n_uint rev = 0;
    while(i < power_sample){
        rev = (rev << 1) | (index & 1);
        index >>= 1;
        i++;
    }
    return rev;
}

void audio_fft(n_byte inverse, n_uint power_sample)
{
    n_uint		NumSamples = 1 << power_sample;    /* Number of bits needed to store indices */
    n_uint		i;
    n_uint		BlockSize, BlockEnd;
    
    n_double   *d_in, *d_ini, *d_out, *d_outi;
    
    n_double angle_numerator = TWO_PI;
    
    if ( inverse )
    {
        angle_numerator = -angle_numerator;
        
        d_in  = frequency;
        d_ini = frequencyi;
        
        d_out  = timedomain;
        d_outi = timedomaini;
    }
    else
    {
        d_in  = timedomain;
        d_ini = timedomaini;
        
        d_out  = frequency;
        d_outi = frequencyi;
    }
    
    /*
     **   Do simultaneous data copy and bit-reversal ordering into outputs...
     */
    
	i=0;
	while(i < NumSamples){
		n_uint j = audio_reverse_bits (i, power_sample);
        d_outi[j] = d_in[i];
        d_out[j]  = d_ini[i];
        i++;
    }
    
    /*
     **   Do the FFT itself...
     */
    
    BlockEnd = 1;
    BlockSize = 2;
    while (BlockSize <= NumSamples)
    {
        n_double delta_angle = angle_numerator / (double)BlockSize;
        n_double sm2 = sin ( -2 * delta_angle );
        n_double sm1 = sin ( -delta_angle );
        n_double cm2 = cos ( -2 * delta_angle );
        n_double cm1 = cos ( -delta_angle );
        n_double w = 2 * cm1;
        
		i=0;
		
        while(  i < NumSamples )
        {
            n_uint j=i;
			n_uint n=0;
            
            n_double ar2 = cm2;
            n_double ar1 = cm1;
            
            n_double ai2 = sm2;
            n_double ai1 = sm1;
			
            while(n < BlockEnd)
            {
                n_double ar0 = w*ar1 - ar2;
                n_double ai0 = w*ai1 - ai2;
                
                ar2 = ar1;
                ar1 = ar0;
                
                ai2 = ai1;
                ai1 = ai0;
                
                {
                    n_uint   k = j + BlockEnd;
                    
                    n_double tr = ar0*d_outi[k] - ai0*d_out[k];
                    n_double ti = ar0*d_out[k] + ai0*d_outi[k];
                    
                    d_outi[k] = d_outi[j] - tr;
                    d_out[k] = d_out[j] - ti;
                    
                    d_outi[j] += tr;
                    d_out[j] += ti;
                }
                j++;
                n++;
            }
       		i += BlockSize;
       	}
        
        BlockEnd = BlockSize;
        BlockSize <<= 1;
        
    }
    
    /*
     **   Need to normalize if inverse transform...
     */
    
    if ( inverse )
    {
        n_double denom = (n_double) NumSamples;
        i = 0;
        while  (i < NumSamples)
        {
            d_outi[i] /= denom;
            d_out[i] /= denom;
            i++;
        }
    }
}

void audio_clear_buffers(n_uint length)
{
    n_uint     loop = 0;
    while (loop < length)
    {
        frequency[loop] = 0;
        timedomain[loop] = 0;
        frequencyi[loop] = 0;
        timedomaini[loop] = 0;
        loop++;
    }
}

void audio_clear_output(n_audio * audio, n_uint length)
{
    n_uint     loop = 0;
    while (loop < length)
    {
        audio[loop] = 0;
        loop++;
    }
}

void audio_equal_input(n_audio * audio, n_uint length)
{
    n_uint     loop = 0;
    while (loop < length)
    {
        timedomain[loop] = audio[loop];
        loop++;
    }
}

void audio_equal_output(n_audio * audio, n_uint length)
{
    n_uint     loop = 0;
    while (loop < length)
    {
        audio[loop] = timedomain[loop];
        loop++;
    }
}

void audio_multiply_output(n_audio * audio, n_uint length)
{
    n_uint     loop = 0;
    while (loop < length)
    {
        audio[loop] *= timedomain[loop];
        loop++;
    }
}

n_uint audio_power(n_audio * audio, n_uint length)
{
    n_uint   loop = 0;
    n_uint   output = 0;
    while  (loop < length)
    {
        n_int  audio_value = audio[loop];
        output += (audio_value * audio_value);
        loop++;
    }
    return output;
}

void audio_set_frequency(n_uint entry, n_uint value)
{
    frequency[entry] = value/1E+00;
}

