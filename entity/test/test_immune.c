/****************************************************************

 test_immune.c

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

#include "../../toolkit/toolkit.h"
#include "../../universe/universe.h"
#include "../entity.h"

#include <stdio.h>

simulated_immune_system systems[10];

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s (line %d)\n", message, __LINE__); \
            return 1; \
        } \
        printf("PASS: %s\n", message); \
    } while(0)

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    printf("ERROR: %s @ %s %ld\n",(const n_string) error_text, location, line_number);
    return -1;
}

int main (void)
{
    n_byte2 random[2] = {0x6727, 0xfd31};
    n_int  result;
    n_int  loop;

    immune_init( &systems[0], ( n_byte2 *)random );

    for ( loop = 0; loop < IMMUNE_ANTIGENS; loop++ )
    {
        TEST_ASSERT( systems[0].antigens[loop] == 0, "immune_init clears antigens" );
    }

    for ( loop = 0; loop < IMMUNE_POPULATION; loop++ )
    {
        TEST_ASSERT( systems[0].antibodies[loop] == 0, "immune_init clears antibodies" );
    }
    
    result =  immune_response( &systems[0], 1, 2560 );
    TEST_ASSERT( result == 0, "immune_response is zero without active antigens" );
    
    return 0;
}
