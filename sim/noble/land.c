/****************************************************************

 land.c

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

#ifndef	_WIN32

#include "../noble/noble.h"

#else

#include "..\noble\noble.h"

#endif

/*
 
 Resources
 
 Wood
 Meat - Fish
 Meat - Animals
 Gold
 Silver
 Tin
 Lead
 Copper
 Stone
 Limestone
 
 Landscape Changes
 
 Path
 Road
 Hut
 Smelter/Blacksmith
 Boat Building
 Boat
 
 Posessions
 
 ?
 */

static n_byte4     m_date;                                  /* save-able */
static n_byte2     m_time;                                  /* save-able */

static n_byte      m_tide_level;                            /* generated */

static n_byte      m_topography_highdef[HI_RES_MAP_AREA * 2]; /* generated */
static n_byte4     m_highres_tide[HI_RES_MAP_AREA/32];      /* generated */

#ifdef NEW_LAND_METHOD
static n_land      m_land;

#else

static n_byte2     m_genetics[2];                      /* save-able */
static n_byte      m_topology[MAP_AREA];               /* generated */
static n_byte2     m_delta_pressure[MAP_AREA];         /* generated */
static n_c_int     m_atmosphere[MAP_AREA];             /* save-able and generate-able */

#endif

n_byte * land_topography_highdef(void)
{
    return m_topography_highdef;
}

void * land_ptr(void)
{
    return &m_date;
}

n_byte4 land_date(void)
{
    return m_date;
}

n_byte4 land_time(void)
{
    return m_time;
}

n_byte land_tide_level(void)
{
    return m_tide_level;
}


n_byte4 * land_highres_tide(void)
{
    return (n_byte4 *)m_highres_tide;
}

void land_cycle(void)
{
    m_time++;
    if (m_time == TIME_DAY_MINUTES)
    {
        m_time = 0;
        m_date++;
    }
    
    land_tide();
}

#define    OPERATOR_AREA(fg, dfg, fdg)                    ((((dfg) * (dfg)) + ((fdg) * (fdg))) >> 6)
#define    OPERATOR_HEIGHT(fg, dfg, fdg)                (((WATER_MAP + fg) * (WATER_MAP + fg)) >> 8 )
#define OPERATOR_WATER(fg, dfg, fdg)                (((WATER_MAP - fg) * (WATER_MAP - fg)) >> 8 )
#define    OPERATOR_SUN(fg, dfg, fdg, ct, st)            (((((ct) * (fg)) + ((st) * (dfg))) >> 4) + WATER_MAP)
#define    OPERATOR_SALT(fg, dfg, fdg, fs)             (((fs*fs)+(dfg*fdg))>>4)

#define    WATER_MAP2                                    (WATER_MAP * 2)

static n_int land_operator(n_int locx, n_int locy, n_byte *specific_kind)
{
    n_int    temp = 0, temp_add;
    n_int    number_sum = 0;
    
    n_int    fg;
    n_int    dfg;
    n_int    fdg;
    
    NA_ASSERT(specific_kind, "specific_kind NULL");
    
    fg  = land_location(locx, locy);
    dfg = land_location(locx + 1, locy);
    fdg = land_location(locx, locy + 1);
    
    dfg = (dfg - fg) * 8;
    fdg = (fdg - fg) * 8;
    
    fg = fg - WATER_MAP;
    
    if(specific_kind[0] != '.')
    {
        number_sum ++;
        temp_add = OPERATOR_AREA(fg, dfg, fdg); /* A */
        if(specific_kind[0] == '+')
        temp += temp_add;
        else
        temp += WATER_MAP2 - temp_add;
    }
    if(specific_kind[1] != '.')
    {
        number_sum ++;
        temp_add = OPERATOR_HEIGHT(fg, dfg, fdg); /* H */
        if(specific_kind[1] == '+')
        temp += temp_add;
        else
        temp += WATER_MAP2 - temp_add;
    }
    if(specific_kind[2] != '.')
    {
        number_sum ++;
        temp_add = OPERATOR_WATER(fg, dfg, fdg); /* W */
        if(specific_kind[2] == '+')
        temp += temp_add;
        else
        temp += WATER_MAP2 - temp_add;
    }
    if(specific_kind[3] != '.')
    {
        if(IS_NIGHT(m_time) == 0)
        {
            /* 180 is minutes in the day / 8 */
            n_int hr = ((((m_time << 6) / 180) + 32) & 255);
            
            n_int weather = weather_seven_values(MAPSPACE_TO_APESPACE(locx), MAPSPACE_TO_APESPACE(locy));
            
            n_int   weather_divide = (105 + ((weather % 3) * 30));
            n_vect2 time_weather;
            
            vect2_direction(&time_weather, hr, weather_divide * 32);
            vect2_offset(&time_weather, 840/ weather_divide, 840/ weather_divide);
            
            number_sum ++;
            temp_add = OPERATOR_SUN(fg, dfg, fdg, time_weather.x, time_weather.y); /* O */
            if(specific_kind[3] == '+')
            temp += temp_add;
            else
            temp += WATER_MAP2 - temp_add;
        }
    }
    if(specific_kind[4] != '.')
    {
        number_sum ++;
        temp_add = OPERATOR_SUN(fg, dfg, fdg, 11, 11); /* U */
        if(specific_kind[4] == '+')
        temp += temp_add;
        else
        temp += WATER_MAP2 - temp_add;
    }
    if(specific_kind[5] != '.')
    {
        n_int fs = -(fg - TIDE_AMPLITUDE_LUNAR - TIDE_AMPLITUDE_SOLAR);
        if ((fs < 0) || (fs > (TIDE_AMPLITUDE_LUNAR + TIDE_AMPLITUDE_SOLAR)*2))
        {
            if(specific_kind[5] == '+') temp=0;
        }
        else
        {
            number_sum ++;
            if(specific_kind[5] == '+')
            temp += OPERATOR_SALT(fg, dfg, fdg, fs); /* S */
        }
    }
    NA_ASSERT(number_sum, "number_sum is ZERO");
    if(number_sum != 0)
    {
        temp = temp / number_sum;
    }
    return (temp);
}

n_int land_operator_interpolated(n_int locx, n_int locy, n_byte * kind)
{
    n_int map_dimension = land_map_dimension();
    n_int map_x = APESPACE_TO_MAPSPACE(locx);
    n_int map_y = APESPACE_TO_MAPSPACE(locy);
    n_int interpolated;
    NA_ASSERT(kind, "kind NULL");
    
    /*  Not bilinear interpolation but linear interpolation. Probably should replace with bilinear (ie each value has x and y dependency) */
    interpolated  = (land_operator((map_x+1)&(map_dimension-1), map_y, kind)*(locx-(map_x << APE_TO_MAP_BIT_RATIO))) >> APE_TO_MAP_BIT_RATIO;
    interpolated += (land_operator((map_x-1)&(map_dimension-1), map_y, kind)*(((map_x+1)<<APE_TO_MAP_BIT_RATIO)-locx)) >> APE_TO_MAP_BIT_RATIO;
    interpolated += (land_operator(map_x, (map_y+1)&(map_dimension-1), kind)*(locy-(map_y<<APE_TO_MAP_BIT_RATIO))) >> APE_TO_MAP_BIT_RATIO;
    interpolated += (land_operator(map_x, (map_y-1)&(map_dimension-1), kind)*(((map_y+1)<<APE_TO_MAP_BIT_RATIO)-locy)) >> APE_TO_MAP_BIT_RATIO;
    
    return interpolated >> 1;
}

n_int land_location_vect(n_vect2 * value)
{
    return land_location(value->x, value->y);
}

weather_values    weather_seven_values(n_int px, n_int py)
{
    n_byte    ret_val;
    n_int    val;
    n_int   map_x = POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(px));
    n_int   map_y = POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(py));
    
    if(IS_DAWNDUSK(m_time))
    {
        return WEATHER_SEVEN_DAWN_DUSK;
    }
    if(IS_NIGHT(m_time))
    {
        ret_val = WEATHER_SEVEN_CLEAR_NIGHT;
    }
    else
    {
        ret_val = WEATHER_SEVEN_SUNNY_DAY;
    }
    
    val = weather_pressure(map_x, map_y);
    
    if ( val == -1)
    {
        return WEATHER_SEVEN_ERROR; /* Error has already been shown */
    }
    
    if(val > WEATHER_RAIN)
    {
        return ret_val+2;
    }
    if(val > WEATHER_CLOUD)
    {
        return ret_val+1;
    }
    
    return ret_val;
}

n_int land_map_dimension(void)
{
    return MAP_DIMENSION;
}

n_int land_map_bits(void)
{
    return MAP_BITS;
}

#ifdef NEW_LAND_METHOD

n_byte2 * land_genetics(void)
{
    return (n_byte2 *)m_land.tiles[0].genetics;
}

n_byte * land_topography(void)
{
    return (n_byte *)m_land.tiles[0].topography;
}

n_c_int * land_weather(n_int tile)
{
    return (n_c_int *)m_land.tiles[tile].atmosphere;
}

void weather_cycle(void)
{
    tile_cycle(&m_land);
/*
    tile_cycle(&m_land);
    tile_cycle(&m_land);
 */

    tile_wind(&m_land);
}

void weather_init(void)
{
    tile_weather_init(&m_land);
}

n_int weather_pressure(n_int px, n_int py)
{
    return tiles_atmosphere(&m_land, 0, 0, px, py);
}

/*
 * The weather is maintained in a 18-bit band from bits_neg
 */


void  weather_wind_vector(n_vect2 * pos, n_vect2 * wind)
{
    n_int    local_pressure;
    NA_ASSERT(pos, "pos NULL");
    NA_ASSERT(wind, "wind NULL");

    if (pos == 0L) return;
    if (wind == 0L) return;

    local_pressure = weather_pressure(pos->x, pos->y);
    wind->x = local_pressure - weather_pressure(pos->x - WEATHER_TO_MAPSPACE(1), pos->y);
    wind->y = local_pressure - weather_pressure(pos->x, pos->y  - WEATHER_TO_MAPSPACE(1));
}

n_byte * land_location_tile(n_int tile)
{
    return tiles_topography_map(&m_land, tile, 0);

}

n_int land_location(n_int px, n_int py)
{
    return tiles_topography(&m_land, 0, 0, px, py);
}

void land_tide(void)
{
    n_int current_time    = m_time + (m_date * TIME_DAY_MINUTES);
    n_int lunar_mins      = current_time % LUNAR_ORBIT_MINS;
    n_int lunar_angle_256 = (((m_time * 255) / 720)+((lunar_mins * 255) / LUNAR_ORBIT_MINS));
    n_int solar_mins      =  current_time    % (TIME_DAY_MINUTES * TIME_YEAR_DAYS);
    n_int solar_angle_256 = (solar_mins * 255) / (TIME_DAY_MINUTES * TIME_YEAR_DAYS);

    n_int lunar = math_sine(lunar_angle_256, NEW_SD_MULTIPLE / TIDE_AMPLITUDE_LUNAR);
    n_int solar = math_sine(solar_angle_256, NEW_SD_MULTIPLE / TIDE_AMPLITUDE_SOLAR);

    NA_ASSERT((((WATER_MAP + lunar + solar) > -1) && ((WATER_MAP + lunar + solar) < 256)), "(WATER_MAP + lunar + solar) outside byte boundaries");

    m_tide_level = (n_byte)(WATER_MAP + lunar + solar);
}

void land_clear(KIND_OF_USE kind, n_byte4 start)
{
    tile_land_erase(&m_land);
    if (kind != KIND_LOAD_FILE)
    {
        m_time = 0;
        m_date = start;
    }
}

void land_seed_genetics(n_byte2 * local_random)
{
    tile_land_random(&m_land, local_random);
}

void land_init(void)
{
    tile_land_erase(&m_land);
    tile_land_init(&m_land);
}

void land_init_high_def(n_byte double_spread)
{
    n_uint   lp = 0;
    n_byte4  value_setting = 0;

    math_bilinear_8_times((n_byte *)m_land.tiles[0].topography[0], m_topography_highdef, double_spread);
    memory_erase((n_byte *)m_highres_tide, sizeof(n_byte4) * HI_RES_MAP_AREA/32);

    while (lp < HI_RES_MAP_AREA)
    {
        n_byte val = m_topography_highdef[lp<<1];
        if ((val > 105) && (val < 151))
        {
            value_setting |= 1 << (lp & 31);
        }

        if ((lp & 31) == 31)
        {
            m_highres_tide[ lp >> 5 ] = value_setting;
            value_setting = 0;
        }
        lp++;

    }
}

#else


n_c_int * land_weather(n_int tile)
{
    return m_atmosphere;
}

n_byte * land_topography(void)
{
    return m_topology;
}

n_byte2 * land_genetics(void)
{
    return (n_byte2 *)m_genetics;
}

static n_int weather_delta(void)
{
    n_int    lx = 0;
    n_int    average = 0;
    n_int    map_dimensions2;
    n_int    map_bits2;
    
    map_dimensions2 = land_map_dimension() / 2;
    
    map_bits2       = land_map_bits() - 1;
    if (map_bits2 < 0) return 0;
    
    while (lx < map_dimensions2)
    {
        n_int ly = 0;
        while (ly < map_dimensions2)
        {
            average += weather_pressure(WEATHER_TO_MAPSPACE(lx), WEATHER_TO_MAPSPACE(ly));
            ly++;
        }
        lx++;
    }
    average = average >> map_bits2;
    return average;
}

/*
 * The weather is maintained in a 18-bit band from bits_neg
 */
static void weather_wrap(n_c_int * section)
{
    const n_int bits_neg = (-131072 * 254) / 256;
    const n_int bits_pos = ( 131071 * 254) / 256;
    n_int max = bits_neg;
    n_int min = bits_pos;
    {
        n_int placement = 0;
        while (placement < (MAP_AREA / 4))
        {
            n_c_int value = section[placement++];
            if (value > max)
            {
                max = value;
            }
            if (value < min)
            {
                min = value;
            }
        }
    }
    if ((min < bits_neg) || (max > bits_pos))
    {
        n_int placement = 0;
        while (placement < (MAP_AREA / 4))
        {
            n_c_int value = section[placement];
            section[placement++] = (value * 253) / 256;
        }
    }
}

void weather_cycle(void)
{
    n_int         local_delta;
    n_int         ly = 0;
    const n_int         map_dimensions = land_map_dimension();
    const n_int         map_bits = land_map_bits();
    
    if (map_bits < 0) return;
    
    local_delta = weather_delta();
    
    while ( ly < map_dimensions )
    {
        n_int    ly_min = ((ly + (map_dimensions-1) ) & ((map_dimensions)-1)) * map_dimensions;
        n_int    ly_plu = ((ly + 1 ) & (map_dimensions-1)) * map_dimensions;
        n_int    ly_neu = ly * map_dimensions;
        n_int    lx = 0;
        while ( lx < map_dimensions )
        {
            n_int    local_atm =
            ((n_int)m_delta_pressure[ lx | ly_neu ] << map_bits)
            - (512 << map_bits)
            - m_atmosphere[ ((lx + 1 ) & (map_dimensions-1)) | ly_neu ]
            + m_atmosphere[ ((lx + (map_dimensions-1) ) & ((map_dimensions)-1)) | ly_neu ]
            - m_atmosphere[ lx | ly_plu ]
            + m_atmosphere[ lx | ly_min ];
            
            m_atmosphere[ lx | ly_neu ] += (local_atm - local_delta) >> map_bits;
            lx++;
        }
        ly++;
    }
    
    weather_wrap(m_atmosphere);
}

void weather_init(void)
{
    const n_int map_dimension = land_map_dimension();
    const n_int map_bits      = land_map_bits();
    n_int ly = 0;
    
    if (map_bits < 0) return;
    
    memory_erase((n_byte *)m_atmosphere, sizeof(n_c_int) * MAP_AREA);
    memory_erase((n_byte *)m_delta_pressure, sizeof(n_byte2) * MAP_AREA);
    
    while ( ly < map_dimension )
    {
        n_int    lx = 0;
        while ( lx < map_dimension )
        {
            n_int     total_land =  land_location(lx,     ly) * 4;
            m_atmosphere[ (map_dimension * ly) + lx ] = (n_c_int)total_land;
            lx++;
        }
        ly++;
    }
    ly=0;
    while ( ly < (map_dimension) )
    {
        n_int        lx = 0;
        n_int        ly_plu = ((ly + 1 ) & ((map_dimension)-1)) * map_dimension;
        n_int        ly_min = ((ly + (map_dimension-1)) & (map_dimension-1)) * map_dimension;
        n_int      ly_neu = (ly * map_dimension);
        while ( lx < (map_dimension) )
        {
            m_delta_pressure[ ly_neu + lx ]
            = (n_byte2)(m_atmosphere[ (( lx + 1 ) & ((map_dimension)-1)) + ly_neu]
                        - m_atmosphere[(( lx + ((map_dimension)-1) ) & ((map_dimension)-1)) + ly_neu]
                        + m_atmosphere[ lx + ly_plu ]
                        - m_atmosphere[ lx + ly_min ]
                        + 512);
            lx++;
        }
        ly++;
    }
    
    ly = 0;
    while( ly < (map_dimension * map_dimension))
    {
        m_atmosphere[ ly ] = 0;
        ly++;
    }
#ifndef FAST_START_UNREALISTIC_INITIAL_WEATHER
    ly = 0;
    while( ly < (map_dimension * 2))
    {
        weather_cycle();
        ly++;
    }
#else
    ly = 0;
    while( ly < (map_dimension))
    {
        weather_cycle();
        ly++;
    }
#endif
}

n_int weather_pressure(n_int px, n_int py)
{
    n_int   dimension = land_map_dimension();
    
    n_int   tpx = ((px) + dimension) % dimension;
    n_int   tpy = ((py) + dimension) % dimension;
    
    return  m_atmosphere[(dimension * tpy) + tpx];
}

void  weather_wind_vector(n_vect2 * pos, n_vect2 * wind)
{
    n_int    local_pressure;
    NA_ASSERT(pos, "pos NULL");
    NA_ASSERT(wind, "wind NULL");
    
    if (pos == 0L) return;
    if (wind == 0L) return;
    
    local_pressure = weather_pressure(pos->x, pos->y);
    wind->x = local_pressure - weather_pressure(pos->x - WEATHER_TO_MAPSPACE(1), pos->y);
    wind->y = local_pressure - weather_pressure(pos->x, pos->y  - WEATHER_TO_MAPSPACE(1));
}

static n_int land_math_memory_location(n_int px, n_int py)
{
#define    POSITIVE_TILE_COORD(num)      ((num+(3*MAP_DIMENSION))&(MAP_DIMENSION-1))
    
    return POSITIVE_TILE_COORD(px) + (POSITIVE_TILE_COORD(py) << MAP_BITS);
}

n_int land_location(n_int px, n_int py)
{
    return m_topology[land_math_memory_location(px, py)];
}

void land_tide(void)
{
    n_int current_time    = m_time + (m_date * TIME_DAY_MINUTES);
    
    {
        n_int lunar_mins      = current_time % LUNAR_ORBIT_MINS;
        n_int lunar_angle_256 = (((m_time * 255) / 720)+((lunar_mins * 255) / LUNAR_ORBIT_MINS));
        n_int solar_mins      =  current_time    % (TIME_DAY_MINUTES * TIME_YEAR_DAYS);
        n_int solar_angle_256 = (solar_mins * 255) / (TIME_DAY_MINUTES * TIME_YEAR_DAYS);
        
        n_int lunar = math_sine(lunar_angle_256, NEW_SD_MULTIPLE / TIDE_AMPLITUDE_LUNAR);
        n_int solar = math_sine(solar_angle_256, NEW_SD_MULTIPLE / TIDE_AMPLITUDE_SOLAR);
        
        NA_ASSERT((((WATER_MAP + lunar + solar) > -1) && ((WATER_MAP + lunar + solar) < 256)), "(WATER_MAP + lunar + solar) outside byte boundaries");
        
        m_tide_level = (n_byte)(WATER_MAP + lunar + solar);
    }
}

void land_clear(KIND_OF_USE kind, n_byte4 start)
{
    n_uint    loop      = 0;
    while (loop < (MAP_AREA))
    {
        m_topology[loop] = 128;
        loop++;
    }
    if (kind != KIND_LOAD_FILE)
    {
        m_time = 0;
        m_date = start;
    }
    
}

static void land_math_round(n_byte * local_map, n_byte * scratch,
                n_memory_location * mem_func)
{
    n_int    local_tile_dimension = 1 << MAP_BITS;
    n_int span_minor = 0;
    /** Perform four nearest neighbor blur runs */
    while (span_minor < 6)
    {
        n_byte    *front, *back;
        n_int    py = 0;
        
        if ((span_minor&1) == 0)
        {
            front = local_map;
            back = scratch;
        }
        else
        {
            front = scratch;
            back = local_map;
        }
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
                        sum += front[(*mem_func)((px+tx),(py+ty))];
                        tx++;
                    }
                    ty++;
                }
                back[(*mem_func)((px),(py))] = (n_byte)(sum / 9);
                px ++;
            }
            py ++;
        }
        span_minor ++;
    }
}


/**
 * This function creates the fractal landscapes and the genetic fur patterns
 * currently.
 * @param local_map       pointer to the map array
 * @param func            the n_patch function that takes the form n_byte2 (n_patch)(n_byte2 * local)
 * @param arg             the pointer that is passed into the patch function */
static void land_math_patch(n_byte * local_map,
                n_memory_location * mem_func,
                n_patch * func, n_byte2 * arg,
                n_int refine)
{
    /** size of the local tiles */
    /** number of 256 x 256 tiles in each dimension */
    const n_int local_tiles = 1 << (MAP_BITS-8);
    const n_int span_minor = (64 >> ((refine&7)^7));
    const n_int span_major = (1 << ((refine&7)^7));
    n_int tile_y = 0;
    
    NA_ASSERT(local_map, "local_map NULL");
    NA_ASSERT(func, "func NULL");
    NA_ASSERT(arg, "arg NULL");
    
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
                    n_int    tseed = (*func)(arg);
                    
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
                                        n_int newloc = (*mem_func)((pointx + (tile_x<<8)) ,(pointy + (tile_y<<8)));
                                        n_int    local_map_point = local_map[newloc] + val3;
                                        if (local_map_point < 0) local_map_point = 0;
                                        if (local_map_point > 255) local_map_point = 255;
                                        local_map[newloc] = (n_byte)local_map_point;
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

static void land_creation(n_byte * local_map, n_byte * scratch, n_byte2 * seed)
{
    n_byte2    local_random[2];
    n_int   refine = 0;
    
    local_random[0] = seed[0];
    local_random[1] = seed[1];
    
    while (refine < 7)
    {
        land_math_patch(local_map, &land_math_memory_location, &math_random, local_random, refine);
        land_math_round(local_map, scratch, &land_math_memory_location);
        refine++;
    }
}

void land_seed_genetics(n_byte2 * local_random)
{
    m_genetics[0] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
    m_genetics[1] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
}

void land_init(void)
{
    n_byte scratch[MAP_AREA];
    n_int loop = 0;
    while (loop < MAP_AREA)
    {
        m_topology[loop] = 128;
        loop++;
    }
    land_creation(m_topology, scratch, m_genetics);
}

void land_init_high_def(n_byte double_spread)
{
    n_uint   lp = 0;
    n_byte4  value_setting = 0;
    
    math_bilinear_8_times(m_topology, m_topography_highdef, double_spread);
    memory_erase((n_byte *)m_highres_tide, sizeof(n_byte4) * HI_RES_MAP_AREA/32);
    
    while (lp < HI_RES_MAP_AREA)
    {
        n_byte val = m_topography_highdef[lp<<1];
        if ((val > 105) && (val < 151))
        {
            value_setting |= 1 << (lp & 31);
        }
        
        if ((lp & 31) == 31)
        {
            m_highres_tide[ lp >> 5 ] = value_setting;
            value_setting = 0;
        }
        lp++;
        
    }
}

#endif

void land_vect2(n_vect2 * output, n_int * actual_z, n_vect2 * location)
{
    n_int loc_x;
    n_int loc_y;
    n_int z;

    NA_ASSERT(output, "output NULL");
    NA_ASSERT(actual_z, "actual_z NULL");
    NA_ASSERT(location, "location NULL");

    if (output == 0L) return;
    if (location == 0L) return;

    loc_x = location->x;
    loc_y = location->y;

    z = land_location(APESPACE_TO_MAPSPACE(loc_x), APESPACE_TO_MAPSPACE(loc_y));

    if (actual_z != 0L)
    {
        *actual_z = z;
    }
    output->x = (z - land_location((APESPACE_TO_MAPSPACE(loc_x) + 1), APESPACE_TO_MAPSPACE(loc_y)));
    output->y = (z - land_location(APESPACE_TO_MAPSPACE(loc_x), (APESPACE_TO_MAPSPACE(loc_y) + 1)));
}

n_int spacetime_after(n_spacetime * initial, n_spacetime * second)
{
    if (initial->date < second->date)
    {
        return 0;
    }
    if (initial->date > second->date)
    {
        return 1;
    }
    if (initial->time > second->time)
    {
        return 1;
    }
    return 0;
}

n_int spacetime_before_now(n_spacetime * initial)
{
    if (initial->date > m_date)
    {
        return 0;
    }
    if (initial->date < m_date)
    {
        return 1;
    }
    if (initial->time < m_time)
    {
        return 1;
    }
    return 0;
}

void spacetime_copy(n_spacetime * to, n_spacetime * from)
{
    to->location[0] = from->location[0];
    to->location[1] = from->location[1];

    to->date = from->date;
    to->time = from->time;
}

void spacetime_set(n_spacetime * set, n_byte2 * location)
{
    set->location[0] = location[0];
    set->location[1] = location[1];
    set->time        = m_time;
    set->date        = m_date;
}

void land_convert_to_map(n_vect2 * value)
{
    value->x = APESPACE_TO_MAPSPACE(value->x);
    value->y = APESPACE_TO_MAPSPACE(value->y);
}

static n_byte2    color_group[256*3];
static n_byte     land_color_initialized = 0;

#define    SUBSTA(c)    ((c<<8)|c)

void land_color_init(void)
{
    static n_byte points[] =
    {
        0, 0, 0, 0,
        106, 43, 70, 120,
        125, 107, 201, 202,
        128, 255, 255, 239,
        150, 88, 169, 79,
        190, 8, 15, 7,
        208, 208, 216, 206,
        255, 255, 255, 255
    };
    
    /* performs a linear interpolation of n 8-bit points to 256 16-bit blend values */
    n_int    lp = 0, lp2 = 0;
    n_int    dr = 0, dg = 0, db = 0;
    n_int    ar = 0, ag = 0, ab = 0, cntr = 0;
    n_int    fp = 0, fl = 0, del_c = 0;
    while (lp < 256)
    {
        if (lp == points[cntr])
        {
            ar = SUBSTA(points[(cntr) | 1]);
            ag = SUBSTA(points[(cntr) | 2]);
            ab = SUBSTA(points[(cntr) | 3]);
            fp = lp;
            cntr += 4;
            
            if (lp != 255)
            {
                fl = points[cntr];
                del_c = (fl - fp);
                dr = SUBSTA(points[(cntr) | 1]);
                dg = SUBSTA(points[(cntr) | 2]);
                db = SUBSTA(points[(cntr) | 3]);
            }
        }
        
        if (del_c == 0)
        {
            return;
        }
        
        if (lp != 255)
        {
            n_int    del_a = (fl - lp), del_b = (lp - fp);
            
            color_group[lp2++] = (n_byte2)(((ar * del_a) + (dr * del_b)) / del_c);
            color_group[lp2++] = (n_byte2)(((ag * del_a) + (dg * del_b)) / del_c);
            color_group[lp2++] = (n_byte2)(((ab * del_a) + (db * del_b)) / del_c);
        }
        else
        {
            color_group[lp2++] = (n_byte2)(ar);
            color_group[lp2++] = (n_byte2)(ag);
            color_group[lp2++] = (n_byte2)(ab);
        }
        lp ++;
    }
    
    color_group[(COLOR_GREY*3)    ] = 0xcccc;
    color_group[(COLOR_GREY*3) + 1] = 0xcccc;
    color_group[(COLOR_GREY*3) + 2] = 0xcccc;
    
    color_group[(COLOR_YELLOW*3)    ] = 0xeeff;
    color_group[(COLOR_YELLOW*3) + 1] = 0xeeff;
    color_group[(COLOR_YELLOW*3) + 2] = 0x2222;
    
    color_group[(COLOR_RED_DARK*3)    ] = (0xeeff * 2) >> 2; /* return to * 3 following debugging */
    color_group[(COLOR_RED_DARK*3) + 1] = 0x0000;
    color_group[(COLOR_RED_DARK*3) + 2] = 0x0000;
    
    color_group[(COLOR_RED*3)    ] = 0xeeff;
    color_group[(COLOR_RED*3) + 1] = 0x0000;
    color_group[(COLOR_RED*3) + 2] = 0x0000;
}


void  land_color_time(n_byte2 * color_fit, n_int toggle_tidedaylight)
{
    n_int   day_rotation = ((land_time() * 255)/TIME_DAY_MINUTES);
    n_int   darken =  math_sine(day_rotation + 64 + 128, NEW_SD_MULTIPLE / 400);
    n_int    loop = 0;
    n_int    sign = 1;
    
    if (land_color_initialized == 0)
    {
        land_color_init();
        land_color_initialized = 1;
    }
    
    if (!toggle_tidedaylight)
    {
        if (darken < 1)
        {
            sign = -1;
        }
        darken = (darken * darken) / 402;
        darken = (sign * darken) + 624;
        
        while(loop < (COLOR_GREY * 3))
        {
            n_int cg_val = color_group[loop];
            n_int response = (cg_val * darken) >> 10;
            
            color_fit[loop] = (n_byte2)response;
            
            loop++;
        }
    }
    while(loop < (256 * 3))
    {
        color_fit[loop] = color_group[loop];
        loop++;
    }
}

