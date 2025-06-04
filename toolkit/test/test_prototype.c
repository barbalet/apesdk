/****************************************************************

 test_prototype.c

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

#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include "../toolkit.h"


typedef enum
{
    KVE_NAME,
    KVE_DATE,
    KVE_AREA,
    KVE_TREE,

    KVE_CENTER,
    KVE_RADIUS,
    KVE_FENCE,
    KVE_HOUSE,

    KVE_INNER,
    KVE_OUTER,
    KVE_DOOR,
    KVE_WINDOW,

    KVE_MAX_VALUES
} known_values_enum;

static n_uint known_hash[KVE_MAX_VALUES] =
{
    0x26e9551159ef5678,
    0x56c15b2dfb9d7076,
    0x8002830f295aced,
    0x88ce68a5243b00b9,
    0xc00f76514ac3f5dd,
    0x98a428b6d8797e14,
    0xd9fdc2b0d6d2b798,
    0x72c484ff4a00348d,
    0xdc1fc446ceb4e227,
    0x54c085c80c03035b,
    0xed653595b7d76f2e,
    0x5ac7d81ea7987d11
};


static n_string_block new_format_comparison = "[{\"name\":\"example file #10003\",\"date\":\"Sunday, June 13, 2021\"},{\"area\":[[500,1500],[1000,1500]],\"name\":\"highlands\",\"tree\":[{\"center\":[2198,19219],\"radius\":[34,34,27,37,38,28,24,27,38,28,19,20,21,18,29,18,34,34,27,37,38,28,24,27,38,28,19,20,21,18,29,18]},{\"center\":[213,10291],\"radius\":[34,34,27,37,38,28,24,27,38,28,19,20,18,18,29,18,34,34,20,37,38,10,24,27,38,28,19,25,21,18,29,18]}],\"fence\":[[[0,0],[1000,1000]],[[500,500],[500,1000]]],\"house\":[{\"inner\":[[-2848,-3030],[-2874,-2913],[-3035,-2949],[-3009,-3066]],\"outer\":[[-2855,-3026],[-2878,-2920],[-3028,-2954],[-3004,-3059]],\"door\":[[[-2933,-3047],[-2923,-3045],[-2922,-3049],[-2932,-3051]]],\"window\":[[[-3008,-3056],[-3030,-2958]]]},{\"inner\":[[-2786,-2833],[-2807,-2737],[-2923,-2763],[-2902,-2859]],\"outer\":[[-2793,-2828],[-2812,-2744],[-2916,-2768],[-2898,-2852]],\"door\":[[[-2849,-2845],[-2840,-2843],[-2839,-2847],[-2849,-2849]]],\"window\":[[[-2816,-2742],[-2913,-2764]]]},{\"inner\":[[-2890,-2917],[-2923,-2763],[-3040,-2790],[-3006,-2943]],\"outer\":[[-2897,-2912],[-2928,-2771],[-3033,-2794],[-3001,-2936]],\"door\":[[[-2909,-2836],[-2907,-2846],[-2903,-2845],[-2905,-2835]]],\"window\":[[[-3005,-2932],[-3035,-2799]]]},{\"inner\":[[-2731,-3004],[-2770,-2829],[-2887,-2855],[-2848,-3030]],\"outer\":[[-2738,-2999],[-2775,-2836],[-2880,-2860],[-2843,-3023]],\"door\":[[[-2866,-2937],[-2864,-2947],[-2868,-2948],[-2870,-2938]]],\"window\":[[[-2736,-2995],[-2771,-2840]]]},{\"inner\":[[-2874,-2913],[-2887,-2855],[-2902,-2859],[-2890,-2917]],\"outer\":[[-2881,-2908],[-2892,-2863],[-2895,-2864],[-2885,-2909]],\"door\":[[[-2899,-2860],[-2890,-2858],[-2890,-2854],[-2900,-2856]],[[-2887,-2914],[-2877,-2912],[-2876,-2916],[-2886,-2918]]]},{\"inner\":[[-2972,-2297],[-2861,-2254],[-2920,-2100],[-3032,-2142]],\"outer\":[[-2969,-2289],[-2869,-2250],[-2924,-2107],[-3024,-2145]],\"door\":[[[-3002,-2214],[-2999,-2223],[-3002,-2225],[-3006,-2216]]],\"window\":[[[-2964,-2291],[-2872,-2255]]]},{\"inner\":[[-2867,-2239],[-2720,-2182],[-2774,-2043],[-2920,-2100]],\"outer\":[[-2864,-2231],[-2727,-2179],[-2777,-2051],[-2912,-2102]],\"door\":[[[-2894,-2163],[-2890,-2173],[-2894,-2174],[-2898,-2165]]],\"window\":[[[-2726,-2174],[-2773,-2053]]]},{\"inner\":[[-3032,-2142],[-2774,-2043],[-2828,-1903],[-3086,-2003]],\"outer\":[[-3029,-2134],[-2781,-2040],[-2831,-1911],[-3078,-2007]],\"door\":[[[-2899,-2089],[-2908,-2093],[-2906,-2097],[-2898,-2093]]],\"window\":[[[-3076,-2002],[-2836,-1909]]]}]}]";



n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @%s, %ld\n", error_text, location, line_number );
    exit(EXIT_FAILURE);
    return -1;
}

void test_prototype_save( n_string file_out, n_object_type type_of, void *  returned_blob)
{
    n_file   *output_file = unknown_json( returned_blob, type_of );
    if ( output_file )
    {
        (void)io_disk_write( output_file, file_out );
        io_file_free( &output_file );
    }
    else
    {
        printf( "no returned output file\n" );
        exit(EXIT_FAILURE);
    }
    unknown_free( &returned_blob, type_of );
}


n_file * test_prototype_file( n_string file_in )
{
    n_file   *in_file = io_file_new();
    n_int    file_error = io_disk_read( in_file, file_in );

    if (file_error ==  -1)
    {
        io_file_free(&in_file);
        exit(EXIT_FAILURE);
        return 0L;
    }

    return in_file;
}


n_file * test_fake_file(void)
{
    return io_file_new_from_string_block((n_char *)new_format_comparison);
}


void * test_prototype_gather( n_file * in_file, n_object_type * type_of )
{
    if ( in_file )
    {
        io_whitespace_json( in_file );
        void *returned_blob = unknown_file_to_tree( in_file, type_of );

        io_file_free( &in_file );
        if ( returned_blob )
        {
            return returned_blob;
        }
        else
        {
            printf( "no returned object\n" );
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf( "reading from disk failed\n" );
        exit(EXIT_FAILURE);

    }
    return 0L;
}


void check_prototype( int argc, const char *argv[] )
{
    n_object_type   type_of;
    (void) test_prototype_gather( test_fake_file(), &type_of );
}


int main( int argc, const char *argv[] )
{
    clock_t start, stop;

    start = clock();

    object_init((n_uint*) known_hash, KVE_MAX_VALUES);
    check_prototype( argc, argv );
    object_close();

    stop = clock();

    printf("delta %6.3f\n", (double)(stop - start) / CLOCKS_PER_SEC);
    exit(EXIT_SUCCESS);
}
