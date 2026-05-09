/****************************************************************
 
 planet.c
 
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

#include "toolkit.h"
#include "script.h"
#include "sim.h"

typedef struct  {
    n_tile_coordinates coord;
    n_int speed;
} planetary_ape;

#define NUMBER_PLANETARY_APES 200

planetary_ape apes[NUMBER_PLANETARY_APES];

void planetary_ape_init(planetary_ape * value, n_byte2 * seed)
{
    value->coord.x = math_random(seed) & 255;
    value->coord.y = math_random(seed) & 255;
    value->coord.facing = math_random(seed) & 255;
    value->coord.tile = math_random(seed) % 6;
    value->speed = 1;
}

void planetary_ape_cycle(planetary_ape * ape)
{
    n_vect2 facing_vector;
    n_vect2 location_vector;

    location_vector.x = ape->coord.x;
    location_vector.y = ape->coord.y;

    vect2_direction(&facing_vector, ape->coord.facing, 32);
    vect2_d(&location_vector, &facing_vector, ape->speed, 512);
    
    ape->coord.x = location_vector.x;
    ape->coord.y = location_vector.y;
    
    tile_resolve_coordinates(&ape->coord);
}

void planet_init(n_byte2 * seed)
{
    n_int loop = 0;
    while (loop < NUMBER_PLANETARY_APES)
    {
        planetary_ape_init(&apes[loop], seed);
        loop++;
    }
}

void planet_cycle(void)
{
    n_int loop = 0;
    while (loop < NUMBER_PLANETARY_APES)
    {
        planetary_ape_cycle(&apes[loop]);
        loop++;
    }
}

void planet_draw(n_byte * buffer)
{
    n_int cell_x = 1, cell_y = 1, cell_offset_x, cell_offset_y;
    n_int location;
    n_int loop = 0;
    while (loop < NUMBER_PLANETARY_APES)
    {
        n_int tile = apes[loop].coord.tile;
        cell_x = 1;
        cell_y = 1;
        switch(tile)
        {
            case 0:
                cell_y = 0;
                break;
            case 1:
                cell_x = 0;
                break;
            case 2:
                cell_x = 1;
                break;
            case 3:
                cell_x = 2;
                break;
            case 4:
                cell_x = 3;
                break;
            case 5:
                cell_y = 2;
                break;
        }
        cell_offset_x = (cell_x * 256 * 4);
        cell_offset_y = (cell_y * 256 * 4 * 4 * 256);
        
        location = (apes[loop].coord.x * 4) | (apes[loop].coord.y * 4 * 256 * 4) | cell_offset_x | cell_offset_y;

        buffer[location | 0] = 0;
        buffer[location | 1] = 255;
        buffer[location | 2] = 0;
        buffer[location | 3] = 0;
        
        loop++;
    }
}
