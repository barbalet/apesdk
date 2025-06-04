/****************************************************************

 speak.c

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



#define _CRT_SECURE_NO_WARNINGS

/** working on the real solution */

#include <stdio.h>
#include <stdlib.h>

#include "entity.h"
#include "entity_internal.h"

/** the sound lengths vary from:

 (8k)  0.185 sec to
 (16k) 0.371 sec to
 (32k) 0.743 sec

 these can contain sounds or pauses
 */

static n_audio  output[AUDIO_FFT_MAX_BUFFER];

static n_uint speak_length( n_byte character )
{
    switch ( character )
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

static n_uint speak_length_total( n_string paragraph )
{
    n_int  loop = 0;
    n_int character;
    n_int length = 0;
    do
    {
        character = paragraph[loop++];
        if ( character != '\n' && character != 0 )
        {
            length += 1 << speak_length( ( n_byte )character );
        }
    }
    while ( character != '\n' && character != 0 );
    return ( n_uint )length;
}

static const n_int set_frequencies[24] =
{
    175, 178, 180, 183,
    185, 188, 191, 193,
    196, 199, 202, 205,
    208, 211, 214, 217,
    220, 223, 227, 229,
    233, 237, 240, 244
};


static const n_int vowel_reorder[8] =
{
    4, 7, 0, 2, 1, 6, 3, 5
};

static const n_int consonant_reorder[16] =
{
    6, 13, 3, 7,   0, 14, 1, 12,
    9, 11, 2, 15,  4, 10, 5, 8
};

static const n_int low_freq[13] =
{
    60000, 45000, 30000, 55000,
    30000, 40000, 35000, 60000,
    40000, 20000, 65000, 40000,
    30000
};

static void speak_freq( n_int *high, n_int *low, n_byte value )
{
    const n_string character_building = "aeiovfstpbjm";
    n_int loop = 0;
    do
    {
        if ( value != character_building[loop] )
        {
            loop++;
        }
    }
    while ( ( loop < 12 ) && ( value != character_building[loop] ) );

    if ( loop < 12 )
    {
        low[1] = low_freq[loop  ];
        low[0] = 1;
        low[3] = low_freq[loop + 1];
        low[2] = 2;

        if ( loop < 4 ) /**< vowel */
        {
            high[0] = set_frequencies[vowel_reorder[loop]];
            high[1] = 600000;
            high[2] = set_frequencies[vowel_reorder[loop + 1]];
            high[3] = 300000;
            high[4] = set_frequencies[vowel_reorder[loop + 2] + 2];
            high[5] = 200000;
            high[6] = set_frequencies[vowel_reorder[loop + 4] + 3];
            high[7] =  50000;
        }
        else /**< consonant */
        {
            high[0] = set_frequencies[consonant_reorder[loop - 4]];
            high[1] = 600000;
            high[2] = set_frequencies[consonant_reorder[loop - 2] + 3];
            high[3] = 400000;
            high[4] = set_frequencies[consonant_reorder[loop + 1] + 8];
            high[5] = 100000;
            high[6] = set_frequencies[consonant_reorder[loop + 3] + 8];
            high[7] =  50000;
        }
    }
    else
    {
        low[0] = 0;
        low[1] = 0;
        low[2] = 0;
        low[3] = 0;
        high[0] = 0;
        high[1] = 0;
        high[2] = 0;
        high[3] = 0;
        high[4] = 0;
        high[5] = 0;
        high[6] = 0;
        high[7] = 0;
    }

}

void speak_out( n_string filename, n_string paragraph )
{
    FILE    *out_file = 0L;
    n_uint   total_length = AUDIO_FFT_MAX_BUFFER * speak_length_total( paragraph ) >> 2;
    n_int    loop         = 0;
    n_byte   found_character;

    if ( total_length < 1 )
    {
        ( void )SHOW_ERROR( "Speaking length is less than one" );
        return;
    }

    out_file = fopen( filename, "w" );

    if ( out_file == 0L )
    {
        ( void )SHOW_ERROR( "Failed create speak file!" );
        return;
    }

    audio_aiff_header( out_file, total_length );
    do
    {
        n_uint    power_sample = ( speak_length( found_character = ( n_byte )paragraph[loop++] ) + AUDIO_FFT_MAX_BITS - 2 );
        n_uint    length = 1 << power_sample;
        n_int     division = AUDIO_FFT_MAX_BUFFER / length;

        audio_clear_buffers( length );

        if ( found_character != '\n' && found_character != 0 )
        {
            if ( found_character != ' ' && found_character != '.' )
            {
                n_int local_high[8], local_low[4];

                speak_freq( local_high, local_low, found_character );

                audio_set_frequency( ( n_uint )( local_high[0] / division ), ( n_uint )( local_high[1] / division ) );
                audio_set_frequency( ( n_uint )( local_high[2] / division ), ( n_uint )( local_high[3] / division ) );
                audio_set_frequency( ( n_uint )( local_high[4] / division ), ( n_uint )( local_high[5] / division ) );
                audio_set_frequency( ( n_uint )( local_high[6] / division ), ( n_uint )( local_high[7] / division ) );

                audio_fft( 1, power_sample );

                audio_equal_output( output, length );

                audio_clear_buffers( length );

                audio_set_frequency( ( n_uint )( local_low[0] ), ( n_uint )( local_low[1] / division ) );
                audio_set_frequency( ( n_uint )( local_low[2] ), ( n_uint )( local_low[3] / division ) );

                audio_fft( 1, power_sample );

                audio_multiply_output( output, length );
            }
            else
            {
                audio_clear_output( output, length );
            }
            audio_aiff_body( out_file, output, length );
        }
    }
    while ( found_character != '\n' && found_character != 0 );

    if ( fclose( out_file ) != 0 )
    {
        ( void )SHOW_ERROR( "Failed to close speak file" );
    }
    return;
}

