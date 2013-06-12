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

#define	WEATHER_WRAP(map_dim2, x)		    (((x)+(map_dim2))&((map_dim2)-1))
#define	WEATHER_MEM(map_dim2, lx, ly, back)	((back) | (((WEATHER_WRAP(map_dim2, ly))<<1) | ((WEATHER_WRAP(map_dim2, lx))<<(MAP_BITS))) )
#define	CONST_BACK(map_dim2)	WEATHER_MEM(map_dim2, 0,0,1)

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

static n_int weather_delta(n_land * local_land, n_weather * local_weather)
{
    n_int    lx = 0;
    n_int    average = 0;
    n_int    map_dimensions2;
    NA_ASSERT(local_land, "local_weather NULL");
    NA_ASSERT(local_weather, "local_weather NULL");
    
    map_dimensions2 = land_map_dimension(local_land) / 2;
    
    while (lx < map_dimensions2)
    {
        n_int ly = 0;
        while (ly < map_dimensions2)
        {
            average += local_weather->atmosphere[ WEATHER_MEM(map_dimensions2, ly, lx, 0) ];
            ly++;
        }
        lx++;
    }
    average = average >> (MAP_BITS-1);
    return average;
}

void weather_cycle(n_land * local_land, n_weather * local_weather)
{
    n_int         local_delta;
    n_c_int       * atmosphere;
    n_int         lx = 0;
    n_int         map_dimensions2;
    
    NA_ASSERT(local_land, "local_land NULL");
    NA_ASSERT(local_weather, "local_weather NULL");

    map_dimensions2 = land_map_dimension(local_land)/ 2;
    
    local_delta = weather_delta(local_land, local_weather);
    atmosphere  = local_weather->atmosphere;
    
    NA_ASSERT(atmosphere, "atmosphere NULL");
    
    while ( lx < map_dimensions2 )
    {
        n_int	lx_min = WEATHER_MEM(map_dimensions2, (lx + (map_dimensions2-1) ) & ((map_dimensions2)-1), 0, 0);
        n_int	lx_plu = WEATHER_MEM(map_dimensions2, (lx + 1 ) & (map_dimensions2-1), 0, 0);
        n_int	lx_val = WEATHER_MEM(map_dimensions2, lx,0,0);
        n_int	ly = 0;
        while ( ly < map_dimensions2 )
        {
            n_int	ly_val = WEATHER_MEM(map_dimensions2, 0,ly,0);
            n_int	local_atm =
                atmosphere[ lx_val | ly_val | CONST_BACK(map_dimensions2) ]
                - atmosphere[ lx_plu | ly_val ]
                + atmosphere[ lx_min| ly_val ]
                - atmosphere[ lx_val| WEATHER_MEM(map_dimensions2, 0, ( ly + 1 ) & (map_dimensions2-1), 0 ) ]
                + atmosphere[ lx_val| WEATHER_MEM(map_dimensions2, 0, ( ly + (map_dimensions2-1) ) & (map_dimensions2-1), 0 ) ];

            atmosphere[ WEATHER_MEM(map_dimensions2, ly,lx,0) ] += (local_atm - local_delta) >> (MAP_BITS-1);
            ly++;
        }

        lx++;
    }
}


void weather_init(n_weather * local_weather, n_land * local_land)
{
    n_byte    *land;
    n_c_int	  *atmosphere;
    n_int	   ly = 0;
    n_int      map_dimension2;
    NA_ASSERT(local_land, "local_land NULL");
    NA_ASSERT(local_weather, "local_weather NULL");
    
    land		= local_land->map;
    atmosphere = local_weather->atmosphere;
    map_dimension2 = land_map_dimension(local_land)/2;
    
    
    NA_ASSERT(land, "land NULL");
    NA_ASSERT(atmosphere, "atmosphere NULL");

    io_erase((n_byte *)local_weather, sizeof(n_weather));

    while ( ly < map_dimension2 )
    {
        n_int	lx = 0;
        while ( lx < map_dimension2 )
        {
            n_uint		offset = ( lx << 1 ) + ( ly << (1+MAP_BITS) );
            n_c_int		total_land = land[ offset ]
                                     + land[ 1 + offset ]
                                     + land[ (map_dimension2*2) + offset ]
                                     + land[ (map_dimension2*2) + 1 + offset ];
            atmosphere[ WEATHER_MEM(map_dimension2, lx, ly, 0) ] = (total_land - 512) * 256;
            lx++;
        }
        ly++;
    }
    ly=0;
    while ( ly < (map_dimension2) )
    {
        n_int		lx = 0;
        n_uint		ly_plu = ( ly + 1 ) & ((map_dimension2)-1) ;
        n_uint		ly_min = ( ly + (map_dimension2-1) ) & (map_dimension2-1) ;
        while ( lx < (map_dimension2) )
        {
            atmosphere[ WEATHER_MEM(map_dimension2, lx, ly, 1) ]
                = atmosphere[ WEATHER_MEM(map_dimension2, ( lx + 1 ) & ((map_dimension2)-1), ly, 0) ]
                  - atmosphere[ WEATHER_MEM(map_dimension2, ( lx + ((map_dimension2)-1) ) & ((map_dimension2)-1), ly, 0) ]
                  + atmosphere[ WEATHER_MEM(map_dimension2, lx, ly_plu, 0) ]
                  - atmosphere[ WEATHER_MEM(map_dimension2, lx, ly_min, 0) ];
            lx++;
        }
        ly++;
    }
    ly = 0;
    while( ly < (map_dimension2 * map_dimension2)) 
    {
        atmosphere[ WEATHER_MEM(map_dimension2, (ly&((map_dimension2)-1)), (ly>>(MAP_BITS-1)), 0) ] = 0;
        ly++;
    }
    ly = 0;
    while( ly < (map_dimension2 * 2))
    {
        weather_cycle(local_land, local_weather);
        ly++;
    }
}

n_int weather_pressure(n_weather * wea, n_int px, n_int py, n_int dimension2)
{
    NA_ASSERT(wea, "wea NULL");
    return  wea->atmosphere[WEATHER_MEM(dimension2, px, py, 0)];
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
    n_int map_dimensions2 = land_map_dimension(local_land)/2;
    const n_int annual_average = 18000;
    const n_int annual_temperature_variance = 3000;
    n_int daily_temperature_variance, daily_offset, annual_offset;
    n_int annual_idx, daily_idx, time_of_day;
    n_uint annual_time, current_time;

    NA_ASSERT(local_land, "local_land NULL");
    NA_ASSERT(wea, "wea NULL");
    
    time_of_day = local_land->time;
    current_time = time_of_day + (TIME_IN_DAYS(local_land->date) * TIME_DAY_MINUTES);
    
    annual_time = current_time - ((current_time/(TIME_YEAR_DAYS*TIME_DAY_MINUTES))*(TIME_YEAR_DAYS*TIME_DAY_MINUTES));
    annual_idx = (annual_time*255/(TIME_YEAR_DAYS*TIME_DAY_MINUTES)) - 64;
    if (annual_idx<0) annual_idx += 256;
    annual_offset = new_sd[annual_idx]*annual_temperature_variance/NEW_SD_MULTIPLE;

    daily_temperature_variance = 2000 + (new_sd[annual_idx]*1000/NEW_SD_MULTIPLE);
    daily_idx = (time_of_day*255/TIME_DAY_MINUTES) - 64;
    if (daily_idx<0) daily_idx += 256;
    daily_offset = new_sd[daily_idx]*daily_temperature_variance/NEW_SD_MULTIPLE;

    return annual_average + annual_offset + daily_offset - (wea->atmosphere[WEATHER_MEM(map_dimensions2, px, py, 0)]/32);
}

void  weather_wind_vector(n_land * local_land, n_weather * wea, n_int px, n_int py, n_int * wind_dx, n_int * wind_dy)
{
    n_int   map_dimensions2 = land_map_dimension(local_land)/2;
    n_int	local_pressure;
    NA_ASSERT(wea, "wea NULL");
    NA_ASSERT(wind_dx, "wind_dx NULL");
    NA_ASSERT(wind_dy, "wind_dy NULL");

    local_pressure = weather_pressure(wea, (px>>1), (py>>1), map_dimensions2);
    *wind_dx = local_pressure - weather_pressure(wea, (px>>1) - 1, (py>>1), map_dimensions2);
    *wind_dy = local_pressure - weather_pressure(wea, (px>>1), (py>>1) - 1, map_dimensions2);
}

n_int	weather_seven_values(n_land * local_land, n_weather * local_weather, n_int px, n_int py)
{
    n_byte	ret_val;
    n_int	val;
    n_int   local_time;
    n_int   map_dimension2 = land_map_dimension(local_land);
    NA_ASSERT(local_land, "local_land NULL");
    NA_ASSERT(local_weather, "local_weather NULL");
    
    local_time = local_land->time;
    
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

    val = weather_pressure(local_weather, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(px)) >> 1, POSITIVE_LAND_COORD(APESPACE_TO_MAPSPACE(py)) >> 1, map_dimension2);

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

n_int land_map_dimension(n_land * land)
{
    (void)land; /* land is not used here */
    return MAP_DIMENSION;
}

static void land_tide(n_land * local_land)
{
    n_int time_of_day;
    n_int current_time;;
    
    NA_ASSERT(local_land, "local_land NULL");
    
    time_of_day     = local_land->time;
    current_time    = time_of_day + (TIME_IN_DAYS(local_land->date) * TIME_DAY_MINUTES);
    
    {
        n_int lunar_mins      = current_time % LUNAR_ORBIT_MINS;
        n_int lunar_angle_256 = (((time_of_day * 255) / 720)+((lunar_mins * 255) / LUNAR_ORBIT_MINS));
        n_int solar_mins      =  current_time    % (TIME_DAY_MINUTES * TIME_YEAR_DAYS);
        n_int solar_angle_256 = (solar_mins * 255) / (TIME_DAY_MINUTES * TIME_YEAR_DAYS);
        
        n_int lunar = (new_sd[lunar_angle_256 & 255]*TIDE_AMPLITUDE_LUNAR)/NEW_SD_MULTIPLE;
        n_int solar = (new_sd[solar_angle_256]      *TIDE_AMPLITUDE_SOLAR)/NEW_SD_MULTIPLE;

        NA_ASSERT((((WATER_MAP + lunar + solar) > -1) && ((WATER_MAP + lunar + solar) < 256)), "(WATER_MAP + lunar + solar) outside byte boundaries");
        
        local_land->tide_level = (n_byte)(WATER_MAP + lunar + solar);
    }
}

void land_cycle(n_land * local_land)
{
    NA_ASSERT(local_land, "local_land NULL");

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
    n_int	temp = 0, temp_add;
    n_int	number_sum = 0;
    
    n_int	fg;
    n_int	dfg;
    n_int	fdg;
    
    NA_ASSERT(local_land, "local_land NULL");
    NA_ASSERT(local_weather, "local_weather NULL");
    NA_ASSERT(specific_kind, "specific_kind NULL");
    
    fg  = QUICK_LAND(local_land, locx, locy);
    dfg = QUICK_LAND(local_land, locx + 1, locy);
    fdg = QUICK_LAND(local_land, locx, locy + 1);
    
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
    NA_ASSERT(number_sum, "number_sum is ZERO");
    if(number_sum != 0)
    {
        temp = temp / number_sum;
    }
    return (temp);
}

n_int land_operator_interpolated(n_land * local_land, n_weather * local_weather, n_int locx, n_int locy, n_byte * kind)
{
    NA_ASSERT(local_land, "local_land NULL");
    NA_ASSERT(local_weather, "local_weather NULL");
    NA_ASSERT(kind, "kind NULL");

    {
        n_int map_dimension = land_map_dimension(local_land);
        n_int map_x = APESPACE_TO_MAPSPACE(locx);
        n_int map_y = APESPACE_TO_MAPSPACE(locy);

        /*  Not bilinear interpolation but linear interpolation. Probably should replace with bilinear (ie each value has x and y dependency) */
        n_int interpolated;
        interpolated = APESPACE_TO_MAPSPACE(
                           land_operator(local_land, local_weather, (map_x+1)&(map_dimension-1), map_y, kind)*(locx-MAPSPACE_TO_APESPACE(map_x)));
        interpolated += APESPACE_TO_MAPSPACE(
                            land_operator(local_land, local_weather, (map_x-1)&(map_dimension-1), map_y, kind)*(MAPSPACE_TO_APESPACE(map_x+1)-locx));
        interpolated += APESPACE_TO_MAPSPACE(
                            land_operator(local_land, local_weather, map_x, (map_y+1)&(map_dimension-1), kind)*(locy-MAPSPACE_TO_APESPACE(map_y)));
        interpolated += APESPACE_TO_MAPSPACE(
                            land_operator(local_land, local_weather, map_x, (map_y-1)&(map_dimension-1), kind)*(MAPSPACE_TO_APESPACE(map_y+1)-locy));
        return interpolated >> 1;
    }
}

void land_clear(n_land * local, KIND_OF_USE kind, n_byte2 start)
{
    NA_ASSERT(local, "local NULL");
    {
        n_byte *local_map = local->map;
        n_uint	loop      = 0;
        NA_ASSERT(local_map, "local_map NULL");
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
}

void land_init(n_land * local, n_byte * scratch)
{
    n_byte2	local_random[2];

    NA_ASSERT(local, "local NULL");
    NA_ASSERT(scratch, "scratch NULL");
    
    local_random[0] = local->genetics[0];
    local_random[1] = local->genetics[1];

    math_patch(local->map, scratch, &math_random, local_random, MAP_BITS, 0, 7, 1);

    land_tide(local);
}

void land_vect2(n_vect2 * output, n_int * actual_z, n_land * local, n_vect2 * location)
{
    n_int loc_x;
    n_int loc_y;
    n_int z;

    NA_ASSERT(output, "output NULL");
    NA_ASSERT(actual_z, "actual_z NULL");
    NA_ASSERT(local, "local NULL");
    NA_ASSERT(location, "location NULL");

    loc_x = location->x;
    loc_y = location->y;
    z = QUICK_LAND(local, APESPACE_TO_MAPSPACE(loc_x), APESPACE_TO_MAPSPACE(loc_y));

    if (actual_z != 0L)
    {
        *actual_z = z;
    }
    output->x = (z - QUICK_LAND(local, (APESPACE_TO_MAPSPACE(loc_x) + 1), APESPACE_TO_MAPSPACE(loc_y)));
    output->y = (z - QUICK_LAND(local, APESPACE_TO_MAPSPACE(loc_x), (APESPACE_TO_MAPSPACE(loc_y) + 1)));
}
