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

#define tile_wind_aim (6 + math_random(tile->genetics) % 91)

static void tile_wrap(n_c_int * section)
{
    n_int placement = 0;
    while (placement < (MAP_AREA))
    {
        n_c_int value = section[placement];
        section[placement++] = (value * 253) / 256;
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

void tile_cycle(n_tile * tile)
{
    const n_int    bits_neg = (-131072 * 254) / 256;
    const n_int    bits_pos = ( 131071 * 254) / 256;
    static n_c_int temp_atmosphere[MAP_AREA];
    static n_int   local_delta = 0;
    n_int          new_delta = 0;
    n_int          ly = 0;
    
    tile->atmosphere_lowest = bits_pos;
    tile->atmosphere_highest = bits_neg;
    
    while ( ly < MAP_DIMENSION )
    {
        n_int    ly_min = ((ly + (MAP_DIMENSION-1) ) & ((MAP_DIMENSION)-1)) * MAP_DIMENSION;
        n_int    ly_plu = ((ly + 1 ) & (MAP_DIMENSION-1)) * MAP_DIMENSION;
        n_int    ly_neu = ly * MAP_DIMENSION;
        n_int    lx = 0;
        while ( lx < MAP_DIMENSION )
        {
            n_int    simple_location = lx | ly_neu;
            n_c_int  value = tile->atmosphere[simple_location];
            
            n_int    local_atm =
                       (2 * tile->atmosphere[ lx | ly_min ])
                     + (2 * tile->atmosphere[ ((lx + (MAP_DIMENSION-1) ) & ((MAP_DIMENSION)-1)) | ly_neu ])
                     - (2 * tile->atmosphere[ ((lx + 1 ) & (MAP_DIMENSION-1)) | ly_neu ])
                     - (2 * tile->atmosphere[ lx | ly_plu ]);
            
            value += (n_c_int) ((local_atm - local_delta) >> MAP_BITS) + tile->delta_pressure[ simple_location];
            temp_atmosphere[ simple_location ] = value;
            new_delta += value;
            tile_atomosphere_range(tile, value);
            lx++;
        }
        ly++;
    }
    
    local_delta = new_delta >> MAP_BITS;
    
    if ((tile->atmosphere_lowest < bits_neg) || (tile->atmosphere_highest > bits_pos))
    {
        tile_wrap(temp_atmosphere);
    }
    
    io_copy((n_byte *)temp_atmosphere, (n_byte *)tile->atmosphere, (sizeof(n_c_int) * MAP_AREA));

}

static void title_wind_calculation(n_tile * tile)
{
    if ((math_random(tile->genetics) & 31) == 0)
    {
        tile->wind_aim_x = tile_wind_aim;
        math_random3(tile->genetics);
        tile->wind_aim_y = tile_wind_aim;
    }
    
    if (tile->wind_aim_x > tile->wind_value_x)
    {
        tile->wind_value_x++;
    }
    if (tile->wind_aim_x < tile->wind_value_x)
    {
        tile->wind_value_x--;
    }
    
    if (tile->wind_aim_y > tile->wind_value_y)
    {
        tile->wind_value_y++;
    }
    if (tile->wind_aim_y < tile->wind_value_y)
    {
        tile->wind_value_y--;
    }
}

void tile_wind(n_tile * tile)
{
    /* Add dynamic wind */
    const n_int   p01 = tile->wind_value_x;
    const n_int   p10 = tile->wind_value_y;
    static n_c_int temp_atmosphere[MAP_AREA];
    n_int         ly = 0;

    title_wind_calculation(tile);
    
    while ( ly < (MAP_DIMENSION-1) )
    {
        n_int    ly_plu = (ly + 1) << MAP_BITS;
        n_int    ly_neu = ly << MAP_BITS;
        n_int    lx = 0;
        while ( lx < (MAP_DIMENSION-1) )
        {
            n_int    calc_point = lx | ly_neu;
            n_int    delta_pressure = tile->delta_pressure[calc_point];
            n_int    tp01 = (p01 * delta_pressure) / tile->delta_pressure_highest;
            n_int    tp10 = (p10 * delta_pressure) / tile->delta_pressure_highest;
            n_int    tp00 = 256 - tp01 - tp10;
            n_int    lx_plu = (lx + 1);
            n_int    local_atm =
                        (tp00 * tile->atmosphere[ calc_point]) +
                        (tp10 * tile->atmosphere[ lx | ly_plu ]) +
                        (tp01 * tile->atmosphere[ lx_plu | ly_neu ]);
            temp_atmosphere[ calc_point ] = (n_c_int)local_atm >> 8;
            lx++;
        }
        ly++;
    }
    
    ly = MAP_DIMENSION - 1;
    
    {
        n_int    ly_plu = (0) << MAP_BITS;
        n_int    ly_neu = ly << MAP_BITS;
        n_int    lx = 0;
        while ( lx < MAP_DIMENSION )
        {
            n_int    calc_point = lx | ly_neu;
            n_int    delta_pressure = tile->delta_pressure[calc_point];
            n_int    tp01 = (p01 * delta_pressure) / tile->delta_pressure_highest;
            n_int    tp10 = (p10 * delta_pressure) / tile->delta_pressure_highest;
            n_int    tp00 = 256 - tp01 - tp10;
            n_int    lx_plu = (lx + 1 ) & (MAP_DIMENSION-1);
            n_int    local_atm =
            (tp00 * tile->atmosphere[ calc_point]) +
            (tp10 * tile->atmosphere[ lx | ly_plu ]) +
            (tp01 * tile->atmosphere[ lx_plu | ly_neu ]);
            temp_atmosphere[ calc_point ] = (n_c_int)local_atm >> 8;
            lx++;
        }
        ly++;
    }
    
    ly = 0;
    while ( ly < (MAP_DIMENSION-1) )
    {
        n_int    ly_plu = ((ly + 1 ) & (MAP_DIMENSION-1)) << MAP_BITS;
        n_int    ly_neu = ly << MAP_BITS;
        n_int    lx = (MAP_DIMENSION-1);
        {
            n_int    calc_point = lx | ly_neu;
            n_int    delta_pressure = tile->delta_pressure[calc_point];
            n_int    tp01 = (p01 * delta_pressure) / tile->delta_pressure_highest;
            n_int    tp10 = (p10 * delta_pressure) / tile->delta_pressure_highest;
            n_int    tp00 = 256 - tp01 - tp10;
            n_int    lx_plu = 0;
            n_int    local_atm =
            (tp00 * tile->atmosphere[ calc_point]) +
            (tp10 * tile->atmosphere[ lx | ly_plu ]) +
            (tp01 * tile->atmosphere[ lx_plu | ly_neu ]);
            temp_atmosphere[ calc_point ] = (n_c_int)local_atm >> 8;
            lx++;
        }
        ly++;
    }
    
    io_copy((n_byte *)temp_atmosphere, (n_byte *)tile->atmosphere, (sizeof(n_c_int) * MAP_AREA));
}

void tile_weather_init(n_tile * tile)
{
    n_int ly = 0;
    
    math_random3(tile->genetics);
    
    tile->delta_pressure_lowest = 0xffff;
    tile->delta_pressure_highest = 0;
    
    tile->wind_value_x = tile_wind_aim;
    tile->wind_aim_y = tile_wind_aim;
    math_random3(tile->genetics);
    tile->wind_value_y = tile_wind_aim;
    tile->wind_aim_x = tile_wind_aim;
    
    io_erase((n_byte *)tile->atmosphere, sizeof(n_c_int) * MAP_AREA);
    io_erase((n_byte *)tile->delta_pressure, sizeof(n_byte2) * MAP_AREA);
    
    while ( ly < MAP_AREA )
    {
        tile->atmosphere[ ly ] = (n_c_int)(tile->topology[ ly ] * 4);
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
            = (n_byte2)(tile->atmosphere[ (( lx + 1 ) & ((MAP_DIMENSION)-1)) + ly_neu]
                        - tile->atmosphere[(( lx + ((MAP_DIMENSION)-1) ) & ((MAP_DIMENSION)-1)) + ly_neu]
                        + tile->atmosphere[ lx + ly_plu ]
                        - tile->atmosphere[ lx + ly_min ]
                        + 512);
            tile->delta_pressure[ ly_neu + lx ] = value;
        
            tile_pressure_range(tile, value);
            
            lx++;
        }
        ly++;
    }
    
    ly = 0;
    while( ly < MAP_AREA)
    {
        tile->atmosphere[ ly ] = 0;
        ly++;
    }
}

void tile_pack(n_tile * tile)
{
    n_int loop = 0;
    while (loop < MAP_AREA)
    {
        tile->topology[loop] = 128;
        loop++;
    }
}

void tile_land_init(n_tile * tile)
{
    static n_byte scratch[MAP_AREA];
    n_byte2    local_random[2];
    n_int      refine = 0;
    local_random[0] = tile->genetics[0];
    local_random[1] = tile->genetics[1];
    
    tile_pack(tile);

    while (refine < 7)
    {
        math_patch(tile->topology, &math_memory_location, &math_random, local_random, refine);
        math_round(tile->topology, scratch, &math_memory_location);
        refine++;
    }
}
