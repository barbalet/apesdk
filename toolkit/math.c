/****************************************************************

 math.c

 =============================================================

 Copyright 1996-2022 Tom Barbalet. All rights reserved.

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

/*! \file   math.c
 *  \brief  This covers vector math, the hash/random mathematics,
   (Newton) square root, the Bresenham's line algorithm, the bilinear
   interpolation and other abstract math that is used in Simulated Ape.
 */

#include "toolkit.h"

/**
 This is used to produce a series of steps between two points useful for drawing
 a line or a line of sight test.
 @param sx The starting x location.
 @param sy The starting y location.
 @param dx The distance to be traveled in the x direction.
 @param dy The distance to be traveled in the y direction.
 @param draw The generic draw function used to traverse the line points.
 @return This is 0 if the line draw is completed successfully and 1 if it exits
 through the generic draw function's request.
 */
n_byte math_join( n_int sx, n_int sy, n_int dx, n_int dy, n_join *draw )
{
    n_int	  px = sx;
    n_int	  py = sy;

    n_pixel	 *local_draw;
    void	 *local_info;

    NA_ASSERT( draw, "draw NULL" );

    if ( draw == 0L )
    {
        return 1;
    }
    if ( draw->pixel_draw == 0L )
    {
        return 1;
    }

    local_draw = draw->pixel_draw;
    local_info = draw->information;

    NA_ASSERT( local_draw, "local_draw NULL" );
    NA_ASSERT( local_info, "local_info NULL" );

    if ( ( *local_draw )( px, py, 0, 0, local_info ) )
    {
        return 1;
    }
    if ( ( dx == 0 ) && ( dy == 0 ) )
    {
        return 0;
    }
    {
        n_int	  dxabs = dx;
        n_int	  dyabs = dy;

        n_int	  sdx = ( dxabs != 0 );
        n_int	  sdy = ( dyabs != 0 );
        if ( dxabs < 0 )
        {
            dxabs = 0 - dxabs;
            sdx = -1;
        }
        if ( dyabs < 0 )
        {
            dyabs = 0 - dyabs;
            sdy = -1;
        }
        if ( dxabs >= dyabs )
        {
            n_int y2 = dxabs >> 1;
            n_int i = 0;
            while ( i++ < dxabs )
            {
                y2 += dyabs;
                if ( y2 >= dxabs )
                {
                    y2 -= dxabs;
                    py += sdy;
                }
                px += sdx;
                if ( ( *local_draw )( px, py, sdx, sdy, local_info ) )
                {
                    return 1;
                }
            }
        }
        else
        {
            n_int x2 = dyabs >> 1;
            n_int i = 0;
            while ( i++ < dyabs )
            {
                x2 += dxabs;
                if ( x2 >= dyabs )
                {
                    x2 -= dyabs;
                    px += sdx;
                }
                py += sdy;
                if ( ( *local_draw )( px, py, sdx, sdy, local_info ) )
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

n_byte math_join_vect2( n_int sx, n_int sy, n_vect2 *vect, n_join *draw )
{
    return math_join( sx, sy, vect->x, vect->y, draw );
}

n_byte math_line_vect( n_vect2 *point1, n_vect2 *point2, n_join *draw )
{
    n_vect2 delta;
    vect2_subtract( &delta, point2, point1 );
    return math_join( point1->x, point1->y, delta.x, delta.y, draw );
}

n_byte math_line( n_int x1, n_int y1, n_int x2, n_int y2, n_join *draw )
{
    n_int dx = x2 - x1;
    n_int dy = y2 - y1;
    return math_join( x1, y1, dx, dy, draw );
}

n_byte4 math_hash_fnv1( n_constant_string key )
{
    n_byte4 hash = 2166136261;
    while ( *key )
    {
        hash = ( 16777619 * hash ) ^ ( n_byte4 )( *key++ );
    }
    return hash;
}

/**
 Creates a near-unique integer value from a block of data. This is
 similar to CRC or other hash methods.
 @param values The data in byte chunks.
 @param length The length of the data in bytes.
 @return The hash value produced.
 */
n_uint math_hash( n_byte *values, n_uint length )
{
    n_uint	loop = 0;
    n_byte2	round[5] = {0xfa78, 0xfad7, 0x53e7, 0xa728, 0x2c81};

    NA_ASSERT( values, "values NULL" );

    if ( sizeof( n_uint ) == 8 )
    {
        n_uint  big_round[4];

        while ( loop < length )
        {
            round[0] ^= round[4];
            round[1] ^= values[loop++];
            math_random3( round );
            math_random3( &round[1] );
            math_random3( &round[2] );
            math_random3( &round[3] );
        }
        big_round[0] = round[0];
        big_round[1] = round[1];
        big_round[2] = round[2];
        big_round[3] = round[3];

        return big_round[0] | ( big_round[1] << 16 ) | ( big_round[2] << 32 ) | ( big_round[3] << 48 );
    }

    while ( loop < length )
    {
        round[1] ^= values[loop++];
        math_random3( round );
    }
    return ( n_uint )( round[0] | ( round[1] << 16 ) );
}

n_int math_tan( n_vect2 *p )
{
    n_int   return_value = 0, best_p;
    n_vect2 vector_facing;
    n_int check_switch = 128;
    vect2_direction( &vector_facing, 0, 8 );
    best_p = vect2_dot( p, &vector_facing, 1, 1 );
    
    do
    {
        n_int   temp_p;
        vect2_direction( &vector_facing, return_value + check_switch, 8 );
        temp_p = vect2_dot( p, &vector_facing, 1, 1 );
        if (temp_p > best_p)
        {
            best_p = temp_p;
            return_value += check_switch;
        }
        
        vect2_direction( &vector_facing, return_value - check_switch + 256, 8 );
        temp_p = vect2_dot( p, &vector_facing, 1, 1 );
        if (temp_p > best_p)
        {
            best_p = temp_p;
            return_value -= check_switch;
        }
        check_switch = check_switch >> 1;
    } while (check_switch);
    return (return_value + 256) & 255;
}

n_int math_spline(n_vect2 * start_vector, n_vect2 * end_vector, n_vect2 * elements, n_int number_elements)
{
    if (number_elements == 0)
    {
        return -1;
    }
    
    n_vect2 start, end;
    
    vect2_subtract( &start, &start_vector[1], &start_vector[0] );
    vect2_subtract( &end, &end_vector[1], &end_vector[0] );
    
    n_int tan_start = math_tan(&start);
    n_int tan_end = math_tan(&end);
    n_int tan_new = tan_start;
    n_int tan_delta;
    
    if (tan_start > tan_end)
    {
        tan_end += 256;
    }
    
    tan_delta = (tan_end - tan_start) / number_elements;
    
    /* pre calculation */
    
    n_int loop = 0;
    
    n_vect2 span;
    
    span.x = start_vector[1].x;
    span.y = start_vector[1].y;

    while (loop < number_elements)
    {
        n_vect2 direction;
        tan_new += tan_delta;
        vect2_direction(&direction, tan_new, 1);
        vect2_delta(&span , &direction);
        loop++;
    }
    
    n_int dx = (end_vector[0].x - start_vector[1].x);
    n_int dy = (end_vector[0].y - start_vector[1].y);
    n_int dix = (span.x - start_vector[1].x);
    n_int diy = (span.y - start_vector[1].y);
    
    n_int divisor = 0;
    
    if (dx && dy)
    {
        divisor = ((dix / dx) + (diy / dy)) / 2;
    } else {
        if (dx)
        {
            divisor = (dix / dx);
        }
        else if (dy)
        {
            divisor = (diy / dy);
        }
    }
    
    if (divisor == 0)
    {
        return -1;
    }
    
    /* final calculation */
    loop = 0;
    
    tan_new = tan_start;
    
    span.x = start_vector[1].x;
    span.y = start_vector[1].y;

    while (loop < number_elements)
    {
        n_vect2 direction;
        tan_new += tan_delta;
        vect2_direction(&direction, tan_new, divisor);
        vect2_delta(&span , &direction);
        elements[loop].x = span.x;
        elements[loop].y = span.y;
        loop++;
    }
    
    return 0;
}

n_int math_spread_byte( n_byte val )
{
    n_int result = ( n_int )( val >> 1 );

    if ( ( val & 1 ) == 1 )
    {
        result = 0 - result;
    }
    return result;
}

#ifdef DEBUG_RANDOM

n_uint debug_random_count = 0;

void math_random_debug_count( n_string string )
{
    printf( "debug random count (%s) %ld\n", string, debug_random_count );
}

#endif

/**
 Generates a random number from two change-able two-byte random number
 values passed into the function in the form of a pointer.
 @param local The pointer leading to both the two-byte numbers used to seed (and change
 in the process).
 @return The two-byte random number produced.
 */

#ifdef DEBUG_RANDOM
n_byte2 math_random_debug( n_byte2 *local, n_string file_string, n_int line_number )
#else
n_byte2 math_random( n_byte2 *local )
#endif
{
    n_byte2 tmp0;
    n_byte2 tmp1;

    tmp0 = local[0];
    tmp1 = local[1];

    local[0] = tmp1;
    switch ( tmp0 & 7 )
    {
    case 0:
        local[1] = ( unsigned short )( tmp1 ^ ( tmp0 >> 1 ) ^ 0xd028 );
        break;
    case 4:
        local[1] = ( unsigned short )( tmp1 ^ ( tmp0 >> 2 ) ^ 0xae08 );
        break;
    case 8:
        local[1] = ( unsigned short )( tmp1 ^ ( tmp0 >> 3 ) ^ 0x6320 );
        break;
    default:
        local[1] = ( unsigned short )( tmp1 ^ ( tmp0 >> 1 ) );
        break;
    }

#ifdef DEBUG_RANDOM
#ifdef VERBOSE_DEBUG_RANDOM
    if ( ( debug_random_count & 15 ) == 0 )
    {
        printf( "\n(%ld %s %ld) %d ", debug_random_count++, file_string, line_number, tmp1 );
    }
    else
    {
        printf( " %d ", tmp1 );
    }
#endif
    debug_random_count++;
#endif

    return ( tmp1 );
}

#ifndef DEBUG_RANDOM

void math_random3( n_byte2 *local )
{
    NA_ASSERT( local, "local NULL" );

    ( void )math_random( local );
    ( void )math_random( local );
    ( void )math_random( local );
}

#endif

/* math_newton_root may need to be obsoleted */
n_uint math_root( n_uint input )
{
    n_uint op  = input;
    n_uint res = 0;
    n_uint one = 1uL << ( ( sizeof( n_uint ) * 8 ) - 2 );
    /* "one" starts at the highest power of four <= than the argument. */
    while ( one > op )
    {
        one >>= 2;
    }
    while ( one != 0 )
    {
        if ( op >= res + one )
        {
            op = op - ( res + one );
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

/* from ASCII 32 - 127, corresponding to the seg14 results */

/* n_byte segment = seg14[ conv[ character_value ]]; */
static const n_byte    conv[ 96 ] =
{
    0, 40, 41, 0, 0, 0, 0, 42, 43, 44, 38, 39, 45, 11, 46, 47, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 48, 49, 0, 50, 0, 51, 0, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 52, 53, 54, 0, 55, 56,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 57, 58, 59, 0, 0
};

/* one bit per segment */
static const n_byte2 seg14[ 60 ] =
{
    0x0000, 0x3F00, 0x1800, 0x36C0, 0x3CC0, 0x19C0, 0x2DC0, 0x2FC0, 0x3800, 0x3FC0,
    0x3DC0, 0x00C0, 0x3BC0, 0x3CA1, 0x2700, 0x3C21, 0x27C0, 0x23C0, 0x2F80, 0x1BC0,
    0x2421, 0x1E00, 0x0354, 0x0700, 0x1B06, 0x1B12, 0x3F00, 0x33C0, 0x3F10, 0x33D0,
    0x2DC0, 0x2021, 0x1F00, 0x030C, 0x1B18, 0x001E, 0x11E0, 0x240C, 0x00FF, 0x00E1,

    0x8001, 0x0101, 0x0001, 0x0014, 0x000A, 0x0008, 0x8000, 0x000C, 0xC000, 0x4008,
    0x04C0, 0xA004, 0x2700, 0x0012, 0x3402, 0x0400, 0x0002, 0x244A, 0x0021, 0x2494
};

n_int math_seg14( n_int character )
{
    return seg14[conv[character]];
}

static n_int math_max( n_int a, n_int b )
{
    return ( a < b ) ? b : a;
}

static n_int math_min( n_int a, n_int b )
{
    return !( b < a ) ? a : b;
}


/*Given three colinear points p, q, r, the function checks if point q lies on line segment 'pr' */
static n_byte  math_on_segment( n_vect2 *p, n_vect2 *q, n_vect2 *r )
{
    if ( ( q->x <= math_max( p->x, r->x ) ) &&
            ( q->x >= math_min( p->x, r->x ) ) &&
            ( q->y <= math_max( p->y, r->y ) ) &&
            ( q->y >= math_min( p->y, r->y ) ) )
    {
        return 1;
    }
    return 0;
}

/* To find orientation of ordered triplet (p, q, r).
   The function returns following values
     0 --> p, q and r are colinear
     1 --> Clockwise
     2 --> Counterclockwise
 */
static n_int math_orientation( n_vect2 *p, n_vect2 *q, n_vect2 *r )
{
    n_int val = ( ( q->y - p->y ) *
                  ( r->x - q->x ) ) -
                ( ( q->x - p->x ) *
                  ( r->y - q->y ) );

    if ( val == 0 )
    {
        return 0;    /* colinear */
    }

    return ( val > 0 ) ? 1 : 2; /* clock or counterclock wise */
}

/* The main function that returns true if line segment 'p1q1'
   and 'p2q2' intersect. */
n_byte math_do_intersect( n_vect2 *p1, n_vect2 *q1, n_vect2 *p2, n_vect2 *q2 )
{
    /* Find the four orientations needed for general and special cases */
    n_int o1 = math_orientation( p1, q1, p2 );
    n_int o2 = math_orientation( p1, q1, q2 );
    n_int o3 = math_orientation( p2, q2, p1 );
    n_int o4 = math_orientation( p2, q2, q1 );

    /* General case */
    if ( o1 != o2 && o3 != o4 )
    {
        return 1;
    }

    /* Special Cases */
    /* p1, q1 and p2 are colinear and p2 lies on segment p1q1 */
    if ( o1 == 0 && math_on_segment( p1, p2, q1 ) )
    {
        return 1;
    }

    /* p1, q1 and p2 are colinear and q2 lies on segment p1q1 */
    if ( o2 == 0 && math_on_segment( p1, q2, q1 ) )
    {
        return 1;
    }

    /* p2, q2 and p1 are colinear and p1 lies on segment p2q2 */
    if ( o3 == 0 && math_on_segment( p2, p1, q2 ) )
    {
        return 1;
    }

    /* p2, q2 and q1 are colinear and q1 lies on segment p2q2 */
    if ( o4 == 0 && math_on_segment( p2, q1, q2 ) )
    {
        return 1;
    }

    return 0; /* Doesn't fall in any of the above cases */
}
