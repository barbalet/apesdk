/****************************************************************

 test_math.c

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

/* this doesn't currently work, it is included here for unit
    testing in the future */

#include "../toolkit.h"

#include <stdio.h>

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    printf( "ERROR: %s @ %s %ld\n", ( const n_string ) error_text, location, line_number );
    return -1;
}

n_int check_root( n_uint value, n_uint squared )
{
    n_uint result = math_root( squared );
    if ( result != value )
    {
        printf( "squared %ld expects value %ld, instead %ld\n", squared, value, result );
        return -1;
    }
    return 0;
}


void check_intersection( void )
{
    n_vect2 p1, p2, q1, q2;

    p1.x = 1;
    p1.y = 1;

    q1.x = 10;
    q1.y = 1;

    p2.x = 1;
    p2.y = 2;

    q2.x = 10;
    q2.y = 2;

    if ( math_do_intersect( &p1, &q1, &p2, &q2 ) != 0 )
    {
        printf( "first intersection should be 0\n" );
        return;
    }

    p1.x = 10;
    p1.y = 0;

    q1.x = 0;
    q1.y = 10;

    p2.x = 0;
    p2.y = 0;

    q2.x = 10;
    q2.y = 10;

    if ( math_do_intersect( &p1, &q1, &p2, &q2 ) != 1 )
    {
        printf( "second intersection should be 1\n" );
        return;

    }

    p1.x = -5;
    p1.y = -5;

    q1.x = 0;
    q1.y = 0;

    p2.x = 1;
    p2.y = 1;

    q2.x = 10;
    q2.y = 10;

    if ( math_do_intersect( &p1, &q1, &p2, &q2 ) != 0 )
    {
        printf( "third intersection should be 0\n" );
        return;

    }
    printf( "Intersections passed fine!\n" );
}

n_byte test_line( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    return 0;
}

void check_lines( void )
{
    n_vect2 start = {24, 5420};
    n_vect2 end = {24, 5420};
    n_byte2 local[2] = {0xef5e, 0x4738};

    n_join joiner;
    joiner.information = 0L;
    joiner.pixel_draw = &test_line;

    ( void )math_join_vect2( -25, 1000, &start, &joiner );
    ( void )math_line_vect( &start, &end, &joiner );
    ( void )math_line( 0, 0, 650, 7000, &joiner );

    ( void )math_hash_fnv1( "HASH right here" );
    ( void )math_hash( "HASH right here", 15 );

    ( void )math_spread_byte( 5 );

    ( void )math_random3( local );
}

n_int check_tan(n_byte initial_direction)
{
    n_vect2 initial_facing;
    vect2_direction( &initial_facing, initial_direction, 8 );
    return math_tan( &initial_facing );
}

void check_math( void )
{
    n_int   loop = 0;
    n_int   sum = 0;
    n_vect2 sum_vect = {0};

    ( void )check_root( 1234, 1522756 );
    ( void )check_root( 4, 17 );
    ( void )check_root( 4, 16 );
    ( void )check_root( 3, 15 );
    ( void )check_root( 3, 14 );
    check_intersection();

    while ( loop < 256 )
    {
        n_vect2 each_vect;
        sum += math_sine( loop, 1 );
        vect2_direction( &each_vect, loop, 1 );
        vect2_d( &sum_vect, &each_vect, 1, 1 );
        loop++;
    }

    if ( sum != 0 )
    {
        printf( "scalar sum expecting 0 instead %ld\n", sum );
    }

    if ( sum_vect.x != 0 || sum_vect.y != 0 )
    {
        printf( "vect sum expecting 0, 0 instead %ld, %ld\n", sum_vect.x, sum_vect.y );
    }
}

int main( int argc, const char *argv[] )
{
    n_int loop = 0;
    printf( " --- test math --- start ----------------------------------------------\n" );

    check_math();

    while (loop < 1000)
    {
        n_int result = check_tan(loop);
        if (result != (loop & 255))
        {
            printf("check_tan(%d) %d\n", loop, result);
        }
        loop++;
    }
    printf( " --- test math ---  end  ----------------------------------------------\n" );

    return 0;
}

