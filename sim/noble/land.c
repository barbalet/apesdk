/****************************************************************

 land.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

/*NOBLEMAKE DEL=""*/

#ifndef	_WIN32

#include "../noble/noble.h"

#else

#include "..\noble\noble.h"

#endif

/*NOBLEMAKE END=""*/
/*NOBLEMAKE VAR=""*/

#define		WEATHER_WRAP(x)		(((x)+(MAP_DIMENSION/2))&((MAP_DIMENSION/2)-1))
#define		WEATHER_MEM(lx, ly, back)	( (back) | (((WEATHER_WRAP(ly))<<1) | ((WEATHER_WRAP(lx))<<(MAP_BITS))) )
#define		CONST_BACK	WEATHER_MEM(0,0,1)

/*NOBLEMAKE END=""*/

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


#define WATER_EVAP_TIDE     (0)
#define WATER_EVAP_REG      (0)
#define WATER_RELEASE_RAIN  (0)

#undef WEATHER_DEBUG

#ifdef WEATHER_DEBUG

#include <stdio.h>

void weather_debug(n_weather * local_weather)
{
    n_int    lx = 0;
    n_c_int  value_max = -2147483648;
    n_c_int  value_min = 2147483647;
    n_c_int    line_average = 0;
    while (lx < (MAP_DIMENSION/2))
    {
        n_int ly = 0;
        n_c_int point_average = 0;
        while (ly < (MAP_DIMENSION/2))
        {
            n_c_int value = local_weather->atmosphere[ WEATHER_MEM(ly,lx,0) ];
            if (value > value_max)
            {
                value_max = value;
            }
            if (value < value_min)
            {
                value_min = value;
            }
            point_average += value;
            ly++;
        }
        line_average += (point_average >> (MAP_BITS-1));
        lx++;
    }
    printf("WD: range %d - %d (%d), average %d, total pressure %d WEATHER_RAIN %d, WEATHER_CLOUD %d\n",value_min, value_max, value_max - value_min, line_average>>(MAP_BITS-1),local_weather->total_pressure, WEATHER_RAIN, WEATHER_CLOUD);
}

void weather_adjust(n_weather * local_weather)
{
    n_int    lx = 0;
    n_c_int  line_average = 0;
    while (lx < (MAP_DIMENSION/2))
    {
        n_int ly = 0;
        n_c_int point_average = 0;
        while (ly < (MAP_DIMENSION/2))
        {
            n_c_int value = local_weather->atmosphere[ WEATHER_MEM(ly,lx,0) ];

            point_average += value;
            ly++;
        }
        line_average += (point_average >> (MAP_BITS-1));
        lx++;
    }    
    line_average = line_average >> (MAP_BITS-1);
    lx = 0;
    while (lx < (MAP_DIMENSION/2))
    {
        n_int ly = 0;
        while (ly < (MAP_DIMENSION/2))
        {
            local_weather->atmosphere[ WEATHER_MEM(ly,lx,0) ] -= line_average;
            ly++;
        }
        lx++;
    }
}

#endif

void weather_cycle(n_land * local_land, n_weather * local_weather)
{
    n_c_int       total_pressure = 0;
    n_int         local_delta = local_weather->total_pressure  >> (MAP_BITS-1);
    n_c_int       * atmosphere  = local_weather->atmosphere;
    n_int         lx = 0;
    n_byte        * local_map = local_land->map;
    n_byte        local_tide = local_land->tide_level;
    while (lx < MAP_DIMENSION)
    {
        n_int ly = 0;
        while (ly < MAP_DIMENSION)
        {
            n_byte  land_value = local_map[lx | (ly << MAP_BITS)];
            n_uint  location = WEATHER_MEM(ly>>1,lx>>1,0);
            
            if (land_value == local_tide)
            {
                atmosphere[ location ] += WATER_EVAP_TIDE;
            }
            else if (land_value < local_tide)
            {
                atmosphere[ location ] += WATER_EVAP_REG;
            }
            
            if ((lx < (MAP_DIMENSION/2))&&(ly < (MAP_DIMENSION/2)))
            {
                n_uint  new_location = WEATHER_MEM(ly,lx,0);
                if (atmosphere[ new_location ] >= WEATHER_RAIN)
                {
                    atmosphere[ new_location ] -= WATER_RELEASE_RAIN;
                }
            }
            
            ly++;
        }
        lx++;
    }
    
    lx = 0;
    while ( lx < (MAP_DIMENSION/2) )
    {
        n_int	lx_min = WEATHER_MEM((lx + ((MAP_DIMENSION/2)-1) ) & ((MAP_DIMENSION/2)-1), 0, 0);
        n_int	lx_plu = WEATHER_MEM((lx + 1 ) & ((MAP_DIMENSION/2)-1), 0, 0);
        n_int	lx_val = WEATHER_MEM(lx,0,0);
        n_int	ly = 0;
        while ( ly < (MAP_DIMENSION/2) )
        {
            n_int	ly_val = WEATHER_MEM(0,ly,0);
            n_int	local_atm =
                atmosphere[ lx_val | ly_val | CONST_BACK ]
                - atmosphere[ lx_plu | ly_val ]
                + atmosphere[ lx_min| ly_val ]
                - atmosphere[ lx_val| WEATHER_MEM(0, ( ly + 1 ) & ((MAP_DIMENSION/2)-1), 0 ) ]
                + atmosphere[ lx_val| WEATHER_MEM(0, ( ly + ((MAP_DIMENSION/2)-1) ) & ((MAP_DIMENSION/2)-1), 0 ) ];

            atmosphere[ WEATHER_MEM(ly,lx,0) ] += (local_atm - local_delta) >> (MAP_BITS-1);
            total_pressure += local_atm;
            ly++;
        }
        lx++;
    }
#ifdef WEATHER_DEBUG

    local_weather->total_pressure = 0; /*total_pressure;*/

#else
    
    local_weather->total_pressure = total_pressure;
    
#endif
    
#ifdef WEATHER_DEBUG
    if (local_land ->time == 1)
    {
        weather_debug(local_weather);
    }
    
    weather_adjust(local_weather);
#endif
    
}

void weather_init(n_weather * local_weather, n_land * local_land)
{
    n_byte    *land		= local_land->map;
    n_c_int	*atmosphere = local_weather->atmosphere;
    n_int	 ly = 0;

    io_erase((n_byte *)local_weather, sizeof(n_weather));

    while ( ly < (MAP_DIMENSION/2) )
    {
        n_int	lx = 0;
        while ( lx < (MAP_DIMENSION/2) )
        {
            n_uint		offset = ( lx << 1 ) + ( ly << (1+MAP_BITS) );
            n_c_int		total_land = land[ offset ]
                                     + land[ 1 + offset ]
                                     + land[ MAP_DIMENSION + offset ]
                                     + land[ MAP_DIMENSION + 1 + offset ];
            atmosphere[ WEATHER_MEM(lx, ly, 0) ] = (total_land - 512) * 256;
            lx++;
        }
        ly++;
    }
    ly=0;
    while ( ly < (MAP_DIMENSION/2) )
    {
        n_int		lx = 0;
        n_uint		ly_plu = ( ly + 1 ) & ((MAP_DIMENSION/2)-1) ;
        n_uint		ly_min = ( ly + ((MAP_DIMENSION/2)-1) ) & ((MAP_DIMENSION/2)-1) ;
        while ( lx < (MAP_DIMENSION/2) )
        {
            atmosphere[ WEATHER_MEM(lx, ly, 1) ]
                = atmosphere[ WEATHER_MEM(( lx + 1 ) & ((MAP_DIMENSION/2)-1), ly, 0) ]
                  - atmosphere[ WEATHER_MEM(( lx + ((MAP_DIMENSION/2)-1) ) & ((MAP_DIMENSION/2)-1), ly, 0) ]
                  + atmosphere[ WEATHER_MEM(lx, ly_plu, 0) ]
                  - atmosphere[ WEATHER_MEM(lx, ly_min, 0) ];
            lx++;
        }
        ly++;
    }
    ly = 0;
    while( ly < ((MAP_DIMENSION/2)*(MAP_DIMENSION/2)) )
    {
        atmosphere[ WEATHER_MEM((ly&((MAP_DIMENSION/2)-1)), (ly>>(MAP_BITS-1)), 0) ] = 0;
        ly++;
    }
    ly = 0;
    while( ly < MAP_DIMENSION )
    {
        weather_cycle(local_land, local_weather);
        ly++;
    }
}


n_int	weather_pressure(n_weather * wea, n_int px, n_int py)
{
    return  wea->atmosphere[WEATHER_MEM(px, py, 0)];
}

/**
 * Returns temperature in degrees C x 1000
 * This is an approximation of temperature which varies daily and annually
 * @param local_land Land object
 * @param wea Weather object
 * @param px x map coordinate
 * @param py y map coordinate
 * @returns Temperature in degrees C x 1000
 */
n_int	weather_temperature(n_land * local_land, n_weather * wea, n_int px, n_int py)
{
    const n_int annual_average = 18000;
    const n_int annual_temperature_variance = 3000;
    n_int daily_temperature_variance, daily_offset, annual_offset;
    n_int annual_idx, daily_idx, time_of_day = local_land->time;
    n_uint annual_time, current_time = time_of_day + (TIME_IN_DAYS(local_land->date) * TIME_DAY_MINUTES);

    annual_time =
        current_time - ((current_time/(TIME_YEAR_DAYS*TIME_DAY_MINUTES))*(TIME_YEAR_DAYS*TIME_DAY_MINUTES));
    annual_idx = (annual_time*255/(TIME_YEAR_DAYS*TIME_DAY_MINUTES)) - 64;
    if (annual_idx<0) annual_idx += 256;
    annual_offset = new_sd[annual_idx]*annual_temperature_variance/NEW_SD_MULTIPLE;

    daily_temperature_variance = 2000 + (new_sd[annual_idx]*1000/NEW_SD_MULTIPLE);
    daily_idx = (time_of_day*255/TIME_DAY_MINUTES) - 64;
    if (daily_idx<0) daily_idx += 256;
    daily_offset = new_sd[daily_idx]*daily_temperature_variance/NEW_SD_MULTIPLE;

    return annual_average + annual_offset + daily_offset - (wea->atmosphere[WEATHER_MEM(px, py, 0)]/32);
}

void weather_wind_vector(n_weather * wea, n_int px, n_int py, n_int * wind_dx, n_int * wind_dy)
{
    n_int	local_pressure = weather_pressure(wea, px, py);
    *wind_dx = local_pressure - weather_pressure(wea, px - 1, py);
    *wind_dy = local_pressure - weather_pressure(wea, px, py - 1);
}

n_int	weather_seven_values(n_land * local_land, n_weather * local_weather, n_int px, n_int py)
{
    n_byte	ret_val;
    n_int	val;
    n_int   local_time = local_land->time;
    if(IS_DAWNDUSK(local_time))
    {
        return 6;
    }
    if(IS_NIGHT(local_time))
    {
        ret_val = 3;
    }
    else
    {
        ret_val = 0;
    }

    val = weather_pressure(local_weather, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(px)) >> 1, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(py)) >> 1);

    if ( val == -1)
    {
        return -1; /* Error has already been shown */
    }
    if(val > WEATHER_RAIN)
        return ret_val+2;
    if(val > WEATHER_CLOUD)
        return ret_val+1;
    return ret_val;
}

/* Not currently used yet */
static n_int time_actual_to_perceived(n_int actual_time)
{
    if (actual_time >= DUSK_END_POINT)
    {
        n_int relative_time = actual_time - DUSK_END_POINT;
        return (238 - 16) + NIGHT_TIME_DIVISION(relative_time);
    }
    if (actual_time >= DAY_END_POINT)
    {
        n_int relative_time = actual_time - DAY_END_POINT;
        return (238 - 16 - 4) + DAWN_DUSK_TIME_DIVISION(relative_time);
    }
    if (actual_time >= DAWN_END_POINT)
    {
        n_int relative_time = actual_time - DAWN_END_POINT;
        return (238 - 16 - 4 - 192) + DAY_TIME_DIVISION(relative_time);
    }
    if (actual_time >= NIGHT_END_POINT)
    {
        n_int relative_time = actual_time - NIGHT_END_POINT;
        return (238 - 16 - 4 - 192 - 4) + DAWN_DUSK_TIME_DIVISION(relative_time);
    }
    {
        n_int relative_time = actual_time;
        return NIGHT_TIME_DIVISION(relative_time);
    }
}

static void land_tide(n_land * local_land)
{
    n_int time_of_day     = local_land->time;
    n_int current_time    = time_of_day + (TIME_IN_DAYS(local_land->date) * TIME_DAY_MINUTES);
    n_int lunar_mins      = current_time % LUNAR_ORBIT_MINS;
    n_int lunar_angle_256 = (((time_of_day * 255) / 720)+((lunar_mins * 255) / LUNAR_ORBIT_MINS));
    n_int solar_mins      =  current_time    % (TIME_DAY_MINUTES * TIME_YEAR_DAYS);
    n_int solar_angle_256 = (solar_mins * 255) / (TIME_DAY_MINUTES * TIME_YEAR_DAYS);
    n_int lunar = (new_sd[lunar_angle_256 & 255]*TIDE_AMPLITUDE_LUNAR)/NEW_SD_MULTIPLE;
    n_int solar = (new_sd[solar_angle_256]      *TIDE_AMPLITUDE_SOLAR)/NEW_SD_MULTIPLE;

    local_land->tide_level = (n_byte)(WATER_MAP + lunar + solar);
}

void land_cycle(n_land * local_land)
{
    local_land->time++;
    if (local_land->time == TIME_DAY_MINUTES)
    {
        local_land->time = 0;
        local_land->date[0]++;
        if(local_land->date[0] == TIME_CENTURY_DAYS)
        {
            local_land->date[0] = 0;
            if(local_land->date[1] != 0xffff)
                local_land->date[1]++;
            else
                local_land->date[1] = 0;
        }

    }

    land_tide(local_land);
}

#define	OPERATOR_AREA(fg, dfg, fdg)					((((dfg) * (dfg)) + ((fdg) * (fdg))) >> 6)
#define	OPERATOR_HEIGHT(fg, dfg, fdg)				(((WATER_MAP + fg) * (WATER_MAP + fg)) >> 8 )
#define OPERATOR_WATER(fg, dfg, fdg)				(((WATER_MAP - fg) * (WATER_MAP - fg)) >> 8 )
#define	OPERATOR_SUN(fg, dfg, fdg, ct, st)			(((((ct) * (fg)) + ((st) * (dfg))) >> 4) + WATER_MAP)
#define	OPERATOR_SALT(fg, dfg, fdg, fs)             (((fs*fs)+(dfg*fdg))>>4)

#define	WATER_MAP2									(WATER_MAP * 2)

static n_int land_operator(n_land * local_land, n_weather * local_weather, n_int locx, n_int locy, n_byte *specific_kind)
{
    n_int	fg  = QUICK_LAND(local_land, locx, locy);
    n_int	dfg = QUICK_LAND(local_land, locx + 1, locy);
    n_int	fdg = QUICK_LAND(local_land, locx, locy + 1);

    n_int	temp = 0, temp_add;
    n_int	number_sum = 0;

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
        if(IS_NIGHT(local_land->time) == 0)
        {
            /* 180 is minutes in the day / 8 */
            n_int hr = ((((local_land->time << 6) / 180) + 32) & 255);

            n_int weather = weather_seven_values(local_land, local_weather, MAPSPACE_TO_APESPACE(locx), MAPSPACE_TO_APESPACE(locy));

            n_int weather_divide = (105 + ((weather % 3) * 30));
            n_int ct = (840 + OLD_SD_NEW_SD(hr+64)) / weather_divide;
            n_int st = (840 + OLD_SD_NEW_SD(hr)) / weather_divide;

            number_sum ++;
            temp_add = OPERATOR_SUN(fg, dfg, fdg, ct, st); /* O */
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
    if(number_sum != 0)
    {
        temp = temp / number_sum;
    }
    return (temp);
}

n_int land_operator_interpolated(n_land * local_land, n_weather * local_weather, n_int locx, n_int locy, n_byte * kind)
{
    n_int map_x = APESPACE_TO_MAPSPACE(locx);
    n_int map_y = APESPACE_TO_MAPSPACE(locy);

    /*  Not bilinear interpolation but linear interpolation. Probably should replace with bilinear (ie each value has x and y dependency) */
    n_int interpolated;
    interpolated = APESPACE_TO_MAPSPACE(
                       land_operator(local_land, local_weather, (map_x+1)&(MAP_DIMENSION-1), map_y, kind)*(locx-MAPSPACE_TO_APESPACE(map_x)));
    interpolated += APESPACE_TO_MAPSPACE(
                        land_operator(local_land, local_weather, (map_x-1)&(MAP_DIMENSION-1), map_y, kind)*(MAPSPACE_TO_APESPACE(map_x+1)-locx));
    interpolated += APESPACE_TO_MAPSPACE(
                        land_operator(local_land, local_weather, map_x, (map_y+1)&(MAP_DIMENSION-1), kind)*(locy-MAPSPACE_TO_APESPACE(map_y)));
    interpolated += APESPACE_TO_MAPSPACE(
                        land_operator(local_land, local_weather, map_x, (map_y-1)&(MAP_DIMENSION-1), kind)*(MAPSPACE_TO_APESPACE(map_y+1)-locy));
    return interpolated >> 1;
}

void land_clear(n_land * local, KIND_OF_USE kind, n_byte2 start)
{
    n_byte *local_map = local->map;
    n_uint	loop      = 0;
    while (loop < (MAP_AREA))
    {
        local_map[loop] = 128;
        loop++;
    }
    if (kind != KIND_LOAD_FILE)
    {
        local->time = 0;
        local->date[0] = start;
        local->date[1] = 0;
    }
}

void land_init(n_land * local, n_byte * scratch)
{
    n_byte2	local_random[2];

    local_random[0] = local->genetics[0];
    local_random[1] = local->genetics[1];

    math_patch(local->map, scratch, &math_random, local_random, MAP_BITS, 0, 7, 1);

    land_tide(local);
}

void land_vect2(n_vect2 * output, n_int * actual_z, n_land * local, n_vect2 * location)
{
    n_int loc_x = location->x;
    n_int loc_y = location->y;
    n_int	z = QUICK_LAND(local, APESPACE_TO_MAPSPACE(loc_x), APESPACE_TO_MAPSPACE(loc_y));

    if (actual_z != 0L)
    {
        *actual_z = z;
    }
    output->x = (z - QUICK_LAND(local, (APESPACE_TO_MAPSPACE(loc_x) + 1), APESPACE_TO_MAPSPACE(loc_y)));
    output->y = (z - QUICK_LAND(local, APESPACE_TO_MAPSPACE(loc_x), (APESPACE_TO_MAPSPACE(loc_y) + 1)));
}
