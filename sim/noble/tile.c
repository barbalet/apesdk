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

#define bits_neg ((-131072 * 254) / 256)
#define bits_pos (( 131071 * 254) / 256)

/*         +---------+
           |    C    |
           |A   0   B|
      A    |         |    B         C
 +---------+---------+---------+---------+
 |         |         |         |         |
 |    1    |    2    |    3    |    4    |
 |         |         |         |         |
 +---------+---------+---------+---------+
      D    |         |    E         F
           |D   5   E|
           |    F    |
           +---------+                       */

static void tile_coordinate_rotate(n_tile_coordinates * coordinates, n_int rotate90, n_int tile)
{
    n_int pos_x = (coordinates->x + MAP_DIMENSION) & (MAP_DIMENSION-1);
    n_int pos_y = (coordinates->y + MAP_DIMENSION) & (MAP_DIMENSION-1);
    n_int pos_facing = coordinates->facing;
    
    if (rotate90 == 0)
    {
        coordinates->x = pos_x;
        coordinates->y = pos_y;
    }
    if (rotate90 == 1)
    {
        coordinates->facing = (pos_facing + 64) & 255;
        coordinates->x = pos_y;
        coordinates->y = MAP_DIMENSION - 1 - pos_x;
    }
    if (rotate90 == 2)
    {
        coordinates->facing = (pos_facing + 128) & 255;
        coordinates->x = MAP_DIMENSION - 1 - pos_x;
        coordinates->y = MAP_DIMENSION - 1 - pos_y;
    }
    if (rotate90 == 3)
    {
        coordinates->facing = (pos_facing + 64 + 128) & 255;
        coordinates->x = MAP_DIMENSION - 1 - pos_y;
        coordinates->y = pos_x;
    }
    coordinates->tile = tile;
}

static void tile_resolve_coordinates(n_tile_coordinates * coordinates)
{
    n_int pos_x = (coordinates->x >= MAP_DIMENSION) - (coordinates->x < 0);
    n_int pos_y = (coordinates->y >= MAP_DIMENSION) - (coordinates->y < 0);
    
    if ((pos_x == 0) && (pos_y == 0))
    {
        return;
    }
    if (pos_y == 0)
    {
        if ((coordinates->tile > 0) && (coordinates->tile < 5))
        {
            n_int new_x = (coordinates->x + MAP_DIMENSION) & (MAP_DIMENSION - 1);
            n_int new_tile = coordinates->tile;
            if (pos_x < 0)
            {
                if (new_tile == 1) /* moving leftwards */
                {
                    new_tile = 4;
                }
                else
                {
                    new_tile--;
                }
            }
            else
            {
                if (new_tile == 4) /* moving rightwards */
                {
                    new_tile = 1;
                }
                else
                {
                    new_tile++;
                }
            }
            coordinates->tile = new_tile;
            coordinates->x = new_x;
        }
        else
        {
            if (coordinates->tile == 0)
            {
                if (pos_x < 0)
                {
                    /* A */ tile_coordinate_rotate(coordinates, 3, 1);
                }
                else
                {
                    /* B */ tile_coordinate_rotate(coordinates, 1, 3);
                }
            }
            else /* coordinates->tile == 5 */
            {
                if (pos_x < 0)
                {
                    /* D */ tile_coordinate_rotate(coordinates, 3, 1);
                }
                else
                {
                    /* E */ tile_coordinate_rotate(coordinates, 1, 3);
                }
            }
/*         +---------+
           |    C    |
           |A   0   B|
      A    |         |    B         C
 +---------+---------+---------+---------+
 |         |         |         |         |
 |    1    |    2    |    3    |    4    |
 |         |         |         |         |
 +---------+---------+---------+---------+
      D    |         |    E         F
           |D   5   E|
           |    F    |
           +---------+                       */
        }
    }
    else if (pos_x == 0)
    {
        if ((coordinates->tile == 0) || (coordinates->tile == 2) || (coordinates->tile == 5))
        {
            n_int new_y = (coordinates->y + MAP_DIMENSION) & (MAP_DIMENSION - 1);
            n_int new_tile = -1;
            if (pos_y < 0)
            {
                if (coordinates->tile == 0)
                {
                    /* C */ tile_coordinate_rotate(coordinates, 2, 4);
                }
                if (coordinates->tile == 2)
                {
                    new_tile = 0;
                }
                if (coordinates->tile == 5)
                {
                    new_tile = 2;
                }
            }
            else
            {
                if (coordinates->tile == 0)
                {
                    new_tile = 2;
                }
                if (coordinates->tile == 2)
                {
                    new_tile = 5;
                }
                if (coordinates->tile == 5)
                {
                    /* F */ tile_coordinate_rotate(coordinates, 2, 4);
                }
            }
            coordinates->tile = new_tile;
            coordinates->y = new_y;
        }
        else
        {
/*         +---------+
           |    C    |
           |A   0   B|
      A    |         |    B         C
 +---------+---------+---------+---------+
 |         |         |         |         |
 |    1    |    2    |    3    |    4    |
 |         |         |         |         |
 +---------+---------+---------+---------+
      D    |         |    E         F
           |D   5   E|
           |    F    |
           +---------+                       */
            
            if (pos_y < 0)
            {
                if (coordinates->tile == 1)
                {
                    /* A */ tile_coordinate_rotate(coordinates, 1, 0);
                }
                if (coordinates->tile == 3)
                {
                    /* B */ tile_coordinate_rotate(coordinates, 3, 0);
                }
                if (coordinates->tile == 4)
                {
                    /* C */ tile_coordinate_rotate(coordinates, 2, 0);
                }
            }
            else
            {
                if (coordinates->tile == 1)
                {
                    /* D */ tile_coordinate_rotate(coordinates, 3, 5);
                }
                if (coordinates->tile == 3)
                {
                    /* E */ tile_coordinate_rotate(coordinates, 1, 5);
                }
                if (coordinates->tile == 4)
                {
                    /* F */ tile_coordinate_rotate(coordinates, 2, 5);
                }
            }
            
        }
    }
    else
    {
        SHOW_ERROR("Shouldn't be needed for landscape creation");
    }
}


static void tile_wrap(n_land * land, n_int tile)
{
    n_c_int * section = land->tiles[tile].atmosphere[1];
    n_int placement = 0;
    while (placement < (MAP_AREA))
    {
        n_c_int value = section[placement];
        section[placement++] = (value * 253) / 256;
    }
}

static void title_wind_calculation(n_land * land, n_int tile)
{
    if ((math_random(land->tiles[tile].genetics) & 31) == 0)
    {
        land->tiles[tile].wind_aim_x = tile_wind_aim;
        math_random3(land->tiles[tile].genetics);
        land->tiles[tile].wind_aim_y = tile_wind_aim;
    }
    
    if (land->tiles[tile].wind_aim_x > land->tiles[tile].wind_value_x)
    {
        land->tiles[tile].wind_value_x++;
    }
    if (land->tiles[tile].wind_aim_x < land->tiles[tile].wind_value_x)
    {
        land->tiles[tile].wind_value_x--;
    }
    
    if (land->tiles[tile].wind_aim_y > land->tiles[tile].wind_value_y)
    {
        land->tiles[tile].wind_value_y++;
    }
    if (land->tiles[tile].wind_aim_y < land->tiles[tile].wind_value_y)
    {
        land->tiles[tile].wind_value_y--;
    }
}

static void tile_pressure_range(n_tile * tile, n_byte2 value)
{
    n_int tiles = 0;
    while (tiles < 1)
    {
        if (value > tile->delta_pressure_highest)
        {
            tile->delta_pressure_highest = value;
        }
        if (value < tile->delta_pressure_lowest)
        {
            tile->delta_pressure_lowest = value;
        }
        tiles++;
    }
}

static void tile_atmosphere_range(n_land * land, n_int tile, n_c_int value)
{
    n_tile * tilePtr = &land->tiles[tile];
    
    if (value > tilePtr->atmosphere_highest)
    {
        tilePtr->atmosphere_highest = value;
    }
    if (value < tilePtr->atmosphere_lowest)
    {
        tilePtr->atmosphere_lowest = value;
    }
}

static n_uint tiles_non_planet(n_int lx, n_int ly)
{
    n_int converted_x = (lx + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    n_int converted_y = (ly + MAP_DIMENSION) & (MAP_DIMENSION - 1);
    return converted_x | (converted_y * MAP_DIMENSION);
}

n_c_int tiles_atmosphere(n_land * land, n_int tile, n_int buffer, n_int lx, n_int ly)
{
    return land->tiles[tile].atmosphere[buffer][tiles_non_planet(lx, ly)];
}

static void tiles_set_atmosphere(n_land * land, n_int tile, n_int buffer, n_int lx, n_int ly, n_c_int value)
{
    land->tiles[tile].atmosphere[buffer][tiles_non_planet(lx, ly)] = value;
}

static void tiles_swap_atmosphere(n_land * land, n_int tile)
{
    io_copy((n_byte *)land->tiles[tile].atmosphere[1], (n_byte *)land->tiles[tile].atmosphere[0], (sizeof(n_c_int) * MAP_AREA));
}

static n_byte2 tiles_pressure(n_land * land, n_int tile, n_int lx, n_int ly)
{
    return land->tiles[tile].delta_pressure[tiles_non_planet(lx, ly)];
}

static void tiles_set_pressure(n_land * land, n_int tile, n_int lx, n_int ly, n_byte2 value)
{
    land->tiles[tile].delta_pressure[tiles_non_planet(lx, ly)] = value;
}

n_byte tiles_topology(n_land * land, n_int tile, n_int buffer, n_int lx, n_int ly)
{
    return land->tiles[tile].topology[0][tiles_non_planet(lx, ly)];
}

static void tiles_set_topology(n_land * land, n_int tile, n_int buffer, n_int lx, n_int ly, n_byte value)
{
    land->tiles[tile].topology[buffer][tiles_non_planet(lx, ly)] = value;
}

static void tiles_swap_topology(n_land * land, n_int tile)
{
    io_copy((n_byte *)&land->tiles[tile].topology[0], (n_byte *)&land->tiles[tile].topology[1], MAP_AREA);
}

static void title_pack_atmosphere(n_land * land, n_int tile)
{
    n_int loop = 0;
    while (loop < MAP_AREA)
    {
        land->tiles[tile].atmosphere[0][loop] = 0;
        loop++;
    }
}

static void title_pack_topology(n_land * land, n_int tile)
{
    n_int loop = 0;
    while (loop < MAP_AREA)
    {
        land->tiles[tile].topology[0][loop] = 128;
        loop++;
    }
}

static void tile_atmosphere_topology(n_land * land, n_int tile)
{
    n_int loop = 0;

    while ( loop < MAP_AREA )
    {
        land->tiles[tile].atmosphere[0][ loop ] = (n_c_int)(land->tiles[tile].topology[0][ loop ] * 4);
        loop++;
    }
}

void tile_cycle(n_land * land, n_int tile)
{
    n_int          new_delta = 0;
    n_int          ly = 0;
    
    land->tiles[tile].atmosphere_lowest = bits_pos;
    land->tiles[tile].atmosphere_highest = bits_neg;
    
    while ( ly < MAP_DIMENSION )
    {
        n_int    lx = 0;
        while ( lx < MAP_DIMENSION )
        {
            n_c_int  value = tiles_atmosphere(land, tile, 0, lx, ly);
            
            n_int    local_atm =
                       (2 * tiles_atmosphere(land, tile, 0, lx, ly-1))
                     + (2 * tiles_atmosphere(land, tile, 0, lx-1, ly))
                     - (2 * tiles_atmosphere(land, tile, 0, lx+1, ly))
                     - (2 * tiles_atmosphere(land, tile, 0, lx, ly+1));
            
            value += (n_c_int) ((local_atm - land->tiles[tile].local_delta) >> MAP_BITS) + tiles_pressure(land, 0, lx, ly);
            
            tiles_set_atmosphere(land, tile, 1, lx, ly, value);
            new_delta += value;
            tile_atmosphere_range(land, tile, value);
            lx++;
        }
        ly++;
    }
    
    land->tiles[tile].local_delta = new_delta >> MAP_BITS;
}

void tile_cycle_cleanup(n_land * land, n_int tile, n_int wind)
{
    if (wind != 0)
    {
        if ((land->tiles[tile].atmosphere_lowest < bits_neg) || (land->tiles[tile].atmosphere_highest > bits_pos))
        {
            tile_wrap(land, tile);
        }
    }
    tiles_swap_atmosphere(land, tile);
}

void tile_wind(n_land * land, n_int tile)
{
    /* Add dynamic wind */
    const n_int   p01 = land->tiles[tile].wind_value_x;
    const n_int   p10 = land->tiles[tile].wind_value_y;
    n_int         ly = 0;

    title_wind_calculation(land, tile);
    
    while ( ly < MAP_DIMENSION )
    {
        n_int    lx = 0;
        while ( lx < MAP_DIMENSION )
        {
            n_int    delta_pressure = tiles_pressure(land, tile, lx, ly);
            n_int    tp01 = (p01 * delta_pressure) / land->tiles->delta_pressure_highest;
            n_int    tp10 = (p10 * delta_pressure) / land->tiles->delta_pressure_highest;
            n_int    tp00 = 256 - tp01 - tp10;
            n_int    local_atm =
                        (tp00 * tiles_atmosphere(land, tile, 0,lx, ly)) +
                        (tp10 * tiles_atmosphere(land, tile, 0,lx, ly+1)) +
                        (tp01 * tiles_atmosphere(land, tile, 0,lx+1, ly));
            
            tiles_set_atmosphere(land, tile, 1, lx, ly, (n_c_int)local_atm >> 8);
            
            lx++;
        }
        ly++;
    }
}

void tile_weather_init(n_land * land, n_int tile)
{
    n_tile * tilePtr = &land->tiles[tile];
    
    n_int ly = 0;
    
    math_random3(tilePtr->genetics);
    
    tilePtr->local_delta = 0;
    
    tilePtr->delta_pressure_lowest = 0xffff;
    tilePtr->delta_pressure_highest = 0;
    
    tilePtr->wind_value_x = tile_wind_aim;
    tilePtr->wind_aim_y = tile_wind_aim;
    math_random3(tilePtr->genetics);
    tilePtr->wind_value_y = tile_wind_aim;
    tilePtr->wind_aim_x = tile_wind_aim;
    
    io_erase((n_byte *)tilePtr->atmosphere, sizeof(n_c_int) * MAP_AREA);
    io_erase((n_byte *)tilePtr->delta_pressure, sizeof(n_byte2) * MAP_AREA);

    tile_atmosphere_topology(land, tile);

    ly=0;
    while ( ly < MAP_DIMENSION )
    {
        n_int       lx = 0;

        while ( lx < MAP_DIMENSION )
        {
            n_byte2 value
            = (n_byte2)(
                        tiles_atmosphere(land, tile, 0, lx + 1, ly)
                        - tiles_atmosphere(land, tile, 0, lx - 1, ly)
                        + tiles_atmosphere(land, tile, 0, lx, ly + 1)
                        - tiles_atmosphere(land, tile, 0, lx, ly - 1)
                        + 512);
            tiles_set_pressure(land, tile, lx, ly, value);
            
            tile_pressure_range(land->tiles, value);
            
            lx++;
        }
        ly++;
    }
    title_pack_atmosphere(land, tile);
}


void tile_land_erase(n_land * land, n_int tile)
{
    title_pack_topology(land, tile);
}

static void tile_round(n_land * land, n_int tile)
{
    n_int    local_tile_dimension = 1 << MAP_BITS;
    n_int    span_minor = 0;
    /** Perform four nearest neighbor blur runs */
    
    while (span_minor < 6)
    {
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
                        sum += tiles_topology(land, tile, (span_minor&1), px + tx, py + ty);
                        tx++;
                    }
                    ty++;
                }
                tiles_set_topology(land, tile, (span_minor&1)^1, px, py, (n_byte)(sum / 9));
                px ++;
            }
            py ++;
        }
        span_minor ++;
    }
}

static void tile_patch(n_land * land, n_int tile, n_int refine)
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
                    n_int    tseed = math_random(land->tiles[tile].genetics);
                    
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
                                        n_int    local_map_point = tiles_topology(land, tile, 0, pointx + (tile_x<<8), pointy + (tile_y<<8)) + val3;
                                        
                                        if (local_map_point < 0) local_map_point = 0;
                                        if (local_map_point > 255) local_map_point = 255;
                                        
                                        tiles_set_topology(land, tile, 0, pointx + (tile_x<<8), pointy + (tile_y<<8), (n_byte)local_map_point);
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

void tile_land_init(n_land * land, n_int tile)
{
    n_int refine = 0;
    while (refine < 7)
    {
        tile_patch(land, tile, refine);
        tile_round(land, tile);
        tiles_swap_topology(land, tile);
        refine++;
    }
}

