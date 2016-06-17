/****************************************************************

 draw.c

 =============================================================

 Copyright 1996-2016 Tom Barbalet. All rights reserved.

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

/* from ASCII 32 - 127, corresponding to the seg14 results */

/* n_byte segment = seg14[ conv[ character_value ]]; */
static const n_byte	conv[ 96 ] =
{
    0, 40, 41, 0, 0, 0, 0, 42, 43, 44, 38, 39, 45, 11, 46, 47, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 48, 49, 0, 50, 0, 51, 0, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 52, 53, 54, 0, 55, 56,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 57, 58, 59, 0, 0
};

/* one bit per segment */
static const n_byte2 seg14[ 60 ] =
{
    0x0000, 0x3F00, 0x1800, 0x36C0, 0x3CC0, 0x19C0, 0x2DC0, 0x2FC0, 0x3800, 0x3FC0,
    0x3DC0, 0x00C0, 0x3BC0, 0x3CA1, 0x2700, 0x3C21, 0x27C0, 0x23C0, 0x2F80, 0x1BC0,
    0x2421, 0x1E00, 0x0354, 0x0700, 0x1B06, 0x1B12, 0x3F00, 0x33C0, 0x3F10, 0x33D0,
    0x2DC0, 0x2021, 0x1F00, 0x030C, 0x1B18, 0x001E, 0x11E0, 0x240C, 0x00FF, 0x00E1,

    0x8001, 0x0101, 0x0001, 0x0014, 0x000A, 0x0008, 0x8000, 0x000C, 0xC000, 0x4008,
    0x04C0, 0xA004, 0x2700, 0x0012, 0x3402, 0x0400, 0x0002, 0x244A, 0x0021, 0x2494
};


#define	MAX_NUMBER_ERRORS	35

static n_byte           number_errors;
static n_string_block	error_array[MAX_NUMBER_ERRORS + 1];

#ifdef BRAIN_ON
static n_uint	tilt_y = 0;
#endif

n_byte	check_about = 0;
n_uint	tilt_z = 118;
n_byte  terrain_turn = 0;

static n_int toggle_weather = 1;
static n_int toggle_brain = 1;
static n_int toggle_braincode = 0;
static n_int toggle_territory = 0;
static n_int toggle_tidedaylight = 0;

n_int draw_toggle_weather(void)
{
    toggle_weather ^= 1;
    return toggle_weather;
}

n_int draw_toggle_brain(void)
{
    toggle_brain ^= 1;
    return toggle_brain;
}

n_int draw_toggle_braincode(void)
{
    toggle_braincode ^= 1;
    return toggle_braincode;
}

n_int draw_toggle_territory(void)
{
    toggle_territory ^= 1;
    return toggle_territory;
}

n_int draw_toggle_tide_daylight(void)
{
    toggle_tidedaylight ^= 1;
    return toggle_tidedaylight;
}

/* this needs to be grouped eventually, it is here as a test */

#define UNDRAW_MAX          100000

static n_byte * undraw_location[UNDRAW_MAX];
static n_byte   undraw_color[UNDRAW_MAX];
static n_int    undraw_count = 0;

void draw_undraw_clear(void)
{
    undraw_count = 0;
}

static void draw_undraw()
{
    if (undraw_count == 0) return;
    undraw_count--;
    do
    {
        *undraw_location[undraw_count] = undraw_color[undraw_count];
        undraw_count--;
    }
    while(undraw_count > -1);
    undraw_count = 0;
}

static n_byte * local_offscreen = 0L;

n_byte * draw_offscreen(n_byte * value)
{
    local_offscreen = value;
    return local_offscreen;
}

static n_byte pixel_color8_hires(n_int px, n_int py, n_int dx, n_int dy, void * information)
{
    n_color8	*local_col = information;
    n_byte      *location = &local_col->screen[ (px<<1) | (py<<(HI_RES_MAP_BITS+1)) | 1 ];
    undraw_location[undraw_count] = location;
    undraw_color[undraw_count] = location[0];
    undraw_count++;

    if (undraw_count > UNDRAW_MAX)
    {
        (void)SHOW_ERROR("Erase count outside limit");
    }
    location[0] = local_col->color;
    return 0;
}

static n_byte pixel_color8(n_int px, n_int py, n_int dx, n_int dy, void * information)
{
    n_color8	*local_col = information;
    local_col->screen[ px | (py<<MAP_BITS) ] = local_col->color;
    return 0;
}

static n_int terrain_dim_x = 512;
static n_int terrain_dim_y = 511;

static n_byte pixel_map(n_int px, n_int py, n_int dx, n_int dy, void * information)
{
    n_byte *byte_info = information;
    byte_info[ px | (py<<MAP_BITS) ] = COLOUR_YELLOW;
    return 0;
}

static n_byte pixel_map_checker(n_int px, n_int py, n_int dx, n_int dy, void * information)
{
    if ((px + py) & 1)
    {
        return pixel_map(px, py, dx, dy, information);
    }
    return 0;
}

static n_byte pixel_overlay(n_int px, n_int py, n_int dx, n_int dy, void * information)
{
    n_byte *byte_info = information;
    byte_info[ px + (py * terrain_dim_x) ] = COLOUR_YELLOW;
    return 0;
}

static n_byte pixel_black(n_int px, n_int py, n_int dx, n_int dy, void * information)
{
    n_byte *byte_info = information;
    byte_info[ px + (py * terrain_dim_x) ] = 0;
    return 0;
}

#ifdef BRAINCODE_ON

static n_byte pixel_grey(n_int px, n_int py, n_int dx, n_int dy, void * information)
{
    n_byte *byte_info = information;
    byte_info[ px + (py * terrain_dim_x) ] = COLOUR_GREY;
    return 0;
}

#endif

n_byte * draw_pointer(n_byte which_one)
{
    if (local_offscreen)
    {
        switch(which_one)
        {
        case NUM_TERRAIN:
            return TERRAINWINDOW(local_offscreen);
            break;
        case NUM_VIEW:
            return VIEWWINDOW(local_offscreen);
            break;
        }
    }
    return 0L;
}

#ifdef MULTITOUCH_CONTROLS

extern touch_control_state tc_state;

void draw_tc_controls(n_join * local_mono)
{
    const n_int half_y = terrain_dim_y / 2;
    n_vect2 point1, point2, point3;

    if (tc_state == TCS_SHOW_CONTROLS)
    {
        vect2_populate(&point1, 5, half_y + TC_OFFSET_Y);
        vect2_populate(&point2, 5, half_y - TC_OFFSET_Y);
        vect2_populate(&point3, TC_FRACTION_X - 5, half_y);

        math_line_vect(&point1, &point2, local_mono);
        math_line_vect(&point2, &point3, local_mono);
        math_line_vect(&point3, &point1, local_mono);

        vect2_populate(&point1, terrain_dim_x - 5, half_y + TC_OFFSET_Y);
        vect2_populate(&point2, terrain_dim_x - 5, half_y - TC_OFFSET_Y);
        vect2_populate(&point3, terrain_dim_x - TC_FRACTION_X + 5, half_y);

        math_line_vect(&point1, &point2, local_mono);
        math_line_vect(&point2, &point3, local_mono);
        math_line_vect(&point3, &point1, local_mono);
    }

    if (tc_state == TCS_LEFT_STATE_CONTROLS)
    {
        vect2_populate(&point1, TC_FRACTION_X - 5, half_y + TC_OFFSET_Y);
        vect2_populate(&point2, TC_FRACTION_X - 5, half_y - TC_OFFSET_Y);
        vect2_populate(&point3, 5, half_y);

        math_line_vect(&point1, &point2, local_mono);
        math_line_vect(&point2, &point3, local_mono);
        math_line_vect(&point3, &point1, local_mono);
    }

    if (tc_state == TCS_RIGHT_STATE_CONTROLS)
    {
        vect2_populate(&point1, terrain_dim_x - TC_FRACTION_X + 5, half_y + TC_OFFSET_Y);
        vect2_populate(&point2, terrain_dim_x - TC_FRACTION_X + 5, half_y - TC_OFFSET_Y);
        vect2_populate(&point3, terrain_dim_x - 5, half_y);

        math_line_vect(&point1, &point2, local_mono);
        math_line_vect(&point2, &point3, local_mono);
        math_line_vect(&point3, &point1, local_mono);
    }
}

#endif

/*	shows the about information */
void draw_about(n_constant_string platform)
{
    n_join	local_draw;
    n_byte *buffer = draw_pointer(NUM_VIEW);
#if (MAP_BITS == 9)
    n_int   line_y_offset = 128 + 24;
    n_int   linx_x_offset = 84;
    n_int   tab_offset  = 36;
#else
    n_int   line_y_offset = 12;
    n_int   linx_x_offset = 12;
    n_int   tab_offset  = 0;
#endif
    if(check_about == 1 || buffer == 0L)
    {
        check_about = 0;
        return;
    }

    local_draw.information = buffer;
    local_draw.pixel_draw  = &pixel_map;
#if (MAP_BITS == 9)
    {
        n_int   loop = 0;
        while (loop < 214)
        {
            n_int  py = (MAP_DIMENSION/2) - 128 + loop;
            const n_int px = (MAP_DIMENSION/2) - 200;
            n_byte * from_point = &buffer[(py*MAP_DIMENSION) + px];
            io_erase(from_point, 400);
            loop++;
        }
    }
#endif

    draw_string(SHORT_VERSION_NAME,     linx_x_offset, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;
#if (MAP_BITS == 9)
    draw_string(FULL_VERSION_COPYRIGHT, linx_x_offset + tab_offset, line_y_offset, &local_draw);
    line_y_offset += 12;

    draw_string(COPYRIGHT_FOLLOW,       linx_x_offset + tab_offset, line_y_offset, &local_draw);
#else
    draw_string(COPYRIGHT_DATE,         linx_x_offset + tab_offset, line_y_offset, &local_draw);
    line_y_offset += 12;

    draw_string(COPYRIGHT_NAME,         linx_x_offset + tab_offset, line_y_offset, &local_draw);
#endif
    line_y_offset += 12;
    line_y_offset += 12;

    draw_string(FULL_DATE,              linx_x_offset, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;

    draw_string(platform,               linx_x_offset + tab_offset, line_y_offset, &local_draw);
#if (MAP_BITS == 9)
    line_y_offset += 12;
    line_y_offset += 12;

    draw_string("This software and Noble Ape are a continuing ", linx_x_offset, line_y_offset, &local_draw);
    line_y_offset += 12;

    draw_string("work of Tom Barbalet begun on 13 June 1996.", linx_x_offset, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;

    draw_string("No apes or cats were harmed in the writing ", linx_x_offset, line_y_offset, &local_draw);
    line_y_offset += 12;

    draw_string("of this software.", linx_x_offset, line_y_offset, &local_draw);
#endif
    check_about = 1;
}

/* draws a string starting at point (off_x,off_y) */

#define	ledfir(x, y, dx, dy, c)	if(((val >> c)&1)) (*local_draw)((x + off_x + offset),((y + off_y)), dx, dy, local_info)


/**
 This is used to produce letter LED style letters through the generic
 draw function specified.
 @param str The string to be drawn.
 @param off_x The starting x location for the string to be drawn.
 @param off_y The starting y location for the string to be drawn.
 @param draw The generic draw function used to draw the character.
 */
void draw_string_line(n_constant_string str, n_int off_x, n_int off_y, n_join * draw)
{
    n_pixel	* local_draw = draw->pixel_draw;
    void	* local_info = draw->information;

    n_int	char_loop = 0;
    while (str[char_loop] > 31)
    {
        n_int	val = seg14[conv[str[char_loop] - 32]];
        n_int	offset = char_loop << 3;
        /* draw the character as a 14-segment LCD/LED output */

        ledfir(3, 8, 0, 0, 15);

        ledfir(3, 2, 0, 0, 14);

        ledfir(1, 0, 4, 0, 13);

        ledfir(6, 1, 0, 2, 12);

        ledfir(6, 5, 0, 2, 11);

        ledfir(1, 8, 4, 0, 10);

        ledfir(0, 5, 0, 2, 9);

        ledfir(0, 1, 0, 2, 8);

        ledfir(4, 4, 1, 0, 7);

        ledfir(1, 4, 1, 0, 6);

        ledfir(3, 5, 0, 2, 5);

        ledfir(4, 6, 0, 1, 4);

        ledfir(2, 6, -1, 1, 3);

        ledfir(4, 2, 1, -1, 2);

        ledfir(1, 1, 1, 1, 1);

        ledfir(3, 1, 0, 2, 0);
        char_loop ++;
    }
}


static n_byte draw_character_line(n_int px, n_int py, n_int dx, n_int dy, void * data)
{
    return math_join(px, py, dx, dy, data);
}

void draw_string(n_constant_string str, n_int off_x, n_int off_y, n_join * draw)
{
    n_join local_pixel;
    local_pixel.pixel_draw = &draw_character_line;
    local_pixel.information = draw;
    draw_string_line(str, off_x, off_y, &local_pixel);
}


/* this is the ocelot landscape algorithm */

#define POS_HIRES(num) ((num+(HI_RES_MAP_DIMENSION*2))&(HI_RES_MAP_DIMENSION-1))

#define CONVERT_X(x, cx)  (n_uint)((POS_HIRES((x)+cx)) )
#define CONVERT_Y(y, cy)  (n_uint)((POS_HIRES((y)+cy)) << HI_RES_MAP_BITS)

#define CONVERT_XY(x,y)   (CONVERT_X(x) | CONVERT_Y(y))

#define	SUBSTA(c)	((c<<8)|c)

static n_byte2	color_group[256*3];

n_byte	land_points[] =
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

void draw_fit(n_byte * points, n_byte2 * color_fit)
{
    /* performs a linear interpolation of n 8-bit points to 256 16-bit blend values */
    n_int	lp = 0, lp2 = 0;
    n_int	dr = 0, dg = 0, db = 0;
    n_int	ar = 0, ag = 0, ab = 0, cntr = 0;
    n_int	fp = 0, fl = 0, del_c = 0;
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
            n_int	del_a = (fl - lp), del_b = (lp - fp);

            color_fit[lp2++] = (n_byte2)(((ar * del_a) + (dr * del_b)) / del_c);
            color_fit[lp2++] = (n_byte2)(((ag * del_a) + (dg * del_b)) / del_c);
            color_fit[lp2++] = (n_byte2)(((ab * del_a) + (db * del_b)) / del_c);
        }
        else
        {
            color_fit[lp2++] = (n_byte2)(ar);
            color_fit[lp2++] = (n_byte2)(ag);
            color_fit[lp2++] = (n_byte2)(ab);
        }
        lp ++;
    }
    if (points == land_points)
    {
        color_fit[(COLOUR_GREY*3)    ] = 0xcccc;
        color_fit[(COLOUR_GREY*3) + 1] = 0xcccc;
        color_fit[(COLOUR_GREY*3) + 2] = 0xcccc;

        color_fit[(COLOUR_YELLOW*3)    ] = 0xeeff;
        color_fit[(COLOUR_YELLOW*3) + 1] = 0xeeff;
        color_fit[(COLOUR_YELLOW*3) + 2] = 0x2222;

        color_fit[(COLOUR_RED_DARK*3)    ] = (0xeeff * 3) >> 2;
        color_fit[(COLOUR_RED_DARK*3) + 1] = 0x0000;
        color_fit[(COLOUR_RED_DARK*3) + 2] = 0x0000;

        color_fit[(COLOUR_RED*3)    ] = 0xeeff;
        color_fit[(COLOUR_RED*3) + 1] = 0x0000;
        color_fit[(COLOUR_RED*3) + 2] = 0x0000;

    }
}

void draw_color_group_update(n_byte2 * color_fit)
{
    io_copy((n_byte *)color_fit, (n_byte *)color_group, sizeof(n_byte2) * 3 * 256);
}

void draw_color_time(n_byte2 * color_fit)
{
    n_int   day_rotation =((land_time()*255)/TIME_DAY_MINUTES);
    n_int   darken =  math_sine(day_rotation + 64 + 128, NEW_SD_MULTIPLE/400);
    n_int	loop = 0;
    n_int	sign = 1;

    if (!toggle_tidedaylight)
    {
        if (darken < 1)
            sign = -1;

        darken = (darken * darken) / 402;
        darken = (sign * darken) + 624;

        while(loop < (COLOUR_GREY * 3))
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

static void draw_terrain_scan(void * void_dtss, void * xlocation, void * unused)
{
    draw_terrain_scan_struct * dtss = (draw_terrain_scan_struct *)void_dtss;
    n_byte  * buf_offscr = (n_byte *) dtss->offscreen;
    n_int     scrx = ((n_int *)xlocation)[0];
    /* take the very bottom pixel */
    n_int     dim_x =  dtss->dimensions.x;
    n_int     dim_y1 = dtss->dimensions.y - 1;
    n_int     pixy = (scrx + (dim_x >> 1)) + (dim_y1 * dim_x );
    n_int     actual = dim_y1;
    /* start with a map point which is below/off the screen */
    n_int     scry = dtss->const_lowdiv2;
    /* rotated and add offset (which will be &ed off) */
    n_int     big_x = dtss->lowest_s + (scrx * dtss->value_vector.y);
    /* rotated and sub offset (subtracted further down) */
    n_int     big_y = dtss->lowest_c - (scrx * dtss->value_vector.x);
    n_byte2 * combined = (n_byte2 *)land_topology_highdef();
    n_int     co_x = dtss->co.x;
    n_int     co_y = dtss->co.y;
    n_int     valc2 = dtss->value_vector.y << 1;
    n_int     vals2 = dtss->value_vector.x << 1;
    while(actual > -1)
    {
        const n_uint   check_change = CONVERT_X((big_x >> 8), co_x) | CONVERT_Y((big_y >> 8), co_y);
        const n_byte2  value        = combined[check_change];
        const n_int    z00          = value & 255;
        const n_byte   col00        = value >> 8;

        n_int          aval         = (scry - z00);
        if (aval < -1) aval = -1;

        scry--;           /* next map point from screen value */
        big_x -= vals2;
        big_y -= valc2;

        while (actual > aval)
        {
            buf_offscr[pixy] = col00;
            pixy -= dim_x;
            actual--;
        }
    }
    io_free(&xlocation);
    io_free(&void_dtss);
}

static void draw_terrain(noble_simulation * local_sim, n_vect2 * dimensions, n_byte threadable)
{
    n_byte   * buf_offscr = draw_pointer(NUM_TERRAIN);

    if (buf_offscr == 0L)
    {
        return;
    }

    if (local_sim->select == 0L)
    {
        io_erase(buf_offscr, dimensions->x * dimensions->y);
        return;
    }
    {
        const n_int    lowest_y = ((dimensions->y + 256) * dimensions->y)/256;
        noble_being * loc_being = local_sim->select;
        draw_terrain_scan_struct dtss;

        n_byte2 *  local_combined = (n_byte2 *)land_topology_highdef();

        /* start at the left-most row */
        n_int scrx = (0 - (dimensions->x >> 1));
        /* find the central map point */
        n_int flatval;
        n_vect2 value_vector;

        vect2_direction(&value_vector, terrain_turn + 128, 105);

        vect2_copy(&(dtss.value_vector), &value_vector);

        dtss.lowest_s = ((value_vector.x * (((lowest_y)) - dimensions->y)));
        dtss.lowest_c = ((value_vector.y * (((lowest_y)) - dimensions->y)));

        being_high_res(loc_being, &dtss.co);

        flatval = local_combined[CONVERT_X((HI_RES_MAP_DIMENSION/2), dtss.co.x) | CONVERT_Y((HI_RES_MAP_DIMENSION/2), dtss.co.y)] & 255;

        if (flatval < WATER_MAP)   /* if the central map point is underwater,*/
        {
            flatval = WATER_MAP;   /*    put it on water level */
        }

        dtss.const_lowdiv2 = (((lowest_y)) >> 1) + flatval;

        vect2_copy(&(dtss.dimensions), dimensions);

        dtss.offscreen = buf_offscr;

        /* repeat until the right-most row is reached */
        while (scrx < (dimensions->x - (dimensions->x >> 1)))
        {
            n_int * screen_x_location = io_new(sizeof(n_int));

            draw_terrain_scan_struct * local_dtss = (draw_terrain_scan_struct *)io_new(sizeof(draw_terrain_scan_struct));

            io_copy((n_byte *)&dtss, (n_byte *)local_dtss, sizeof(draw_terrain_scan_struct));

            screen_x_location[0] = scrx;

            if (threadable)
            {
                execute_add(((execute_function*)draw_terrain_scan), (void*)local_dtss, (void*)screen_x_location, 0L);
            }
            else
            {
                draw_terrain_scan(local_dtss, screen_x_location, 0L);
            }

            scrx++;               /* next column */
        }
        if (threadable)
        {
            execute_complete_added();
        }
    }
}

#define mndivmin		15
#define mndivhr			180

#define mndivmonth		7
#define mndivyear		91

/*
 * kind = 0, erase
 * kind = 1, draw-cycle
 */
static void	draw_meters(noble_simulation * local_sim)
{
    noble_being  * loc_being =   local_sim->select;
    n_pixel 	 * local_draw = &pixel_overlay;
    n_pixel      * local_draw_black = &pixel_black;
    n_byte		 * local_info = draw_pointer(NUM_TERRAIN);
    const n_byte * local_icon;
    n_int		   ha1 = 6;
    n_int		   ha2 = 0;
    n_int		   hr = 0;
    n_join		   local_kind;
    n_join		   local_kind_black;
    n_genetics    *genetics = being_genetics(loc_being);

    if (local_info == 0L)
    {
        return;
    }

    local_kind.pixel_draw = local_draw;
    local_kind.information = local_info;
    local_kind_black.pixel_draw = local_draw_black;
    local_kind_black.information = local_info;

    while (hr < 41)
    {
        if ((hr != 40) && (hr != 0))
        {
            math_join(6, 5+hr, 38, 0, &local_kind_black);
        }

        if ((hr&1) == 0)
        {
            (*local_draw)(5 , 5 + hr, 0, 0, local_info);
            (*local_draw)(45, 5 + hr, 0, 0, local_info);
            (*local_draw)(5 + hr, 45, 0, 0, local_info);
            (*local_draw)(5 + hr, 5,  0, 0, local_info);
        }

        hr ++;
    }

    hr = 0;

    while (hr < 12)
    {
        n_vect2 hour_clock;
        vect2_direction(&hour_clock, ((hr << 8) / 12), 320);

        (void)math_join((25 + (hour_clock.x / 5)), (25 + (hour_clock.y / 5)), (hour_clock.x / 17), (hour_clock.y / 17), &local_kind);
        hr++;
    }

#define FACING_OFFSIDE  ((terrain_dim_x-(512-411)))
#define SP_EN_OFFSIDE   ((terrain_dim_x-(512-325)))

#define GENETICS_X      (terrain_dim_x-69)
#define GENETICS_Y      (55)

#define GENDER_X        (terrain_dim_x-110)
#define GENDER_Y        (10)

    if (local_sim->select)
    {
        hr = 0;
        while (hr < 41)
        {

            if ((hr != 40) && (hr != 0))
            {
                math_join(51 + 5 + FACING_OFFSIDE, 5+hr, 38, 0, &local_kind_black);
                math_join(51 + 55 + SP_EN_OFFSIDE, 5+hr, 6, 0, &local_kind_black);
                math_join(51 + 55 + 18 + SP_EN_OFFSIDE, 5+hr, 6, 0, &local_kind_black);
            }

            if ((hr&1) == 0)
            {
                (*local_draw)(50 + 5 + FACING_OFFSIDE, 5 + hr, 0, 0, local_info);
                (*local_draw)(50 + 45+ FACING_OFFSIDE, 5 + hr, 0, 0, local_info);
                (*local_draw)(50 + 5 + hr+ FACING_OFFSIDE, 45, 0, 0, local_info);
                (*local_draw)(50 + 5 + hr+ FACING_OFFSIDE, 5, 0, 0, local_info);

                (*local_draw)(58 + 55 + SP_EN_OFFSIDE, 5 + hr, 0, 0, local_info);
                (*local_draw)(50 + 55 + SP_EN_OFFSIDE, 5 + hr, 0, 0, local_info);

                (*local_draw)(58 + 55 + 18 + SP_EN_OFFSIDE, 5 + hr, 0, 0, local_info);
                (*local_draw)(50 + 55 + 18 + SP_EN_OFFSIDE, 5 + hr, 0, 0, local_info);
            }
            hr ++;
        }
        hr = 0;
        while (hr < 9)
        {
            (*local_draw)(50 + 55 + hr + SP_EN_OFFSIDE, 5, 0, 0, local_info);
            (*local_draw)(50 + 55 + hr + SP_EN_OFFSIDE, 45, 0, 0, local_info);
            (*local_draw)(50 + 55 + 18 + hr + SP_EN_OFFSIDE, 5, 0, 0, local_info);
            (*local_draw)(50 + 55 + 18 + hr + SP_EN_OFFSIDE, 45, 0, 0, local_info);
            hr += 2;
        }

        hr = 0;

        while (hr < 8)
        {
            n_vect2 facing_clock;
            vect2_direction(&facing_clock, (hr << 5), 320);
            (void)math_join((25 + 50 + (facing_clock.x / 5))+ FACING_OFFSIDE, (25 + (facing_clock.y / 5)),
                            (facing_clock.x / 17), (facing_clock.y / 17), &local_kind);
            hr++;
        }

        (void)math_join(50 + 55 + 18+ SP_EN_OFFSIDE, 25, -1, 0, &local_kind);
        (void)math_join(58 + 55 + 18+ SP_EN_OFFSIDE, 25, 1, 0, &local_kind);
        (void)math_join(50 + 55 + 18+ SP_EN_OFFSIDE, 15, -2, 0, &local_kind);
        (void)math_join(58 + 55 + 18+ SP_EN_OFFSIDE, 15, 2, 0, &local_kind);
        (void)math_join(50 + 55 + 18+ SP_EN_OFFSIDE, 35, -2, 0, &local_kind);
        (void)math_join(58 + 55 + 18+ SP_EN_OFFSIDE, 35, 2, 0, &local_kind);

        /* draw genetics */
        while (ha2 < CHROMOSOMES)
        {
            n_uint	ha3 = 0;
            n_genetics	genetic_block = genetics[ha2];
            ha1 = 0;
            while (ha3 < (sizeof(n_genetics)*4))
            {
                n_int four = ( genetic_block >> (ha3 * 2) ) & 3 ;
                if ( four != 0 )
                    (void)math_join((ha1)+GENETICS_X,  GENETICS_Y + (ha2 * 10), 0, 7, &local_kind);
                ha1++;
                if ( four == 3 )
                    (void)math_join((ha1)+GENETICS_X, GENETICS_Y + (ha2 * 10), 0, 7, &local_kind);
                ha1 += 3;
                ha3++;
            }
            ha2++;
        }

        /* draw sex */
        (void)math_join(5+GENDER_X, (10)+GENDER_Y, 5, (11), &local_kind);
        (void)math_join(10+GENDER_X, (21)+GENDER_Y, 5, (-11), &local_kind);
        (void)math_join(15+GENDER_X, (10)+GENDER_Y, -10, 0, &local_kind);
        if (FIND_SEX(GET_I(loc_being)) == SEX_FEMALE)
        {
            (void)math_join(10+GENDER_X, (20)+GENDER_Y, 0, (6), &local_kind);
            (void)math_join(8+GENDER_X, (23)+GENDER_Y, 4, 0, &local_kind);
        }
        else
        {
            (void)math_join(15+GENDER_X, (10)+GENDER_Y, 4, (-4), &local_kind);
            (void)math_join(19+GENDER_X, (6)+GENDER_Y, -2, 0, &local_kind);
            (void)math_join(19+GENDER_X, (6)+GENDER_Y, 0, (2), &local_kind);
        }

        /* draw direction facing */
        {
            n_vect2 direction_facing;

            vect2_direction(&direction_facing, 128 + 64+ 256 - terrain_turn + loc_being->delta.direction_facing, 63 * 32);

            (void)math_join_vect2(75+ FACING_OFFSIDE, 25, &direction_facing, &local_kind);
        }

        {
            n_int   local_speed = being_speed(loc_being);
            n_int   local_energy = being_energy(loc_being);
            n_int   local_x = being_location_x(loc_being);
            n_int   local_y = being_location_y(loc_being);

            if (local_speed != 0)
            {
                (void)math_join(106 +SP_EN_OFFSIDE, (45-local_speed), 6, 0, &local_kind);
            }
            if (local_energy > 127)
            {
                (void)math_join(106 + 18 + SP_EN_OFFSIDE, (45-(local_energy >> 7)), 6, 0, &local_kind);

            }
            local_icon = &icns[weather_seven_values(local_x, local_y) << 7];
        }

    }
    else
    {
        /* still give weather even with no Noble Apes */
        local_icon = &icns[weather_seven_values(0, 0) << 7];
    }

    {
        n_vect2 year_hand;
        n_vect2 month_hand;
        n_vect2 hour_hand;
        n_vect2 minute_hand;

        vect2_direction(&year_hand, ((land_date()) << 6) / mndivyear, 5440);
        vect2_direction(&month_hand, ((land_date()) << 6) / mndivmonth, 5440);
        vect2_direction(&hour_hand, ((land_time()) << 6) / mndivhr, 2688);
        vect2_direction(&minute_hand, ((land_time()) << 6) / mndivmin, 2016);

        vect2_rotate90(&year_hand);
        vect2_rotate90(&month_hand);
        vect2_rotate90(&hour_hand);
        vect2_rotate90(&minute_hand);

        (void)math_join_vect2(17, 25, &year_hand, &local_kind);
        (void)math_join_vect2(33, 25, &month_hand, &local_kind);

        (void)math_join_vect2(25, 25, &hour_hand, &local_kind);
        (void)math_join_vect2(25, 25, &minute_hand, &local_kind);
    }
    ha1 = 0;
    while (ha1 < 32)
    {
        n_uint icon_stripe = (local_icon[(ha1<<2)|3] << 0) | (local_icon[(ha1<<2)|2] << 8)
                             | (local_icon[(ha1<<2)|1] << 16) | (local_icon[(ha1<<2)|0] << 24);
        ha2 = 0;
        while ( ha2 < 32 )
        {
            if ((icon_stripe >> (31-ha2)) & 1)
            {
                (*local_draw)(5 + ha2, 55 + ha1, 0, 0, local_info);
            }
            ha2++;
        }
        ha1++;
    }

}

#define	ACTIVE_PIXEL(px,py)		sketch_psetc(POSITIVE_LAND_COORD(px),POSITIVE_LAND_COORD(py),COLOUR_RED)
#define	ERASER_PIXEL(px,py)		sketch_psetc(POSITIVE_LAND_COORD(px),POSITIVE_LAND_COORD(py), \
                                              (n_byte)local_val)

/*
 * kind = 0, draw normal ape
 * kind = 1, draw selected ape
 * kind = 2, erase normal ape
 * kind = 3, erase selected ape
 */
static void draw_apeloc(noble_simulation * sim, noble_being  *bei, n_join * draw)
{
    n_pixel     *local_draw = draw->pixel_draw;
    void	    *local_info = draw->information;
    n_int		  start = -1, stop = 2;
    n_int		time_coef = sim->real_time >> 4;
    n_int	    start_point = ((time_coef &3 )) + 3;
    n_vect2     location;
    n_vect2     delta;

    being_space(bei, &location);
    land_convert_to_map(&location);

    delta.y = start;
    while (delta.y < stop)
    {
        delta.x = start;
        while (delta.x < stop)
        {
            n_vect2 screen;

            vect2_add(&screen, &location, &delta);

            (*local_draw)(POSITIVE_LAND_COORD(screen.x), POSITIVE_LAND_COORD(screen.y), 0, 0, local_info);
            delta.x++;
        }
        delta.y++;
    }
    if (bei == sim->select)
    {
        n_int ty = -1;
        while (ty < 2)
        {
            (*local_draw)(POSITIVE_LAND_COORD(location.x + ty), POSITIVE_LAND_COORD(location.y - 2 ), 0, 0, local_info);
            (*local_draw)(POSITIVE_LAND_COORD(location.x + ty), POSITIVE_LAND_COORD(location.y + 2 ), 0, 0, local_info);
            (*local_draw)(POSITIVE_LAND_COORD(location.x - 2 ), POSITIVE_LAND_COORD(location.y + ty), 0, 0, local_info);
            (*local_draw)(POSITIVE_LAND_COORD(location.x + 2 ), POSITIVE_LAND_COORD(location.y + ty), 0, 0, local_info);
            ty++;
        }
        start_point++;
    }
    if (being_awake(sim, bei) && (being_state(bei) & BEING_STATE_SPEAKING))
    {
        n_int	local_facing = ((((being_facing(bei))>>2) + 4) & 63) >> 3;
        /* D  C
        G       F

        H       E
           B  A */
        n_color8	*local_col = local_info;
        local_col->color = COLOUR_GREY;
        if(local_facing == 0 || local_facing == 7)
            (*local_draw)(POSITIVE_LAND_COORD(location.x + start_point ), POSITIVE_LAND_COORD(location.y - 2 ),
                          0, 0, local_info); /* F */
        if(local_facing == 1 || local_facing == 0)
            (*local_draw)(POSITIVE_LAND_COORD(location.x + start_point ), POSITIVE_LAND_COORD(location.y + 2 ),
                          0, 0, local_info); /* E */
        if(local_facing == 2 || local_facing == 1)
            (*local_draw)(POSITIVE_LAND_COORD(location.x + 2 ), POSITIVE_LAND_COORD(location.y + start_point ),
                          0, 0, local_info); /* A */
        if(local_facing == 3 || local_facing == 2)
            (*local_draw)(POSITIVE_LAND_COORD(location.x - 2 ), POSITIVE_LAND_COORD(location.y + start_point ),
                          0, 0, local_info); /* B */
        if(local_facing == 4 || local_facing == 3)
            (*local_draw)(POSITIVE_LAND_COORD(location.x - start_point ), POSITIVE_LAND_COORD(location.y + 2 ),
                          0, 0, local_info); /* H */
        if(local_facing == 5 || local_facing == 4)
            (*local_draw)(POSITIVE_LAND_COORD(location.x - start_point ), POSITIVE_LAND_COORD(location.y - 2 ),
                          0, 0, local_info); /* G */
        if(local_facing == 6 || local_facing == 5)
            (*local_draw)(POSITIVE_LAND_COORD(location.x - 2 ), POSITIVE_LAND_COORD(location.y - start_point ),
                          0, 0, local_info); /* D */
        if(local_facing == 7 || local_facing == 6)
            (*local_draw)(POSITIVE_LAND_COORD(location.x + 2 ), POSITIVE_LAND_COORD(location.y - start_point ),
                          0, 0, local_info); /* C */
    }
}


/*
 * kind = 0, draw normal ape
 * kind = 1, draw selected ape
 * kind = 2, erase normal ape
 * kind = 3, erase selected ape
 */
static void draw_apeloc_hires(noble_simulation * sim, noble_being  *bei, n_join * draw)
{
    n_pixel     *local_draw = draw->pixel_draw;
    void	    *local_info = draw->information;
    n_int		 start = -1, stop = 2;
    n_int		 time_coef = sim->real_time >> 4;
    n_int	     start_point = ((time_coef &3 )) + 3;

    n_vect2      location;
    n_vect2      delta;

    being_high_res(bei, &location);

    delta.y = start;
    while (delta.y < stop)
    {
        delta.x = start;
        while (delta.x < stop)
        {
            n_vect2 screen_point;
            vect2_add(&screen_point, &location, &delta);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(screen_point.x), POSITIVE_LAND_COORD_HIRES(screen_point.y), 0, 0, local_info);
            delta.x++;
        }
        delta.y++;
    }
    if (bei == sim->select)
    {
        n_int ty = -1;
        while (ty < 2)
        {
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x + ty), POSITIVE_LAND_COORD_HIRES(location.y - 2 ), 0, 0, local_info);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x + ty), POSITIVE_LAND_COORD_HIRES(location.y + 2 ), 0, 0, local_info);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x - 2 ), POSITIVE_LAND_COORD_HIRES(location.y + ty), 0, 0, local_info);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x + 2 ), POSITIVE_LAND_COORD_HIRES(location.y + ty), 0, 0, local_info);
            ty++;
        }
        start_point++;
    }
    if (being_awake(sim, bei) && (being_state(bei) & BEING_STATE_SPEAKING))
    {
        n_int	local_facing = ((((being_facing(bei))>>2) + 4) & 63) >> 3;
        /* D  C
         G       F

         H       E
         B  A */
        n_color8	*local_col = local_info;
        local_col->color = COLOUR_GREY;
        if(local_facing == 0 || local_facing == 7)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x + start_point ),
                          POSITIVE_LAND_COORD_HIRES(location.y - 2 ),
                          0, 0, local_info); /* F */
        if(local_facing == 1 || local_facing == 0)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x + start_point ),
                          POSITIVE_LAND_COORD_HIRES(location.y + 2 ),
                          0, 0, local_info); /* E */
        if(local_facing == 2 || local_facing == 1)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x + 2 ),
                          POSITIVE_LAND_COORD_HIRES(location.y + start_point ),
                          0, 0, local_info); /* A */
        if(local_facing == 3 || local_facing == 2)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x - 2 ),
                          POSITIVE_LAND_COORD_HIRES(location.y + start_point ),
                          0, 0, local_info); /* B */
        if(local_facing == 4 || local_facing == 3)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x - start_point ),
                          POSITIVE_LAND_COORD_HIRES(location.y + 2 ),
                          0, 0, local_info); /* H */
        if(local_facing == 5 || local_facing == 4)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x - start_point ),
                          POSITIVE_LAND_COORD_HIRES(location.y - 2 ),
                          0, 0, local_info); /* G */
        if(local_facing == 6 || local_facing == 5)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x - 2 ),
                          POSITIVE_LAND_COORD_HIRES(location.y - start_point ),
                          0, 0, local_info); /* D */
        if(local_facing == 7 || local_facing == 6)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(location.x + 2 ),
                          POSITIVE_LAND_COORD_HIRES(location.y - start_point ),
                          0, 0, local_info); /* C */
    }
}

static void draw_region(noble_being * local)
{
    n_join	 local_draw;
    n_byte * draw = draw_pointer(NUM_VIEW);
    n_int    ly = 63;

    if (draw == 0L) return;

    local_draw.information = draw;
    local_draw.pixel_draw  = &pixel_map_checker;

    while (ly < MAP_DIMENSION)
    {
        math_line(0, ly, MAP_DIMENSION, ly, &local_draw);
        math_line(ly, 0, ly, MAP_DIMENSION, &local_draw);
        ly += 64;
    }

#ifdef TERRITORY_ON

    local_draw.pixel_draw  = &pixel_map;

    ly = 0;
    while (ly < TERRITORY_DIMENSION)
    {
        n_int lx = 0;
        while (lx < TERRITORY_DIMENSION)
        {
            n_string_block string_draw;
            n_int   value = local->events.territory[lx + (ly * TERRITORY_DIMENSION)].familiarity;
            if (value)
            {
                sprintf(string_draw,"%ld",value);
                draw_string(string_draw, (lx*64)+2 , (ly*64)+5, &local_draw);
            }
            lx++;
        }
        ly++;
    }
#endif
}

static void draw_weather(void)
{
    n_int map_dimensions2 = land_map_dimension()/2;
    n_color8	 local_col;
    n_pixel	   * local_draw = &pixel_color8;
    void	   * local_info = &local_col;
    n_int        py = 0;
    local_col.color = COLOUR_GREY;
    local_col.screen = draw_pointer(NUM_VIEW);
    if (local_col.screen == 0L)
    {
        return;
    }
    while(py < (map_dimensions2))
    {
        n_int	px = 0;
        n_int	scr_y = py << 1;
        while(px < (map_dimensions2))
        {
            n_int	scr_x = px << 1;
            n_int	tmp = weather_pressure(WEATHER_TO_MAPSPACE(px), WEATHER_TO_MAPSPACE(py)); /* from weather dimension to map dimension */
            if(tmp > WEATHER_CLOUD)
            {
                (*local_draw)(scr_x+1, scr_y+1, 0, 0, local_info);
            }
            if(tmp > WEATHER_RAIN)
            {
                (*local_draw)(scr_x, scr_y , 0, 0, local_info);
            }
            px++;
        }
        py++;
    }
}

static void draw_count_number(n_uint count, n_string value)
{
    n_uint	lp = 0, division = 1000000;
    while (lp < 6)
    {
        if ((count + 1) > division)
        {
            if (division != 0)
            {
                value[lp] = (n_byte)('0' + ((count / division) % 10));
            }
            else
            {
                value[lp] = (n_byte)('0');
            }
        }
        division /= 10;
        lp++;
    }
    value[6] = ('0' + ((count / 1) % 10));
}

static void draw_metrics(n_uint bcps, n_uint fps, n_join * local_mono)
{
    n_int offset_y = 100;

    n_string_block  bcps_string = {' ', ' ', ' ', ' ', ' ', ' ', 'X', ' ',
                                   'B', 'P', 'S', ' ', ' ', ' ', ' ', ' ', ' ', 0
                                  };
    n_string_block  fps_string = {' ', ' ', ' ', ' ', ' ', ' ', 'X', ' ',
                                  'F', 'P', 'S', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 0
                                 };

    if (bcps)
    {
        draw_count_number(bcps, bcps_string);
        draw_string(bcps_string, terrain_dim_x - 98, offset_y, local_mono);
        offset_y += 18;
    }
    if (fps)
    {
        draw_count_number(fps, fps_string);
        draw_string(fps_string, terrain_dim_x - 98, offset_y, local_mono);
    }
}

/* draws the rotating brain, this is always draw and never erase */

#ifdef BRAIN_ON

static void draw_brain(noble_simulation *local_sim, n_vect2 * dimensions)
{
    n_byte  draw_big = 1;
    if ((local_sim->select == 0L) || (number_errors != 0))
    {
        return;
    }

    {
        n_byte      * local       = being_brain(local_sim->select);
        n_join	      local_mono;
        n_uint        turn_y = tilt_z;
        n_uint        turn_z = tilt_y;
        n_pixel	    * local_draw_brain = &pixel_overlay;
        void	    * local_info_brain = draw_pointer(NUM_TERRAIN);
        n_int	      lpx  = 0;
        n_int	      loop = 0;
        n_int         center_x = dimensions->x >> 1;
        n_int         center_y = dimensions->y >> 1;
        n_int	      act_x2a, term_1a, term_2a;

        n_byte	    * brainptr = local;
        n_byte	    * obrainptr = &local[ 32 * 32 * 32 ];
        n_int         a11, a12, a13, a21, a23, a31, a32, a33;

        n_vect2       vect_y, vect_z;

        vect2_direction(&vect_y, turn_y, 105);

        vect2_direction(&vect_z, turn_z, NEW_SD_MULTIPLE/1152);

        a32 =  vect_y.x;
        a12 =  vect_y.y;
        a21 =  vect_z.x;
        a23 =  vect_z.y;

        a11 = -((a32 * a23) >> 8);
        a13 =  ((a32 * a21) >> 8);
        a31 =  ((a12 * a23) >> 8);
        a33 = -((a12 * a21) >> 8);

        local_mono.pixel_draw  = &pixel_overlay;
        local_mono.information = draw_pointer(NUM_TERRAIN);

        draw_metrics(local_sim->delta_cycles, local_sim->delta_frames, &local_mono);

        if (local == 0L)
        {
            return;
        }

        a12 = (a12 * 1152) >> 8;
        a32 = (a32 * 1152) >> 8;
        act_x2a = -((a21 + a23) << 4);
        term_1a = -((a11 + a12 + a13) << 4);
        term_2a = -((a31 + a32 + a33) << 4);
        while (lpx < 32)
        {
            n_int	lpy = 0;
            n_int	term_1 = term_1a;
            n_int	term_2 = term_2a;
            while (lpy < 32)
            {
                n_int	act_x1 = term_1 ;
                n_int	act_x2 = act_x2a;
                n_int	act_y1 = term_2 ;
                n_int	lpz = 0;
                while (lpz < 32)
                {
                    if ((brainptr[loop] ^ obrainptr[loop]) >> 5)
                    {
                        /* TODO Fix this re-scaling bug */
                        /*
                        n_int	scr_z = 256 + (act_x1 >> 11);
                        n_int	s_x = ((act_x2 * scr_z) >> 16) + center_x;
                        n_int	s_y = ((act_y1 * scr_z) >> 16) + center_y - 120;
                        (*local_draw_brain)(s_x, s_y, 0, 0, local_info_brain);
                        if ((act_x1 > 0) && draw_big)
                        {
                            (*local_draw_brain)(s_x - 1, s_y, 0, 0, local_info_brain);
                            (*local_draw_brain)(s_x + 1, s_y, 0, 0, local_info_brain);
                            (*local_draw_brain)(s_x, s_y - 1, 0, 0, local_info_brain);
                            (*local_draw_brain)(s_x, s_y + 1, 0, 0, local_info_brain);
                        }*/
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

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
    n_int	           loop = 0;
    n_byte	           error_char_copy;
    n_string_block     simulation_date_time = {0};
    n_string_block     simulation_date_time_error = {0};
    n_int              position = 0;

    if (error_text)
    {
#ifdef NOBLE_APE_ASSERT
        printf("ERROR: %s\n", error_text);
#endif

#ifdef EXPLICIT_DEBUG
        printf("ERROR: %s (%s, %ld)\n", error_text, location, line_number);

#endif
        io_time_to_string(simulation_date_time);

        io_string_write(simulation_date_time_error, simulation_date_time, &position);
        io_string_write(simulation_date_time_error, " ", &position);


        if(number_errors == MAX_NUMBER_ERRORS)
        {
            io_string_write(simulation_date_time_error, " ** Maximum errors reached **" , &position);
        }
        else
        {
            io_string_write(simulation_date_time_error, (n_string)error_text, &position);
        }
    }
    else
    {
        number_errors = 0;
        return 0;
    }

    if (io_command_line_execution())
    {
        io_console_out(simulation_date_time_error);
        return -1;
    }
    do
    {
        error_char_copy = error_array[number_errors][loop] = simulation_date_time_error[loop];
        loop++;
    }
    while((loop < STRING_BLOCK_SIZE) && (error_char_copy != 0));

    error_array[number_errors][loop] = 0;

    if(number_errors != MAX_NUMBER_ERRORS)
    {
        number_errors++;
    }
    return -1;
}

static void draw_remains(noble_simulation * sim, n_byte * screen)
{
    noble_remains * remains = sim->remains;
    n_int           loop = 0;
    while(loop < remains->count)
    {
        dead_body * body = &(remains->bodies[loop]);
        n_int       lx = 510;
        n_vect2     location;

        location.x = body->location[0];
        location.y = body->location[1];

        land_convert_to_map(&location);

        while (lx < 515)
        {
            screen[((location.x + lx)&(MAP_DIMENSION-1)) + (((location.y)&(MAP_DIMENSION-1)) * MAP_DIMENSION)] = COLOUR_YELLOW;
            screen[((location.x)&(MAP_DIMENSION-1)) + (((location.y + lx)&(MAP_DIMENSION-1)) * MAP_DIMENSION)] = COLOUR_YELLOW;
            lx++;
        }
        loop++;
    }
}

static void draw_tides(n_byte * map, n_byte * screen, n_byte tide)
{
    n_byte	tide_compress[45];
    n_int   lp = 0;
    n_byte	tide_point = tide - 106;
    n_int	ar = 106;
    n_int	dr = 128;
    n_int	fl = tide_point;
    n_int	fp = 0;

    if (toggle_tidedaylight)
    {
        return;
    }

    while (lp < 45)
    {
        tide_compress[lp] = (n_byte)(((ar * (fl - lp)) + (dr * (lp - fp))) / (fl-fp));
        if (lp == tide_point)
        {
            ar = 128;
            dr = 150;
            fl = 150 - 106;
            fp = tide_point;
        }
        lp++;
    }

    lp = 0;
    while (lp < MAP_AREA)
    {
        n_byte val = map[lp];
        if ((val > 105) && (val < 151))
        {
            screen[lp] = tide_compress[val - 106];
        }
        lp++;
    }
}

static void draw_tides_hi_res(n_byte * data, n_byte4 * block, n_byte tide)
{
    n_byte	tide_compress[45];
    n_uint  lp = 0;
    n_byte	tide_point = tide - 106;
    n_int	ar = 106;
    n_int	dr = 128;
    n_int	fl = tide_point;
    n_int	fp = 0;

    if (toggle_tidedaylight)
    {
        return;
    }

    while (lp < 45)
    {
        tide_compress[lp] = (n_byte)(((ar * (fl - lp)) + (dr * (lp - fp))) / (fl-fp));
        if (lp == tide_point)
        {
            ar = 128;
            dr = 150;
            fl = 150 - 106;
            fp = tide_point;
        }
        lp++;
    }
    lp = 0;
    while (lp < HI_RES_MAP_AREA)
    {
        n_byte4  block_group = block[lp >> 5];

        if (block_group != 0)
        {
            n_int local_loop = lp<<1;
            n_int local_loop_end = local_loop+64;
            n_int lp = 0;
            while (local_loop < local_loop_end)
            {
                if ((block_group >> lp) & 1)
                {
                    data[local_loop|1] = tide_compress[data[local_loop] - 106];
                }
                local_loop += 2;
                lp++;
            }
        }
        lp += 32;
    }
}


static void draw_apes_loop(noble_simulation * local_sim, noble_being * bei, void * data)
{
    n_join			local_8bit;
    n_color8        local_col;

    /* makes this use thread safe */
    io_copy(data, (n_byte*)&local_col, sizeof(n_color8));

    if (being_los(local_sim->select, (n_byte2)being_location_x(bei), (n_byte2)being_location_y(bei)) == 1)
    {
        local_col.color = COLOUR_RED;
    }
    else
    {
        local_col.color = COLOUR_RED_DARK;
    }

    if (local_col.screen == land_topology_highdef())
    {
        local_8bit.pixel_draw = &pixel_color8_hires;
    }
    else
    {
        local_8bit.pixel_draw  = &pixel_color8;
    }

    local_8bit.information = &local_col;

    if (local_col.screen == land_topology_highdef())
    {
        draw_apeloc_hires(local_sim, bei, &local_8bit);
    }
    else
    {
        draw_apeloc(local_sim, bei, &local_8bit);
    }

}

static void draw_apes(noble_simulation * local_sim, n_byte lores)
{
    n_color8		local_col;

    if (lores == 0) /* set up drawing environ */
    {
        draw_undraw();
        local_col.screen = land_topology_highdef();
    }
    else
    {
        local_col.screen = draw_pointer(NUM_VIEW);
        io_copy(land_topology(), local_col.screen, MAP_AREA);
    }

    if (lores == 0)
    {
        static n_byte local_tide;
        if (local_tide != land_tide_level())
        {
            local_tide = land_tide_level();
            draw_tides_hi_res(land_topology_highdef(), land_highres_tide(), local_tide);
        }
    }
    else
    {
        draw_tides(land_topology(), local_col.screen, land_tide_level());
        if (toggle_territory)
        {
            draw_region(local_sim->select);
        }
        draw_remains(local_sim, local_col.screen);
    }

    if (local_sim->select)
    {
        being_loop_no_thread(local_sim, 0L, draw_apes_loop, &local_col);
    }

}

static void draw_errors(noble_simulation * local_sim)
{
    n_join			local_mono;
    local_mono.pixel_draw  = &pixel_overlay;
    local_mono.information = draw_pointer(NUM_TERRAIN);
    if(number_errors != 0)
    {
        n_int	loop = 0;
        while(loop< (number_errors+1))
        {
            draw_string(error_array[loop], 40, (loop*12) + 62, &local_mono);
            loop++;
        }
    }
}

#ifdef BRAINCODE_ON

static void draw_line_braincode(n_string pointer, n_int line)
{
    n_join	local_mono;
    local_mono.pixel_draw  = &pixel_grey;
    local_mono.information = draw_pointer(NUM_TERRAIN);
    draw_string(pointer, 4 + (terrain_dim_x/2) - 256, (line*12) + 246 + (terrain_dim_y/2) - 256, &local_mono);
}

#endif

void  draw_terrain_coord(n_int * co_x, n_int * co_y)
{
    *co_x = terrain_dim_x;
    *co_y = terrain_dim_y;
}


void  draw_window(n_int dim_x, n_int dim_y)
{
    terrain_dim_x = dim_x;
    terrain_dim_y = dim_y;
}

void  draw_cycle(void)
{
    noble_simulation * local_sim = sim_sim();
    n_vect2            local_vect;
    n_join             local_mono;

    if (sim_new()) return;
    if (check_about) return;

    local_mono.pixel_draw  = &pixel_overlay;
    local_mono.information = draw_pointer(NUM_TERRAIN);

    local_vect.x = terrain_dim_x;
    local_vect.y = terrain_dim_y;

    draw_apes(local_sim, 0);    /* hi res */
    draw_apes(local_sim, 1);    /* lo res */

#ifdef EXECUTE_THREADED
    /* TODO: Make the threaded draw command line safe */
    if (io_command_line_execution())
    {
        draw_terrain(local_sim, &local_vect, 0);
    }
    else
    {
        draw_terrain(local_sim, &local_vect, 1);
    }
#else
    draw_terrain(local_sim, &local_vect, 0);
#endif
    draw_meters(local_sim);
    draw_errors(local_sim); /* 12 */

#ifdef BRAIN_ON
    if (toggle_brain)
    {
        draw_brain(local_sim, &local_vect);
    }
#else
    draw_metrics(0, local_sim->delta_frames, &local_mono);
#endif

#ifdef BRAINCODE_ON
    if (toggle_braincode)
    {
        console_populate_braincode(local_sim, draw_line_braincode);
    }
#endif


#ifdef WEATHER_ON
    if (toggle_weather)
    {
        draw_weather(); /* 10 */
    }
#endif

#ifdef MULTITOUCH_CONTROLS
    draw_tc_controls(&local_mono);
#endif
}
