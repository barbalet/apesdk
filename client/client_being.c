/****************************************************************

 client_being.c

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

#include "client.h"

#ifdef BRAIN_ON
n_byte * being_brain(noble_being * value)
{
    return 0L;
}
#endif

n_int being_female(noble_being * value)
{
    return 0; /*TODO: genetics handling */
}

n_int being_speaking(noble_being * value)
{
    return value->state; /* TODO: Add speaking constant check */
}

n_int being_energy(noble_being * value)
{
    return value->stored_energy;
}

n_byte being_facing(noble_being * value)
{
    return (n_byte)value->direction_facing;
}

n_genetics * being_genetics(noble_being * value)
{
    return (n_genetics *)value->genetics;
}

void being_high_res(noble_being * value, n_vect2 * vector)
{
    
}

n_byte being_line_of_sight(noble_being * local, n_vect2 * location)
{
    return 0;
}

n_int being_location_x(noble_being * value)
{
    return value->location.x;
}

n_int being_location_y(noble_being * value)
{
    return value->location.y;
}

void being_space(noble_being * value, n_vect2 * vector)
{
    vector->x = value->location.x;
    vector->y = value->location.y;
}

n_byte being_speed(noble_being * value)
{
    return (n_byte)value->velocity;
}

n_byte2 being_state(noble_being * value)
{
    return (n_byte2)value->state;
}
