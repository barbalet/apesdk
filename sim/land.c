/****************************************************************

 land.c

 =============================================================

 Copyright 1996-2022 Tom Barbalet. All rights reserved.

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

#include "../toolkit/toolkit.h"

#include "sim.h"

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

static n_land      m_land;

n_byte *land_topography_highdef( void )
{
    return m_land.topography_highdef;
}

n_land *land_ptr( void )
{
    return &m_land;
}

n_byte4 land_date( void )
{
    return m_land.date;
}

n_byte4 land_time( void )
{
    return m_land.time;
}

n_byte land_tide_level( void )
{
    return m_land.tide_level;
}


n_byte4 *land_highres_tide( void )
{
    return ( n_byte4 * )m_land.highres_tide;
}

void land_cycle( void )
{
    m_land.time++;
    if ( m_land.time == TIME_DAY_MINUTES )
    {
        m_land.time = 0;
        m_land.date++;
    }

    land_tide();
}

/* all this hardcoding will need to be de-hardcoded in the future */
void math_bilinear_8_times( n_byte *side512, n_byte *data, n_byte double_spread )
{
    n_int loop_y = 0;

    NA_ASSERT( side512, "side512 NULL" );
    NA_ASSERT( data, "data NULL" );

    if ( side512 == 0L )
    {
        return;
    }
    if ( data == 0L )
    {
        return;
    }

    while ( loop_y < HI_RES_MAP_DIMENSION )
    {
        n_int loop_x = 0;
        while ( loop_x < HI_RES_MAP_DIMENSION )
        {
            /* find the micro x (on the map used for bilinear interpolation) */
            n_int mic_x = ( loop_x & 7 );
            /* find the micro y (on the map used for bilinear interpolation) */
            n_int mic_y = ( loop_y & 7 );

            n_int mac_x = ( loop_x >> 3 );
            n_int mac_y = ( loop_y >> 3 );

            n_uint px0 = ( n_uint )( mac_x );
            n_uint py0 = ( n_uint )( mac_y * MAP_DIMENSION );

            n_uint px1 = ( mac_x + 1 ) & ( MAP_DIMENSION - 1 );
            n_uint py1 = ( ( mac_y + 1 ) & ( MAP_DIMENSION - 1 ) ) * MAP_DIMENSION;

            n_int z00 = side512[px0 | py0];

            n_int z01 = side512[px1 | py0];
            n_int z10 = side512[px0 | py1] - z00;
            n_int z11 = side512[px1 | py1] - z01 - z10;
            n_uint point = ( n_uint )( loop_x + ( loop_y * HI_RES_MAP_DIMENSION ) );
            n_byte value;

            z01 = ( z01 - z00 ) << 3;
            z10 = z10 << 3;

            value = ( n_byte )( ( z00 + ( ( ( z01 * mic_x ) + ( z10 * mic_y ) + ( z11 * mic_x * mic_y ) ) >> 6 ) ) );
            if ( double_spread )
            {
                data[( point << 1 ) | 1] = data[point << 1] = value;
            }
            else
            {
                data[point] = value;
            }
            loop_x++;
        }
        loop_y++;
    }
}

#define    OPERATOR_AREA(fg, dfg, fdg)                 ((((dfg) * (dfg)) + ((fdg) * (fdg))) >> 6)
#define    OPERATOR_HEIGHT(fg, dfg, fdg)               (((WATER_MAP + fg) * (WATER_MAP + fg)) >> 8 )
#define    OPERATOR_WATER(fg, dfg, fdg)                (((WATER_MAP - fg) * (WATER_MAP - fg)) >> 8 )
#define    OPERATOR_SUN(fg, dfg, fdg, ct, st)          (((((ct) * (fg)) + ((st) * (dfg))) >> 4) + WATER_MAP)
#define    OPERATOR_SALT(fg, dfg, fdg, fs)             (((fs*fs)+(dfg*fdg))>>4)

#define    WATER_MAP2                                  (WATER_MAP * 2)

static n_int land_operator( n_int locx, n_int locy, n_byte *specific_kind )
{
    n_int    temp = 0, temp_add;
    n_int    number_sum = 0;

    n_int    fg;
    n_int    dfg;
    n_int    fdg;

    NA_ASSERT( specific_kind, "specific_kind NULL" );

    fg  = land_location( locx, locy );
    dfg = land_location( locx + 1, locy );
    fdg = land_location( locx, locy + 1 );

    dfg = ( dfg - fg ) * 8;
    fdg = ( fdg - fg ) * 8;

    fg = fg - WATER_MAP;

    if ( specific_kind[0] != '.' )
    {
        number_sum ++;
        temp_add = OPERATOR_AREA( fg, dfg, fdg ); /* A */
        if ( specific_kind[0] == '+' )
        {
            temp += temp_add;
        }
        else
        {
            temp += WATER_MAP2 - temp_add;
        }
    }
    if ( specific_kind[1] != '.' )
    {
        number_sum ++;
        temp_add = OPERATOR_HEIGHT( fg, dfg, fdg ); /* H */
        if ( specific_kind[1] == '+' )
        {
            temp += temp_add;
        }
        else
        {
            temp += WATER_MAP2 - temp_add;
        }
    }
    if ( specific_kind[2] != '.' )
    {
        number_sum ++;
        temp_add = OPERATOR_WATER( fg, dfg, fdg ); /* W */
        if ( specific_kind[2] == '+' )
        {
            temp += temp_add;
        }
        else
        {
            temp += WATER_MAP2 - temp_add;
        }
    }
    if ( specific_kind[3] != '.' )
    {
        if ( IS_NIGHT( m_land.time ) == 0 )
        {
            /* 180 is minutes in the day / 8 */
            n_int hr = ( ( ( ( m_land.time << 6 ) / 180 ) + 32 ) & 255 );

            n_int weather = weather_seven_values( MAPSPACE_TO_APESPACE( locx ), MAPSPACE_TO_APESPACE( locy ) );

            n_int   weather_divide = ( 105 + ( ( weather % 3 ) * 30 ) );
            n_vect2 time_weather;

            vect2_direction( &time_weather, hr, weather_divide * 32 );
            vect2_offset( &time_weather, 840 / weather_divide, 840 / weather_divide );

            number_sum ++;
            temp_add = OPERATOR_SUN( fg, dfg, fdg, time_weather.x, time_weather.y ); /* O */
            if ( specific_kind[3] == '+' )
            {
                temp += temp_add;
            }
            else
            {
                temp += WATER_MAP2 - temp_add;
            }
        }
    }
    if ( specific_kind[4] != '.' )
    {
        number_sum ++;
        temp_add = OPERATOR_SUN( fg, dfg, fdg, 11, 11 ); /* U */
        if ( specific_kind[4] == '+' )
        {
            temp += temp_add;
        }
        else
        {
            temp += WATER_MAP2 - temp_add;
        }
    }
    if ( specific_kind[5] != '.' )
    {
        n_int fs = -( fg - TIDE_AMPLITUDE_LUNAR - TIDE_AMPLITUDE_SOLAR );
        if ( ( fs < 0 ) || ( fs > ( TIDE_AMPLITUDE_LUNAR + TIDE_AMPLITUDE_SOLAR ) * 2 ) )
        {
            if ( specific_kind[5] == '+' )
            {
                temp = 0;
            }
        }
        else
        {
            number_sum ++;
            if ( specific_kind[5] == '+' )
            {
                temp += OPERATOR_SALT( fg, dfg, fdg, fs );    /* S */
            }
        }
    }
    NA_ASSERT( number_sum, "number_sum is ZERO" );
    if ( number_sum != 0 )
    {
        temp = temp / number_sum;
    }
    return ( temp );
}

n_int land_operator_interpolated( n_int locx, n_int locy, n_byte *kind )
{
    n_int map_dimension = land_map_dimension();
    n_int map_x = APESPACE_TO_MAPSPACE( locx );
    n_int map_y = APESPACE_TO_MAPSPACE( locy );
    n_int interpolated;
    NA_ASSERT( kind, "kind NULL" );

    /*  Not bilinear interpolation but linear interpolation. Probably should replace with bilinear (ie each value has x and y dependency) */
    interpolated  = ( land_operator( ( map_x + 1 ) & ( map_dimension - 1 ), map_y, kind ) * ( locx - ( map_x << APE_TO_MAP_BIT_RATIO ) ) ) >> APE_TO_MAP_BIT_RATIO;
    interpolated += ( land_operator( ( map_x - 1 ) & ( map_dimension - 1 ), map_y, kind ) * ( ( ( map_x + 1 ) << APE_TO_MAP_BIT_RATIO ) - locx ) ) >> APE_TO_MAP_BIT_RATIO;
    interpolated += ( land_operator( map_x, ( map_y + 1 ) & ( map_dimension - 1 ), kind ) * ( locy - ( map_y << APE_TO_MAP_BIT_RATIO ) ) ) >> APE_TO_MAP_BIT_RATIO;
    interpolated += ( land_operator( map_x, ( map_y - 1 ) & ( map_dimension - 1 ), kind ) * ( ( ( map_y + 1 ) << APE_TO_MAP_BIT_RATIO ) - locy ) ) >> APE_TO_MAP_BIT_RATIO;

    return interpolated >> 1;
}

n_int land_location_vect( n_vect2 *value )
{
    return land_location( value->x, value->y );
}

weather_values    weather_seven_values( n_int px, n_int py )
{
    n_byte    ret_val;
    n_int    val;
    n_int   map_x = POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( px ) );
    n_int   map_y = POSITIVE_LAND_COORD( APESPACE_TO_MAPSPACE( py ) );

    if ( IS_DAWNDUSK( m_land.time ) )
    {
        return WEATHER_SEVEN_DAWN_DUSK;
    }
    if ( IS_NIGHT( m_land.time ) )
    {
        ret_val = WEATHER_SEVEN_CLEAR_NIGHT;
    }
    else
    {
        ret_val = WEATHER_SEVEN_SUNNY_DAY;
    }

    val = weather_pressure( map_x, map_y );

    if ( val == -1 )
    {
        return WEATHER_SEVEN_ERROR; /* Error has already been shown */
    }

    if ( val > WEATHER_RAIN )
    {
        return ret_val + 2;
    }
    if ( val > WEATHER_CLOUD )
    {
        return ret_val + 1;
    }

    return ret_val;
}

n_int land_map_dimension( void )
{
    return MAP_DIMENSION;
}

n_int land_map_bits( void )
{
    return MAP_BITS;
}


n_byte2 *land_genetics( void )
{
    return ( n_byte2 * )m_land.tiles[0].genetics;
}

n_byte *land_topography( void )
{
    return ( n_byte * )m_land.tiles[0].topography;
}

n_c_int *land_weather( n_int tile )
{
    return ( n_c_int * )m_land.tiles[tile].atmosphere;
}

void weather_cycle( void )
{
    tile_cycle( &m_land );
    /*
        tile_cycle(&m_land);
        tile_cycle(&m_land);
     */

    tile_wind( &m_land );
}

void weather_init( void )
{
    tile_weather_init( &m_land );
}

n_int weather_pressure( n_int px, n_int py )
{
    return tiles_atmosphere( &m_land, 0, 0, px, py );
}

/*
 * The weather is maintained in a 18-bit band from bits_neg
 */


void  weather_wind_vector( n_vect2 *pos, n_vect2 *wind )
{
    n_int    local_pressure;
    NA_ASSERT( pos, "pos NULL" );
    NA_ASSERT( wind, "wind NULL" );

    if ( pos == 0L )
    {
        return;
    }
    if ( wind == 0L )
    {
        return;
    }

    local_pressure = weather_pressure( pos->x, pos->y );
    wind->x = local_pressure - weather_pressure( pos->x - WEATHER_TO_MAPSPACE( 1 ), pos->y );
    wind->y = local_pressure - weather_pressure( pos->x, pos->y  - WEATHER_TO_MAPSPACE( 1 ) );
}

n_byte *land_location_tile( n_int tile )
{
    return tiles_topography_map( &m_land, tile, 0 );

}

n_int land_location( n_int px, n_int py )
{
    return tiles_topography( &m_land, 0, 0, px, py );
}

void land_tide( void )
{
    n_int current_time    = m_land.time + ( m_land.date * TIME_DAY_MINUTES );
    n_int lunar_mins      = current_time % LUNAR_ORBIT_MINS;
    n_int lunar_angle_256 = ( ( ( m_land.time * 255 ) / 720 ) + ( ( lunar_mins * 255 ) / LUNAR_ORBIT_MINS ) );
    n_int solar_mins      =  current_time    % ( TIME_DAY_MINUTES * TIME_YEAR_DAYS );
    n_int solar_angle_256 = ( solar_mins * 255 ) / ( TIME_DAY_MINUTES * TIME_YEAR_DAYS );

    n_int lunar = math_sine( lunar_angle_256, NEW_SD_MULTIPLE / TIDE_AMPLITUDE_LUNAR );
    n_int solar = math_sine( solar_angle_256, NEW_SD_MULTIPLE / TIDE_AMPLITUDE_SOLAR );

    NA_ASSERT( ( ( ( WATER_MAP + lunar + solar ) > -1 ) && ( ( WATER_MAP + lunar + solar ) < 256 ) ), "(WATER_MAP + lunar + solar) outside byte boundaries" );

    m_land.tide_level = ( n_byte )( WATER_MAP + lunar + solar );
}

void land_clear( KIND_OF_USE kind, n_byte4 start )
{
    tile_land_erase( &m_land );
    if ( kind != KIND_LOAD_FILE )
    {
        m_land.time = 5 * TIME_HOUR_MINUTES;
        m_land.date = start;
    }
}

void land_seed_genetics( n_byte2 *local_random )
{
    tile_land_random( &m_land, local_random );
}

void land_init( void )
{
    tile_land_init( &m_land );
}

void land_init_high_def( n_byte double_spread )
{
    n_uint   lp = 0;
    n_byte4  value_setting = 0;

    math_bilinear_8_times( ( n_byte * )m_land.tiles[0].topography[0], m_land.topography_highdef, double_spread );
    memory_erase( ( n_byte * )m_land.highres_tide, sizeof( n_byte4 ) * HI_RES_MAP_AREA / 32 );

    while ( lp < HI_RES_MAP_AREA )
    {
        n_byte val = m_land.topography_highdef[lp << 1];
        if ( ( val > 105 ) && ( val < 151 ) )
        {
            value_setting |= 1 << ( lp & 31 );
        }

        if ( ( lp & 31 ) == 31 )
        {
            m_land.highres_tide[ lp >> 5 ] = value_setting;
            value_setting = 0;
        }
        lp++;

    }
}


void land_vect2( n_vect2 *output, n_int *actual_z, n_vect2 *location )
{
    n_int loc_x;
    n_int loc_y;
    n_int z;

    NA_ASSERT( output, "output NULL" );
    NA_ASSERT( actual_z, "actual_z NULL" );
    NA_ASSERT( location, "location NULL" );

    if ( output == 0L )
    {
        return;
    }
    if ( location == 0L )
    {
        return;
    }

    loc_x = location->x;
    loc_y = location->y;

    z = land_location( APESPACE_TO_MAPSPACE( loc_x ), APESPACE_TO_MAPSPACE( loc_y ) );

    if ( actual_z != 0L )
    {
        *actual_z = z;
    }
    output->x = ( z - land_location( ( APESPACE_TO_MAPSPACE( loc_x ) + 1 ), APESPACE_TO_MAPSPACE( loc_y ) ) );
    output->y = ( z - land_location( APESPACE_TO_MAPSPACE( loc_x ), ( APESPACE_TO_MAPSPACE( loc_y ) + 1 ) ) );
}

n_int spacetime_after( n_spacetime *initial, n_spacetime *second )
{
    if ( initial->date < second->date )
    {
        return 0;
    }
    if ( initial->date > second->date )
    {
        return 1;
    }
    if ( initial->time > second->time )
    {
        return 1;
    }
    return 0;
}

n_int spacetime_before_now( n_spacetime *initial )
{
    if ( initial->date > m_land.date )
    {
        return 0;
    }
    if ( initial->date < m_land.date )
    {
        return 1;
    }
    if ( initial->time < m_land.time )
    {
        return 1;
    }
    return 0;
}

void spacetime_copy( n_spacetime *to, n_spacetime *from )
{
    to->location[0] = from->location[0];
    to->location[1] = from->location[1];

    to->date = from->date;
    to->time = from->time;
}

void spacetime_set( n_spacetime *set, n_byte2 *location )
{
    set->location[0] = location[0];
    set->location[1] = location[1];
    set->time        = m_land.time;
    set->date        = m_land.date;
}

void land_convert_to_map( n_vect2 *value )
{
    value->x = APESPACE_TO_MAPSPACE( value->x );
    value->y = APESPACE_TO_MAPSPACE( value->y );
}

static n_byte2    color_group[256 * 3];
static n_byte     land_color_initialized = 0;

#define    SUBSTA(c)    ((c<<8)|c)

void land_color_init( void )
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
    while ( lp < 256 )
    {
        if ( lp == points[cntr] )
        {
            ar = SUBSTA( points[( cntr ) | 1] );
            ag = SUBSTA( points[( cntr ) | 2] );
            ab = SUBSTA( points[( cntr ) | 3] );
            fp = lp;
            cntr += 4;

            if ( lp != 255 )
            {
                fl = points[cntr];
                del_c = ( fl - fp );
                dr = SUBSTA( points[( cntr ) | 1] );
                dg = SUBSTA( points[( cntr ) | 2] );
                db = SUBSTA( points[( cntr ) | 3] );
            }
        }

        if ( del_c == 0 )
        {
            return;
        }

        if ( lp != 255 )
        {
            n_int    del_a = ( fl - lp ), del_b = ( lp - fp );
            color_group[lp2++] = ( n_byte2 )( ( ( ar * del_a ) + ( dr * del_b ) ) / del_c );
            color_group[lp2++] = ( n_byte2 )( ( ( ag * del_a ) + ( dg * del_b ) ) / del_c );
            color_group[lp2++] = ( n_byte2 )( ( ( ab * del_a ) + ( db * del_b ) ) / del_c );
        }
        else
        {
            color_group[lp2++] = ( n_byte2 )( ar );
            color_group[lp2++] = ( n_byte2 )( ag );
            color_group[lp2++] = ( n_byte2 )( ab );
        }
        lp ++;
    }

    color_group[( COLOR_WHITE * 3 )    ] = 0xffff;
    color_group[( COLOR_WHITE * 3 ) + 1] = 0xffff;
    color_group[( COLOR_WHITE * 3 ) + 2] = 0xffff;

    color_group[( COLOR_BLUE * 3 )    ] = 0x5500;
    color_group[( COLOR_BLUE * 3 ) + 1] = 0x5500;
    color_group[( COLOR_BLUE * 3 ) + 2] = 0xeeff;

    color_group[( COLOR_RED_DARK * 3 )    ] = ( 0xeeff * 2 ) >> 2; /* return to * 3 following debugging */
    color_group[( COLOR_RED_DARK * 3 ) + 1] = 0x0000;
    color_group[( COLOR_RED_DARK * 3 ) + 2] = 0x0000;

    color_group[( COLOR_RED * 3 )    ] = 0xeeff;
    color_group[( COLOR_RED * 3 ) + 1] = 0x0000;
    color_group[( COLOR_RED * 3 ) + 2] = 0x0000;
}


void  land_color_time( n_byte2 *color_fit, n_int toggle_tidedaylight )
{
    n_int   day_rotation = ( ( land_time() * 255 ) / TIME_DAY_MINUTES );
    n_int   darken =  math_sine( day_rotation + 64 + 128, NEW_SD_MULTIPLE / 400 );
    n_int   loop = 0;
    n_int   sign = 1;
    if ( land_color_initialized == 0 )
    {
        land_color_init();
        land_color_initialized = 1;
    }
    if ( !toggle_tidedaylight )
    {
        if ( darken < 1 )
        {
            sign = -1;
        }
        darken = ( darken * darken ) / 402;
        darken = ( sign * darken ) + 624;
        while ( loop < ( NON_INTERPOLATED * 3 ) )
        {
            n_int cg_val = color_group[loop];
            n_int response = ( cg_val * darken ) >> 10;
            color_fit[loop] = ( n_byte2 )response;
            loop++;
        }
    }
    while ( loop < ( 256 * 3 ) )
    {
        color_fit[loop] = color_group[loop];
        loop++;
    }
}

void  land_color_time_8bit( n_byte *color_fit, n_int toggle_tidedaylight )
{
    n_int   day_rotation = ( ( land_time() * 255 ) / TIME_DAY_MINUTES );
    n_int   darken =  math_sine( day_rotation + 64 + 128, NEW_SD_MULTIPLE / 400 );
    n_int   loop = 0;
    n_int   sign = 1;
    if ( land_color_initialized == 0 )
    {
        land_color_init();
        land_color_initialized = 1;
    }
    if ( !toggle_tidedaylight )
    {
        if ( darken < 1 )
        {
            sign = -1;
        }
        darken = ( darken * darken ) / 402;
        darken = ( sign * darken ) + 624;
        while ( loop < ( NON_INTERPOLATED * 3 ) )
        {
            n_int cg_val = color_group[loop];
            n_int response = ( cg_val * darken ) >> 10;
            color_fit[loop] = ( n_byte )( response >> 8 );
            loop++;
        }
    }
    while ( loop < ( 256 * 3 ) )
    {
        color_fit[loop] = ( n_byte )( color_group[loop] >> 8 );
        loop++;
    }
}

void io_time_to_string( n_string value )
{
    n_int minutes = land_time();
    n_int days = land_date();
    n_int military_time = ( minutes % 60 );
    n_int hours = ( minutes / 60 );
    n_int days_month = ( days % 28 ) + 1;
    n_int month = ( ( days / 28 ) % 13 ) + 1;
    n_int years = days / ( 28 * 13 );

    military_time += hours * 100;

    value[0] = '0' + ( military_time / 1000 ) % 10;
    value[1] = '0' + ( military_time / 100 ) % 10;
    value[2] = ':';
    value[3] = '0' + ( military_time / 10 ) % 10;
    value[4] = '0' + ( military_time / 1 ) % 10;
    value[5] = ' ';
    value[6] = '0' + ( days_month / 10 ) % 10;
    value[7] = '0' + ( days_month / 1 ) % 10;
    value[8] = '/';
    value[9] = '0' + ( month / 10 ) % 10;
    value[10] = '0' + ( month / 1 ) % 10;
    value[11] = '/';
    io_number_to_string( &value[12], ( n_uint )years );
}
