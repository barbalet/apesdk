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
typedef	double	n_float;

/* macros */
#define TWO_PI (6.2831853071795864769252867665590057683943L)

/* the sound lengths vary from:
 
 (8k)  0.185 sec to
 (16k) 0.371 sec to
 (32k) 0.743 sec
 
 these can contain sounds or pauses
 */

#define ACT_BITS      (14)
#define ACT_BUFFER    (1<<ACT_BITS)
#define	MAX_BUFFER    (ACT_BUFFER*4)

static n_float frequency[MAX_BUFFER];
static n_float timedomain[MAX_BUFFER];
static n_float frequencyi[MAX_BUFFER];
static n_float timedomaini[MAX_BUFFER];

n_audio  output[MAX_BUFFER];;

n_uint	ReverseBits (n_uint index, n_uint power_sample)
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

void fft_float (n_byte inverse, n_float * RealIn, n_float * ImagIn, n_float * RealOut, n_float * ImagOut, n_uint power_sample)
{
    n_uint		NumSamples = 1 << power_sample;    /* Number of bits needed to store indices */
    n_uint		i, j, k, n;
    n_uint		BlockSize, BlockEnd;
    
    n_double angle_numerator = TWO_PI;
    n_double tr, ti;
    n_double ar0, ar1, ar2, ai0, ai1, ai2;
    
    
    if ( inverse )
        angle_numerator = -angle_numerator;
    
    /*
     **   Do simultaneous data copy and bit-reversal ordering into outputs...
     */
    
	i=0;
	while(i < NumSamples){
		j = ReverseBits (i, power_sample);
        RealOut[j] = RealIn[i];
        ImagOut[j] = ImagIn[i];
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
            ar2 = cm2;
            ar1 = cm1;
            
            ai2 = sm2;
            ai1 = sm1;
			
			j=i;
			n=0;
            while(n < BlockEnd)
            {
                ar0 = w*ar1 - ar2;
                ar2 = ar1;
                ar1 = ar0;
                
                ai0 = w*ai1 - ai2;
                ai2 = ai1;
                ai1 = ai0;
                
                k = j + BlockEnd;
                
				tr = ar0*RealOut[k] - ai0*ImagOut[k];
				ti = ar0*ImagOut[k] + ai0*RealOut[k];
				
				RealOut[k] = RealOut[j] - tr;
                ImagOut[k] = ImagOut[j] - ti;
                
                RealOut[j] += tr;
                ImagOut[j] += ti;
                
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
        double denom = (double)NumSamples;
        
        for ( i=0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }
}

static n_uint speak_length(n_byte character)
{
    switch (character)
    {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
            return 1;
        case 's':
        case 'm':
        case '.':
            return 2;
        default:
            return 0;
    }
}

static n_uint speak_length_total(n_string paragraph)
{
    n_int  loop = 0;
    n_byte character;
    n_uint length = 0;
    do {
        character = paragraph[loop++];
        length += 1 << speak_length(character);
    } while (character != '\n');
    return length;
}

void speak_aiff_header(FILE *fptr, n_uint total_samples)
{
	n_uint totalsize;
	/* Write the form chunk */
	fprintf(fptr,"FORM");
	totalsize = 4 + 8 + 18 + 8 + 2 * total_samples + 8;
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
	fputc((total_samples & 0xff000000) >> 24,fptr);   /* Samples */
	fputc((total_samples & 0x00ff0000) >> 16,fptr);
	fputc((total_samples & 0x0000ff00) >> 8,fptr);
	fputc((total_samples & 0x000000ff),fptr);
	fputc(0,fptr);                               /* Size = 16 */
	fputc(16,fptr);
	fputc(0x40,fptr);                            /* 10 byte sample rate */
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
	fputc((2*total_samples+8 & 0xff000000) >> 24,fptr);/* Size      */
	fputc((2*total_samples+8 & 0x00ff0000) >> 16,fptr);
	fputc((2*total_samples+8 & 0x0000ff00) >> 8,fptr);
	fputc((2*total_samples+8 & 0x000000ff),fptr);
	fputc(0,fptr);                                /* Offset    */
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);                                /* Block     */
	fputc(0,fptr);
	fputc(0,fptr);
	fputc(0,fptr);
	
}

static void speak_aiff_body(FILE * fptr, n_audio *samples, n_uint number_samples)
{
    fwrite(samples,number_samples,2,fptr);
}

static void speak_make(n_string filename, n_string paragraph)
{
    FILE     *out_file = 0L;
    
    n_uint   total_length = ACT_BUFFER * speak_length_total(paragraph);
    n_int    loop        = 0;
    n_byte    found_character;
    out_file = fopen(filename,"w");
    
    if (out_file == 0L)
    {
        (void)SHOW_ERROR("Failed create speak file!");
        return;
    }
    
    speak_aiff_header(out_file, total_length);
    do
    {
        n_uint    power_sample = (speak_length(found_character = paragraph[loop++])+ACT_BITS);
        n_uint    length = 1 << power_sample;
        n_int     division = MAX_BUFFER/length;
        n_int     loop = 0;
        while (loop < length)
        {
            frequency[loop] = 0;
            timedomain[loop] = 0;
            frequencyi[loop] = 0;
            timedomaini[loop] = 0;
            loop++;
        }

        if (found_character !=' ' && found_character != '.')
        {
            frequency[400/division] = 200000/division;
            frequency[600/division] = 800000/division;
            frequency[900/division] = 800000/division;
            frequency[700/division] = 800000/division;
            frequency[450/division] = 200000/division;
            frequency[460/division] = 900000/division;
            frequency[490/division] = 900000/division;

            
            fft_float(1, frequency, frequencyi, timedomaini, timedomain, power_sample);
            
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
                frequencyi[loop] = 0;
                timedomaini[loop] = 0;
                loop++;
            }

            frequency[0] = 30000/division;
            frequency[1] = 30000/division;
            frequency[2] = 30000/division;
            frequency[3] = 30000/division;

            fft_float(1, frequency, frequencyi, timedomaini, timedomain, power_sample);

            loop = 0;
            while (loop < length)
            {
                output[loop] *= timedomain[loop];
                loop++;
            }
        }
        else
        {
            loop = 0;
            while (loop < length)
            {
                output[loop] = 0;
                loop++;
            }
        }
        speak_aiff_body(out_file, output, length);

    }while (found_character != '\n');
    

    fclose(out_file);
}

void speak_out(n_string filename, n_string paragraph)
{
    speak_make(filename, paragraph);
}