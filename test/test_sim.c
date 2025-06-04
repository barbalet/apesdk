/****************************************************************

 test_sim.c

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

#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"
#include "../entity/entity.h"
#include "../universe/universe.h"

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    return -1;
}

n_uint test_distance_moved( simulated_group *group, n_int show_stopped )
{
    n_int loop = 0;
    n_uint total_moved = 0;
    while ( loop < group->num )
    {
        simulated_being *being = &group->beings[loop];
        n_byte        speed = being_speed( being );
        if ( IS_NIGHT( land_time() ) == 0 )
        {
            if ( ( speed == 0 ) && show_stopped )
            {
                n_string_block name_string;
                n_string_block time_string;
                spacetime_to_string( time_string );
                being_name_simple( being, name_string );
                printf( "%s %s stopped\n", time_string, name_string );
            }
        }
        total_moved += speed;
        loop++;
    }
    return total_moved;
}

void test_total_moved( simulated_group *group )
{
#ifdef DEBUG_LACK_OF_MOVEMENT
    n_int loop = 0;
    while ( loop < group->num )
    {
        simulated_being *being = &group->beings[loop];
        n_int        total_moved = being_total_movement( being );
        if ( total_moved == 0 )
        {
            n_string_block name_string;
            n_file *description = unknown_json( file_being( being ), OBJECT_OBJECT );
            being_name_simple( being, name_string );
            printf( "%s stopped\n", name_string );
            io_file_debug( description );

        }
        loop++;
    }
#endif
}


#define FIRST_RANDOM (17963)

n_int test_hash( void )
{
    simulated_group *group = sim_group();
    simulated_being       *first_being = &( group->beings[0] );
    simulated_being_constant *first_being_constant = &( first_being->constant );
    simulated_being_delta *first_being_delta = &( first_being->delta );
    simulated_being_events *first_being_events = &( first_being->events );
    simulated_being_brain *first_being_brain = &( first_being->braindata );
    simulated_immune_system *first_being_immune = &( first_being->immune_system );
    simulated_being_volatile *first_being_volatile = &( first_being->changes );

    n_byte2 random = being_random( first_being );

    printf( "random %d\n", random );
    if ( random != FIRST_RANDOM )
    {
        printf( "#define FIRST_RANDOM (%ld)\n", random );

        return 1;
    }
    return 0;
}

#define DELTA_0 (0)
#define MOVED_0 (0)
#define DELTA_1 (1700)
#define MOVED_1 (1011942)
#define DELTA_2 (56)
#define MOVED_2 (1680334)
#define DELTA_3 (455)
#define MOVED_3 (1703892)
#define TOTAL_MOVED (2484092)
#define LINE_OF_SIGHT_COUNT (9)
#define NUMBER_BEINGS (128)

n_int test_sim_run( void )
{
    n_int   counter = 0;
    n_int   return_value = 0;
    simulated_group *group = sim_group();
    n_uint distance_moved = 0;

    while ( counter < ( 512 * 4 ) )
    {
        n_uint distance_delta;
        sim_cycle();
        distance_delta = test_distance_moved( group, 0 );

        distance_moved += distance_delta;
        if ( ( counter & 511 ) == 0 )
        {
            printf( "%ld distance moved %ld running total %ld\n", counter, distance_delta, distance_moved );

            if ( counter == 0 )
            {
                if ( ( distance_delta != DELTA_0 ) || ( distance_moved != MOVED_0 ) )
                {
                    printf( "#define DELTA_0 (%ld)\n", distance_delta );
                    printf( "#define MOVED_0 (%ld)\n", distance_moved );
                    return_value |= 1;
                }
            }
            if ( counter == 512 )
            {
                if ( ( distance_delta != DELTA_1 ) || ( distance_moved != MOVED_1 ) )
                {
                    printf( "#define DELTA_1 (%ld)\n", distance_delta );
                    printf( "#define MOVED_1 (%ld)\n", distance_moved );
                    return_value |= 1;
                }
            }
            if ( counter == 1024 )
            {
                if ( ( distance_delta != DELTA_2 ) || ( distance_moved != MOVED_2 ) )
                {
                    printf( "#define DELTA_2 (%ld)\n", distance_delta );
                    printf( "#define MOVED_2 (%ld)\n", distance_moved );
                    return_value |= 1;
                }
            }
            if ( counter == 1536 )
            {
                if ( ( distance_delta != DELTA_3 ) || ( distance_moved != MOVED_3 ) )
                {
                    printf( "#define DELTA_3 (%ld)\n", distance_delta );
                    printf( "#define MOVED_3 (%ld)\n", distance_moved );
                    return_value |= 1;
                }
            }
        }
        counter ++;
    }

    printf( "total distance moved %ld\n", distance_moved );

    if ( distance_moved != TOTAL_MOVED )
    {
        printf( "#define TOTAL_MOVED (%ld)\n", distance_moved );
        return_value |= 1;
    }

    {
        n_int number_beings = group->num;
        n_int outer_loop = 0;
        n_int line_of_sight_count = 0;
        while ( outer_loop < ( number_beings - 1 ) )
        {
            n_int inner_loop = 1 + outer_loop;
            while ( inner_loop < number_beings )
            {
                n_vect2 location_vect;
                being_space( &group->beings[inner_loop], &location_vect );
                line_of_sight_count += being_line_of_sight( &group->beings[outer_loop],
                                       &location_vect );
                inner_loop++;
            }
            outer_loop++;
        }

        if ( ( line_of_sight_count != LINE_OF_SIGHT_COUNT ) || ( number_beings != NUMBER_BEINGS ) )
        {
            printf( "#define LINE_OF_SIGHT_COUNT (%ld)\n", line_of_sight_count );
            printf( "#define NUMBER_BEINGS (%ld)\n", number_beings );

            return_value |= 1;
        }
    }

    test_total_moved( group );
    return return_value;
}

int main( int argc, const char *argv[] )
{
    printf( " --- test sim --- start -----------------------------------------------\n" );

    sim_init( KIND_START_UP, 0x12738291, MAP_AREA, 0 );

    if ( test_sim_run() != 0 )
    {
        printf( "#1 test_sim_run\n" );
        return 1;
    }

    if ( test_hash() != 0 )
    {
        printf( "#1 test_hash\n" );
        return 1;
    }

    sim_close();

    sim_init( KIND_START_UP, 0x12738291, MAP_AREA, 0 );

    if ( test_sim_run() != 0 )
    {
        printf( "#2 test_sim_run\n" );
        return 1;
    }

    if ( test_hash() != 0 )
    {
        printf( "#2 test_hash\n" );
        return 1;
    }

    sim_init( KIND_NEW_SIMULATION, 0x12738291, MAP_AREA, 0 );

    if ( test_sim_run() != 0 )
    {
        printf( "#3 test_sim_run\n" );
        return 1;
    }

    if ( test_hash() != 0 )
    {
        printf( "#3 test_hash\n" );
        return 1;
    }

    sim_close();

    printf( " --- test sim ---  end  -----------------------------------------------\n" );

    return 0;
}
