/****************************************************************

 draw.c

 =============================================================

 Copyright 1996-2023 Tom Barbalet. All rights reserved.

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

#ifndef    _WIN32

#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"
#include "../shared.h"

#else

#include "..\toolkit\toolkit.h"
#include "..\script\script.h"
#include "..\sim\sim.h"
#include "..\shared.h"

#endif



#include "gui.h"

#include <stdio.h>

/* the weather/time of day icons hard coded */

static const n_byte	icns[896] =
{
    /* sunny day */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x0A, 0x80, 0x00, 0x01, 0x0A, 0x84, 0x00, 0x04, 0x8A, 0x89, 0x00,
    0x02, 0x4A, 0x92, 0x00, 0x09, 0x2A, 0xA4, 0x80, 0x04, 0x9F, 0xC9, 0x00, 0x02, 0x70, 0x72, 0x00,
    0x01, 0x60, 0x34, 0x00, 0x00, 0xC0, 0x18, 0x00, 0x1F, 0x80, 0x0F, 0xC0, 0x00, 0x80, 0x08, 0x00,
    0x3F, 0x80, 0x0F, 0xE0, 0x00, 0x80, 0x08, 0x00, 0x1F, 0x80, 0x0F, 0xC0, 0x00, 0xC0, 0x18, 0x00,
    0x01, 0x60, 0x34, 0x00, 0x02, 0x70, 0x72, 0x00, 0x04, 0x9F, 0xC9, 0x00, 0x09, 0x2A, 0xA4, 0x80,
    0x02, 0x4A, 0x92, 0x00, 0x04, 0x8A, 0x89, 0x00, 0x01, 0x0A, 0x84, 0x00, 0x00, 0x0A, 0x80, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* cloudy day */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x0A, 0x80, 0x00, 0x01, 0x0A, 0x84, 0x00, 0x04, 0x8A, 0x89, 0x00,
    0x02, 0x4A, 0x92, 0x00, 0x09, 0x2A, 0xA4, 0x80, 0x04, 0x9F, 0xC9, 0x00, 0x02, 0x70, 0x72, 0x00,
    0x01, 0x60, 0x34, 0x00, 0x00, 0xC0, 0x18, 0x00, 0x1F, 0x80, 0x0F, 0xC0, 0x00, 0xB8, 0x08, 0x00,
    0x01, 0xC7, 0x1C, 0x00, 0x0E, 0x00, 0xE3, 0x00, 0x10, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x40,
    0x20, 0x00, 0x00, 0x40, 0x20, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80,
    0x10, 0x00, 0x00, 0x80, 0x08, 0x00, 0x00, 0x40, 0x08, 0x00, 0x00, 0x40, 0x08, 0x01, 0xC0, 0x40,
    0x07, 0x8E, 0x38, 0x80, 0x00, 0x70, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* rainy day */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00,
    0x00, 0x38, 0xE3, 0x80, 0x01, 0xC0, 0x1C, 0x60, 0x02, 0x00, 0x00, 0x10, 0x04, 0x00, 0x00, 0x08,
    0x04, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x10, 0x02, 0x00, 0x00, 0x10,
    0x02, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x08, 0x01, 0x00, 0x38, 0x08,
    0x00, 0xF1, 0xD7, 0x10, 0x00, 0xAE, 0xAA, 0xE0, 0x01, 0x55, 0x55, 0x40, 0x02, 0xAA, 0xAA, 0x80,
    0x05, 0x55, 0x55, 0x00, 0x0A, 0xAA, 0xAA, 0x00, 0x05, 0x55, 0x54, 0x00, 0x0A, 0xAA, 0xA8, 0x00,
    0x05, 0x55, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* clear night */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF1, 0xFF, 0xFF, 0xFF, 0xE6, 0xFF, 0xFF, 0xFF, 0xE7, 0xFF, 0xFF, 0xFF, 0xE6, 0xFF, 0xFF, 0xFF,
    0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF,
    0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* cloudy night */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0x7F, 0xFF,
    0xF1, 0xEA, 0xAE, 0xBF, 0xE7, 0x55, 0x55, 0x5F, 0xE6, 0xAA, 0xAA, 0xAF, 0xE7, 0x55, 0x55, 0x57,
    0xF2, 0xAA, 0xAA, 0xAF, 0xFF, 0x55, 0x55, 0x5F, 0xFF, 0xAA, 0xAA, 0xAF, 0xFF, 0x55, 0x55, 0x5F,
    0xFB, 0xAA, 0xAA, 0xAF, 0xF5, 0xD5, 0x55, 0x57, 0xEA, 0xAA, 0xAA, 0xAF, 0xD5, 0xFD, 0xF7, 0xDF,
    0xEB, 0xEF, 0xBE, 0xFF, 0xFF, 0x55, 0x57, 0x57, 0xFA, 0xAA, 0xAA, 0xAF, 0xF5, 0x55, 0x55, 0x57,
    0xEA, 0xAA, 0xAA, 0xEB, 0xF5, 0x55, 0x55, 0x57, 0xFA, 0xAA, 0xAA, 0xAF, 0xF5, 0x55, 0x55, 0xDF,
    0xFA, 0xAA, 0xAA, 0xFF, 0xFD, 0x55, 0x55, 0x7F, 0xFA, 0xAA, 0xAA, 0xFF, 0xFD, 0x55, 0xD5, 0x7F,
    0xFF, 0xAF, 0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* rainy night */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xE2, 0x3E, 0x7F, 0xFE, 0x00, 0x00, 0x1F, 0xFC, 0x88, 0x88, 0x8F,
    0xDC, 0x00, 0x00, 0x0F, 0xFE, 0x22, 0x22, 0x2F, 0xFE, 0x00, 0x00, 0x1F, 0xFE, 0x88, 0x88, 0x9F,
    0xFE, 0x00, 0x00, 0x1F, 0xFB, 0x22, 0x22, 0x2F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0x88, 0xB8, 0x8F,
    0xFF, 0xF1, 0xD7, 0x1F, 0xFF, 0xAE, 0xAA, 0xFF, 0xFF, 0x55, 0x55, 0x7F, 0xFE, 0xAA, 0xAA, 0xFF,
    0xFD, 0x55, 0x55, 0xFF, 0xFA, 0xAA, 0xAB, 0xFF, 0xFD, 0x55, 0x57, 0xFF, 0xFA, 0xAA, 0xBF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* sunrise/sunset */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xD5, 0xFB, 0xFF, 0xFF, 0xAA, 0xBF, 0xFF, 0x77, 0x55, 0x55, 0xF5, 0xAB, 0xA0, 0xAA, 0xAA,
    0x55, 0x40, 0x01, 0x55, 0xAA, 0x80, 0x00, 0x2A, 0x54, 0x00, 0x00, 0x55, 0xA0, 0x00, 0x00, 0x2A,
    0x40, 0x00, 0x00, 0x55, 0xA0, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02, 0x00, 0x0A,
    0x00, 0x0A, 0x80, 0x05, 0x01, 0x0A, 0x84, 0x02, 0x04, 0x8A, 0x89, 0x05, 0x02, 0x4A, 0x92, 0x02,
    0x09, 0x2A, 0xA4, 0x80, 0x04, 0x9F, 0xC9, 0x00, 0x02, 0x70, 0x72, 0x00, 0x01, 0x60, 0x34, 0x00,
    0x00, 0xC0, 0x18, 0x00, 0x1F, 0x80, 0x0F, 0xC0, 0x00, 0x80, 0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
};

#define	MAX_NUMBER_ERRORS	35

static n_byte           character_location;

static n_byte           number_errors;
static n_string_block	error_array[MAX_NUMBER_ERRORS + 1];

static n_byte           weather_grayscale[MAP_AREA];

#ifdef BRAIN_ON
static n_uint	tilt_y = 0;
#endif

n_byte	check_about = 0;
n_uint	tilt_z = 118;

static n_int toggle_weather = 1;
static n_int toggle_brain = 1;
static n_int toggle_braincode = 0;
static n_int toggle_territory = 0;
static n_int toggle_tidedaylight = 0;

static n_int toggle_follow = 1;
static n_int toggle_social_web = 0;

static n_int changing_size = 0;

static n_vect2 selected_location;

static n_byte *local_offscreen = 0L;

void draw_point( n_int x, n_int y )
{
    n_byte *draw = VIEWWINDOW( local_offscreen );
    draw[x + ( y * 512 )] = 255;
}

n_vect2 *draw_selected_location( void )
{
    return &selected_location;
}

n_int draw_toggle_follow( void )
{
    toggle_follow ^= 1;
    return toggle_follow;
}

n_int draw_toggle_social_web( void )
{
    toggle_social_web ^= 1;
    return toggle_social_web;
}

n_int draw_toggle_weather( void )
{
    toggle_weather ^= 1;
    return toggle_weather;
}

n_int draw_toggle_brain( void )
{
    toggle_brain ^= 1;
    return toggle_brain;
}

n_int draw_toggle_braincode( void )
{
    toggle_braincode ^= 1;
    return toggle_braincode;
}

n_int draw_toggle_territory( void )
{
    toggle_territory ^= 1;
    return toggle_territory;
}

n_int draw_toggle_tide_daylight( void )
{
    toggle_tidedaylight ^= 1;
    return toggle_tidedaylight;
}

n_int draw_toggle_tide_daylight_value( void )
{
    return toggle_tidedaylight;
}

/* this needs to be grouped eventually, it is here as a test */

#define UNDRAW_MAX          (100000+ (HI_RES_MAP_DIMENSION * HI_RES_MAP_DIMENSION / 16))

static n_byte *undraw_location[UNDRAW_MAX];
static n_byte   undraw_color[UNDRAW_MAX];
static n_int    undraw_count = 0;



void draw_undraw(void)
{
    if ( undraw_count == 0 )
    {
        return;
    }
    undraw_count--;
    do
    {
        *undraw_location[undraw_count] = undraw_color[undraw_count];
        undraw_count--;
    }
    while ( undraw_count > -1 );
    undraw_count = 0;
}

void draw_undraw_clear( void )
{
    undraw_count = 0;
}

n_byte *draw_offscreen( n_byte *value )
{
    local_offscreen = value;
    return local_offscreen;
}

static n_byte pixel_color8_hires( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    n_color8	*local_col = information;
    n_byte      *location = &local_col->screen[ ( px << 1 ) | ( py << ( HI_RES_MAP_BITS + 1 ) ) | 1 ];
    undraw_location[undraw_count] = location;
    undraw_color[undraw_count] = location[0];
    undraw_count++;

    if ( undraw_count > UNDRAW_MAX )
    {
        ( void )SHOW_ERROR( "Erase count outside limit" );
    }
    location[0] = local_col->color;
    return 0;
}

static n_byte pixel_color8( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    n_color8	*local_col = information;
    local_col->screen[ px | ( py << MAP_BITS ) ] = local_col->color;
    return 0;
}

static n_int window_dim_x = 512;
static n_int window_dim_y = 511;

static n_byte pixel_map( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    n_color8 local_color;
    local_color.color = COLOR_WHITE;
    local_color.screen = information;
    return pixel_color8( px, py, dx, dy, &local_color );
}

static n_byte pixel_map_hires( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    n_color8 local_color;
    local_color.color = COLOR_WHITE;
    local_color.screen = information;
    return pixel_color8_hires( px, py, dx, dy, &local_color );
}

static n_byte pixel_map_checker( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    if ( ( px + py ) & 1 )
    {
        return pixel_map( px, py, dx, dy, information );
    }
    return 0;
}

static n_byte pixel_white( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    n_byte *byte_info = information;
    byte_info[ px + ( py * window_dim_x ) ] = COLOR_WHITE;
    return 0;
}


static n_int smart_character_toggle;
static n_int bold_character_toggle;


static n_byte pixel_control( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    n_byte *byte_info = information;
    byte_info[ px + ( py * window_dim_x ) ] = smart_character_toggle ? COLOR_BLUE : COLOR_WHITE;
    if ( bold_character_toggle )
    {
        byte_info[ px + 1 + ( ( py + 1 ) * window_dim_x ) ] = smart_character_toggle ? COLOR_BLUE : COLOR_WHITE;
    }
    return 0;
}

static n_byte pixel_white_checker( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    if ( ( px + py ) & 1 )
    {
        n_byte *byte_info = information;
        byte_info[ px + ( py * window_dim_x ) ] = COLOR_WHITE;
    }
    return 0;
}

static n_byte pixel_black( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    n_byte *byte_info = information;
    byte_info[ px + ( py * window_dim_x ) ] = COLOR_BLACK;
    return 0;
}

n_byte *draw_pointer( n_int which_one )
{
    if ( local_offscreen )
    {
        switch ( which_one )
        {
        case NUM_TERRAIN:
            return TERRAINWINDOW( local_offscreen );
        case NUM_VIEW:
            return VIEWWINDOW( local_offscreen );
        case NUM_CONTROL:
            return CONTROLWINDOW( local_offscreen );
        }
    }
    return 0L;
}

#ifdef MULTITOUCH_CONTROLS

extern touch_control_state tc_state;

void draw_tc_controls( n_join *local_mono )
{
    const n_int half_y = window_dim_y / 2;
    n_vect2 point1, point2, point3;

    if ( tc_state == TCS_SHOW_CONTROLS )
    {
        vect2_populate( &point1, 5, half_y + TC_OFFSET_Y );
        vect2_populate( &point2, 5, half_y - TC_OFFSET_Y );
        vect2_populate( &point3, TC_FRACTION_X - 5, half_y );

        math_line_vect( &point1, &point2, local_mono );
        math_line_vect( &point2, &point3, local_mono );
        math_line_vect( &point3, &point1, local_mono );

        vect2_populate( &point1, window_dim_x - 5, half_y + TC_OFFSET_Y );
        vect2_populate( &point2, window_dim_x - 5, half_y - TC_OFFSET_Y );
        vect2_populate( &point3, window_dim_x - TC_FRACTION_X + 5, half_y );

        math_line_vect( &point1, &point2, local_mono );
        math_line_vect( &point2, &point3, local_mono );
        math_line_vect( &point3, &point1, local_mono );
    }

    if ( tc_state == TCS_LEFT_STATE_CONTROLS )
    {
        vect2_populate( &point1, TC_FRACTION_X - 5, half_y + TC_OFFSET_Y );
        vect2_populate( &point2, TC_FRACTION_X - 5, half_y - TC_OFFSET_Y );
        vect2_populate( &point3, 5, half_y );

        math_line_vect( &point1, &point2, local_mono );
        math_line_vect( &point2, &point3, local_mono );
        math_line_vect( &point3, &point1, local_mono );
    }

    if ( tc_state == TCS_RIGHT_STATE_CONTROLS )
    {
        vect2_populate( &point1, window_dim_x - TC_FRACTION_X + 5, half_y + TC_OFFSET_Y );
        vect2_populate( &point2, window_dim_x - TC_FRACTION_X + 5, half_y - TC_OFFSET_Y );
        vect2_populate( &point3, window_dim_x - 5, half_y );

        math_line_vect( &point1, &point2, local_mono );
        math_line_vect( &point2, &point3, local_mono );
        math_line_vect( &point3, &point1, local_mono );
    }
}

#endif


void draw_about( void )
{
    check_about = 1;
}

/*	shows the about information */
static void draw_about_information( n_byte terrain )
{
    n_join	local_draw;
    n_byte *buffer = draw_pointer( terrain ? NUM_TERRAIN : NUM_CONTROL );
    n_int   line_y_offset = 100;
    n_int   linx_x_offset = CHARACTER_WIDTH * 3;
    n_int   tab_offset  = CHARACTER_WIDTH * 5;

    if ( buffer == 0L )
    {
        check_about = 0;
        return;
    }

    local_draw.information = buffer;
    local_draw.pixel_draw  = &pixel_black;

    draw_string( SHORT_VERSION_NAME,     linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( FULL_DATE,              linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( FULL_VERSION_COPYRIGHT, linx_x_offset + tab_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( COPYRIGHT_FOLLOW,       linx_x_offset + tab_offset, line_y_offset, &local_draw );

    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "This software is a continuing work of ", linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "Tom Barbalet begun on 13 June 1996.", linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "No apes or cats were harmed in the ", linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "writing of this software.", linx_x_offset, line_y_offset, &local_draw );

    line_y_offset = 100 - 1;
    linx_x_offset = ( CHARACTER_WIDTH * 3 ) - 1;

    local_draw.pixel_draw  = &pixel_white;

    draw_string( SHORT_VERSION_NAME,     linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( FULL_DATE,              linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( FULL_VERSION_COPYRIGHT, linx_x_offset + tab_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( COPYRIGHT_FOLLOW,       linx_x_offset + tab_offset, line_y_offset, &local_draw );

    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "This software is a continuing work of ", linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "Tom Barbalet begun on 13 June 1996.", linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "No apes or cats were harmed in the ", linx_x_offset, line_y_offset, &local_draw );
    line_y_offset += CHARACTER_HEIGHT;

    draw_string( "writing of this software.", linx_x_offset, line_y_offset, &local_draw );
}

/* draws a string starting at point (off_x,off_y) */

#define	ledfir(x, y, dx, dy, c)	if(((val >> c)&1)) (*local_draw)((x + off_x + offset),(y + off_y), dx, dy, local_info)

static void draw_character( n_pixel *local_draw, n_int offset, void *local_info, n_int off_x, n_int off_y, n_int val )
{
    ledfir( 3, 8, 0, 0, 15 ); //P

    ledfir( 3, 2, 0, 0, 14 ); //O

    ledfir( 0, 0, 5, 0, 13 ); //N

    ledfir( 6, 0, 0, 4, 12 ); //M

    ledfir( 6, 4, 0, 3, 11 ); //L

    ledfir( 0, 8, 5, 0, 10 ); //K

    ledfir( 0, 4, 0, 4, 9 ); //J

    ledfir( 0, 0, 0, 3, 8 ); //I

    ledfir( 3, 4, 3, 0, 7 ); //H

    ledfir( 0, 4, 3, 0, 6 ); //G

    ledfir( 3, 4, 0, 4, 5 ); //F

    ledfir( 3, 5, 3, 3, 4 ); //E

    ledfir( 3, 5, -3, 3, 3 ); //D

    ledfir( 3, 3, 3, -3, 2 ); //C

    ledfir( 0, 0, 3, 3, 1 ); //B

    ledfir( 3, 0, 0, 4, 0 ); //A
}

/**
 This is used to produce letter LED style letters through the generic
 draw function specified.
 @param str The string to be drawn.
 @param off_x The starting x location for the string to be drawn.
 @param off_y The starting y location for the string to be drawn.
 @param draw The generic draw function used to draw the character.
 */
static void draw_string_line( n_constant_string str, n_int off_x, n_int off_y, n_join *draw )
{
    n_pixel     *local_draw = draw->pixel_draw;
    void     *local_info = draw->information;

    n_int    char_loop = 0;
    while ( str[char_loop] > 31 )
    {
        n_int    val = math_seg14( str[char_loop] - 32 );
        n_int    offset = char_loop << 3;
        /* draw the character as a 14-segment LCD/LED output */
        draw_character( local_draw, offset, local_info, off_x, off_y, val );
        char_loop ++;
    }
}

static void draw_smart_string_line( n_constant_string str, n_int off_x, n_int off_y, n_join *draw )
{
    n_pixel *local_draw = draw->pixel_draw;
    void     *local_info = draw->information;
    n_int     char_loop = 0;
    n_int     char_location = 0;

    while ( str[char_loop] > 31 )
    {
        n_byte   character = ( n_byte )str[char_loop];
        n_int    val = math_seg14( character - 32 );
        n_int    offset = char_location << 3;
        /* draw the character as a 14-segment LCD/LED output */

        if ( character == '<' )
        {
            bold_character_toggle = 1;
            char_location++;
        }
        else if ( character == '>' )
        {
            bold_character_toggle = 0;
        }
        else if ( character == '*' )
        {
            smart_character_toggle ^= 1;
            if ( smart_character_toggle == 0 )
            {
                character_location++;
            }
        }
        else
        {
            if ( smart_character_toggle )
            {
                sim_control_set( off_x + offset, off_y, character_location, character );
            }
            draw_character( local_draw, offset, local_info, off_x, off_y, val );
            char_location++;
        }
        char_loop ++;
    }
}



static n_byte draw_character_line( n_int px, n_int py, n_int dx, n_int dy, void *data )
{
    return math_join( px, py, dx, dy, data );
}

void draw_string( n_constant_string str, n_int off_x, n_int off_y, n_join *draw )
{
    n_join local_pixel;
    local_pixel.pixel_draw = &draw_character_line;
    local_pixel.information = draw;
    draw_string_line( str, off_x, off_y, &local_pixel );
}

static void draw_smart_string( n_constant_string str, n_int off_x, n_int off_y, n_join *draw )
{
    n_join local_pixel;
    local_pixel.pixel_draw = &draw_character_line;
    local_pixel.information = draw;
    draw_smart_string_line( str, off_x, off_y, &local_pixel );
}

/* this is the ocelot landscape algorithm */

#define POS_HIRES(num) ((num+(HI_RES_MAP_DIMENSION*2))&(HI_RES_MAP_DIMENSION-1))

#define CONVERT_X(x, cx)  (n_uint)((POS_HIRES((x)+cx)) )
#define CONVERT_Y(y, cy)  (n_uint)((POS_HIRES((y)+cy)) << HI_RES_MAP_BITS)

#define CONVERT_XY(x,y)   (CONVERT_X(x) | CONVERT_Y(y))

typedef struct
{
    n_int     const_lowdiv2;
    n_int     lowest_s;
    n_int     lowest_c;

    n_vect2   co;

    n_vect2   dimensions;
    n_vect2   value_vector;
    n_byte    *offscreen;
} draw_terrain_scan_struct;

static void draw_terrain_scan( void *void_dtss, void *xlocation, void *unused )
{
    draw_terrain_scan_struct *dtss = ( draw_terrain_scan_struct * )void_dtss;
    n_byte   *buf_offscr = ( n_byte * ) dtss->offscreen;
    n_int     scrx = ( ( n_int * )xlocation )[0];
    /* take the very bottom pixel */
    n_int     dim_x =  dtss->dimensions.x;
    n_int     dim_y1 = dtss->dimensions.y - 1;
    n_int     pixy = ( scrx + ( dim_x >> 1 ) ) + ( dim_y1 * dim_x );
    n_int     actual = dim_y1;
    /* start with a map point which is below/off the screen */
    n_int     scry = dtss->const_lowdiv2;
    /* rotated and add offset (which will be &ed off) */
    n_int     big_x = dtss->lowest_s + ( scrx * dtss->value_vector.y );
    /* rotated and sub offset (subtracted further down) */
    n_int     big_y = dtss->lowest_c - ( scrx * dtss->value_vector.x );
    n_byte2 *combined = ( n_byte2 * )land_topography_highdef();
    n_int     co_x = dtss->co.x;
    n_int     co_y = dtss->co.y;
    n_int     valc2 = dtss->value_vector.y << 1;
    n_int     vals2 = dtss->value_vector.x << 1;
    while ( actual > -1 )
    {
        const n_uint   check_change = CONVERT_X( ( big_x >> 8 ), co_x ) | CONVERT_Y( ( big_y >> 8 ), co_y );
        const n_byte2  value        = combined[check_change];
        const n_int    z00          = value & 255;
        const n_byte   col00        = value >> 8;

        n_int          aval         = ( scry - z00 );
        if ( aval < -1 )
        {
            aval = -1;
        }

        scry--;           /* next map point from screen value */
        big_x -= vals2;
        big_y -= valc2;

        while ( actual > aval )
        {
            buf_offscr[pixy] = col00;
            pixy -= dim_x;
            actual--;
        }
    }
    memory_free( &xlocation );
    memory_free( &void_dtss );
}

static void draw_terrain( simulated_group *group, n_vect2 *dimensions )
{
    n_byte    *buf_offscr = draw_pointer( NUM_TERRAIN );
    if ( buf_offscr == 0L )
    {
        return;
    }

    if ( group->select == 0L )
    {
        memory_erase( buf_offscr, ( n_uint )( dimensions->x * dimensions->y ) );
        return;
    }
    {
        const n_int    lowest_y = ( ( dimensions->y + 256 ) * dimensions->y ) / 256;
        simulated_being *loc_being = group->select;
        draw_terrain_scan_struct dtss;

        n_byte2   *local_combined = ( n_byte2 * )land_topography_highdef();

        /* start at the left-most row */
        n_int scrx = ( 0 - ( dimensions->x >> 1 ) );
        /* find the central map point */
        n_int flatval;
        n_vect2 value_vector;

        vect2_direction( &value_vector, being_facing( loc_being ) + 128, 105 );

        vect2_copy( &( dtss.value_vector ), &value_vector );

        dtss.lowest_s = ( ( value_vector.x * ( ( ( lowest_y ) ) - dimensions->y ) ) );
        dtss.lowest_c = ( ( value_vector.y * ( ( ( lowest_y ) ) - dimensions->y ) ) );

        being_high_res( loc_being, &dtss.co );

        flatval = local_combined[CONVERT_X( ( HI_RES_MAP_DIMENSION / 2 ), dtss.co.x ) | CONVERT_Y( ( HI_RES_MAP_DIMENSION / 2 ), dtss.co.y )] & 255;

        if ( flatval < WATER_MAP ) /* if the central map point is underwater,*/
        {
            flatval = WATER_MAP;   /*    put it on water level */
        }

        dtss.const_lowdiv2 = ( ( ( lowest_y ) ) >> 1 ) + flatval;

        vect2_copy( &( dtss.dimensions ), dimensions );

        dtss.offscreen = buf_offscr;

        /* repeat until the right-most row is reached */
        while ( scrx < ( dimensions->x - ( dimensions->x >> 1 ) ) )
        {
            n_int *screen_x_location = memory_new( sizeof( n_int ) );

            draw_terrain_scan_struct *local_dtss = ( draw_terrain_scan_struct * )memory_new( sizeof( draw_terrain_scan_struct ) );

            memory_copy( ( n_byte * )&dtss, ( n_byte * )local_dtss, sizeof( draw_terrain_scan_struct ) );

            screen_x_location[0] = scrx;

            draw_terrain_scan( local_dtss, screen_x_location, 0L );
            scrx++;               /* next column */
        }
    }
}

#define mndivmin		15
#define mndivhr			180

#define mndivmonth		7
#define mndivyear		91

#define YELLOW_CHECKER_LINE(px, py, dx, dy) math_join(px, py, dx, dy, &local_kind_yellow_checker)
#define YELLOW_LINE(px, py, dx, dy) math_join(px, py, dx, dy, &local_kind_yellow)
#define BLACK_LINE(px, py, dx, dy) math_join(px, py, dx, dy, &local_kind_black)
#define YELLOW_VECT_LINE(px, py, vector)    math_join_vect2(px, py, vector, &local_kind_yellow)

#define FACING_OFFSIDE  (window_dim_x-101)
#define SP_EN_OFFSIDE   (window_dim_x-187)

#define GENETICS_X      (window_dim_x-178)
#define GENETICS_Y      (8)

#define GENDER_X        (window_dim_x-110)
#define GENDER_Y        (10)

static void draw_meters( simulated_group *group )
{
    n_pixel	  *local_draw_yellow = &pixel_white;
    n_pixel	  *local_draw_yellow_checker = &pixel_white_checker;
    n_pixel       *local_draw_black = &pixel_black;
    n_byte	  *local_info = draw_pointer( NUM_TERRAIN );

    n_join		   local_kind_yellow;
    n_join		   local_kind_yellow_checker;
    n_join		   local_kind_black;
    const n_byte *local_icon;
    n_int		   ha1 = 6;
    n_int		   ha2 = 0;
    n_int		   hr = 0;
    if ( local_info == 0L )
    {
        return;
    }

    local_kind_yellow.pixel_draw = local_draw_yellow;
    local_kind_yellow.information = local_info;
    local_kind_yellow_checker.pixel_draw = local_draw_yellow_checker;
    local_kind_yellow_checker.information = local_info;
    local_kind_black.pixel_draw = local_draw_black;
    local_kind_black.information = local_info;

    while ( hr < 41 )
    {
        if ( ( hr != 40 ) && ( hr != 0 ) )
        {
            BLACK_LINE( 6, 5 + hr, 38, 0 );
        }
        hr ++;
    }

    YELLOW_CHECKER_LINE( 5, 5, 40, 0 );
    YELLOW_CHECKER_LINE( 5, 5, 0, 40 );
    YELLOW_CHECKER_LINE( 45, 5, 0, 40 );
    YELLOW_CHECKER_LINE( 5, 45, 40, 0 );

    hr = 0;

    while ( hr < 12 )
    {
        n_vect2 hour_clock;
        vect2_direction( &hour_clock, ( ( hr << 8 ) / 12 ), 320 );

        YELLOW_LINE( ( 25 + ( hour_clock.x / 5 ) ), ( 25 + ( hour_clock.y / 5 ) ), ( hour_clock.x / 17 ), ( hour_clock.y / 17 ) );
        hr++;
    }

    if ( group->select )
    {
        simulated_being   *loc_being = group->select;

        hr = 0;
        while ( hr < 41 )
        {

            if ( ( hr != 40 ) && ( hr != 0 ) )
            {
                BLACK_LINE( 51 + 5 + FACING_OFFSIDE, 5 + hr, 38, 0 );
                BLACK_LINE( 51 + 55 + SP_EN_OFFSIDE, 5 + hr, 6, 0 );
                BLACK_LINE( 51 + 55 + 18 + SP_EN_OFFSIDE, 5 + hr, 6, 0 );
            }
            hr ++;
        }

        YELLOW_CHECKER_LINE( 50 + 5 + FACING_OFFSIDE, 5, 0, 40 );
        YELLOW_CHECKER_LINE( 50 + 45 + FACING_OFFSIDE, 5, 0, 40 );
        YELLOW_CHECKER_LINE( 50 + 5 + FACING_OFFSIDE, 45, 40, 0 );
        YELLOW_CHECKER_LINE( 50 + 5 + FACING_OFFSIDE, 5, 40, 0 );

        YELLOW_CHECKER_LINE( 58 + 55 + SP_EN_OFFSIDE, 5, 0, 40 );
        YELLOW_CHECKER_LINE( 50 + 55 + SP_EN_OFFSIDE, 5, 0, 40 );

        YELLOW_CHECKER_LINE( 58 + 55 + 18 + SP_EN_OFFSIDE, 5, 0, 40 );
        YELLOW_CHECKER_LINE( 50 + 55 + 18 + SP_EN_OFFSIDE, 5, 0, 40 );

        YELLOW_CHECKER_LINE( 50 + 55 + SP_EN_OFFSIDE, 5, 9, 0 );
        YELLOW_CHECKER_LINE( 50 + 55 + SP_EN_OFFSIDE, 45, 9, 0 );
        YELLOW_CHECKER_LINE( 50 + 55 + 18 + SP_EN_OFFSIDE, 5, 9, 0 );
        YELLOW_CHECKER_LINE( 50 + 55 + 18 + SP_EN_OFFSIDE, 45, 9, 0 );

        hr = 0;

        while ( hr < 8 )
        {
            n_vect2 facing_clock;
            vect2_direction( &facing_clock, ( hr << 5 ), 320 );
            YELLOW_LINE( ( 25 + 50 + ( facing_clock.x / 5 ) ) + FACING_OFFSIDE, ( 25 + ( facing_clock.y / 5 ) ),
                         ( facing_clock.x / 17 ), ( facing_clock.y / 17 ) );
            hr++;
        }

        YELLOW_LINE( 50 + 55 + 18 + SP_EN_OFFSIDE - 1, 25, 1, 0 );
        YELLOW_LINE( 58 + 55 + 18 + SP_EN_OFFSIDE, 25, 1, 0 );
        YELLOW_LINE( 50 + 55 + 18 + SP_EN_OFFSIDE - 2, 15, 2, 0 );
        YELLOW_LINE( 58 + 55 + 18 + SP_EN_OFFSIDE, 15, 2, 0 );
        YELLOW_LINE( 50 + 55 + 18 + SP_EN_OFFSIDE - 2, 35, 2, 0 );
        YELLOW_LINE( 58 + 55 + 18 + SP_EN_OFFSIDE, 35, 2, 0 );

        {
            n_genetics    *genetics = being_genetics( loc_being );
            /* draw genetics */
            while ( ha2 < CHROMOSOMES )
            {
                n_uint	ha3 = 0;
                n_genetics	genetic_block = genetics[ha2];
                ha1 = 0;
                while ( ha3 < ( sizeof( n_genetics ) * 4 ) )
                {
                    n_int four = ( genetic_block >> ( ha3 * 2 ) ) & 3 ;
                    if ( four != 0 )
                    {
                        YELLOW_LINE( ( ha1 ) + GENETICS_X,  GENETICS_Y + ( ha2 * 10 ), 0, 7 );
                    }
                    ha1++;
                    if ( four == 3 )
                    {
                        YELLOW_LINE( ( ha1 ) + GENETICS_X, GENETICS_Y + ( ha2 * 10 ), 0, 7 );
                    }
                    ha1 += 3;
                    ha3++;
                }
                ha2++;
            }
        }

        /* draw sex */

        YELLOW_LINE( GENDER_X + 5, GENDER_Y + 10, 5, 11 );
        YELLOW_LINE( GENDER_X + 10, GENDER_Y + 21, 5, -11 );
        YELLOW_LINE( GENDER_X + 15, GENDER_Y + 10, -10, 0 );


        if ( being_female( loc_being ) )
        {
            YELLOW_LINE( GENDER_X + 10, GENDER_Y + 20, 0, 6 );
            YELLOW_LINE( GENDER_X + 8,  GENDER_Y + 23, 4, 0 );
        }
        else
        {
            YELLOW_LINE( GENDER_X + 15, GENDER_Y + 10, 4, -4 );
            YELLOW_LINE( GENDER_X + 19, GENDER_Y + 6, -2, 0 );
            YELLOW_LINE( GENDER_X + 19, GENDER_Y + 6, 0, 2 );
        }
        /* draw direction facing */
        {
            n_vect2 direction_facing;
            vect2_direction( &direction_facing, being_facing( loc_being ), 63 * 32 );
            YELLOW_VECT_LINE( 75 + FACING_OFFSIDE, 25, &direction_facing );
        }

        {
            n_int   local_speed = being_speed( loc_being );
            n_int   local_energy = being_energy( loc_being );
            n_int   local_x = being_location_x( loc_being );
            n_int   local_y = being_location_y( loc_being );

            if ( local_speed != 0 )
            {
                YELLOW_LINE( 106 + SP_EN_OFFSIDE, ( 45 - local_speed ), 6, 0 );
            }
            if ( local_energy > 127 )
            {
                YELLOW_LINE( 106 + 18 + SP_EN_OFFSIDE, ( 45 - ( local_energy >> 7 ) ), 6, 0 );
            }
            local_icon = &icns[weather_seven_values( local_x, local_y ) << 7];
        }

    }
    else
    {
        /* still give weather even with no Simulated Apes */
        local_icon = &icns[weather_seven_values( 0, 0 ) << 7];
    }

    {
        n_int date_multiplier = ( land_date() << 6 );
        n_int time_multiplier = ( land_time() << 6 );
        n_vect2 year_hand;
        n_vect2 month_hand;
        n_vect2 hour_hand;
        n_vect2 minute_hand;

        vect2_direction( &year_hand, ( date_multiplier / mndivyear ), 5440 );
        vect2_direction( &month_hand, ( date_multiplier / mndivmonth ), 5440 );
        vect2_direction( &hour_hand, ( time_multiplier / mndivhr ), 2688 );
        vect2_direction( &minute_hand, ( time_multiplier / mndivmin ), 2016 );

        vect2_rotate90( &year_hand );
        vect2_rotate90( &month_hand );
        vect2_rotate90( &hour_hand );
        vect2_rotate90( &minute_hand );

        YELLOW_VECT_LINE( 17, 25, &year_hand );
        YELLOW_VECT_LINE( 33, 25, &month_hand );

        YELLOW_VECT_LINE( 25, 25, &hour_hand );
        YELLOW_VECT_LINE( 25, 25, &minute_hand );
    }
    ha1 = 0;
    while ( ha1 < 32 )
    {
        n_uint icon_stripe = ( n_uint )( ( local_icon[( ha1 << 2 ) | 3] << 0 ) |
                                         ( local_icon[( ha1 << 2 ) | 2] << 8 ) |
                                         ( local_icon[( ha1 << 2 ) | 1] << 16 ) |
                                         ( local_icon[( ha1 << 2 ) | 0] << 24 ) );
        n_int startRun = -1;
        n_int stopRun = -1;
        ha2 = 0;
        while ( ha2 < 32 )
        {
            if ( ( icon_stripe >> ( 31 - ha2 ) ) & 1 )
            {
                if ( startRun < 0 )
                {
                    startRun = ha2;
                }
                stopRun = ha2;
            }
            if ( ( stopRun < ha2 ) && ( startRun > -1 ) )
            {
                YELLOW_LINE( 60 + startRun, 8 + ha1, stopRun - startRun, 0 );
                startRun = -1;
            }
            ha2++;
        }

        if ( ( stopRun < ha2 ) && ( startRun > -1 ) )
        {
            YELLOW_LINE( 60 + startRun, 8 + ha1, stopRun - startRun, 0 );
            startRun = -1;
        }

        ha1++;
    }

}


static void draw_feelers( simulated_being *bei, n_color8 *local_info, n_vect2 *location, n_int start_point, n_pixel *local_draw, n_byte hires )
{
    n_int    local_facing = ( ( ( ( being_facing( bei ) ) >> 2 ) + 4 ) & 63 ) >> 3;
    /* D  C
     G       F

     H       E
     B  A */
    n_color8    *local_col = local_info;
    local_col->color = COLOR_WHITE;
    if ( local_facing == 0 || local_facing == 7 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x + start_point ),
                             POSITIVE_LAND_COORD_HIRES( location->y - 2 ),
                             0, 0, local_info ); /* F */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x + start_point ),
                             POSITIVE_LAND_COORD( location->y - 2 ),
                             0, 0, local_info ); /* F */
        }
    }
    if ( local_facing == 1 || local_facing == 0 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x + start_point ),
                             POSITIVE_LAND_COORD_HIRES( location->y + 2 ),
                             0, 0, local_info ); /* E */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x + start_point ),
                             POSITIVE_LAND_COORD( location->y + 2 ),
                             0, 0, local_info ); /* E */
        }
    }
    if ( local_facing == 2 || local_facing == 1 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x + 2 ),
                             POSITIVE_LAND_COORD_HIRES( location->y + start_point ),
                             0, 0, local_info ); /* A */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x + 2 ),
                             POSITIVE_LAND_COORD( location->y + start_point ),
                             0, 0, local_info ); /* A */
        }
    }
    if ( local_facing == 3 || local_facing == 2 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x - 2 ),
                             POSITIVE_LAND_COORD_HIRES( location->y + start_point ),
                             0, 0, local_info ); /* B */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x - 2 ),
                             POSITIVE_LAND_COORD( location->y + start_point ),
                             0, 0, local_info ); /* B */
        }
    }
    if ( local_facing == 4 || local_facing == 3 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x - start_point ),
                             POSITIVE_LAND_COORD_HIRES( location->y + 2 ),
                             0, 0, local_info ); /* H */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x - start_point ),
                             POSITIVE_LAND_COORD( location->y + 2 ),
                             0, 0, local_info ); /* H */
        }
    }
    if ( local_facing == 5 || local_facing == 4 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x - start_point ),
                             POSITIVE_LAND_COORD_HIRES( location->y - 2 ),
                             0, 0, local_info ); /* G */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x - start_point ),
                             POSITIVE_LAND_COORD( location->y - 2 ),
                             0, 0, local_info ); /* G */
        }
    }
    if ( local_facing == 6 || local_facing == 5 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x - 2 ),
                             POSITIVE_LAND_COORD_HIRES( location->y - start_point ),
                             0, 0, local_info ); /* D */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x - 2 ),
                             POSITIVE_LAND_COORD( location->y - start_point ),
                             0, 0, local_info ); /* D */

        }
    }
    if ( local_facing == 7 || local_facing == 6 )
    {
        if ( hires )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location->x + 2 ),
                             POSITIVE_LAND_COORD_HIRES( location->y - start_point ),
                             0, 0, local_info ); /* C */
        }
        else
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location->x + 2 ),
                             POSITIVE_LAND_COORD( location->y - start_point ),
                             0, 0, local_info ); /* C */
        }
    }
}


#define	ACTIVE_PIXEL(px,py)		sketch_psetc(POSITIVE_LAND_COORD(px),POSITIVE_LAND_COORD(py),COLOR_RED)
#define	ERASER_PIXEL(px,py)		sketch_psetc(POSITIVE_LAND_COORD(px),POSITIVE_LAND_COORD(py), \
                                              (n_byte)local_val)

/*
 * kind = 0, draw normal ape
 * kind = 1, draw selected ape
 * kind = 2, erase normal ape
 * kind = 3, erase selected ape
 */
static void draw_apeloc( simulated_group *group, simulated_being  *bei, n_join *draw )
{
    simulated_timing *timing = sim_timing();
    n_pixel     *local_draw = draw->pixel_draw;
    void	    *local_info = draw->information;
    n_int		start = -1, stop = 2;
    n_int		time_coef = timing->real_time >> 4;
    n_int	    start_point = ( ( time_coef & 3 ) ) + 3;
    n_vect2     location;
    n_vect2     delta;
    being_space( bei, &location );
    land_convert_to_map( &location );
    delta.y = start;
    while ( delta.y < stop )
    {
        delta.x = start;
        while ( delta.x < stop )
        {
            n_vect2 screen;

            vect2_add( &screen, &location, &delta );

            ( *local_draw )( POSITIVE_LAND_COORD( screen.x ), POSITIVE_LAND_COORD( screen.y ), 0, 0, local_info );
            delta.x++;
        }
        delta.y++;
    }
    if ( bei == group->select )
    {
        n_int ty = -1;

        if ( toggle_follow )
        {
            selected_location.x = location.x;
            selected_location.y = location.y;
        }

        while ( ty < 2 )
        {
            ( *local_draw )( POSITIVE_LAND_COORD( location.x + ty ), POSITIVE_LAND_COORD( location.y - 2 ), 0, 0, local_info );
            ( *local_draw )( POSITIVE_LAND_COORD( location.x + ty ), POSITIVE_LAND_COORD( location.y + 2 ), 0, 0, local_info );
            ( *local_draw )( POSITIVE_LAND_COORD( location.x - 2 ), POSITIVE_LAND_COORD( location.y + ty ), 0, 0, local_info );
            ( *local_draw )( POSITIVE_LAND_COORD( location.x + 2 ), POSITIVE_LAND_COORD( location.y + ty ), 0, 0, local_info );
            ty++;
        }
        start_point++;
    }
    if ( being_speaking( bei ) )
    {
        draw_feelers( bei, local_info, &location, start_point, local_draw, 0 );
    }
}


/*
 * kind = 0, draw normal ape
 * kind = 1, draw selected ape
 * kind = 2, erase normal ape
 * kind = 3, erase selected ape
 */
static void draw_apeloc_hires( simulated_group *group, simulated_being  *bei, n_join *draw )
{
    simulated_timing *timing = sim_timing();

    n_pixel     *local_draw = draw->pixel_draw;
    void	    *local_info = draw->information;
    n_int		 start = -1, stop = 2;
    n_int		 time_coef = timing->real_time >> 4;
    n_int	     start_point = ( ( time_coef & 3 ) ) + 3;

    n_vect2      location;
    n_vect2      delta;

    being_high_res( bei, &location );

    delta.y = start;
    while ( delta.y < stop )
    {
        delta.x = start;
        while ( delta.x < stop )
        {
            n_vect2 screen_point;
            vect2_add( &screen_point, &location, &delta );
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( screen_point.x ), POSITIVE_LAND_COORD_HIRES( screen_point.y ), 0, 0, local_info );
            delta.x++;
        }
        delta.y++;
    }
    if ( bei == group->select )
    {
        n_int ty = -1;
        while ( ty < 2 )
        {
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location.x + ty ), POSITIVE_LAND_COORD_HIRES( location.y - 2 ), 0, 0, local_info );
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location.x + ty ), POSITIVE_LAND_COORD_HIRES( location.y + 2 ), 0, 0, local_info );
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location.x - 2 ), POSITIVE_LAND_COORD_HIRES( location.y + ty ), 0, 0, local_info );
            ( *local_draw )( POSITIVE_LAND_COORD_HIRES( location.x + 2 ), POSITIVE_LAND_COORD_HIRES( location.y + ty ), 0, 0, local_info );
            ty++;
        }
        start_point++;
    }
    if ( being_speaking( bei ) )
    {
        draw_feelers( bei, local_info, &location, start_point, local_draw, 1 );
    }
}

static void draw_region_hires( n_color8 *color )
{
    n_join	 local_draw;
    n_int    step = ( HI_RES_MAP_DIMENSION / 16 );
    n_int    ly = step - 1;

    local_draw.information = color->screen;

    if ( local_draw.information == 0L )
    {
        return;
    }
    local_draw.pixel_draw = &pixel_map_hires;
    while ( ly < HI_RES_MAP_DIMENSION )
    {
        math_line( 0, ly, HI_RES_MAP_DIMENSION, ly, &local_draw );
        math_line( ly, 0, ly, HI_RES_MAP_DIMENSION, &local_draw );
        ly += step;
    }
}


static void draw_region( simulated_being *local )
{
    n_join	 local_draw;
    n_byte *draw = draw_pointer( NUM_VIEW );
    n_int    ly = 31;

    if ( draw == 0L )
    {
        return;
    }

    local_draw.information = draw;
    local_draw.pixel_draw  = &pixel_map_checker;

    while ( ly < MAP_DIMENSION )
    {
        math_line( 0, ly, MAP_DIMENSION, ly, &local_draw );
        math_line( ly, 0, ly, MAP_DIMENSION, &local_draw );
        ly += 32;
    }

#ifdef TERRITORY_ON

    local_draw.pixel_draw  = &pixel_map;

    ly = 0;
    while ( ly < TERRITORY_DIMENSION )
    {
        n_int lx = 0;
        while ( lx < TERRITORY_DIMENSION )
        {
            n_string_block string_draw;
            n_int   value = local->events.territory[lx + ( ly * TERRITORY_DIMENSION )].familiarity;
            if ( value )
            {
                io_number_to_string( string_draw, ( n_uint )value );
                draw_string( string_draw, ( lx * 32 ) + 2, ( ly * 32 ) + 5, &local_draw );
            }
            lx++;
        }
        ly++;
    }
#endif
}

#ifdef ALPHA_WEATHER_DRAW

n_byte *draw_weather_grayscale( void )
{
    return weather_grayscale;
}

static void draw_weather( n_int toggle )
{
    if ( toggle )
    {
        n_c_int *local_pressure = land_weather( 0 );
        n_int loop = 0;
        while ( loop < MAP_AREA )
        {
            n_int value = local_pressure[ loop ] >> 7;

            if ( value < 0 )
            {
                value = 0;
            }
            if ( value > 255 )
            {
                value = 255;
            }
            weather_grayscale[ loop ] = ( n_byte )value;
            loop++;
        }
    }
    else
    {
        memory_erase( weather_grayscale, MAP_AREA );
    }
}

#else

n_byte *draw_weather_grayscale( void )
{
    return 0L;
}

static void draw_weather( n_int toggle )
{
    n_int map_dimensions = land_map_dimension();
    n_color8	 local_col;
    n_pixel	    *local_draw = &pixel_color8;
    void	    *local_info = &local_col;
    n_int        py = 0;

    if ( toggle == 0 )
    {
        return;
    }

    local_col.color = COLOR_WHITE;
    local_col.screen = draw_pointer( NUM_VIEW );
    if ( local_col.screen == 0L )
    {
        return;
    }
    while ( py < ( map_dimensions ) )
    {
        n_int	px = 0;
        n_int	scr_y = py;
        while ( px < ( map_dimensions ) )
        {
            n_int	scr_x = px;
            n_int	tmp = weather_pressure( WEATHER_TO_MAPSPACE( px ), WEATHER_TO_MAPSPACE( py ) ); /* from weather dimension to map dimension */

            if ( ( ( scr_x & 1 ) == 1 ) && ( ( scr_y & 1 ) == 1 ) )
            {
                if ( tmp > WEATHER_CLOUD )
                {
                    ( *local_draw )( scr_x, scr_y, 0, 0, local_info );
                }
            }
            if ( ( ( scr_x & 1 ) == 0 ) && ( ( scr_y & 1 ) == 0 ) )
            {
                if ( tmp > WEATHER_RAIN )
                {
                    ( *local_draw )( scr_x, scr_y, 0, 0, local_info );
                }
            }
            px++;
        }
        py++;
    }
}
#endif

static void draw_count_number( n_uint count, n_string value )
{
    n_uint	lp = 0, division = 1000000;
    while ( lp < 6 )
    {
        if ( ( count + 1 ) > division )
        {
            if ( division != 0 )
            {
                value[lp] = ( n_char )( '0' + ( ( count / division ) % 10 ) );
            }
            else
            {
                value[lp] = ( n_char )( '0' );
            }
        }
        division /= 10;
        lp++;
    }
    value[6] = ( '0' + ( ( count / 1 ) % 10 ) );
}

static void draw_metrics( n_uint bcps, n_uint fps, n_join *local_mono )
{
    n_int offset_y = 8;

    if ( bcps )
    {
        n_string_block  bcps_string = {' ', ' ', ' ', ' ', ' ', ' ', 'X', ' ',
                                       'B', '/', 'S', ' ', ' ', ' ', ' ', ' ', ' ', 0
                                      };
        draw_count_number( bcps, bcps_string );
        draw_string( bcps_string, ( window_dim_x / 2 ) - 60, offset_y, local_mono );
        offset_y += CHARACTER_HEIGHT;
    }
    if ( fps )
    {
        n_string_block  fps_string = {' ', ' ', ' ', ' ', ' ', ' ', 'X', ' ',
                                      'F', '/', 'S', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 0
                                     };
        draw_count_number( fps, fps_string );
        draw_string( fps_string, ( window_dim_x / 2 ) - 60, offset_y, local_mono );
    }
}

/* draws the rotating brain, this is always draw and never erase */

#ifdef BRAIN_ON
#ifndef SIMULATED_APE_CLIENT

static void draw_brain( simulated_group *group, n_vect2 *dimensions )
{
    n_byte  draw_big = 1;
    if ( ( group->select == 0L ) || ( number_errors != 0 ) || changing_size )
    {
        return;
    }

    {
        n_byte       *local       = being_brain( group->select );
        n_join	      local_mono;
        n_int         turn_y = ( n_int )tilt_z;
        n_int         turn_z = ( n_int )tilt_y;
        n_pixel	     *local_draw_brain = &pixel_white;
        void	     *local_info_brain = draw_pointer( NUM_TERRAIN );
        n_int	      lpx  = 0;
        n_int	      loop = 0;
        n_int         center_x = dimensions->x >> 1;
        n_int         center_y = dimensions->y >> 1;
        n_int	      act_x2a, term_1a, term_2a;

        n_byte	     *brainptr = local;
        n_byte	     *obrainptr = &local[ 32 * 32 * 32 ];
        n_int         a11, a12, a13, a21, a23, a31, a32, a33;

        n_vect2       vect_y, vect_z;

        vect2_direction( &vect_y, turn_y, 105 );
        vect2_direction( &vect_z, turn_z, 23 ); //NEW_SD_MULTIPLE/1152);

        a32 =  vect_y.x;
        a12 =  vect_y.y;
        a21 =  vect_z.x;
        a23 =  vect_z.y;

        a11 = -( ( a32 * a23 ) >> 8 );
        a13 =  ( ( a32 * a21 ) >> 8 );
        a31 =  ( ( a12 * a23 ) >> 8 );
        a33 = -( ( a12 * a21 ) >> 8 );

        local_mono.pixel_draw  = &pixel_white;
        local_mono.information = draw_pointer( NUM_TERRAIN );

        if ( local == 0L )
        {
            return;
        }

        a12 = ( a12 * 1152 ) >> 8;
        a32 = ( a32 * 1152 ) >> 8;
        act_x2a = -( ( a21 + a23 ) << 4 );
        term_1a = -( ( a11 + a12 + a13 ) << 4 );
        term_2a = -( ( a31 + a32 + a33 ) << 4 );
        while ( lpx < 32 )
        {
            n_int	lpy = 0;
            n_int	term_1 = term_1a;
            n_int	term_2 = term_2a;
            while ( lpy < 32 )
            {
                n_int	act_x1 = term_1 ;
                n_int	act_x2 = act_x2a;
                n_int	act_y1 = term_2 ;
                n_int	lpz = 0;
                while ( lpz < 32 )
                {
                    if ( ( brainptr[loop] ^ obrainptr[loop] ) >> 5 )
                    {
                        /* TODO Fix this re-scaling bug */
                        n_int	scr_z = 256 + ( act_x1 >> 11 );
                        n_int	s_x = ( ( act_x2 * scr_z ) >> 17 ) + center_x;
                        n_int	s_y = ( ( act_y1 * scr_z ) >> 17 ) + center_y - 100; //120
                        ( *local_draw_brain )( s_x, s_y, 0, 0, local_info_brain );
                        if ( ( act_x1 > 0 ) && draw_big )
                        {
                            ( *local_draw_brain )( s_x - 1, s_y, 0, 0, local_info_brain );
                            ( *local_draw_brain )( s_x + 1, s_y, 0, 0, local_info_brain );
                            ( *local_draw_brain )( s_x, s_y - 1, 0, 0, local_info_brain );
                            ( *local_draw_brain )( s_x, s_y + 1, 0, 0, local_info_brain );
                        }
                    }
                    act_x1 += a13;
                    act_x2 += a23;
                    act_y1 += a33;
                    loop++;
                    lpz++;
                }
                term_1 += a12;
                term_2 += a32;
                lpy++;
            }
            term_1a += a11;
            term_2a += a31;
            act_x2a += a21;
            lpx++;
        }
        tilt_y = ( tilt_y + 2 ) & 255;
    }
}

#endif
#endif

n_int draw_error( n_constant_string error_text, n_constant_string location, n_int line_number )
{
    n_int	           loop = 0;
    n_int	           error_char_copy;
    n_string_block     simulation_date_time = {0};
    n_string_block     simulation_date_time_error = {0};
    n_int              position = 0;

    if ( error_text )
    {
#ifdef SIMULATED_APE_ASSERT
        printf( "ERROR: %s\n", error_text );
#endif

#ifdef EXPLICIT_DEBUG
        printf( "ERROR: %s (%s, %ld)\n", error_text, location, line_number );
#endif
        io_time_to_string( simulation_date_time );

        io_string_write( simulation_date_time_error, simulation_date_time, &position );
        io_string_write( simulation_date_time_error, " ", &position );


        if ( number_errors == MAX_NUMBER_ERRORS )
        {
            io_string_write( simulation_date_time_error, " ** Maximum errors reached **", &position );
        }
        else
        {
            io_string_write( simulation_date_time_error, ( n_string )error_text, &position );
        }
    }
    else
    {
        number_errors = 0;
        return 0;
    }

    if ( io_command_line_execution() )
    {
        io_console_out( simulation_date_time_error );
        return -1;
    }
    do
    {
        error_char_copy = error_array[number_errors][loop] = simulation_date_time_error[loop];
        loop++;
    }
    while ( ( loop < STRING_BLOCK_SIZE ) && ( error_char_copy != 0 ) );

    error_array[number_errors][loop] = 0;

    if ( number_errors != MAX_NUMBER_ERRORS )
    {
        number_errors++;
    }
    return -1;
}

#ifndef SIMULATED_APE_CLIENT
static void draw_remains( simulated_group *group, n_byte *screen )
{
    simulated_remains *remains = &( group->remains );
    n_int           loop = 0;
    while ( loop < remains->count )
    {
        simulated_idead_body *body = &( remains->bodies[loop] );
        n_int             lx = MAP_DIMENSION - 2;
        n_vect2           location;

        vect2_byte2( &location, body->location );
        land_convert_to_map( &location );

        while ( lx < ( MAP_DIMENSION + 3 ) )
        {
            screen[( ( location.x + lx ) & ( MAP_DIMENSION - 1 ) ) + ( ( ( location.y ) & ( MAP_DIMENSION - 1 ) ) * MAP_DIMENSION )] = COLOR_WHITE;
            screen[( ( location.x ) & ( MAP_DIMENSION - 1 ) ) + ( ( ( location.y + lx ) & ( MAP_DIMENSION - 1 ) ) * MAP_DIMENSION )] = COLOR_WHITE;
            lx++;
        }
        loop++;
    }
}
#endif

static void draw_tides( n_byte *map, n_byte *screen, n_byte tide )
{
    n_byte	tide_compress[45];
    n_int   lp = 0;
    n_byte	tide_point = tide - 106;
    n_int	ar = 106;
    n_int	dr = 128;
    n_int	fl = tide_point;
    n_int	fp = 0;

    if ( toggle_tidedaylight )
    {
        return;
    }

    while ( lp < 45 )
    {
        tide_compress[lp] = ( n_byte )( ( ( ar * ( fl - lp ) ) + ( dr * ( lp - fp ) ) ) / ( fl - fp ) );
        if ( lp == tide_point )
        {
            ar = 128;
            dr = 150;
            fl = 150 - 106;
            fp = tide_point;
        }
        lp++;
    }

    lp = 0;
    while ( lp < MAP_AREA )
    {
        n_byte val = map[lp];
        if ( ( val > 105 ) && ( val < 151 ) )
        {
            screen[lp] = tide_compress[val - 106];
        }
        lp++;
    }
}

static void draw_tides_hi_res( n_byte *data, n_byte4 *block, n_byte tide )
{
    n_byte	tide_compress[45];
    n_int  lp = 0;
    n_byte	tide_point = tide - 106;
    n_int	ar = 106;
    n_int	dr = 128;
    n_int	fl = tide_point;
    n_int	fp = 0;

    if ( toggle_tidedaylight )
    {
        return;
    }

    while ( lp < 45 )
    {
        tide_compress[lp] = ( n_byte )( ( ( ar * ( fl - lp ) ) + ( dr * ( lp - fp ) ) ) / ( fl - fp ) );
        if ( lp == tide_point )
        {
            ar = 128;
            dr = 150;
            fl = 150 - 106;
            fp = tide_point;
        }
        lp++;
    }
    lp = 0;
    while ( lp < HI_RES_MAP_AREA )
    {
        n_byte4  block_group = block[lp >> 5];

        if ( block_group != 0 )
        {
            n_int local_loop = lp << 1;
            n_int local_loop_end = local_loop + 64;
            n_int lp = 0;
            while ( local_loop < local_loop_end )
            {
                if ( ( block_group >> lp ) & 1 )
                {
                    data[local_loop | 1] = tide_compress[data[local_loop] - 106];
                }
                local_loop += 2;
                lp++;
            }
        }
        lp += 32;
    }
}

static void draw_ape( simulated_group *group, simulated_being *being, n_byte color, n_color8 *col8 )
{
    n_vect2         location_vect;
    n_join          local_8bit;

    col8->color = color;
    being_space( being, &location_vect );

    if ( col8->screen == land_topography_highdef() )
    {
        local_8bit.pixel_draw = &pixel_color8_hires;
    }
    else
    {
        local_8bit.pixel_draw  = &pixel_color8;
    }
    local_8bit.information = col8;
    if ( col8->screen == land_topography_highdef() )
    {
        draw_apeloc_hires( group, being, &local_8bit );
    }
    else
    {
        draw_apeloc( group, being, &local_8bit );
    }
}

static void draw_apes_loop( simulated_group *group, simulated_being *bei, void *data )
{
    n_join			local_8bit;
    n_color8        local_col;
    n_vect2         location_vect;
    /* makes this use thread safe */
    memory_copy( data, ( n_byte * )&local_col, sizeof( n_color8 ) );

    being_space( bei, &location_vect );
    if ( being_line_of_sight( group->select, &location_vect ) == 1 )
    {
        local_col.color = COLOR_RED;
    }
    else
    {
        local_col.color = COLOR_RED_DARK;
    }

    if ( local_col.screen == land_topography_highdef() )
    {
        local_8bit.pixel_draw = &pixel_color8_hires;
    }
    else
    {
        local_8bit.pixel_draw  = &pixel_color8;
    }

    local_8bit.information = &local_col;

    if ( local_col.screen == land_topography_highdef() )
    {
        draw_apeloc_hires( group, bei, &local_8bit );
    }
    else
    {
        draw_apeloc( group, bei, &local_8bit );
    }

}

static void draw_apes( simulated_group *group, n_byte lores )
{
    n_color8		local_col;

    if ( lores ) /* set up drawing environ */
    {
        local_col.screen = draw_pointer( NUM_VIEW );
        memory_copy( land_topography(), local_col.screen, MAP_AREA );
    }
    else
    {
        draw_undraw();
        local_col.screen = land_topography_highdef();
    }
    if ( lores )
    {
        draw_tides( land_topography(), local_col.screen, land_tide_level() );
        if ( toggle_territory )
        {
            draw_region( group->select );
        }
#ifndef SIMULATED_APE_CLIENT
        draw_remains( group, local_col.screen );
#endif
    }
    else
    {
        static n_byte local_tide;
        if ( local_tide != land_tide_level() )
        {
            local_tide = land_tide_level();
            draw_tides_hi_res( land_topography_highdef(), land_highres_tide(), local_tide );
        }
        if ( toggle_territory )
        {
            draw_region_hires( &local_col );
        }
    }

    if ( group->select )
    {
        loop_no_thread( group, 0L, draw_apes_loop, &local_col );
    }
}

static void draw_social_web( simulated_group *group, n_byte lores )
{
    n_int loop = 0;
    n_int friends_count = command_relationship_count( 0 );
    n_int enemies_count = command_relationship_count( 1 );
    n_int attract_count = command_relationship_count( 2 );
    n_color8        local_col;


    if ( lores ) /* set up drawing environ */
    {
        local_col.screen = draw_pointer( NUM_VIEW );
    }
    else
    {
        local_col.screen = land_topography_highdef();
    }

    loop = 0;
    while ( loop < attract_count )
    {
        simulated_being *attract = command_relationship_being( group, 2, loop );
        if ( attract )
        {
            draw_ape( group, attract, COLOR_BLUE, &local_col );
        }
        loop++;
    }
    loop = 0;
    while ( loop < friends_count )
    {
        simulated_being *friend = command_relationship_being( group, 0, loop );
        if ( friend )
        {
            draw_ape( group, friend, COLOR_WHITE, &local_col );
        }
        loop++;
    }
    loop = 0;
    while ( loop < enemies_count )
    {
        simulated_being *enemy = command_relationship_being( group, 1, loop );
        if ( enemy )
        {
            draw_ape( group, enemy, COLOR_BLACK, &local_col );
        }
        loop++;
    }
}

static void draw_errors( void )
{
    n_join			local_mono;
    local_mono.pixel_draw  = &pixel_white;
    local_mono.information = draw_pointer( NUM_TERRAIN );
    if ( number_errors != 0 )
    {
        n_int	loop = 0;
        while ( loop < ( number_errors + 1 ) )
        {
            draw_string( error_array[loop], 40, ( loop * CHARACTER_HEIGHT ) + 62, &local_mono );
            loop++;
        }
    }
}

#ifdef DEBUG_CONTROL_STRING
static n_uint debug_hash_old = 0;
#endif

static void draw_control( simulated_group *group, n_int window_dim_x, n_int window_dim_y )
{
    n_string       string_stats;
    n_join         local_mono;
    n_int          location_ptr = 0;
    n_int          max_characters;


    max_characters = ( window_dim_x - ( 2 * CHARACTER_WIDTH ) ) / CHARACTER_WIDTH;

    if ( group == 0L )
    {
        return;
    }

    if ( sim_get_writing_output() )
    {
        return;
    }

    sim_control_erase( window_dim_x, window_dim_y, max_characters );
    character_location = 1;

    smart_character_toggle = 0;
    bold_character_toggle = 0;

    local_mono.pixel_draw  = &pixel_control;
    local_mono.information = draw_pointer( NUM_CONTROL );

    memory_erase( local_mono.information, ( n_uint )( window_dim_x * window_dim_y ) );
    string_stats = sim_output_string();

#ifdef DEBUG_CONTROL_STRING
    {
        n_uint new_hash = math_hash( string_stats, io_length( string_stats, STRING_BLOCK_SIZE ) );
        if ( new_hash != debug_hash_old )
        {
            printf( "%s\n\n", string_stats );
            debug_hash_old = new_hash;
        }
    }
#endif
    {
        n_char character = 'Y';
        n_int  line_count = 0;
        do
        {
            n_string_block string_line = {0};
            n_string_block string_second_line = {0};
            n_int location_line_ptr = 0;

            do
            {
                character = string_stats[location_ptr++];
                if ( character != '\n' )
                {
                    string_line[location_line_ptr++] = character;
                }
            }
            while ( ( character != '\n' ) && ( location_ptr < STRING_BLOCK_SIZE ) );

            if ( location_line_ptr > max_characters )
            {
                n_int is_space = 0;
                n_int second_line_location = max_characters;
                do
                {
                    n_char character = string_line[location_line_ptr];
                    if ( is_space )
                    {
                        string_second_line[second_line_location] = ' ';
                    }
                    else
                    {
                        string_second_line[second_line_location] = character;
                        string_line[location_line_ptr] = 0;
                        location_line_ptr--;
                    }
                    second_line_location--;
                    if ( character == ' ' && location_line_ptr < ( max_characters - 2 ) )
                    {
                        is_space = 1;
                    }
                }
                while ( second_line_location > -1 );
            }
            draw_smart_string( string_line, CHARACTER_WIDTH, CHARACTER_WIDTH + ( line_count * CHARACTER_HEIGHT ), &local_mono );
            if ( string_second_line[0] )
            {
                line_count++;
                draw_smart_string( string_second_line, CHARACTER_WIDTH, CHARACTER_WIDTH + ( line_count * CHARACTER_HEIGHT ), &local_mono );
            }
            line_count++;
        }
        while ( ( string_stats[location_ptr] != 0 ) && ( location_ptr < STRING_BLOCK_SIZE ) );
    }
}


#ifdef BRAINCODE_ON
static void draw_line_braincode( n_string pointer, n_int line )
{
    n_join	local_mono;
    if ( changing_size )
    {
        return;
    }
    local_mono.pixel_draw  = &pixel_white;
    local_mono.information = draw_pointer( NUM_TERRAIN );
    draw_string( pointer, 4 + ( window_dim_x / 2 ) - 256, ( line * CHARACTER_HEIGHT ) + 68, &local_mono );
}
#endif

void  draw_terrain_coord( n_int *co_x, n_int *co_y )
{
    *co_x = window_dim_x;
    *co_y = window_dim_y;
}


void  draw_window( n_int dim_x, n_int dim_y )
{
    window_dim_x = dim_x;
    window_dim_y = dim_y;
}

#ifdef SKELETON_RENDER
static void  draw_skeleton( simulated_group *group )
{
    n_genetics *genetics = 0L;

    if ( group->select )
    {
        genetics = group->select->constant.genetics;
    }


    if ( genetics != 0L )
    {

        n_int dim_x = window_dim_x;
        n_int dim_y = window_dim_y;

        /** set this to a non-zero value to show key points on the skeleton
         which may be useful for debugging */
        /* doesn't work with Ofast optimization */
        n_vect2 dim;
        n_vect2 tp;
        n_vect2 bp;

        n_byte  show_skeleton_keypoints = 0;

        dim.x = dim_x;
        dim.y = dim_y;

        tp.x = dim_x * 10 / 100;
        tp.y = dim_y * 10 / 100;

        bp.x = dim_x * 40 / 100;
        bp.y = dim_y * 90 / 100;

        vascular_draw( genetics, draw_pointer( NUM_TERRAIN ),
                       &dim,

                       &tp,
                       &bp,

                       1, 0, /* don't erase here */
                       30, 0, 20, 20, 0,

                       show_skeleton_keypoints );
    }
}
#endif

void  draw_cycle( n_byte size_changed, n_byte kind )
{
    simulated_group *group = sim_group();

    if ( sim_new() )
    {
        if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_TERRAIN ) )
        {
            draw_about_information( 1 );
        }
#ifdef APESIM_IOS
        if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_CONTROL ) )
        {
            draw_about_information( 0 );
        }
#endif
        return;
    }
    changing_size = size_changed;
    if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_CONTROL ) )
    {
        draw_control( group, window_dim_x, window_dim_y );
    }

    if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_VIEW ) )
    {
        draw_apes( group, 1 );  /* lo res */
    }
    if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_TERRAIN ) )
    {
        draw_apes( group, 0 );  /* hi res */
    }

    if ( toggle_social_web )
    {
        if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_VIEW ) )
        {
            draw_social_web( group, 1 );
        }
        if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_TERRAIN ) )
        {
            draw_social_web( group, 0 );
        }
    }

    if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_TERRAIN ) )
    {
        simulated_timing *timing = sim_timing();

        n_vect2            local_vect;
        n_join             local_mono;

        local_mono.pixel_draw  = &pixel_white;
        local_mono.information = draw_pointer( NUM_TERRAIN );

        local_vect.x = window_dim_x;
        local_vect.y = window_dim_y;

        draw_terrain( group, &local_vect );
#ifdef BRAIN_ON
        if ( toggle_brain )
        {
            draw_brain( group, &local_vect );
        }
#endif
#ifdef BRAINCODE_ON
        if ( toggle_braincode )
        {
            command_populate_braincode( group, draw_line_braincode );
        }
#endif
        draw_meters( group );
        draw_errors(); /* 12 */
#ifndef SIMULATED_APE_CLIENT
        draw_metrics( 0, timing->delta_frames, &local_mono );
#endif

#ifdef SKELETON_RENDER
        if ( group->select != 0L )
        {
            draw_skeleton( group );
        }
#endif
    }
    if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_VIEW ) )
    {
#ifdef WEATHER_ON
        draw_weather( toggle_weather ); /* 10 */
#endif
    }
    if ( CHECK_DRAW_WINDOW( kind, DRAW_WINDOW_TERRAIN ) )
    {
        if ( check_about )
        {
            draw_about_information( 1 );
        }
    }

#ifdef MULTITOUCH_CONTROLS
    draw_tc_controls( &local_mono );
#endif
}
