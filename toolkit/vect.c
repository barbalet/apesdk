/****************************************************************

 vect.c

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

/*! \file   vect.c
 *  \brief  This covers vector math in Simulated Ape.
 */

#include "toolkit.h"
#include "stdio.h"

/** \brief new_sd stands for new sine dump and hold the sine and cosine values for the simulation */
static const n_int	new_sd[256] =
{
    0, 659, 1318, 1977, 2634, 3290, 3944, 4595, 5244, 5889, 6531, 7169, 7802, 8431, 9055, 9673,
    10286, 10892, 11492, 12085, 12671, 13249, 13819, 14380, 14933, 15477, 16012, 16537, 17052, 17557, 18051, 18534,
    19007, 19467, 19916, 20353, 20778, 21190, 21590, 21976, 22349, 22709, 23055, 23387, 23706, 24009, 24299, 24573,
    24833, 25078, 25308, 25523, 25722, 25906, 26074, 26226, 26363, 26484, 26589, 26677, 26750, 26807, 26847, 26871,
    26880, 26871, 26847, 26807, 26750, 26677, 26589, 26484, 26363, 26226, 26074, 25906, 25722, 25523, 25308, 25078,
    24833, 24573, 24299, 24009, 23706, 23387, 23055, 22709, 22349, 21976, 21590, 21190, 20778, 20353, 19916, 19467,
    19007, 18534, 18051, 17557, 17052, 16537, 16012, 15477, 14933, 14380, 13819, 13249, 12671, 12085, 11492, 10892,
    10286, 9673, 9055, 8431, 7802, 7169, 6531, 5889, 5244, 4595, 3944, 3290, 2634, 1977, 1318, 659,
    0, -659, -1318, -1977, -2634, -3290, -3944, -4595, -5244, -5889, -6531, -7169, -7802, -8431, -9055, -9673,
    -10286, -10892, -11492, -12085, -12671, -13249, -13819, -14380, -14933, -15477, -16012, -16537, -17052, -17557, -18051, -18534,
    -19007, -19467, -19916, -20353, -20778, -21190, -21590, -21976, -22349, -22709, -23055, -23387, -23706, -24009, -24299, -24573,
    -24833, -25078, -25308, -25523, -25722, -25906, -26074, -26226, -26363, -26484, -26589, -26677, -26750, -26807, -26847, -26871,
    -26880, -26871, -26847, -26807, -26750, -26677, -26589, -26484, -26363, -26226, -26074, -25906, -25722, -25523, -25308, -25078,
    -24833, -24573, -24299, -24009, -23706, -23387, -23055, -22709, -22349, -21976, -21590, -21190, -20778, -20353, -19916, -19467,
    -19007, -18534, -18051, -17557, -17052, -16537, -16012, -15477, -14933, -14380, -13819, -13249, -12671, -12085, -11492, -10892,
    -10286, -9673, -9055, -8431, -7802, -7169, -6531, -5889, -5244, -4595, -3944, -3290, -2634, -1977, -1318, -659
};

void area2_add( n_area2 *area, n_vect2 *vect, n_byte first )
{
    if ( first )
    {
        area->bottom_right.x = vect->x;
        area->bottom_right.y = vect->y;

        area->top_left.x = vect->x;
        area->top_left.y = vect->y;
        return;
    }

    if ( vect->x < area->top_left.x )
    {
        area->top_left.x = vect->x;
    }
    if ( vect->y < area->top_left.y )
    {
        area->top_left.y = vect->y;
    }

    if ( vect->x > area->bottom_right.x )
    {
        area->bottom_right.x = vect->x;
    }
    if ( vect->y > area->bottom_right.y )
    {
        area->bottom_right.y = vect->y;
    }
}

/**
 * Converts an array of n_byte2 to a 2d vector (n_vect2)
 * @param converter the vector to hold the information.
 * @param input the n_byte2 that is converted to the n_vect2.
 */
void vect2_byte2( n_vect2 *converter, n_byte2 *input )
{
    NA_ASSERT( converter, "converter NULL" );
    NA_ASSERT( input, "input NULL" );

    if ( converter == 0L )
    {
        return;
    }
    if ( input == 0L )
    {
        return;
    }

    converter->x = input[0];
    converter->y = input[1];
}

/**
 * Adds two 2d vectors into a resultant vector.
 * @param equals the vector that holds the result.
 * @param initial the first vector to be added.
 * @param second the second vector to be added.
 */
void vect2_add( n_vect2 *equals, n_vect2 *initial, n_vect2 *second )
{
    NA_ASSERT( equals, "equals NULL" );
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );

    if ( equals == 0L )
    {
        return;
    }
    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }

    equals->x = initial->x + second->x;
    equals->y = initial->y + second->y;
}


void vect2_center( n_vect2 *center, n_vect2 *initial, n_vect2 *second )
{
    vect2_add( center, initial, second );
    center->x = center->x / 2;
    center->y = center->y / 2;
}

void vect2_scalar_multiply( n_vect2 *value, n_int multiplier )
{
    value->x = value->x * multiplier;
    value->y = value->y * multiplier;
}

void vect2_scalar_divide( n_vect2 *value, n_int divisor )
{
    value->x = value->x / divisor;
    value->y = value->y / divisor;
}

void vect2_scalar_bitshiftdown( n_vect2 *value, n_int bitshiftdown )
{
    value->x = value->x >> bitshiftdown;
    value->y = value->y >> bitshiftdown;
}

/**
 * Subtracts one 2d vector from another 2d vector into a resultant vector.
 * @param equals the vector that holds the result.
 * @param initial the first vector.
 * @param second the second vector to be subtracted.
 */
void vect2_subtract( n_vect2 *equals, n_vect2 *initial, n_vect2 *second )
{
    NA_ASSERT( equals, "equals NULL" );
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );

    if ( equals == 0L )
    {
        return;
    }
    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }

    equals->x = initial->x - second->x;
    equals->y = initial->y - second->y;
}

void vect2_divide( n_vect2 *equals, n_vect2 *initial, n_vect2 *second, n_int divisor )
{
    vect2_subtract( equals, second, initial );

    if ( equals == 0L || ( divisor == 0 ) )
    {
        return;
    }
    equals->x = equals->x / divisor;
    equals->y = equals->y / divisor;
}

/**
 * Multiplies one 2d vector with another 2d vector times a multiplier divided by a divisor.
 * @param equals the vector that holds the result.
 * @param initial the first vector.
 * @param second the second vector to be multiplied.
 * @param multiplier the scalar multiplier.
 * @param divisor the the scalar divisor.
 */
void vect2_multiplier( n_vect2 *equals, n_vect2 *initial, n_vect2 *second,
                       n_int multiplier, n_int divisor )
{
    NA_ASSERT( equals, "equals NULL" );
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );
    NA_ASSERT( divisor, "divisor ZERO" );

    if ( equals == 0L )
    {
        return;
    }
    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }
    if ( divisor == 0L )
    {
        return;
    }

    equals->x = ( multiplier * initial->x * second->x ) / divisor;
    equals->y = ( multiplier * initial->y * second->y ) / divisor;
}

/**
 * Adds one 2d vector with another 2d vector times a multiplier divided by a divisor.
 * @param initial the first vector that takes the summation of the second vector.
 * @param second the second vector to be multiplied.
 * @param multiplier the scalar multiplier.
 * @param divisor the the scalar divisor.
 */
void vect2_d( n_vect2 *initial, n_vect2 *second, n_int multiplier, n_int divisor )
{
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );
    NA_ASSERT( divisor, "divisor ZERO" );

    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }
    if ( divisor == 0L )
    {
        return;
    }

    initial->x += ( ( multiplier * second->x ) / divisor );
    initial->y += ( ( multiplier * second->y ) / divisor );
}

/**
 This produces the dot product of two vectors with the scalar multiplier
 and divisor noted.
 @param initial The first vector
 @param second  The second vector
 @param multiplier The numerator multiplier
 @param divisor The divisor multiplier
 @return The resultant scalar
 */
n_int vect2_dot( n_vect2 *initial, n_vect2 *second,
                 n_int multiplier, n_int divisor )
{
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );
    NA_ASSERT( divisor, "divisor ZERO" );

    if ( initial == 0L )
    {
        return 0;
    }
    if ( second == 0L )
    {
        return 0;
    }

    return ( multiplier * ( ( initial->x * second->x ) + ( initial->y * second->y ) ) ) / divisor;
}

n_int vect2_distance_under( n_vect2 *first, n_vect2 *second, n_int distance )
{
    n_vect2 difference = {{0, 0}};
    n_int   distance_squ;
    vect2_subtract( &difference, first, second );
    distance_squ = ( difference.x * difference.x ) + ( difference.y * difference.y );
    return ( distance * distance ) > distance_squ;
}

/**
 This produces a sine value
 @param direction 256 units per rotation
 @param divisor The divisor for the output value
 @return The sine value
 */
n_int math_sine( n_int direction, n_int divisor )
{
    NA_ASSERT( divisor, "divisor ZERO" );
    return new_sd[( direction ) & 255] / ( divisor );
}

void vect2_rotate90( n_vect2 *rotation )
{
    n_int  temp = rotation->y;
    rotation->y = 0 - rotation->x;
    rotation->x = temp;
}

/**
 This produces a direction vector
 @param initial The vector output
 @param direction 256 units per rotation
 @param divisor The divisor for the output value
 */
void vect2_direction( n_vect2 *initial, n_int direction, n_int divisor )
{
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( divisor, "divisor ZERO" );
    initial->x = ( ( new_sd[( ( direction ) + 64 ) & 255] ) / ( divisor ) );
    initial->y = ( ( new_sd[( direction ) & 255] ) / ( divisor ) );
}

void vect2_delta( n_vect2 *initial, n_vect2 *delta )
{
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( delta, "delta NULL" );

    if ( initial == 0L )
    {
        return;
    }
    if ( delta == 0L )
    {
        return;
    }

    initial->x += delta->x;
    initial->y += delta->y;
}

void vect2_offset( n_vect2 *initial, n_int dx, n_int dy )
{
    NA_ASSERT( initial, "initial NULL" );

    if ( initial == 0L )
    {
        return;
    }

    initial->x += dx;
    initial->y += dy;
}

void vect2_back_byte2( n_vect2 *converter, n_byte2 *output )
{
    NA_ASSERT( converter, "converter NULL" );
    NA_ASSERT( output, "output NULL" );

    if ( converter == 0L )
    {
        return;
    }
    if ( output == 0L )
    {
        return;
    }

    if ( converter->x > 65535 )
    {
        converter->x = 65535;
    }
    if ( converter->y > 65535 )
    {
        converter->y = 65535;
    }
    if ( converter->x < 0 )
    {
        converter->x = 0;
    }
    if ( converter->y < 0 )
    {
        converter->y = 0;
    }

    output[0] = ( n_byte2 ) converter->x;
    output[1] = ( n_byte2 ) converter->y;
}

void  vect2_copy( n_vect2 *to, n_vect2 *from )
{
    to->x = from->x;
    to->y = from->y;
}

void  vect2_populate( n_vect2 *value, n_int x, n_int y )
{
    value->x = x;
    value->y = y;
}

void vect2_rotation( n_vect2 *location, n_vect2 *rotation )
{
    n_vect2 temp;

    temp.x = ( ( location->x * rotation->x ) + ( location->y * rotation->y ) ) / SINE_MAXIMUM;
    temp.y = ( ( location->x * rotation->y ) - ( location->y * rotation->x ) ) / SINE_MAXIMUM;

    location->x = temp.x;
    location->y = temp.y;
}

void vect2_rotation_bitshift( n_vect2 *location, n_vect2 *rotation )
{
    n_vect2 temp;

    temp.x = ( ( location->x * rotation->x ) + ( location->y * rotation->y ) ) >> 15;
    temp.y = ( ( location->x * rotation->y ) - ( location->y * rotation->x ) ) >> 15;

    location->x = temp.x;
    location->y = temp.y;
}

n_int vect2_nonzero( n_vect2 *nonzero )
{
    return ( ( nonzero->x != 0 ) || ( nonzero->y != 0 ) );
}

n_vect2 *vect2_min_max_init( void )
{
    n_vect2 *min_max = memory_new( 2 * sizeof( n_vect2 ) );
    if ( min_max == 0L )
    {
        return 0L;
    }
    vect2_populate( &min_max[0], BIG_INTEGER, BIG_INTEGER );
    vect2_populate( &min_max[1], BIG_NEGATIVE_INTEGER, BIG_NEGATIVE_INTEGER );
    return min_max;
}

void vect2_min_max_permutation( n_vect2 * points, n_vect2 * minmax)
{
    n_int px = points->x;
    n_int py = points->y;
    if ( px < minmax[0].x )
    {
        minmax[0].x = px;
    }
    if ( py < minmax[0].y )
    {
        minmax[0].y = py;
    }

    if ( px > minmax[1].x )
    {
        minmax[1].x = px;
    }
    if ( py > minmax[1].y )
    {
        minmax[1].y = py;
    }
}

void vect2_min_max( n_vect2 *points, n_int number, n_vect2 *maxmin )
{
    n_int loop = 0;

    while ( loop < number )
    {
        n_int px = points[loop].x;
        n_int py = points[loop].y;
        if ( px < maxmin[0].x )
        {
            maxmin[0].x = px;
        }
        if ( py < maxmin[0].y )
        {
            maxmin[0].y = py;
        }

        if ( px > maxmin[1].x )
        {
            maxmin[1].x = px;
        }
        if ( py > maxmin[1].y )
        {
            maxmin[1].y = py;
        }
        loop++;
    }
}

void vect3_double( n_vect3 *converter, n_double *input )
{
    NA_ASSERT( converter, "converter NULL" );
    NA_ASSERT( input, "input NULL" );

    if ( converter == 0L )
    {
        return;
    }
    if ( input == 0L )
    {
        return;
    }

    converter->x = input[0];
    converter->y = input[1];
    converter->z = input[1];
}

void vect3_add( n_vect3 *equals, n_vect3 *initial, n_vect3 *second )
{
    NA_ASSERT( equals, "equals NULL" );
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );

    if ( equals == 0L )
    {
        return;
    }
    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }

    equals->x = initial->x + second->x;
    equals->y = initial->y + second->y;
    equals->z = initial->z + second->z;
}


void vect3_center( n_vect3 *center, n_vect3 *initial, n_vect3 *second )
{
    vect3_add( center, initial, second );
    center->x = center->x / 2;
    center->y = center->y / 2;
    center->z = center->z / 2;
}

void vect3_subtract( n_vect3 *equals, n_vect3 *initial, n_vect3 *second )
{
    NA_ASSERT( equals, "equals NULL" );
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );

    if ( equals == 0L )
    {
        return;
    }
    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }

    equals->x = initial->x - second->x;
    equals->y = initial->y - second->y;
    equals->z = initial->z - second->z;
}

void vect3_divide( n_vect3 *equals, n_vect3 *initial, n_vect3 *second, n_double divisor )
{
    vect3_subtract( equals, second, initial );

    if ( equals == 0L || ( divisor == 0 ) )
    {
        return;
    }
    equals->x = equals->x / divisor;
    equals->y = equals->y / divisor;
    equals->z = equals->z / divisor;
}

void vect3_multiplier( n_vect3 *equals, n_vect3 *initial, n_vect3 *second,
                       n_double multiplier, n_double divisor )
{
    NA_ASSERT( equals, "equals NULL" );
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );
    NA_ASSERT( divisor != 0, "divisor ZERO" );

    if ( equals == 0L )
    {
        return;
    }
    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }
    if ( divisor == 0L )
    {
        return;
    }

    equals->x = ( multiplier * initial->x * second->x ) / divisor;
    equals->y = ( multiplier * initial->y * second->y ) / divisor;
    equals->z = ( multiplier * initial->z * second->z ) / divisor;
}

void vect3_d( n_vect3 *initial, n_vect3 *second, n_double multiplier, n_double divisor )
{
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );
    NA_ASSERT( divisor != 0, "divisor ZERO" );

    if ( initial == 0L )
    {
        return;
    }
    if ( second == 0L )
    {
        return;
    }
    if ( divisor == 0L )
    {
        return;
    }

    initial->x += ( ( multiplier * second->x ) / divisor );
    initial->y += ( ( multiplier * second->y ) / divisor );
    initial->z += ( ( multiplier * second->z ) / divisor );
}

n_double vect3_dot( n_vect3 *initial, n_vect3 *second,
                    n_double multiplier, n_double divisor )
{
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( second, "second NULL" );
    NA_ASSERT( divisor != 0, "divisor ZERO" );

    if ( initial == 0L )
    {
        return 0;
    }
    if ( second == 0L )
    {
        return 0;
    }

    return ( multiplier * ( ( initial->x * second->x ) + ( initial->y * second->y ) + ( initial->z * second->z ) ) ) / divisor;
}

void vect3_delta( n_vect3 *initial, n_vect3 *delta )
{
    NA_ASSERT( initial, "initial NULL" );
    NA_ASSERT( delta, "delta NULL" );

    if ( initial == 0L )
    {
        return;
    }
    if ( delta == 0L )
    {
        return;
    }

    initial->x += delta->x;
    initial->y += delta->y;
    initial->z += delta->z;
}

void vect3_offset( n_vect3 *initial, n_double dx, n_double dy, n_double dz )
{
    NA_ASSERT( initial, "initial NULL" );

    if ( initial == 0L )
    {
        return;
    }

    initial->x += dx;
    initial->y += dy;
    initial->z += dz;
}

void vect3_back_double( n_vect3 *converter, n_double *output )
{
    NA_ASSERT( converter, "converter NULL" );
    NA_ASSERT( output, "output NULL" );

    if ( converter == 0L )
    {
        return;
    }
    if ( output == 0L )
    {
        return;
    }

    output[0] = converter->x;
    output[1] = converter->y;
    output[2] = converter->z;
}

void  vect3_copy( n_vect3 *to, n_vect3 *from )
{
    to->x = from->x;
    to->y = from->y;
    to->z = from->z;
}

void  vect3_populate( n_vect3 *value, n_double x, n_double y, n_double z )
{
    value->x = x;
    value->y = y;
    value->z = z;
}

n_int vect3_nonzero( n_vect3 *nonzero )
{
    return ( ( nonzero->x != 0 ) || ( nonzero->y != 0 ) || ( nonzero->z != 0 ) );
}

n_array * vect2_memory_list_number_array(memory_list * list, n_int number)
{
    n_array * array = 0L;
    if ((list == 0L) || (number == 0))
    {
        return 0L;
    }
    if (list->count)
    {
        n_int count = 0;
        while (count < list->count)
        {
            n_vect2 * vects = (n_vect2 *) list->data;
            array_add_empty(&array, array_array(object_vect2_pointer(&vects[count * number], number)));
            count++;
        }
    }
    return array;
}


n_int vect2_unwrap_number( n_array * array, n_vect2 * entry, n_int number)
{
    n_int     out_value = 0;
    if (array)
    {
        memory_list * elements = object_list_vect2(array);
        if (elements->count == number)
        {
            n_vect2 * vect = (n_vect2 *)elements->data;
            n_int count = 0;
            while (count < number)
            {
                entry[count].data[0] = vect[count].data[0];
                entry[count].data[1] = vect[count].data[1];
                count++;
            }
            out_value = 1;
        }
        memory_list_free(&elements);
    }
    return out_value;
}

n_int vect2_unwrap_number_entry( n_string pass_through, n_byte * buffer, n_int number)
{
    return vect2_unwrap_number( (n_array *) pass_through, (n_vect2 *) buffer, number);
}

n_int vect2_unwrap_quad( n_string pass_through, n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 4);

}

n_int vect2_unwrap_line( n_string pass_through, n_byte * buffer)
{
    return vect2_unwrap_number_entry( pass_through, buffer, 2);
}

