/****************************************************************

 math.c

 =============================================================

 Copyright 1996-2019 Tom Barbalet. All rights reserved.

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

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/

/*! \file   math.c
 *  \brief  This covers vector math, the hash/random mathematics,
   (Newton) square root, the Bresenham's line algorithm, the bilinear
   interpolation and other abstract math that is used in Noble Ape.
 */

#include "noble.h"

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

void area2_add(n_area2 * area, n_vect2 * vect, n_byte first)
{
    if (first)
    {
        area->bottom_right.x = vect->x;
        area->bottom_right.y = vect->y;
        
        area->top_left.x = vect->x;
        area->top_left.y = vect->y;
        return;
    }
    
    if (vect->x < area->top_left.x)
    {
        area->top_left.x = vect->x;
    }
    if (vect->y < area->top_left.y)
    {
        area->top_left.y = vect->y;
    }

    if (vect->x > area->bottom_right.x)
    {
        area->bottom_right.x = vect->x;
    }
    if (vect->y > area->bottom_right.y)
    {
        area->bottom_right.y = vect->y;
    }   
}

/**
 * Converts an array of n_byte2 to a 2d vector (n_vect2)
 * @param converter the vector to hold the information.
 * @param input the n_byte2 that is converted to the n_vect2.
 */
void vect2_byte2(n_vect2 * converter, n_byte2 * input)
{
    NA_ASSERT(converter, "converter NULL");
    NA_ASSERT(input, "input NULL");

    if (converter == 0L) return;
    if (input == 0L) return;

    converter->x = input[0];
    converter->y = input[1];
}

/**
 * Adds two 2d vectors into a resultant vector.
 * @param equals the vector that holds the result.
 * @param initial the first vector to be added.
 * @param second the second vector to be added.
 */
void vect2_add(n_vect2 * equals, n_vect2 * initial, n_vect2 * second)
{
    NA_ASSERT(equals, "equals NULL");
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");

    if (equals == 0L) return;
    if (initial == 0L) return;
    if (second == 0L) return;

    equals->x = initial->x + second->x;
    equals->y = initial->y + second->y;
}


void vect2_center(n_vect2 * center, n_vect2 * initial, n_vect2 * second)
{
    vect2_add(center, initial, second);
    center->x = center->x / 2;
    center->y = center->y / 2;
}

void vect2_scalar_multiply(n_vect2 * value, n_int multiplier)
{
    value->x = value->x * multiplier;
    value->y = value->y * multiplier;
}

void vect2_scalar_divide(n_vect2 * value, n_int divisor)
{
    value->x = value->x / divisor;
    value->y = value->y / divisor;
}

void vect2_scalar_bitshiftdown(n_vect2 * value, n_int bitshiftdown)
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
void vect2_subtract(n_vect2 * equals, n_vect2 * initial, n_vect2 * second)
{
    NA_ASSERT(equals, "equals NULL");
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");

    if (equals == 0L) return;
    if (initial == 0L) return;
    if (second == 0L) return;

    equals->x = initial->x - second->x;
    equals->y = initial->y - second->y;
}

void vect2_divide(n_vect2 * equals, n_vect2 * initial, n_vect2 * second, n_int divisor)
{
    vect2_subtract(equals, second, initial);
    
    if (equals == 0L || (divisor == 0))
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
void vect2_multiplier(n_vect2 * equals, n_vect2 * initial, n_vect2 * second,
                      n_int multiplier, n_int divisor)
{
    NA_ASSERT(equals, "equals NULL");
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    NA_ASSERT(divisor, "divisor ZERO");

    if (equals == 0L) return;
    if (initial == 0L) return;
    if (second == 0L) return;
    if (divisor == 0L) return;

    equals->x = (multiplier * initial->x * second->x) / divisor;
    equals->y = (multiplier * initial->y * second->y) / divisor;
}

/**
 * Adds one 2d vector with another 2d vector times a multiplier divided by a divisor.
 * @param initial the first vector that takes the summation of the second vector.
 * @param second the second vector to be multiplied.
 * @param multiplier the scalar multiplier.
 * @param divisor the the scalar divisor.
 */
void vect2_d(n_vect2 * initial, n_vect2 * second, n_int multiplier, n_int divisor)
{
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    NA_ASSERT(divisor, "divisor ZERO");

    if (initial == 0L) return;
    if (second == 0L) return;
    if (divisor == 0L) return;

    initial->x += ( (multiplier * second->x) / divisor);
    initial->y += ( (multiplier * second->y) / divisor);
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
n_int vect2_dot(n_vect2 * initial, n_vect2 * second,
                n_int multiplier, n_int divisor)
{
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    NA_ASSERT(divisor, "divisor ZERO");

    if (initial == 0L) return 0;
    if (second == 0L) return 0;

    return (multiplier * ((initial->x * second->x) + (initial->y * second->y))) / divisor;
}

n_int vect2_distance_under(n_vect2 * first, n_vect2 * second, n_int distance)
{
    n_vect2 difference;
    n_int   distance_squ;
    vect2_subtract(&difference, first, second);
    distance_squ = (difference.x * difference.x) + (difference.y * difference.y);
    return (distance * distance) > distance_squ;
}

/**
 This produces a sine value
 @param direction 256 units per rotation
 @param divisor The divisor for the output value
 @return The sine value
 */
n_int math_sine(n_int direction, n_int divisor)
{
    NA_ASSERT(divisor, "divisor ZERO");
    return new_sd[(direction)&255] / (divisor);
}

void vect2_rotate90(n_vect2 * rotation)
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
void vect2_direction(n_vect2 * initial, n_int direction, n_int divisor)
{
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(divisor, "divisor ZERO");
    initial->x = ((new_sd[((direction)+64)&255]) / (divisor));
    initial->y = ((new_sd[(direction)&255]) / (divisor));
}

void vect2_delta(n_vect2 * initial, n_vect2 * delta)
{
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(delta, "delta NULL");
    
    if (initial == 0L) return;
    if (delta == 0L) return;
    
    initial->x += delta->x;
    initial->y += delta->y;
}

void vect2_offset(n_vect2 * initial, n_int dx, n_int dy)
{
    NA_ASSERT(initial, "initial NULL");

    if (initial == 0L) return;

    initial->x += dx;
    initial->y += dy;
}

void vect2_back_byte2(n_vect2 * converter, n_byte2 * output)
{
    NA_ASSERT(converter, "converter NULL");
    NA_ASSERT(output, "output NULL");

    if (converter == 0L) return;
    if (output == 0L) return;

    if (converter->x > 65535) converter->x = 65535;
    if (converter->y > 65535) converter->y = 65535;
    if (converter->x < 0) converter->x = 0;
    if (converter->y < 0) converter->y = 0;

    output[0] = (n_byte2) converter->x;
    output[1] = (n_byte2) converter->y;
}

void  vect2_copy(n_vect2 * to, n_vect2 * from)
{
    to->x = from->x;
    to->y = from->y;
}

void  vect2_populate(n_vect2 * value, n_int x, n_int y)
{
    value->x = x;
    value->y = y;
}

void vect2_rotation(n_vect2 * location, n_vect2 * rotation)
{
    n_vect2 temp;

    temp.x = ((location->x * rotation->x) + (location->y * rotation->y)) / SINE_MAXIMUM;
    temp.y = ((location->x * rotation->y) - (location->y * rotation->x)) / SINE_MAXIMUM;

    location->x = temp.x;
    location->y = temp.y;
}

void vect2_rotation_bitshift(n_vect2 * location, n_vect2 * rotation)
{
    n_vect2 temp;
    
    temp.x = ((location->x * rotation->x) + (location->y * rotation->y)) >> 15;
    temp.y = ((location->x * rotation->y) - (location->y * rotation->x)) >> 15;
    
    location->x = temp.x;
    location->y = temp.y;
}

n_int vect2_nonzero(n_vect2 * nonzero)
{
    return ((nonzero->x != 0) || (nonzero->y != 0));
}

n_vect2 * vect2_min_max_init(void)
{
    n_vect2 * min_max = memory_new(2 * sizeof(n_vect2));
    if (min_max == 0L)
    {
        return 0L;
    }
    vect2_populate(&min_max[0], 2147483647, 2147483647);
    vect2_populate(&min_max[1], -2147483648, -2147483648);
    return min_max;
}

void vect2_min_max(n_vect2 * points, n_int number, n_vect2 * maxmin)
{
    n_int loop = 0;

    while(loop < number)
    {
        n_int px = points[loop].x;
        n_int py = points[loop].y;
        if (px < maxmin[0].x)
        {
            maxmin[0].x = px;
        }
        if (py < maxmin[0].y)
        {
            maxmin[0].y = py;
        }

        if (px > maxmin[1].x)
        {
            maxmin[1].x = px;
        }
        if (py > maxmin[1].y)
        {
            maxmin[1].y = py;
        }
        loop++;
    }
}

void vect3_double(n_vect3 * converter, n_double * input)
{
    NA_ASSERT(converter, "converter NULL");
    NA_ASSERT(input, "input NULL");
    
    if (converter == 0L) return;
    if (input == 0L) return;
    
    converter->x = input[0];
    converter->y = input[1];
    converter->z = input[1];
}

void vect3_add(n_vect3 * equals, n_vect3 * initial, n_vect3 * second)
{
    NA_ASSERT(equals, "equals NULL");
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    
    if (equals == 0L) return;
    if (initial == 0L) return;
    if (second == 0L) return;
    
    equals->x = initial->x + second->x;
    equals->y = initial->y + second->y;
    equals->z = initial->z + second->z;
}


void vect3_center(n_vect3 * center, n_vect3 * initial, n_vect3 * second)
{
    vect3_add(center, initial, second);
    center->x = center->x / 2;
    center->y = center->y / 2;
    center->z = center->z / 2;
}

void vect3_subtract(n_vect3 * equals, n_vect3 * initial, n_vect3 * second)
{
    NA_ASSERT(equals, "equals NULL");
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    
    if (equals == 0L) return;
    if (initial == 0L) return;
    if (second == 0L) return;
    
    equals->x = initial->x - second->x;
    equals->y = initial->y - second->y;
    equals->z = initial->z - second->z;
}

void vect3_divide(n_vect3 * equals, n_vect3 * initial, n_vect3 * second, n_double divisor)
{
    vect3_subtract(equals, second, initial);
    
    if (equals == 0L || (divisor == 0))
    {
        return;
    }
    equals->x = equals->x / divisor;
    equals->y = equals->y / divisor;
    equals->z = equals->z / divisor;
}

void vect3_multiplier(n_vect3 * equals, n_vect3 * initial, n_vect3 * second,
                      n_double multiplier, n_double divisor)
{
    NA_ASSERT(equals, "equals NULL");
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    NA_ASSERT(divisor != 0, "divisor ZERO");
    
    if (equals == 0L) return;
    if (initial == 0L) return;
    if (second == 0L) return;
    if (divisor == 0L) return;
    
    equals->x = (multiplier * initial->x * second->x) / divisor;
    equals->y = (multiplier * initial->y * second->y) / divisor;
    equals->z = (multiplier * initial->z * second->z) / divisor;
}

void vect3_d(n_vect3 * initial, n_vect3 * second, n_double multiplier, n_double divisor)
{
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    NA_ASSERT(divisor != 0, "divisor ZERO");
    
    if (initial == 0L) return;
    if (second == 0L) return;
    if (divisor == 0L) return;
    
    initial->x += ( (multiplier * second->x) / divisor);
    initial->y += ( (multiplier * second->y) / divisor);
    initial->z += ( (multiplier * second->z) / divisor);
}

n_double vect3_dot(n_vect3 * initial, n_vect3 * second,
                n_double multiplier, n_double divisor)
{
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(second, "second NULL");
    NA_ASSERT(divisor != 0, "divisor ZERO");
    
    if (initial == 0L) return 0;
    if (second == 0L) return 0;
    
    return (multiplier * ((initial->x * second->x) + (initial->y * second->y) + (initial->z * second->z))) / divisor;
}

void vect3_delta(n_vect3 * initial, n_vect3 * delta)
{
    NA_ASSERT(initial, "initial NULL");
    NA_ASSERT(delta, "delta NULL");
    
    if (initial == 0L) return;
    if (delta == 0L) return;
    
    initial->x += delta->x;
    initial->y += delta->y;
    initial->z += delta->z;
}

void vect3_offset(n_vect3 * initial, n_double dx, n_double dy, n_double dz)
{
    NA_ASSERT(initial, "initial NULL");
    
    if (initial == 0L) return;
    
    initial->x += dx;
    initial->y += dy;
    initial->z += dz;
}

void vect3_back_double(n_vect3 * converter, n_double * output)
{
    NA_ASSERT(converter, "converter NULL");
    NA_ASSERT(output, "output NULL");
    
    if (converter == 0L) return;
    if (output == 0L) return;
    
    output[0] = converter->x;
    output[1] = converter->y;
    output[2] = converter->z;
}

void  vect3_copy(n_vect3 * to, n_vect3 * from)
{
    to->x = from->x;
    to->y = from->y;
    to->z = from->z;
}

void  vect3_populate(n_vect3 * value, n_double x, n_double y, n_double z)
{
    value->x = x;
    value->y = y;
    value->z = z;
}

n_int vect3_nonzero(n_vect3 * nonzero)
{
    return ((nonzero->x != 0) || (nonzero->y != 0) || (nonzero->z != 0));
}

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
n_byte math_join(n_int sx, n_int sy, n_int dx, n_int dy, n_join * draw)
{
    n_int	  px = sx;
    n_int	  py = sy;

    n_pixel	* local_draw;
    void	* local_info;

    NA_ASSERT(draw, "draw NULL");

    if (draw == 0L) return 1;
    if (draw->pixel_draw == 0L) return 1;

    local_draw = draw->pixel_draw;
    local_info = draw->information;

    NA_ASSERT(local_draw, "local_draw NULL");
    NA_ASSERT(local_info, "local_info NULL");

    if ((*local_draw)(px, py, 0, 0, local_info))
    {
        return 1;
    }
    if ((dx == 0) && (dy == 0))
    {
        return 0;
    }
    {
        n_int	  dxabs = dx;
        n_int	  dyabs = dy;

        n_int	  sdx = (dxabs != 0);
        n_int	  sdy = (dyabs != 0);
        if (dxabs < 0)
        {
            dxabs = 0 - dxabs;
            sdx = -1;
        }
        if (dyabs < 0)
        {
            dyabs = 0 - dyabs;
            sdy = -1;
        }
        if (dxabs >= dyabs)
        {
            n_int y2 = dxabs >> 1;
            n_int i = 0;
            while (i++ < dxabs)
            {
                y2 += dyabs;
                if (y2 >= dxabs)
                {
                    y2 -= dxabs;
                    py += sdy;
                }
                px += sdx;
                if ((*local_draw)(px, py, sdx, sdy, local_info))
                    return 1;
            }
        }
        else
        {
            n_int x2 = dyabs >> 1;
            n_int i = 0;
            while (i++ < dyabs)
            {
                x2 += dxabs;
                if (x2 >= dyabs)
                {
                    x2 -= dyabs;
                    px += sdx;
                }
                py += sdy;
                if ((*local_draw)(px, py, sdx, sdy, local_info))
                    return 1;
            }
        }
    }
    return 0;
}

n_byte math_join_vect2(n_int sx, n_int sy, n_vect2 * vect, n_join * draw)
{
    return math_join(sx, sy, vect->x, vect->y, draw);
}

n_byte math_line_vect(n_vect2 * point1, n_vect2 * point2, n_join * draw)
{
    n_vect2 delta;
    vect2_subtract(&delta, point2, point1);
    return math_join(point1->x, point1->y, delta.x, delta.y, draw);
}

n_byte math_line(n_int x1, n_int y1, n_int x2, n_int y2, n_join * draw)
{
    n_int dx = x2 - x1;
    n_int dy = y2 - y1;
    return math_join(x1, y1, dx, dy, draw);
}

n_byte4 math_hash_fnv1(n_constant_string key)
{
    n_byte4 hash = 2166136261;
    while(*key)
        hash = (16777619 * hash) ^ (n_byte4)(*key++);
    return hash;
}

/**
 Creates a near-unique integer value from a block of data. This is
 similar to CRC or other hash methods.
 @param values The data in byte chunks.
 @param length The length of the data in bytes.
 @return The hash value produced.
 */
n_uint math_hash(n_byte * values, n_uint length)
{
    n_uint	loop = 0;
    n_byte2	round[5]= {0xfa78, 0xfad7, 0x53e7, 0xa728, 0x2c81};

    NA_ASSERT(values, "values NULL");

    if (sizeof(n_uint) == 8)
    {
        n_uint  big_round[4];

        while( loop < length)
        {
            round[0]^=round[4];
            round[1]^=values[loop++];
            math_random3(round);
            math_random3(&round[1]);
            math_random3(&round[2]);
            math_random3(&round[3]);
        }
        big_round[0] = round[0];
        big_round[1] = round[1];
        big_round[2] = round[2];
        big_round[3] = round[3];

        return big_round[0] | (big_round[1]<<16) | (big_round[2] << 32) | (big_round[3]<<48);
    }

    while(loop<length)
    {
        round[1]^=values[loop++];
        math_random3(round);
    }
    return round[0] | (round[1]<<16);
}

#define		NUMBER_TURN_TOWARDS_POINTS	8

/**
 Calculates the direction location needs to turn to turn towards
 a vector.
 @param p The x vector direction.
 @param fac The current direction facing.
 @param turn The number of facing angle units that could be turned
 (it may not be the number of angle units turned).
 @return The new direction facing value.
 */
n_byte math_turn_towards(n_vect2 * p, n_byte fac, n_byte turn)
{
    n_int track[NUMBER_TURN_TOWARDS_POINTS] =
    {
        64, 64, 32, 16, 8, 4, 2, 1
    };
    n_vect2 vector_facing;
    n_int best_p;
    n_int best_f = fac;
    n_int loop = turn;

    NA_ASSERT(p, "p NULL");

    vect2_direction(&vector_facing, best_f, 32);

    best_p = vect2_dot(p, &vector_facing, 1, 1);

    while (loop < NUMBER_TURN_TOWARDS_POINTS)
    {
        n_int loc_track = track[loop];
        n_int loc_f = (best_f + loc_track) & 255;
        n_int project1;

        vect2_direction(&vector_facing, loc_f, 32);
        project1 = vect2_dot(p, &vector_facing, 1, 1);

        if (project1 > best_p)
        {
            best_f = loc_f;
            best_p = project1;
        }
        else
        {
            n_int loc_f2 = (best_f + 256 - loc_track) & 255;
            n_int project2;

            vect2_direction(&vector_facing, loc_f, 32);
            project2 = vect2_dot(p, &vector_facing, 1, 1);

            if (project2 > best_p)
            {
                best_f = loc_f2;
                best_p = project2;
            }
        }
        loop++;
    }
    return (n_byte)best_f;
}

n_int math_spread_byte(n_byte val)
{
    n_int result = (n_int)(val >> 1);

    if ((val & 1) == 1)
    {
        result = 0 - result;
    }
    return result;
}

/**
 Generates a random number from two change-able two-byte random number
 values passed into the function in the form of a pointer.
 @param local The pointer leading to both the two-byte numbers used to seed (and change
 in the process).
 @return The two-byte random number produced.
 */
n_byte2 math_random(n_byte2 * local)
{
    n_byte2 tmp0;
    n_byte2 tmp1;
    
    tmp0 = local[0];
    tmp1 = local[1];
    
    local[0] = tmp1;
    switch (tmp0 & 7)
    {
        case 0:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 1) ^ 0xd028);
            break;
        case 4:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 2) ^ 0xae08);
            break;
        case 8:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 3) ^ 0x6320);
            break;
        default:
            local[1] = (unsigned short)(tmp1 ^ (tmp0 >> 1));
            break;
    }
    return (tmp1);
}

void math_random3(n_byte2 * local)
{
    NA_ASSERT(local, "local NULL");

    (void)math_random(local);
    (void)math_random(local);
    (void)math_random(local);
}

/* all this hardcoding will need to be de-hardcoded in the future */
void math_bilinear_8_times(n_byte * side512, n_byte * data, n_byte double_spread)
{
    n_int loop_y = 0;

    NA_ASSERT(side512, "side512 NULL");
    NA_ASSERT(data, "data NULL");

    if (side512 == 0L) return;
    if (data == 0L) return;

    while (loop_y < HI_RES_MAP_DIMENSION)
    {
        n_int loop_x = 0;
        while (loop_x < HI_RES_MAP_DIMENSION)
        {
            /* find the micro x (on the map used for bilinear interpolation) */
            n_int mic_x = ( loop_x & 7);
            /* find the micro y (on the map used for bilinear interpolation) */
            n_int mic_y = ( loop_y & 7);

            n_int mac_x = (loop_x >> 3);
            n_int mac_y = (loop_y >> 3);

            n_uint px0 = (mac_x);
            n_uint py0 = (mac_y * MAP_DIMENSION);

            n_uint px1 = (mac_x + 1) & (MAP_DIMENSION-1);
            n_uint py1 = ((mac_y + 1) & (MAP_DIMENSION-1)) * MAP_DIMENSION;

            n_int z00 = side512[px0|py0];

            n_int z01 = side512[px1|py0];
            n_int z10 = side512[px0|py1] - z00;
            n_int z11 = side512[px1|py1] - z01 - z10;
            n_uint point = loop_x + (loop_y * HI_RES_MAP_DIMENSION);
            n_byte value;

            z01 = (z01 - z00) << 3;
            z10 = z10 << 3;

            value = (n_byte)((z00 + (((z01 * mic_x) + (z10 * mic_y) + (z11 * mic_x * mic_y) ) >> 6)));
            if (double_spread)
            {
                data[(point<<1)|1] = data[point<<1] = value;
            }
            else
            {
                data[point] = value;
            }
            loop_x++;
        }
        loop_y++;
    }
}

/* math_newton_root may need to be obsoleted */
n_uint math_root(n_uint input)
{
    n_uint op  = input;
    n_uint res = 0;
    n_uint one = 1uL << ((sizeof(n_uint) * 8) - 2);
    /* "one" starts at the highest power of four <= than the argument. */
    while (one > op)
    {
        one >>= 2;
    }
    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
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

n_int math_seg14(n_int character)
{
    return seg14[conv[character]];
}

n_byte * math_general_allocation(n_byte * bc0, n_byte * bc1, n_int i)
{
    if (BRAINCODE_ADDRESS(i) < BRAINCODE_SIZE)
    {
        /** address within this being */
        return &bc0[BRAINCODE_ADDRESS(i)];
    }
    /** Address within the other being */
    return &bc1[BRAINCODE_ADDRESS(i) - BRAINCODE_SIZE];
}


static n_int math_max(n_int a, n_int b)
{
    return (a<b)?b:a;
}

static n_int math_min(n_int a, n_int b)
{
    return !(b<a)?a:b;
}


/*Given three colinear points p, q, r, the function checks if point q lies on line segment 'pr' */
static n_byte  math_on_segment(n_vect2 * p, n_vect2 * q, n_vect2 * r)
{
    if ((q->x <= math_max(p->x, r->x)) &&
        (q->x >= math_min(p->x, r->x)) &&
        (q->y <= math_max(p->y, r->y)) &&
        (q->y >= math_min(p->y, r->y)))
        return 1;
    return 0;
}

/* To find orientation of ordered triplet (p, q, r).
   The function returns following values
     0 --> p, q and r are colinear
     1 --> Clockwise
     2 --> Counterclockwise
 */
static n_int math_orientation(n_vect2 * p, n_vect2 * q, n_vect2 * r)
{
    n_int val = ((q->y - p->y) *
                 (r->x - q->x)) -
                ((q->x - p->x) *
                 (r->y - q->y));
    
    if (val == 0) return 0;  /* colinear */
    
    return (val > 0)? 1: 2; /* clock or counterclock wise */
}

/* The main function that returns true if line segment 'p1q1'
   and 'p2q2' intersect. */
n_byte math_do_intersect(n_vect2 * p1, n_vect2 * q1, n_vect2 * p2, n_vect2 * q2)
{
    /* Find the four orientations needed for general and special cases */
    n_int o1 = math_orientation(p1, q1, p2);
    n_int o2 = math_orientation(p1, q1, q2);
    n_int o3 = math_orientation(p2, q2, p1);
    n_int o4 = math_orientation(p2, q2, q1);
    
    /* General case */
    if (o1 != o2 && o3 != o4) return 1;
    
    /* Special Cases */
    /* p1, q1 and p2 are colinear and p2 lies on segment p1q1 */
    if (o1 == 0 && math_on_segment(p1, p2, q1)) return 1;
    
    /* p1, q1 and p2 are colinear and q2 lies on segment p1q1 */
    if (o2 == 0 && math_on_segment(p1, q2, q1)) return 1;
    
    /* p2, q2 and p1 are colinear and p1 lies on segment p2q2 */
    if (o3 == 0 && math_on_segment(p2, p1, q2)) return 1;
    
    /* p2, q2 and q1 are colinear and q1 lies on segment p2q2 */
    if (o4 == 0 && math_on_segment(p2, q1, q2)) return 1;
    
    return 0; /* Doesn't fall in any of the above cases */
}

void math_general_execution(n_int instruction, n_int is_constant0, n_int is_constant1,
                            n_byte * addr0, n_byte * addr1, n_int value0, n_int * i,
                            n_int is_const0, n_int is_const1,
                            n_byte * pspace,
                            n_byte *bc0, n_byte *bc1,
                            n_int braincode_min_loop)
{
    /** Logical and */
    switch( instruction )
    {
    case BRAINCODE_AND:
        if (is_constant0)
        {
            addr0[0] &= addr1[0];
        }
        else
        {
            if ((addr0[0]>127) && (addr1[0]>127)) *i += BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
        /** Logical or */
    case BRAINCODE_OR:
        if (is_constant0)
        {
            addr0[0] |= addr1[0];
        }
        else
        {
            if ((addr0[0]>127) || (addr1[0]>127)) *i += BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
        /** Move a byte, with no particular alignment */
    case BRAINCODE_MOV:
        if ((!is_constant0) && (!is_constant1))
        {
            addr1[0] = addr0[0];
        }
        else
        {
            addr1[0] = (n_byte)value0;
        }
        break;
        /** Move a block of instructions */
    case BRAINCODE_MVB:
    {
        n_int ptr0, ptr1, n, instructions_to_copy, dat = 0;

        if (!is_constant0)
        {
            ptr0 = BRAINCODE_ADDRESS(*i + ((n_int)addr0[0]*BRAINCODE_BYTES_PER_INSTRUCTION));
        }
        else
        {
            ptr0 = BRAINCODE_ADDRESS(*i + ((n_int)value0*BRAINCODE_BYTES_PER_INSTRUCTION));
        }

        ptr1 = BRAINCODE_ADDRESS(*i + ((n_int)is_const0 * BRAINCODE_BYTES_PER_INSTRUCTION));

        instructions_to_copy = 1 + (pspace[1]%BRAINCODE_BLOCK_COPY);

        while (dat < instructions_to_copy)
        {
            if (ptr0 < BRAINCODE_SIZE)
            {
                addr0 = &bc0[ptr0];
            }
            else
            {
                addr0 = &bc1[ptr0 - BRAINCODE_SIZE];
            }

            if (ptr1 < BRAINCODE_SIZE)
            {
                addr1 = &bc0[ptr1];
            }
            else
            {
                addr1 = &bc1[ptr1 - BRAINCODE_SIZE];
            }

            for (n = 0; n < BRAINCODE_BYTES_PER_INSTRUCTION; n++)
            {
                addr1[n] = addr0[n];
            }
            dat++;
            ptr0 = BRAINCODE_ADDRESS(ptr0 + BRAINCODE_BYTES_PER_INSTRUCTION);
            ptr1 = BRAINCODE_ADDRESS(ptr1 + BRAINCODE_BYTES_PER_INSTRUCTION);
        }
    }
    break;

    /** Add */
    case BRAINCODE_ADD:
        if ((!is_constant0) && (!is_constant1))
        {
            addr1[0] += addr0[0];
        }
        else
        {
            addr1[0] += value0;
        }
        break;
        /** Subtract */
    case BRAINCODE_SUB:
        if ((!is_constant0) && (!is_constant1))
        {
            addr1[0] -= addr0[0];
        }
        else
        {
            addr1[0] -= value0;
        }
        break;
        /** Multiply */
    case BRAINCODE_MUL:
        if ((!is_constant0) && (!is_constant1))
        {
            addr1[0] *= addr0[0];
        }
        else
        {
            addr1[0] *= value0;
        }
        break;
        /** Divide */
    case BRAINCODE_DIV:
        if ((!is_constant0) && (!is_constant1))
        {
            addr1[0] >>= (addr0[0]%4);
        }
        else
        {
            addr1[0] >>= (value0%4);
        }
        break;
        /** Modulus */
    case BRAINCODE_MOD:
        if ((!is_constant0) && (!is_constant1))
        {
            if (addr0[0] != 0)
            {
                addr1[0] %= addr0[0];
            }
        }
        else
        {
            if (value0 != 0)
            {
                addr1[0] %= value0;
            }
        }
        break;
        /** Count up or down */
    case BRAINCODE_CTR:
        if (addr0[0] > 127)
        {
            if (addr1[0] < 255)
            {
                addr1[0]++;
            }
            else
            {
                addr1[0]=0;
            }
        }
        else
        {
            if (addr1[0] > 0)
            {
                addr1[0]--;
            }
            else
            {
                addr1[0]=255;
            }
        }
        break;
        /** Goto */
    case BRAINCODE_JMP:
    {
        n_int v0 = is_const0;
        n_int v1 = is_const1;
        n_int i2 = (*i + (((v0*256) + v1)*BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;
        if (i2 <= *i)
        {
            if ((*i-i2) < braincode_min_loop)
            {
                i2 = *i - braincode_min_loop;
                if (i2 < 0) i2 += BRAINCODE_SIZE;
            }
        }
        *i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
        break;
    }
    /** Goto if zero */
    case BRAINCODE_JMZ:
    {
        n_int v0 = is_const0;

        if (v0 == 0)
        {
            n_int i2 = (*i + ((n_int) is_const1 *BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;

            if (i2 <= *i)
            {
                if ((*i-i2) < braincode_min_loop)
                {
                    i2 = *i - braincode_min_loop;
                    if (i2 < 0) i2 += BRAINCODE_SIZE;
                }
            }
            *i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
    }
    /** Goto if not zero */
    case BRAINCODE_JMN:
    {
        n_int v0 = is_const0;
        if (v0 != 0)
        {
            n_int i2 = (*i + ((n_int) is_const1 *BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;
            if (i2 <= *i)
            {
                if ((*i-i2) < braincode_min_loop)
                {
                    i2 = *i - braincode_min_loop;
                    if (i2 < 0) i2 += BRAINCODE_SIZE;
                }
            }
            *i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
    }
    /** Goto and decrement if not zero */
    case BRAINCODE_DJN:
        if (addr0[0]-1 != 0)
        {
            n_int i2 = (*i + ((n_int) is_const1 *BRAINCODE_BYTES_PER_INSTRUCTION)) % BRAINCODE_SIZE;
            addr0[0]--;

            if (i2 <= *i)
            {
                if ((*i-i2) < braincode_min_loop)
                {
                    i2 = *i - braincode_min_loop;
                    if (i2 < 0) i2 += BRAINCODE_SIZE;
                }
            }
            *i = i2-BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        break;
        /** If two values are equal then skip the next n instructions */
    case BRAINCODE_SEQ:
        if ((!is_constant0) && (!is_constant1))
        {
            if (addr1[0] == addr0[0])
            {
                *i = (*i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
            }
        }
        else
        {
            if (addr1[0] == value0)
            {
                *i = (*i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
            }
        }
        break;
        /** If two values are not equal then skip the next n instructions */
    case BRAINCODE_SNE:
        if ((!is_constant0) && (!is_constant1))
        {
            if (addr1[0] != addr0[0])
            {
                *i = (*i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
            }
        }
        else
        {
            if (addr1[0] != value0)
            {
                *i = (*i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
            }
        }
        break;
        /** Skip the next n instructions if less than */
    case BRAINCODE_SLT:
        if ((!is_constant0) && (!is_constant1))
        {
            if (addr1[0] < addr0[0])
            {
                *i = (*i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
            }
        }
        else
        {
            if (addr1[0] < value0)
            {
                *i = (*i + (BRAINCODE_BYTES_PER_INSTRUCTION * (1 + (n_int)pspace[0]))) % BRAINCODE_SIZE;
            }
        }
        break;
        /** No operation (data only) */
    case BRAINCODE_DAT0:
    case BRAINCODE_DAT1:
        break;
        /** swap */
    case BRAINCODE_SWP:
    {
        n_byte tmp = addr0[0];
        addr0[0] = addr1[0];
        addr1[0] = tmp;
        break;
    }
    /** invert */
    case BRAINCODE_INV:
        if (is_constant0)
        {
            addr0[0] = 255 - addr0[0];
        }
        else
        {
            addr1[0] = 255 - addr1[0];
        }
        break;
        /** Save to Pspace */
    case BRAINCODE_STP:
    {
        n_byte v0 = (n_byte)is_const0;
        n_byte v1 = (n_byte)is_const1;
        pspace[v0 % BRAINCODE_PSPACE_REGISTERS] = v1;
        break;
    }
    /** Load from Pspace */
    case BRAINCODE_LTP:
    {
        n_byte v0 = (n_byte)is_const0;
        addr1[0] = pspace[v0 % BRAINCODE_PSPACE_REGISTERS];
        break;
    }
    }
}

/* graph code for 3 byte color drawing */

void graph_erase(n_byte * buffer, n_vect2 * img, n_rgba32 * color)
{
#ifdef FOUR_BYTE_COLOR
    n_byte4 * buffer4 = (n_byte4 *)buffer;
    n_int i = 0;
    while (i < img->x)
    {
        buffer4[i++] = color->thirtytwo;
    }
    i = 1;
    while (i < img->y)
    {
        memory_copy((n_byte *)buffer4, (n_byte *)&buffer4[ i++ * img->x], img->x * 4);
    }
#endif
}

/* draws a line */
void graph_line(n_byte * buffer,
                n_vect2 * img,
                n_vect2 * previous,
                n_vect2 * current,
                n_rgba32 * color,
                n_byte thickness)
{
#ifdef FOUR_BYTE_COLOR
    n_byte4 * buffer4 = (n_byte4*)buffer;
#endif
    n_int i,max;
    n_vect2 delta;
    n_vect2 absdelta;
    
    vect2_subtract(&delta, current, previous);
    
    vect2_copy(&absdelta, &delta);
    
    if (absdelta.x < 0) absdelta.x = -delta.x;
    if (absdelta.y < 0) absdelta.y = -delta.y;
    
    max = absdelta.x;
    if (absdelta.y > max) max = absdelta.y;
    
    for (i=0; i<max; i++)
    {
        n_int xx = previous->x + (i*(current->x - previous->x)/max);
        if ((xx > -1) && (xx < img->x))
        {
            n_int yy = previous->y + (i*(current->y-previous->y)/max);
            if ((yy > -1) && (yy < img->y))
            {
#ifdef FOUR_BYTE_COLOR
                n_int n = (yy*img->x + xx);
                buffer4[n] = color->thirtytwo;
#else
                n_int n = (yy*img->x + xx);
                buffer[n] = color->rgba.r;
#endif
            }
        }
    }
}


/**
 * @brief Draws a curve using three points
 * @param buffer Image buffer (three bytes per pixel)
 * @param img_width Width of the image
 * @param img_height Height of the image
 * @param x0 x coordinate of the start point
 * @param y0 y coordinate of the start point
 * @param x1 x coordinate of the middle point
 * @param y1 y coordinate of the middle point
 * @param x0 x coordinate of the end point
 * @param y0 y coordinate of the end point
 * @param r red
 * @param g green
 * @param b blue
 * @param radius_percent Radius of the curve as a percentage
 * @param start_thickness Thickness of the curve at the start point
 * @param end_thickness Thickness of the curve at the end point
 */
void graph_curve(n_byte * buffer,
                 n_vect2 * img,
                 n_vect2 * pt0,
                 n_vect2 * pt1,
                 n_vect2 * pt2,
                 n_rgba32 * color,
                 n_byte radius_percent,
                 n_uint start_thickness,
                 n_uint end_thickness)
{
    n_int pts[8];
    
    n_vect2 current;
    n_vect2 previous = {0, 0};
    
    n_uint i;
    const n_uint divisions = 20;
    n_double c[5],d[5],f;
    
    /** turn three points into four using the curve radius */
    pts[0] = pt0->x;
    pts[1] = pt0->y;
    
    pts[2] = pt1->x + ((pt0->x - pt1->x)*radius_percent/100);
    pts[3] = pt1->y + ((pt0->y - pt1->y)*radius_percent/100);
    
    pts[4] = pt1->x + ((pt2->x - pt1->x)*radius_percent/100);
    pts[5] = pt1->y + ((pt2->y - pt1->y)*radius_percent/100);
    
    pts[6] = pt2->x;
    pts[7] = pt2->y;
    
    c[0] = (-pts[0*2] + 3 * pts[1*2] - 3 * pts[2*2] + pts[3*2]) / 6.0;
    c[1] = (3 * pts[0*2] - 6 * pts[1*2] + 3 * pts[2*2]) / 6.0;
    c[2] = (-3 * pts[0*2] + 3 * pts[2*2]) / 6.0;
    c[3] = (pts[0*2] + 4 * pts[1*2] + pts[2*2]) / 6.0;
    
    d[0] = (-pts[(0*2)+1] + 3 * pts[(1*2)+1] - 3 * pts[(2*2)+1] + pts[(3*2)+1]) / 6.0;
    d[1] = (3 * pts[(0*2)+1] - 6 * pts[(1*2)+1] + 3 * pts[(2*2)+1]) / 6.0;
    d[2] = (-3 * pts[(0*2)+1] + 3 * pts[(2*2)+1]) / 6.0;
    d[3] = (pts[(0*2)+1] + 4 * pts[(1*2)+1] + pts[(2*2)+1]) / 6.0;
    
    for (i = 0; i < divisions; i++)
    {
        f = (n_double)i / (n_double)divisions;
        current.x = (n_int)((c[2] + f * (c[1] + f * c[0])) * f + c[3]);
        current.y = (n_int)((d[2] + f * (d[1] + f * d[0])) * f + d[3]);
        
        if (i > 0)
        {
            graph_line(buffer, img,
                       &previous, &current,
                       color,
                       (n_byte)(start_thickness +
                       ((end_thickness - start_thickness) * i / divisions)));
        }
        vect2_copy(&previous, &current);
    }
}

#define  MAX_POLYGON_CORNERS 1000

/**
 * @brief Draw a filled polygon
 * @param points Array containing 2D points
 * @param no_of_points The number of 2D points
 * @param r Red
 * @param g Green
 * @param b Blue
 * @param transparency Degree of transparency
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img_width Image width
 * @param img_height Image height
 */
void graph_fill_polygon(n_vect2 * points, n_int no_of_points,
                        n_rgba32 * color, n_byte transparency,
                        n_byte * buffer, n_vect2 * img)
{
#ifdef FOUR_BYTE_COLOR
    n_byte4 * buffer4 = (n_byte4*) buffer;
#endif
    n_int nodes, nodeX[MAX_POLYGON_CORNERS], i, j, swap, n, x, y;
    n_int min_x = 99999, min_y = 99999;
    n_int max_x = -99999, max_y = -99999;
    
    for (i = 0; i < no_of_points; i++)
    {
        x = points[i].x;
        y = points[i].y;
        if ((x==9999) || (y==9999)) continue;
        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
        if (x > max_x) max_x = x;
        if (y > max_y) max_y = y;
    }
    
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= img->x) max_x = img->x - 1;
    if (max_y >= img->y) max_y = img->y - 1;
    
    for (y = min_y; y <= max_y; y++)
    {
        /**  Build a list of nodes */
        nodes = 0;
        j = no_of_points-1;
        for (i = 0; i < no_of_points; i++)
        {
            if (((points[i].y < y) && (points[j].y >= y)) ||
                ((points[j].y < y) && (points[i].y >= y)))
            {
                nodeX[nodes++] =
                points[i].x + (y - points[i].y) *
                (points[j].x - points[i].x) /
                (points[j].y - points[i].y);
            }
            j = i;
            if (nodes == MAX_POLYGON_CORNERS) break;
        }
        
        /**  Sort the nodes, via a simple “Bubble” sort */
        i = 0;
        while (i < nodes-1)
        {
            if (nodeX[i] > nodeX[i+1])
            {
                swap = nodeX[i];
                nodeX[i] = nodeX[i+1];
                nodeX[i+1] = swap;
                if (i) i--;
            }
            else
            {
                i++;
            }
        }
        
        /**  Fill the pixels between node pairs */
        for (i = 0; i < nodes; i += 2)
        {
            if (nodeX[i] >= max_x) break;
            if (nodeX[i+1] > min_x)
            {
                /** range check */
                if (nodeX[i] <= min_x) nodeX[i] = min_x+1;
                if (nodeX[i+1] >= max_x) nodeX[i+1] = max_x-1;
                
                for (x = nodeX[i]; x < nodeX[i+1]; x++)
                {
#ifdef FOUR_BYTE_COLOR
                    if (transparency == 0)
                    {
                        n = ((y*img->x)+x);
                        buffer4[n] = color->thirtytwo;
                    }
                    else
                    {
                        n = ((y*img->x)+x) * 4;
                        buffer[n]   = ((color->rgba.b*(255-transparency)) + (buffer[n]*transparency))/256;
                        buffer[n+1] = ((color->rgba.g*(255-transparency)) + (buffer[n+1]*transparency))/256;
                        buffer[n+2] = ((color->rgba.r*(255-transparency)) + (buffer[n+2]*transparency))/256;
                        buffer[n+3] = 0;
                    }
#else
                    n = ((y*img->x)+x);
                    if (transparency == 0)
                    {
                        buffer[n] = color->rgba.b;
                    }
                    else
                    {
                        buffer[n]   = ((color->rgba.b*(255-transparency)) + (buffer[n]*transparency))/256;
                    }
#endif
                }
            }
        }
    }
}
