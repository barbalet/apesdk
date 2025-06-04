/****************************************************************

 test_sim_time.c

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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"
#include "../entity/entity.h"
#include "../universe/universe.h"

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    exit(EXIT_FAILURE);
    return -1;
}

int main ( int c, char **v )
{
    n_uint  count = 0;
    clock_t startTime = clock();
    clock_t deltaTime = ( CLOCKS_PER_SEC * 60 * 30 ) + startTime;
    n_uint  xorchoff = 0;

    if ( c == 2 )
    {
        n_int      string_length = io_length( ( n_byte * )v[1], STRING_BLOCK_SIZE );

        if ( string_length > 0 )
        {
            xorchoff = math_hash( ( n_byte * )v[1], ( n_uint )string_length );
        }
    }
    printf( " --- test sim time ---  start  -----------------------------------------------\n" );

    printf( "start rand %lu\n", startTime ^ xorchoff );

    sim_init( KIND_START_UP, startTime ^ xorchoff, MAP_AREA, 0 );
    do
    {
        sim_cycle();
        count++;
    }
    while ( deltaTime > clock() );
    sim_close();
    printf( "count %lu\n", count );

    printf( " --- test sim time ---  end  -----------------------------------------------\n" );

    exit(EXIT_SUCCESS);

}
