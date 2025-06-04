/****************************************************************

 apesdk_include.c

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

#include "apesdk_include.h"

#include "../sim/sim.h"
#include "../shared.h"
#include "../universe/universe.h"

void* apesdk_init( n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size )
{
    void* result = NULL;
    
    result = sim_init(KIND_START_UP, randomise, offscreen_size, landbuffer_size);
    sim_set_output(1);

    if (result != NULL)
    {
        shared_init(NUM_VIEW, randomise);
        shared_init(NUM_TERRAIN, randomise);
        shared_init(NUM_CONTROL, randomise);
    }

    return result;
}


void apesdk_close( void )
{
    sim_close();
}

void apesdk_cycle( n_uint ticks )
{
    shared_cycle(ticks, NUM_VIEW);
    shared_cycle(ticks, NUM_TERRAIN);
    shared_cycle(ticks, NUM_CONTROL);
}

n_byte * apesdk_draw( n_int fIdentification, n_int dim_x, n_int dim_y )
{
    return shared_draw(fIdentification, dim_x, dim_y, 0);
}

n_string apesdk_get_output_string( void )
{
    return sim_output_string();
}
