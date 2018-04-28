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


/*
           +---------+
           |         |
           |    0    |
           |         |
 +---------+---------+---------+---------+
 |         |         |         |         |
 |    1    |    2    |    3    |    4    |
 |         |         |         |         |
 +---------+---------+---------+---------+
           |         |
           |    5    |
           |         |
           +---------+
 */

static void tile_resolve_coordinates(n_tile_coordinates * coordinates)
{
    n_int pos_x = (coordinates->x >= MAP_DIMENSION) - (coordinates->x < 0);
    n_int pos_y = (coordinates->y >= MAP_DIMENSION) - (coordinates->y < 0);
    
    if ((pos_x == 0) && (pos_y == 0))
    {
        return;
    }
    {
        n_int new_x = coordinates->x & (MAP_DIMENSION - 1);
        n_int new_y = coordinates->y & (MAP_DIMENSION - 1);

        if (pos_x == 0)
        {
            
        } else if (pos_y == 0)
        {
            
        } else
        {
            
        }
    }
}


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

n_byte tiles_topology(n_land * land, n_int tile, n_int lx, n_int ly)
{
    n_int converted_x = (lx + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    n_int converted_y = (ly + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    return land->tiles->topology[converted_x | (converted_y * MAP_DIMENSION)];
}

static void tiles_set_topology(n_land * land, n_int tile, n_int lx, n_int ly, n_byte value)
{
    n_int converted_x = (lx + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    n_int converted_y = (ly + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    land->tiles->topology[converted_x | (converted_y * MAP_DIMENSION)] = value;
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

        while ( lx < MAP_DIMENSION )
        {
            n_byte2 value
            = (n_byte2)(
                        tiles_atomosphere(land, 0, lx + 1, ly)
                        - tiles_atomosphere(land, 0, lx - 1, ly)
                        + tiles_atomosphere(land, 0, lx, ly + 1)
                        - tiles_atomosphere(land, 0, lx, ly - 1)
                        + 512);
            land->tiles->delta_pressure[ lx | (ly * MAP_DIMENSION) ] = value;
        
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

static void title_pack_static(n_byte * buffer)
{
    n_int loop = 0;
    while (loop < MAP_AREA)
    {
        buffer[loop] = 128;
        loop++;
    }
}

void tile_pack(n_land * land)
{
    title_pack_static(land->tiles->topology);
}

static void tile_round(n_land * land)
{
    static n_byte scratch[2][MAP_AREA];
    n_int    local_tile_dimension = 1 << MAP_BITS;
    n_int span_minor = 0;
    /** Perform four nearest neighbor blur runs */
    
    io_copy(land->tiles->topology, (n_byte*)&scratch[0], MAP_AREA);
    
    while (span_minor < 6)
    {
        n_byte    *front = (n_byte*)&scratch[(span_minor&1)], *back = (n_byte*)&scratch[(span_minor&1)^1];
        n_int    py = 0;
        
        while (py < local_tile_dimension)
        {
            n_int    px = 0;
            while (px < local_tile_dimension)
            {
                n_int    sum = 0;
                n_int    ty = -1;
                while (ty < 2)
                {
                    n_int    tx = -1;
                    while (tx < 2)
                    {
                        n_int converted_x = (px + tx + MAP_DIMENSION) & (MAP_DIMENSION - 1);
                        n_int converted_y = (py + ty + MAP_DIMENSION) & (MAP_DIMENSION - 1);
                        sum += front[ converted_x | (converted_y * MAP_DIMENSION)];
                        tx++;
                    }
                    ty++;
                }
                {
                    n_int converted_x = (px + MAP_DIMENSION) & (MAP_DIMENSION - 1);
                    n_int converted_y = (py + MAP_DIMENSION) & (MAP_DIMENSION - 1);
                    back[ converted_x | (converted_y * MAP_DIMENSION)] = (n_byte)(sum / 9);
                }
                px ++;
            }
            py ++;
        }
        span_minor ++;
    }
    io_copy((n_byte*)&scratch[1], land->tiles->topology, MAP_AREA);

}

static void tile_patch(n_land * land, n_int refine)
{
    /** size of the local tiles */
    /** number of 256 x 256 tiles in each dimension */
    const n_int local_tiles = 1 << (MAP_BITS-8);
    const n_int span_minor = (64 >> ((refine&7)^7));
    const n_int span_major = (1 << ((refine&7)^7));
    n_int tile_y = 0;
    
    /** begin the tile traversal in the y dimension */
    while (tile_y < local_tiles)
    {
        /** begin the tile traversal in the x dimension */
        n_int tile_x = 0;
        while (tile_x < local_tiles)
        {
            /** scan through the span_minor values */
            n_int    py = 0;
            while (py < span_minor)
            {
                n_int    px = 0;
                while (px < span_minor)
                {
                    /** each of the smaller tiles are based on 256 * 256 tiles */
                    n_int    val1 = ((px << 2) + (py << 10));
                    n_int    ty = 0;
                    n_int    tseed = math_random(land->tiles->genetics);
                    
                    while (ty < 4)
                    {
                        n_int    tx = 0;
                        while (tx < 4)
                        {
                            n_int    val2 = (tseed >> (tx | (ty << 2)));
                            n_int    val3 = ((((val2 & 1) << 1)-1) * 20);
                            n_int    my = 0;
                            
                            val2 = (tx | (ty << 8));
                            
                            while (my < span_major)
                            {
                                n_int    mx = 0;
                                while (mx < span_major)
                                {
                                    n_int    point = ((mx | (my << 8)) + (span_major * (val1 + val2)));
                                    n_int    pointx = (point & 255);
                                    n_int    pointy = (point >> 8);
                                    /** perform rotation on 2,3,6,7,10,11 etc */
                                    if (refine&2)
                                    {
                                        n_int pointx_tmp = pointx + pointy;
                                        pointy = pointx - pointy;
                                        pointx = pointx_tmp;
                                    }
                                    {
                                        /** include the wrap around for the 45 degree rotation cases in particular */
                                        n_int    local_map_point = tiles_topology(land, 0, pointx + (tile_x<<8), pointy + (tile_y<<8)) + val3;
                                        
                                        if (local_map_point < 0) local_map_point = 0;
                                        if (local_map_point > 255) local_map_point = 255;
                                        
                                        tiles_set_topology(land, 0, pointx + (tile_x<<8), pointy + (tile_y<<8), (n_byte)local_map_point);
                                    }
                                    mx++;
                                }
                                my++;
                            }
                            tx++;
                        }
                        ty++;
                    }
                    px++;
                }
                py++;
            }
            tile_x++;
        }
        tile_y++;
    }
}


n_int tile_memory_location(n_int px, n_int py)
{
#define    POSITIVE_TILE_COORD(num)      ((num+(3*MAP_DIMENSION))&(MAP_DIMENSION-1))
    
    return POSITIVE_TILE_COORD(px) + (POSITIVE_TILE_COORD(py) << MAP_BITS);
}

void tile_land_init(n_land * land)
{
    n_int      refine = 0;
    tile_pack(land);
    while (refine < 7)
    {
        tile_patch(land, refine);
        tile_round(land);
        refine++;
    }
}

