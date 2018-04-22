/****************************************************************

 tile.c

 =============================================================

 Copyright 1996-2018 Tom Barbalet. All rights reserved.

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

#include "noble.h"

#define tile_wind_aim (6 + math_random(land->tiles->genetics) % 91)

static void tile_wrap(n_c_int * section)
{
    n_int placement = 0;
    while (placement < (MAP_AREA))
    {
        n_c_int value = section[placement];
        section[placement++] = (value * 253) / 256;
    }
}

static void title_wind_calculation(n_land * land)
{
    if ((math_random(land->tiles->genetics) & 31) == 0)
    {
        land->tiles->wind_aim_x = tile_wind_aim;
        math_random3(land->tiles->genetics);
        land->tiles->wind_aim_y = tile_wind_aim;
    }
    
    if (land->tiles->wind_aim_x > land->tiles->wind_value_x)
    {
        land->tiles->wind_value_x++;
    }
    if (land->tiles->wind_aim_x < land->tiles->wind_value_x)
    {
        land->tiles->wind_value_x--;
    }
    
    if (land->tiles->wind_aim_y > land->tiles->wind_value_y)
    {
        land->tiles->wind_value_y++;
    }
    if (land->tiles->wind_aim_y < land->tiles->wind_value_y)
    {
        land->tiles->wind_value_y--;
    }
}

static void tile_pressure_range(n_tile * tile, n_byte2 value)
{
    if (value > tile->delta_pressure_highest)
    {
        tile->delta_pressure_highest = value;
    }
    if (value < tile->delta_pressure_lowest)
    {
        tile->delta_pressure_lowest = value;
    }
}

static void tile_atomosphere_range(n_tile * tile, n_c_int value)
{
    if (value > tile->atmosphere_highest)
    {
        tile->atmosphere_highest = value;
    }
    if (value < tile->atmosphere_lowest)
    {
        tile->atmosphere_lowest = value;
    }
}

static n_c_int tiles_atomosphere(n_land * land, n_int tile, n_int lx, n_int ly)
{
    n_int converted_x = (lx + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    n_int converted_y = (ly + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    return land->tiles->atmosphere[converted_x | (converted_y * MAP_DIMENSION)];
}

static n_byte2 tiles_pressure(n_land * land, n_int tile, n_int lx, n_int ly)
{
    n_int converted_x = (lx + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    n_int converted_y = (ly + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    return land->tiles->delta_pressure[converted_x | (converted_y * MAP_DIMENSION)];
}

void tile_cycle(n_land * land)
{
    const n_int    bits_neg = (-131072 * 254) / 256;
    const n_int    bits_pos = ( 131071 * 254) / 256;
    static n_c_int temp_atmosphere[1][MAP_AREA];
    static n_int   local_delta = 0;
    n_int          new_delta = 0;
    n_int          ly = 0;
    
    land->tiles->atmosphere_lowest = bits_pos;
    land->tiles->atmosphere_highest = bits_neg;
    
    while ( ly < MAP_DIMENSION )
    {
        n_int    lx = 0;
        while ( lx < MAP_DIMENSION )
        {
            n_c_int  value = tiles_atomosphere(land, 0, lx, ly);
            
            n_int    local_atm =
                       (2 * tiles_atomosphere(land, 0, lx, ly-1))
                     + (2 * tiles_atomosphere(land, 0, lx-1, ly))
                     - (2 * tiles_atomosphere(land, 0, lx+1, ly))
                     - (2 * tiles_atomosphere(land, 0, lx, ly+1));
            
            value += (n_c_int) ((local_atm - local_delta) >> MAP_BITS) + tiles_pressure(land, 0, lx, ly);
            temp_atmosphere[0][ lx | (ly * MAP_DIMENSION) ] = value;
            new_delta += value;
            tile_atomosphere_range(land->tiles, value);
            lx++;
        }
        ly++;
    }
    
    local_delta = new_delta >> MAP_BITS;
    
    if ((land->tiles->atmosphere_lowest < bits_neg) || (land->tiles->atmosphere_highest > bits_pos))
    {
        tile_wrap(temp_atmosphere[0]);
    }
    
    io_copy((n_byte *)&temp_atmosphere[0], (n_byte *)land->tiles->atmosphere, (sizeof(n_c_int) * MAP_AREA));

}

void tile_wind(n_land * land)
{
    /* Add dynamic wind */
    const n_int   p01 = land->tiles->wind_value_x;
    const n_int   p10 = land->tiles->wind_value_y;
    static n_c_int temp_atmosphere[1][MAP_AREA];
    n_int         ly = 0;

    title_wind_calculation(land);
    
    while ( ly < MAP_DIMENSION )
    {
        n_int    lx = 0;
        while ( lx < MAP_DIMENSION )
        {
            n_int    delta_pressure = tiles_pressure(land, 0, lx, ly);
            n_int    tp01 = (p01 * delta_pressure) / land->tiles->delta_pressure_highest;
            n_int    tp10 = (p10 * delta_pressure) / land->tiles->delta_pressure_highest;
            n_int    tp00 = 256 - tp01 - tp10;
            n_int    local_atm =
                        (tp00 * tiles_atomosphere(land, 0, lx, ly)) +
                        (tp10 * tiles_atomosphere(land, 0, lx, ly+1)) +
                        (tp01 * tiles_atomosphere(land, 0, lx+1, ly));
            temp_atmosphere[0][  lx | (ly * MAP_DIMENSION) ] = (n_c_int)local_atm >> 8;
            lx++;
        }
        ly++;
    }
    io_copy((n_byte *)(temp_atmosphere[0]), (n_byte *)land->tiles->atmosphere, (sizeof(n_c_int) * MAP_AREA));
}

void tile_weather_init(n_land * land)
{
    n_int ly = 0;
    
    math_random3(land->tiles->genetics);
    
    land->tiles->delta_pressure_lowest = 0xffff;
    land->tiles->delta_pressure_highest = 0;
    
    land->tiles->wind_value_x = tile_wind_aim;
    land->tiles->wind_aim_y = tile_wind_aim;
    math_random3(land->tiles->genetics);
    land->tiles->wind_value_y = tile_wind_aim;
    land->tiles->wind_aim_x = tile_wind_aim;
    
    io_erase((n_byte *)land->tiles->atmosphere, sizeof(n_c_int) * MAP_AREA);
    io_erase((n_byte *)land->tiles->delta_pressure, sizeof(n_byte2) * MAP_AREA);
    
    while ( ly < MAP_AREA )
    {
        land->tiles->atmosphere[ ly ] = (n_c_int)(land->tiles->topology[ ly ] * 4);
        ly++;
    }
    ly=0;
    while ( ly < MAP_DIMENSION )
    {
        n_int       lx = 0;
        n_uint      ly_plu = ((ly + 1 ) & ((MAP_DIMENSION)-1)) * MAP_DIMENSION;
        n_uint      ly_min = ((ly + (MAP_DIMENSION-1)) & (MAP_DIMENSION-1)) * MAP_DIMENSION;
        n_uint      ly_neu = (ly * MAP_DIMENSION);
        while ( lx < MAP_DIMENSION )
        {
            n_byte2 value
            = (n_byte2)(land->tiles->atmosphere[ (( lx + 1 ) & ((MAP_DIMENSION)-1)) + ly_neu]
                        - land->tiles->atmosphere[(( lx + ((MAP_DIMENSION)-1) ) & ((MAP_DIMENSION)-1)) + ly_neu]
                        + land->tiles->atmosphere[ lx + ly_plu ]
                        - land->tiles->atmosphere[ lx + ly_min ]
                        + 512);
            land->tiles->delta_pressure[ ly_neu + lx ] = value;
        
            tile_pressure_range(land->tiles, value);
            
            lx++;
        }
        ly++;
    }
    
    ly = 0;
    while( ly < MAP_AREA)
    {
        land->tiles->atmosphere[ ly ] = 0;
        ly++;
    }
}

void tile_pack(n_land * land)
{
    n_int loop = 0;
    while (loop < MAP_AREA)
    {
        land->tiles->topology[loop] = 128;
        loop++;
    }
}

void tile_land_init(n_land * land)
{
    static n_byte scratch[MAP_AREA];
    n_byte2    local_random[2];
    n_int      refine = 0;
    local_random[0] = land->tiles->genetics[0];
    local_random[1] = land->tiles->genetics[1];
    
    tile_pack(land);

    while (refine < 7)
    {
        math_patch(land->tiles->topology, &math_memory_location, &math_random, local_random, refine);
        math_round(land->tiles->topology, scratch, &math_memory_location);
        refine++;
    }
}
