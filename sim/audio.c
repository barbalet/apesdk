/****************************************************************

 audio.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

/*! \file   audio.c
 *  \brief  Handles audio output for the ApeSDK Toolkit

 */

#include "../toolkit/toolkit.h"
#include <math.h>
#include <stdio.h>

static n_double frequency[AUDIO_FFT_MAX_BUFFER];
static n_double timedomain[AUDIO_FFT_MAX_BUFFER];
static n_double frequencyi[AUDIO_FFT_MAX_BUFFER];
static n_double timedomaini[AUDIO_FFT_MAX_BUFFER];


void audio_buffer_clear( n_audio *buffer, n_int size )
{
    n_int loop = 0;
    while ( loop < size )
    {
        buffer[loop++] = 0;
    }
}

void audio_buffer_double_clear( n_double *buffer, n_int size )
{
    n_int loop = 0;
    while ( loop < size )
    {
        buffer[loop++] = 0E+00;
    }
}

void audio_buffer_copy_to_audio( n_double *buffer_double, n_audio *buffer_audio, n_int size )
{
    n_int loop = 0;
    while ( loop < size )
    {
        buffer_audio[loop] = ( n_audio )buffer_double[loop];
        loop++;
    }
}

void audio_buffer_copy_to_double( n_audio *buffer_audio, n_double *buffer_double, n_int size )
{
    n_int loop = 0;
    while ( loop < size )
    {
        buffer_double[loop] = ( n_double )buffer_audio[loop] ;
        loop++;
    }
}

void audio_buffer_copy_to_double_double( n_double *buffer_double_to, n_double *buffer_double_from, n_int size )
{
    n_int loop = 0;
    while ( loop < size )
    {
        buffer_double_to[loop] = buffer_double_from[loop];
        loop++;
    }
}


/**
 * Creates a bit reversed value of a particular index value.
 * @param index value to be bit reversed.
 * @param power_sample the size in bits of the value to be reversed.
 * @return reversed bit value.
 */
static n_uint	audio_reverse_bits ( n_uint index, n_uint power_sample )
{
    n_uint i = 0;
    n_uint rev = 0;
    while ( i < power_sample )
    {
        rev = ( rev << 1 ) | ( index & 1 );
        index >>= 1;
        i++;
    }
    return rev;
}

/*
 * Complex Fast Fourier Transform
 */

void audio_new_fft(
    n_uint     NumBits,
    n_int      InverseTransform,
    n_double    *RealIn,
    n_double    *ImagIn,
    n_double    *RealOut,
    n_double    *ImagOut )
{
    n_uint NumSamples = 1 << NumBits;
    n_uint i, j;
    n_uint k, n;
    n_uint BlockSize, BlockEnd;

    n_double angle_numerator = TWO_PI;
    n_double tr, ti;     /* temp real, temp imaginary */

    if ( InverseTransform )
    {
        angle_numerator = -angle_numerator;
    }

    /*
     **   Do simultaneous data copy and bit-reversal ordering into outputs...
     */

    for ( i = 0; i < NumSamples; i++ )
    {
        j = audio_reverse_bits ( i, NumBits );
        RealOut[j] = RealIn[i];
        ImagOut[j] = ImagIn[i];
    }

    /*
     **   Do the FFT itself...
     */

    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
    {

        n_double delta_angle = angle_numerator / ( double )BlockSize;

        n_double sm2 = sin ( -2 * delta_angle );
        n_double sm1 = sin ( -delta_angle );
        n_double cm2 = cos ( -2 * delta_angle );
        n_double cm1 = cos ( -delta_angle );
        n_double w = 2 * cm1;
        n_double ar0, ar1, ar2, ai0, ai1, ai2;

        for ( i = 0; i < NumSamples; i += BlockSize )
        {
            ar2 = cm2;
            ar1 = cm1;

            ai2 = sm2;
            ai1 = sm1;

            for ( j = i, n = 0; n < BlockEnd; j++, n++ )
            {
                ar0 = w * ar1 - ar2;
                ar2 = ar1;
                ar1 = ar0;

                ai0 = w * ai1 - ai2;
                ai2 = ai1;
                ai1 = ai0;

                k = j + BlockEnd;
                tr = ar0 * RealOut[k] - ai0 * ImagOut[k];
                ti = ar0 * ImagOut[k] + ai0 * RealOut[k];

                RealOut[k] = RealOut[j] - tr;
                ImagOut[k] = ImagOut[j] - ti;

                RealOut[j] += tr;
                ImagOut[j] += ti;
            }
        }
        BlockEnd = BlockSize;
    }

    /*
     **   Need to normalize if inverse transform...
     */

    if ( InverseTransform )
    {
        float denom = ( float )NumSamples;
        for ( i = 0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }
}

/**
 * Perform a fast fourier transform.
 * @param inverse if this is an inverse FFT.
 * @param power_sample the number of bits of the FFT cell size.
 */
void audio_fft( n_byte inverse, n_uint power_sample )
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

    i = 0;
    while ( i < NumSamples )
    {
        n_uint j = audio_reverse_bits ( i, power_sample );
        d_outi[j] = d_in[i];
        d_out[j]  = d_ini[i];
        i++;
    }

    /*
     **   Do the FFT itself...
     */

    BlockEnd = 1;
    BlockSize = 2;
    while ( BlockSize <= NumSamples )
    {
        n_double delta_angle = angle_numerator / ( n_double )BlockSize;
        n_double sm2 = sin ( ( n_double ) - 2.0f * delta_angle );
        n_double sm1 = sin ( ( n_double ) - delta_angle );
        n_double cm2 = cos ( ( n_double ) - 2.0f * delta_angle );
        n_double cm1 = cos ( ( n_double ) - delta_angle );
        n_double w = 2 * cm1;

        i = 0;

        while (  i < NumSamples )
        {
            n_uint j = i;
            n_uint n = 0;

            n_double ar2 = cm2;
            n_double ar1 = cm1;

            n_double ai2 = sm2;
            n_double ai1 = sm1;

            while ( n < BlockEnd )
            {
                n_double ar0 = w * ar1 - ar2;
                n_double ai0 = w * ai1 - ai2;

                ar2 = ar1;
                ar1 = ar0;

                ai2 = ai1;
                ai1 = ai0;

                {
                    n_uint   k = j + BlockEnd;

                    n_double tr = ar0 * d_outi[k] - ai0 * d_out[k];
                    n_double ti = ar0 * d_out[k] + ai0 * d_outi[k];

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
        n_double denom = ( n_double ) NumSamples;
        i = 0;
        while  ( i < NumSamples )
        {
            d_outi[i] /= denom;
            d_out[i] /= denom;
            i++;
        }
    }
}

/**
 * Clears all the buffers associated with the FFT.
 * @param length the length of the buffer to be cleared.
 */
void audio_clear_buffers( n_uint length )
{
    n_uint     loop = 0;
    while ( loop < length )
    {
        frequency[loop] = 0;
        timedomain[loop] = 0;
        frequencyi[loop] = 0;
        timedomaini[loop] = 0;
        loop++;
    }
}

/**
 * Clears an audio output buffer
 * @param audio the audio buffer to be cleared.
 * @param length the length of the buffer to be cleared.
 */
void audio_clear_output( n_audio *audio, n_uint length )
{
    n_uint     loop = 0;
    while ( loop < length )
    {
        audio[loop] = 0;
        loop++;
    }
}

/**
* Sets an audio ouput buffer to the FFT time-domain buffer.
* @param audio the audio buffer to be set.
* @param length the length of the buffer to be set.
*/
void audio_equal_output( n_audio *audio, n_uint length )
{
    n_uint     loop = 0;
    while ( loop < length )
    {
        audio[loop] = ( n_audio )timedomain[loop];
        loop++;
    }
}

/**
 * Sets an audio ouput buffer to the FFT time-domain buffer.
 * @param audio the audio buffer to be set.
 * @param length the length of the buffer to be set.
 */
static void audio_equal_input( n_audio *audio, n_uint length )
{
    n_uint     loop = 0;
    while ( loop < length )
    {
        timedomain[loop] = ( n_double )audio[loop];
        loop++;
    }
}

/**
 * Multiplies an audio ouput buffer to the FFT time-domain buffer.
 * @param audio the audio buffer to be multiplied.
 * @param length the length of the buffer to be multiplied.
 */
void audio_multiply_output( n_audio *audio, n_uint length )
{
    n_uint     loop = 0;
    while ( loop < length )
    {
        audio[loop] *= timedomain[loop];
        loop++;
    }
}

/**
 * Sets frequency values in the frequency FFT buffer.
 * @param entry the frequency entry point.
 * @param value the specific value to set in the frequency entry.
 */
void audio_set_frequency( n_uint entry, n_uint value )
{
    frequency[entry] = value / 1E+00;
    frequencyi[entry] = 0E+00;
}


void audio_low_frequency( n_audio *buffer, n_int number_freq, n_int debug )
{
    audio_equal_input( buffer, AUDIO_FFT_MAX_BUFFER );
    audio_fft( 0, 15 );

    audio_fft( 1, 15 );
    audio_equal_output( buffer, AUDIO_FFT_MAX_BUFFER );
}

static void audio_aiff_uint( n_byte *buffer, n_uint value )
{
    buffer[0] = ( value & 0xff000000 ) >> 24;
    buffer[1] = ( value & 0x00ff0000 ) >> 16;
    buffer[2] = ( value & 0x0000ff00 ) >> 8;
    buffer[3] = ( value & 0x000000ff ) >> 0;
}

static n_uint audio_aiff_byte( n_byte *buffer )
{
    n_uint value = buffer[3];
    value |= ( n_uint )( buffer[2] << 8 );
    value |= ( n_uint )( buffer[1] << 16 );
    value |= ( n_uint )( buffer[0] << 24 );
    return value;
}

void audio_aiff_body( void *fptr, n_audio *samples, n_uint number_samples )
{
    fwrite( samples, number_samples, sizeof( n_audio ), ( FILE * )fptr );
}

static void audio_header( n_byte *header )
{
    header[0] =  'F';
    header[1] =  'O';
    header[2] =  'R';
    header[3] =  'M';

    header[8]  = 'A';
    header[9]  = 'I';
    header[10] = 'F';
    header[11] = 'F';

    header[12] = 'C';
    header[13] = 'O';
    header[14] = 'M';
    header[15] = 'M';

    header[19] = 18;

    header[21] = 1;

    header[27] = 16;

    header[28] = 0x40;
    header[29] = 0x0e;
    header[30] = 0xac;
    header[31] = 0x44;

    header[38] = 'S';
    header[39] = 'S';
    header[40] = 'N';
    header[41] = 'D';
}

static n_uint audio_total_size( n_uint total_samples )
{
    return ( sizeof( n_audio ) * total_samples ) + 46;
}

static n_uint audio_sound_size( n_uint total_samples )
{
    return ( sizeof( n_audio ) * total_samples ) + 8;
}

void audio_aiff_header( void *fptr, n_uint total_samples )
{
    n_byte header[54] = {0};
    audio_header( header );
    audio_aiff_uint( &header[4],  audio_total_size( total_samples ) );
    audio_aiff_uint( &header[22], total_samples );
    audio_aiff_uint( &header[42], audio_sound_size( total_samples ) );
    fwrite( header, 54, 1, ( FILE * )fptr );
}

static n_int audio_aiff_comparison( n_byte *compare1, n_byte *compare2, n_int start, n_int end )
{
    n_int loop_section = start;

    while ( loop_section < end )
    {
        if ( compare1[loop_section] != compare2[loop_section] )
        {
            printf( "failed at point %ld\n", loop_section );
            return loop_section;
        }
        loop_section++;
    }
    return -1;
}

n_int audio_aiff_is_header( void *fptr, n_uint *samples )
{
    n_byte read_header[54] = {0};
    n_byte compare_header[54] = {0};

    audio_header( compare_header );

    fread( read_header, 54, 1, ( FILE * )fptr );

    if ( audio_aiff_comparison( read_header, compare_header, 0, 4 ) != -1 )
    {
        return 0;
    }
    if ( audio_aiff_comparison( read_header, compare_header, 8, 22 ) != -1 )
    {
        return 0;
    }
    if ( audio_aiff_comparison( read_header, compare_header, 26, 42 ) != -1 )
    {
        return 0;
    }
    if ( audio_aiff_comparison( read_header, compare_header, 46, 54 ) != -1 )
    {
        return 0;
    }

    *samples = audio_aiff_byte( &read_header[22] );

    return 1;
}
