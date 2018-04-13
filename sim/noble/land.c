/****************************************************************

 land.c

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
static n_byte2     m_genetics[2];                           /* save-able */
static n_byte2     m_time;                                  /* save-able */

static n_byte      m_topology[MAP_AREA];                    /* generated */
static n_byte      m_topology_highdef[HI_RES_MAP_AREA * 2]; /* generated */
static n_byte4     m_highres_tide[HI_RES_MAP_AREA/32];      /* generated */
static n_byte2     m_delta_pressure[ MAP_AREA];         /* generated */
static n_c_int	   m_atmosphere[ MAP_AREA];             /* save-able and generate-able */
static n_byte      m_tide_level;                            /* generated */
static n_byte      m_wind_value_x; /* 6 to 96 */
static n_byte      m_wind_value_y; /* 6 to 96 */
static n_byte      m_wind_aim_x;  /* 6 to 96 */
static n_byte      m_wind_aim_y;  /* 6 to 96 */

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

n_byte2 * land_genetics(void)
{
    return (n_byte2 *)m_genetics;
}

n_byte land_tide_level(void)
{
    return m_tide_level;
}

n_byte * land_topology_highdef(void)
{
    return (n_byte *)m_topology_highdef;
}

n_byte * land_topology(void)
{
    return (n_byte *)m_topology;
}

n_byte4 * land_highres_tide(void)
{
    return (n_byte4 *)m_highres_tide;
}

n_c_int * land_weather(void)
{
    return (n_c_int *)m_atmosphere;
}


static n_byte weather_wind_aim(void)
{
    return 6 + math_random(m_genetics) % 91;
}

/*
 * The weather is maintained in a 18-bit band from bits_neg
 */
static void weather_wrap(n_c_int * section)
{
    n_int placement = 0;
    while (placement < (MAP_AREA))
    {
        n_c_int value = section[placement];
        section[placement++] = (value * 253) / 256;
    }
}

void weather_cycle(void)
{
    const n_int bits_neg = (-131072 * 254) / 256;
    const n_int bits_pos = ( 131071 * 254) / 256;
    n_int max = bits_neg;
    n_int min = bits_pos;
    static n_int  local_delta = 0;
    n_int         new_delta = 0;
    n_int         map_dimensions = land_map_dimension();
    n_int         map_bits = land_map_bits();
    n_int         ly = 0;

    if (map_bits < 0) return;
    
    while ( ly < map_dimensions )
    {
        n_int	ly_min = ((ly + (map_dimensions-1) ) & ((map_dimensions)-1)) * map_dimensions;
        n_int	ly_plu = ((ly + 1 ) & (map_dimensions-1)) * map_dimensions;
        n_int	ly_neu = ly * map_dimensions;
        n_int	lx = 0;
        while ( lx < map_dimensions )
        {
            n_int   simple_location = lx | ly_neu;
            n_int	local_atm =
              (2 * m_atmosphere[ lx | ly_min ])
            + (2 * m_atmosphere[ ((lx + (map_dimensions-1) ) & ((map_dimensions)-1)) | ly_neu ])
            - (2 * m_atmosphere[ ((lx + 1 ) & (map_dimensions-1)) | ly_neu ])
            - (2 * m_atmosphere[ lx | ly_plu ]);
            n_c_int value = (n_c_int) ((local_atm - local_delta) >> map_bits) + m_delta_pressure[ simple_location];
            
            m_atmosphere[ simple_location ] += value;
            value = m_atmosphere[ simple_location ];
            new_delta += value;
            if (value < min)
            {
                min = value;
            }
            else if (value > max)
            {
                max = value;
            }

            lx++;
        }
        ly++;
    }
    
    local_delta = new_delta >> map_bits;
    
    if ((min < bits_neg) || (max > bits_pos))
    {
        weather_wrap(m_atmosphere);
    }
}

void weather_wind(void)
{
    n_int         ly = 0;
    n_int         map_dimensions = land_map_dimension();
    n_int         map_bits = land_map_bits();
    
    /* Add dynamic wind */
    const n_int   p01 = m_wind_value_x;
    const n_int   p10 = m_wind_value_y;
    const n_int   p00 = 256 - p01 - p10;
    const n_int   delta = 1;
    static n_c_int temp_atmosphere[MAP_AREA];
    

    if ((math_random(m_genetics) & 31) == 0)
    {
        m_wind_aim_x = weather_wind_aim();
        math_random3(m_genetics);
        m_wind_aim_y = weather_wind_aim();
    }

    if (m_wind_aim_x > m_wind_value_x)
    {
        m_wind_value_x++;
    }
    if (m_wind_aim_x < m_wind_value_x)
    {
        m_wind_value_x--;
    }
    
    if (m_wind_aim_y > m_wind_value_y)
    {
        m_wind_value_y++;
    }
    if (m_wind_aim_y < m_wind_value_y)
    {
        m_wind_value_y--;
    }
    
    while ( ly < map_dimensions )
    {
        n_int    ly_plu = ((ly + delta ) & (map_dimensions-1)) << map_bits;
        n_int    ly_neu = ly << map_bits;
        n_int    lx = 0;
        while ( lx < map_dimensions )
        {
            n_int    lx_plu = (lx + delta ) & (map_dimensions-1);
            n_int    local_atm =
            (p00 * m_atmosphere[ lx | ly_neu ]) +
            (p10 * m_atmosphere[ lx | ly_plu ]) +
            (p01 * m_atmosphere[ lx_plu | ly_neu ]);
            temp_atmosphere[ lx | ly_neu ] = (n_c_int)local_atm >> 8;
            lx++;
        }
        ly++;
    }
    io_copy((n_byte *)temp_atmosphere, (n_byte *)m_atmosphere, (sizeof(n_c_int) * MAP_AREA));
}

void weather_init(void)
{
    n_int map_dimension = land_map_dimension();
    n_int map_bits      = land_map_bits();
    n_int ly = 0;

    math_random3(m_genetics);
    
    m_wind_value_x = weather_wind_aim();
    m_wind_aim_y = weather_wind_aim();
    math_random3(m_genetics);
    m_wind_value_y = weather_wind_aim();
    m_wind_aim_x = weather_wind_aim();
    
    if (map_bits < 0) return;

    io_erase((n_byte *)m_atmosphere, sizeof(n_c_int) * MAP_AREA);
    io_erase((n_byte *)m_delta_pressure, sizeof(n_byte2) * MAP_AREA);

    while ( ly < map_dimension )
    {
        n_int	lx = 0;
        while ( lx < map_dimension )
        {
            m_atmosphere[ (map_dimension * ly) + lx ] = (n_c_int)(land_location(lx,ly)*4);
            lx++;
        }
        ly++;
    }
    ly=0;
    while ( ly < (map_dimension) )
    {
        n_int		lx = 0;
        n_uint		ly_plu = ((ly + 1 ) & ((map_dimension)-1)) * map_dimension;
        n_uint		ly_min = ((ly + (map_dimension-1)) & (map_dimension-1)) * map_dimension;
        n_uint      ly_neu = (ly * map_dimension);
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

}

n_int weather_pressure(n_int px, n_int py)
{
    n_int   dimension = land_map_dimension();

    n_int   tpx = (MAPSPACE_TO_WEATHER(px) + dimension) % dimension;
    n_int   tpy = (MAPSPACE_TO_WEATHER(py) + dimension) % dimension;

    return  m_atmosphere[(dimension * tpy) + tpx];
}

void  weather_wind_vector(n_vect2 * pos, n_vect2 * wind)
{
    n_int	local_pressure;
    NA_ASSERT(pos, "pos NULL");
    NA_ASSERT(wind, "wind NULL");

    if (pos == 0L) return;
    if (wind == 0L) return;

    local_pressure = weather_pressure(pos->x, pos->y);
    wind->x = local_pressure - weather_pressure(pos->x - WEATHER_TO_MAPSPACE(1), pos->y);
    wind->y = local_pressure - weather_pressure(pos->x, pos->y  - WEATHER_TO_MAPSPACE(1));
}

weather_values	weather_seven_values(n_int px, n_int py)
{
    n_byte	ret_val;
    n_int	val;
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

    if (val > WEATHER_RAIN)
    {
        return ret_val+2;
    }
    if (val > WEATHER_CLOUD)
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


n_int land_location(n_int px, n_int py)
{
    return m_topology[math_memory_location(px, py)];
}

n_int land_location_vect(n_vect2 * value)
{
    return land_location(value->x, value->y);
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

#define	OPERATOR_AREA(fg, dfg, fdg)					((((dfg) * (dfg)) + ((fdg) * (fdg))) >> 6)
#define	OPERATOR_HEIGHT(fg, dfg, fdg)				(((WATER_MAP + fg) * (WATER_MAP + fg)) >> 8 )
#define OPERATOR_WATER(fg, dfg, fdg)				(((WATER_MAP - fg) * (WATER_MAP - fg)) >> 8 )
#define	OPERATOR_SUN(fg, dfg, fdg, ct, st)			(((((ct) * (fg)) + ((st) * (dfg))) >> 4) + WATER_MAP)
#define	OPERATOR_SALT(fg, dfg, fdg, fs)             (((fs*fs)+(dfg*fdg))>>4)

#define	WATER_MAP2									(WATER_MAP * 2)

static n_int land_operator(n_int locx, n_int locy, n_byte *specific_kind)
{
    n_int	temp = 0, temp_add;
    n_int	number_sum = 0;

    n_int	fg;
    n_int	dfg;
    n_int	fdg;

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

void land_clear(KIND_OF_USE kind, n_byte4 start)
{
    n_uint	loop      = 0;
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

void land_creation(n_byte * local_map, n_byte * scratch, n_byte2 * seed, execute_thread_stub * exec)
{
    n_byte2	local_random[2];
    n_int   refine = 0;

    local_random[0] = seed[0];
    local_random[1] = seed[1];

    while (refine < 7)
    {
        math_patch(local_map, &math_memory_location, &math_random, local_random, refine);
#if 0
        math_round(local_map, scratch, &math_memory_location, exec);
#else
        math_round(local_map, scratch, &math_memory_location);
#endif
        refine++;
    }
}

void land_seed_genetics(n_byte2 * local_random)
{
    m_genetics[0] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
    m_genetics[1] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
}

void land_init(n_byte * scratch, execute_thread_stub * exec)
{
    math_pack(m_topology, scratch);
    land_creation(m_topology, scratch, m_genetics, exec);
}

void land_init_high_def(n_byte double_spread)
{
    n_uint   lp = 0;
    n_byte4  value_setting = 0;

    math_bilinear_8_times(m_topology, m_topology_highdef, double_spread);
    io_erase((n_byte *)m_highres_tide, sizeof(n_byte4) * HI_RES_MAP_AREA/32);

    while (lp < HI_RES_MAP_AREA)
    {
        n_byte val = m_topology_highdef[lp<<1];
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
