/****************************************************************

 test_gui.c

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

 ****************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "../toolkit/toolkit.h"
#include "../shared.h"
#include "../script/script.h"
#include "../sim/sim.h"
#include "../entity/entity.h"
#include "../universe/universe.h"
#include "../gui/gui.h"

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s (line %d)\n", message, __LINE__); \
            return 1; \
        } \
    } while(0)

n_int test_gui_run( void )
{
    n_int   counter = 0;
    n_int   tickCounter = 0x12738291;

    TEST_ASSERT( shared_init( 0, tickCounter ) == 0, "view window initializes" );
    tickCounter += 2601;
    TEST_ASSERT( shared_init( 1, tickCounter ) == 1, "terrain window initializes" );
    tickCounter += 2601;
    TEST_ASSERT( shared_init( 2, tickCounter ) == 2, "control window initializes" );
    TEST_ASSERT( shared_simulation_started() != 0, "simulation starts from GUI init" );
    tickCounter += 2601;

    while ( counter < ( 512 * 4 ) )
    {
        TEST_ASSERT( shared_cycle( tickCounter, 0 ) >= SHARED_CYCLE_OK, "view cycle returns a valid state" );
        tickCounter += 2601;
        TEST_ASSERT( shared_cycle( tickCounter, 1 ) >= SHARED_CYCLE_OK, "terrain cycle returns a valid state" );
        tickCounter += 2601;
        TEST_ASSERT( shared_cycle( tickCounter, 2 ) >= SHARED_CYCLE_OK, "control cycle returns a valid state" );
        tickCounter += 2601;

        TEST_ASSERT( shared_draw(0, 512, 512, 0 ) != 0L, "view draw returns a buffer" );
        tickCounter += 2601;
        TEST_ASSERT( shared_draw(0, 512, 512, 1 ) != 0L, "terrain draw returns a buffer" );
        tickCounter += 2601;
        TEST_ASSERT( shared_draw(0, 512, 512, 2 ) != 0L, "control draw returns a buffer" );
        tickCounter += 2601;

        counter ++;
    }

    shared_close();
    TEST_ASSERT( shared_simulation_started() == 0, "simulation stops after GUI close" );
    return 0;
}

int main( int argc, const char *argv[] )
{
    n_int result;
    printf( " --- test gui --- start -----------------------------------------------\n" );
    result = test_gui_run();
    printf( " --- test gui ---  end  -----------------------------------------------\n" );
    exit(result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
