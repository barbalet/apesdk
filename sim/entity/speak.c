/****************************************************************
 
 speak.c
 
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

#define ACT_BITS      (13)
#define ACT_BUFFER    (1<<ACT_BITS)
#define	MAX_BUFFER    (ACT_BUFFER*4)

static n_float frequency[MAX_BUFFER];
static n_float timedomain[MAX_BUFFER];
static n_float frequencyi[MAX_BUFFER];
static n_float timedomaini[MAX_BUFFER];

n_audio  output[MAX_BUFFER];

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
    n_uint		i, j;
    n_uint		BlockSize, BlockEnd;
    
    n_double angle_numerator = TWO_PI;
    
    
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
                    
                    n_double tr = ar0*RealOut[k] - ai0*ImagOut[k];
                    n_double ti = ar0*ImagOut[k] + ai0*RealOut[k];
                    
                    RealOut[k] = RealOut[j] - tr;
                    ImagOut[k] = ImagOut[j] - ti;
                    
                    RealOut[j] += tr;
                    ImagOut[j] += ti;
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
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
            i++;
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
        case 'p':
        case 'b':
        case 'j':
        case 't':
            return 0;
        default:
            return 2;
    }
}

static n_uint speak_length_total(n_string paragraph)
{
    n_int  loop = 0;
    n_byte character;
    n_uint length = 0;
    do {
        character = paragraph[loop++];
		if (character != '\n' && character != 0)
		{
			length += 1 << speak_length(character);
		}
    } while (character != '\n' && character != 0);
    return length;
}

void speak_aiff_header(FILE * fptr, n_uint total_samples)
{
    n_byte header[54] = {0};
    io_aiff_header(header);
    io_aiff_uint(&header[4],  io_aiff_total_size(total_samples));
    io_aiff_uint(&header[22], total_samples);
    io_aiff_uint(&header[42], io_aiff_sound_size(total_samples));
    fwrite(header, 54, 1, fptr);
}

static void speak_aiff_body(FILE * fptr, n_audio *samples, n_uint number_samples)
{
    fwrite(samples,number_samples,2,fptr);
}

const static n_int set_frequencies[24] = {
    175,178,180,183,
    185,188,191,193,
    196,199,202,205,
    208,211,214,217,
    220,223,227,229,
    233,237,240,244
};


const static n_int vowel_reorder[8] = {
    4, 7, 0, 2, 1, 6, 3, 5
};

const static n_int consonant_reorder[16] =
{
    6, 13, 3, 7,   0, 14, 1, 12,
    9, 11, 2, 15,  4, 10, 5, 8
};

const static n_int low_freq[13]=
{
    60000, 45000, 30000, 55000,
    30000, 40000, 35000, 60000,
    40000, 20000, 65000, 40000,
    30000
};

static void speak_freq(n_int * high, n_int * low, n_byte value)
{
    const n_string character_building = "aeiovfstpbjm";
    n_int loop = 0;
    do
    {
        if (value != character_building[loop])
        {
            loop++;
        }
    }
    while ((loop<12) && (value != character_building[loop]));

    if (loop < 12)
    {
        low[1] = low_freq[loop  ]; low[0] = 1;
        low[3] = low_freq[loop+1]; low[2] = 2;

        if (loop < 4) /* vowel */
        {
            high[0] = set_frequencies[vowel_reorder[loop]];     high[1] = 600000;
            high[2] = set_frequencies[vowel_reorder[loop+1]];   high[3] = 300000;
            high[4] = set_frequencies[vowel_reorder[loop+2]+2]; high[5] = 200000;
            high[6] = set_frequencies[vowel_reorder[loop+4]+3]; high[7] =  50000;
        }
        else /*consonant */
        {
            high[0] = set_frequencies[consonant_reorder[loop-4]];   high[1] = 600000;
            high[2] = set_frequencies[consonant_reorder[loop-2]+3]; high[3] = 400000;
            high[4] = set_frequencies[consonant_reorder[loop+1]+8]; high[5] = 100000;
            high[6] = set_frequencies[consonant_reorder[loop+3]+8]; high[7] =  50000;
        }
    }
    else
    {
        low[0] = 0;  low[1] = 0;
        low[2] = 0;  low[3] = 0;
        high[0] = 0; high[1] = 0;
        high[2] = 0; high[3] = 0;
        high[4] = 0; high[5] = 0;
        high[6] = 0; high[7] = 0;
    }
    
}

static void speak_make(n_string filename, n_string paragraph)
{
    FILE     *out_file = 0L;
    
    n_uint   total_length = ACT_BUFFER * speak_length_total(paragraph);
    n_int    loop        = 0;
    n_byte    found_character;
    
    if (total_length < 1)
    {
        (void)SHOW_ERROR("Speaking length is less than one");
        return;
    }
    
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

		if (found_character != '\n' && found_character != 0)
		{
		
			if (found_character !=' ' && found_character != '.')
			{
				n_int local_high[8], local_low[4];
				
				
				speak_freq(local_high, local_low,found_character);

				
				
				frequency[local_high[0]/division] = local_high[1]/division;
				frequency[local_high[2]/division] = local_high[3]/division;
				frequency[local_high[4]/division] = local_high[5]/division;
				frequency[local_high[6]/division] = local_high[7]/division;

				
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

				frequency[local_low[0]] = local_low[1]/division;
				frequency[local_low[2]] = local_low[3]/division;


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
		}
    } while (found_character != '\n' && found_character != 0);
    

    fclose(out_file);
}

void speak_out(n_string filename, n_string paragraph)
{
    speak_make(filename, paragraph);
}
