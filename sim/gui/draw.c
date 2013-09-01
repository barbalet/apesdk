/****************************************************************

 draw.c

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

#include "gui.h"

#include <stdio.h>

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

/* the weather/time of day icons hard coded */


#ifndef GRAPHLESS_GUI

extern void  graph_draw(noble_simulation * local_sim, n_byte * graph, n_int dim_x, n_int dim_y);

#endif

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

n_int control_error(n_byte * error_text);


static n_uint	tilt_y = 0;


n_byte	check_about = 0;


n_uint	tilt_z = 118;

static n_int toggle_weather = 1;
static n_int toggle_brain = 1;
static n_int toggle_braincode = 0;
static n_int toggle_territory = 0;

static n_int draw_drag_x1, draw_drag_x2, draw_drag_y1, draw_drag_y2;
static n_byte draw_drag_on = 0;

static n_int drag_time = 0;

void draw_update_drag(n_byte drag_on, n_int x1, n_int x2, n_int y1, n_int y2)
{
    
    draw_drag_on = drag_on;

    if (drag_on == 0)
    {
        return;
    }
    
    if (x2 >= x1)
    {
        draw_drag_x1 = x1;
        draw_drag_x2 = x2;
    }
    else
    {
        draw_drag_x1 = x2;
        draw_drag_x2 = x1;
    }

    if (y2 >= y1)
    {
        draw_drag_y1 = y1;
        draw_drag_y2 = y2;
    }
    else
    {
        draw_drag_y1 = y2;
        draw_drag_y2 = y1;
    }
    
    if (draw_drag_x1 < 0)
    {
        draw_drag_x1 = 0;
    }
    if (draw_drag_y1 < 0)
    {
        draw_drag_y1 = 0;
    }

    if (draw_drag_x2 > 511)
    {
        draw_drag_x2 = 511;
    }
    if (draw_drag_y2 > 511)
    {
        draw_drag_y2 = 511;
    }
}

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

/*NOBLEMAKE END=""*/

/*
 sketch functions are used for low-level pixel drawing
 */

#ifdef REVERSE_BIT

#define	BIT_SPAN(spx)		(1<<((spx)&7))

#else

#define	BIT_SPAN(spx)		(128>>((spx)&7))

#endif


/*NOBLEMAKE END=""*/

/* this needs to be grouped eventually, it is here as a test */

#define UNDRAW_MAX          4000

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

static n_byte pixel_color8_hires(n_int px, n_int py, void * information)
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

static n_byte pixel_color8(n_int px, n_int py, void * information)
{
    n_color8	*local_col = information;
    local_col->screen[ px | (py<<MAP_BITS) ] = local_col->color;
    return 0;
}

static n_int terrain_dim_x = 512;
static n_int terrain_dim_y = 511;

static n_byte pixel_map(n_int px, n_int py, void * information)
{
    n_byte *byte_info = information;
    byte_info[ px | (py<<MAP_BITS) ] = COLOUR_YELLOW;
    return 0;
}

static n_byte pixel_overlay(n_int px, n_int py, void * information)
{
    n_byte *byte_info = information;
    byte_info[ px + (py * terrain_dim_x) ] = COLOUR_YELLOW;
    return 0;
}

static n_byte pixel_grey(n_int px, n_int py, void * information)
{
    n_byte *byte_info = information;
    byte_info[ px + (py * terrain_dim_x) ] = COLOUR_GREY;
    return 0;
}

n_byte * draw_pointer(n_byte which_one)
{
    n_byte *local_buffer = local_offscreen;

    if (local_buffer == 0L)
    {
        return local_buffer;
    }

    switch(which_one)
    {
    case NUM_TERRAIN:
        return TERRAINWINDOW(local_buffer);
        break;
    case NUM_VIEW:
        return VIEWWINDOW(local_buffer);
        break;
    case NUM_GRAPH:
        return GRAPHWINDOW(local_buffer);
        break;
    }
    return 0L;
}



static void draw_drag(void)
{
    n_int loop = draw_drag_x1;
    n_color8    dotted_line;
    
    dotted_line.color = COLOUR_RED;
    dotted_line.screen = VIEWWINDOW(local_offscreen);
    
    drag_time ++;
    
    if (drag_time == 16)
    {
        drag_time = 0;
    }
    
    while (loop <= draw_drag_x2)
    {
        if (((loop + draw_drag_y1 + drag_time) >> 2) & 1)
        {
            pixel_color8(loop, draw_drag_y1, &dotted_line);
        }
        if (((loop + draw_drag_y2 - drag_time) >> 2) & 1)
        {
            pixel_color8(loop, draw_drag_y2, &dotted_line);
        }
        loop++;
    }
    
    loop = draw_drag_y1;
    
    while (loop <= draw_drag_y2)
    {
        if (((loop + draw_drag_x1 - drag_time) >> 2) & 1)
        {
            pixel_color8(draw_drag_x1, loop, &dotted_line);
        }
        
        if (((loop + draw_drag_x2 + drag_time) >> 2) & 1)
        {
            pixel_color8(draw_drag_x2, loop, &dotted_line);
        }
        loop++;
    }

}

/*	shows the about information */

#define	TAB_LENGTH	(36)

void draw_about(n_constant_string platform)
{
    n_join	local_draw;
    n_byte *buffer = draw_pointer(NUM_VIEW);
    n_int   loop = 0;
    n_int   line_y_offset = 128 + 24;

    if(check_about == 1 || buffer == 0L)
    {
        check_about = 0;
        return;
    }

    local_draw.information = buffer;
    local_draw.pixel_draw  = &pixel_map;

    while (loop < 214)
    {
        n_int  py = (GRAPH_WINDOW_HEIGHT/2) - 128 + loop;
        const n_int px = (GRAPH_WINDOW_WIDTH/2) - 200;
        n_byte * from_point = &buffer[(py*GRAPH_WINDOW_WIDTH) + px];
        io_erase(from_point, 400);
        loop++;
    }

    draw_string(SHORT_VERSION_NAME,     84, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;

    draw_string(FULL_VERSION_COPYRIGHT, 84 + TAB_LENGTH, line_y_offset, &local_draw);
    line_y_offset += 12;

    draw_string(COPYRIGHT_FOLLOW,       84 + TAB_LENGTH, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;


    draw_string(FULL_DATE,              84, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;

    draw_string(platform, 84 + TAB_LENGTH, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;

    draw_string("This software and Noble Ape are a continuing ", 84, line_y_offset, &local_draw);
    line_y_offset += 12;

    draw_string("work of Tom Barbalet begun on 13 June 1996.", 84, line_y_offset, &local_draw);
    line_y_offset += 12;
    line_y_offset += 12;


    draw_string("No apes or cats were harmed in the writing ", 84, line_y_offset, &local_draw);
    line_y_offset += 12;

    draw_string("of this software.", 84, line_y_offset, &local_draw);

    check_about = 1;
}


/* draws a string starting at point (off_x,off_y) */

#define	ledfir(x,y,c)	if(((val>> c )&1)) (*local_draw)((x + off_x + offset),((y + off_y)),local_info)

/**
 This is used to produce letter LED style letters through the generic
 draw function specified.
 @param str The string to be drawn.
 @param off_x The starting x location for the string to be drawn.
 @param off_y The starting y location for the string to be drawn.
 @param draw The generic draw function used to draw the character.
 */
void draw_string(n_constant_string str, n_int off_x, n_int off_y, n_join * draw)
{
    n_pixel	* local_draw = draw->pixel_draw;
    void	* local_info = draw->information;

    n_int	char_loop = 0;
    while (str[char_loop] > 31)
    {
        n_int	val = seg14[conv[str[char_loop] - 32]];
        n_int	offset = char_loop << 3;
        /* draw the character as a 14-segment LCD/LED output */
        ledfir(3, 8, 15);
        ledfir(3, 2, 14);
        ledfir(1, 0, 13);
        ledfir(2, 0, 13);
        ledfir(3, 0, 13);
        ledfir(4, 0, 13);
        ledfir(5, 0, 13);
        ledfir(6, 3, 12);
        ledfir(6, 2, 12);
        ledfir(6, 1, 12);
        ledfir(6, 5, 11);
        ledfir(6, 6, 11);
        ledfir(6, 7, 11);
        ledfir(5, 8, 10);
        ledfir(4, 8, 10);
        ledfir(3, 8, 10);
        ledfir(2, 8, 10);
        ledfir(1, 8, 10);
        ledfir(0, 7, 9);
        ledfir(0, 6, 9);
        ledfir(0, 5, 9);
        ledfir(0, 1, 8);
        ledfir(0, 2, 8);
        ledfir(0, 3, 8);
        ledfir(4, 4, 7);
        ledfir(5, 4, 7);
        ledfir(2, 4, 6);
        ledfir(1, 4, 6);
        ledfir(3, 7, 5);
        ledfir(3, 6, 5);
        ledfir(3, 5, 5);
        ledfir(5, 7, 4);
        ledfir(4, 6, 4);
        ledfir(1, 7, 3);
        ledfir(2, 6, 3);
        ledfir(5, 1, 2);
        ledfir(4, 2, 2);
        ledfir(1, 1, 1);
        ledfir(2, 2, 1);
        ledfir(3, 1, 0);
        ledfir(3, 2, 0);
        ledfir(3, 3, 0);
        char_loop ++;
    }
}

static n_byte2 draw_genetic_patch(n_byte2 * value)
{
    static	n_uint	location;
    n_byte2	return_value = 0;
    if (value == 0L)
    {
        location = 0;
    }
    else
    {
        /* this number of genetics contributes to the fur */
        return_value = value[location%6];
        location++;
    }
    return return_value;
}


/* this is the ocelot landscape algorithm */

#define POS_HIRES(num) ((num+(4096*2))&4095)

#define CONVERT_X(x, cx)  (n_uint)((POS_HIRES((x)+cx)) )
#define CONVERT_Y(y, cy)  (n_uint)((POS_HIRES((y)+cy)) << 12)

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

n_byte	fur_points[] =
{
    0,	 0,	 0,   0,
    60, 1,    1,  1,
    96, 92,  51,  23,
    160, 128,  128,  128,
    190, 250, 250, 250,
    255, 255, 255, 255,
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
        color_fit[(COLOUR_YELLOW*3) + 2] = 0x0000;

        color_fit[(COLOUR_RED_DARK*3)    ] = (0xeeff * 3) >> 2;
        color_fit[(COLOUR_RED_DARK*3) + 1] = 0x0000;
        color_fit[(COLOUR_RED_DARK*3) + 2] = 0x0000;

        color_fit[(COLOUR_RED*3)    ] = 0xeeff;
        color_fit[(COLOUR_RED*3) + 1] = 0x0000;
        color_fit[(COLOUR_RED*3) + 2] = 0x0000;

        io_copy((n_byte *)color_fit, (n_byte *)color_group, sizeof(n_byte2) * 3 * 256);
    }
}

void draw_color_time(n_byte2 * color_fit, n_byte2 time)
{
    n_int	darken = ((400 * new_sd[(((time*255)/(60*24))+64+128)&255])/NEW_SD_MULTIPLE);
    n_int	loop = 0;
    n_int	sign = 1;

    if (darken < 1)
        sign = -1;

    darken = (darken * darken) / 400;
    darken = (sign * darken) + 624;

    while(loop < (COLOUR_GREY * 3))
    {
        n_int cg_val = color_group[loop];
        n_int response = (cg_val * darken) >> 10;

        color_fit[loop] = (n_byte2)response;
        loop++;
    }

    while(loop < (256 * 3))
    {
        color_fit[loop] = color_group[loop];
        loop++;
    }
}

static void draw_terrain(noble_simulation * local_sim, n_int dim_x, n_int dim_y)
{
    n_byte * buf_offscr = draw_pointer(NUM_TERRAIN);

    if (buf_offscr == 0L)
    {
        return;
    }

    if (local_sim->select == NO_BEINGS_FOUND)
    {
        io_erase(buf_offscr, dim_x * dim_y);
        return;
    }
    {
        const n_int    lowest_y = ((dim_y + 256) * dim_y)/256;
        n_byte2      * combined = (n_byte2 *)local_sim->highres;
        noble_being * loc_being = &(local_sim->beings[local_sim->select]);
        const n_int turn = being_facing(loc_being);
        const n_int co_x = APESPACE_TO_HR_MAPSPACE(being_location_x(loc_being));
        const n_int co_y = APESPACE_TO_HR_MAPSPACE(being_location_y(loc_being));

        /* get the local cos value for the turn angle */
        const n_int valc = (new_sd[((turn) + 128) & 255] / 105);
        /* get the local sin value for the turn angle */
        const n_int vals = (new_sd[((turn) + 128 + 64) & 255] / 105);

        const n_int valc2 = (valc << 1);
        const n_int vals2 = (vals << 1);
        n_byte * loc_offscr = buf_offscr;

        /* start at the left-most row */
        n_int scrx = (0 - (dim_x >> 1));

        const n_int   lowest_s = ((vals * (((lowest_y)) - dim_y)));
        const n_int   lowest_c = ((valc * (((lowest_y)) - dim_y)));

        /* find the central map point */
        n_int flatval = combined[CONVERT_X(2048, co_x) | CONVERT_Y(2048, co_y)] & 255;

        n_int const_lowdiv2;

        if (flatval < WATER_MAP)   /* if the central map point is underwater,*/
        {
            flatval = WATER_MAP;   /*    put it on water level */
        }

        const_lowdiv2 = (((lowest_y)) >> 1) + flatval;

        while (scrx < (dim_x - (dim_x >> 1)))   /* repeat until the right-most row is reached */
        {
            /* take the very bottom pixel */
            n_int pixy = (scrx + (dim_x >> 1)) + ((dim_y - 1) * dim_x );

            n_int actual = (dim_y - 1);

            /* start with a map point which is below/off the screen */
            n_int scry = const_lowdiv2;
            /* rotated and add offset (which will be &ed off) */
            n_int big_x = lowest_s + (scrx * valc);
            /* rotated and sub offset (subtracted further down) */
            n_int big_y = lowest_c - (scrx * vals);

            while(actual > -1)
            {
                const n_uint check_change = CONVERT_X((big_x >> 8),co_x) | CONVERT_Y((big_y >> 8), co_y);
                const n_byte2  value = combined[check_change];
                const n_int    z00 = value & 255;
                const n_byte   col00   = value >> 8;
                n_int aval = (scry - z00);
                if (aval < -1) aval = -1;
                while (actual > aval)
                {
                    loc_offscr[pixy] = col00;
                    pixy -= dim_x;
                    actual--;
                }
                scry--;           /* next map point from screen value */
                big_x -= vals2;
                big_y -= valc2;
            }
            scrx++;               /* next column */
        }
    }
}

#define mndivmin		15
#define mndivhr			180

#define mndivmonth		7
#define mndivyear		91



#define FH_SCREEN(x)  (x)
#define FH_DELTA(x)   (x)

/*
 * kind = 0, erase
 * kind = 1, draw-cycle
 */
static void	draw_meters(noble_simulation * local_sim)
{
    n_land       * loc_land  =   local_sim->land;
    noble_being  * loc_being = &(local_sim->beings[local_sim->select]);
    n_pixel 	 * local_draw = &pixel_overlay;
    n_byte		 * local_info = draw_pointer(NUM_TERRAIN);
    const n_byte * local_icon;
    n_int		   ha1 = 6;
    n_int		   ha2 = 0;
    n_int		   hr = 0;
    n_int		   pntx = 0;
    n_int		   pnty;
    n_join		   local_kind;
    n_genetics    *genetics = being_genetics(loc_being);

    if (local_info == 0L)
    {
        return;
    }

    local_kind.pixel_draw = local_draw;
    local_kind.information = local_info;

    while (pntx < 41)
    {
        (*local_draw)(5 , 5 + pntx, local_info);
        (*local_draw)(45, 5 + pntx, local_info);
        (*local_draw)(5 + pntx, 45, local_info);
        (*local_draw)(5 + pntx, 5, local_info);
        pntx += 2;
    }

    hr =0;

    while (hr < 12)
    {
        n_int	phr = (((hr << 8) / 12) & 255);
        n_int	pnty = new_sd[(64 + phr) & 255] / 320;
        pntx = new_sd[phr & 255] / 320;
        (void)math_join((25 + (pntx / 5)), (25 + (pnty / 5)), (pntx / 17), (pnty / 17), &local_kind);
        hr++;
    }

#define FACING_OFFSIDE  ((terrain_dim_x-(512-411)))
#define SP_EN_OFFSIDE   ((terrain_dim_x-(512-325)))

#define GENETICS_X      ((terrain_dim_x-(512-(182))))
#define GENETICS_Y      (50)

#define GENDER_X        (terrain_dim_x-(512-312))

    if (local_sim->select != NO_BEINGS_FOUND)
    {
        pntx = 0;
        while (pntx < 41)
        {
            (*local_draw)(50 + 5 + FACING_OFFSIDE, 5 + pntx, local_info);
            (*local_draw)(50 + 45+ FACING_OFFSIDE, 5 + pntx, local_info);
            (*local_draw)(50 + 5 + pntx+ FACING_OFFSIDE, 45, local_info);
            (*local_draw)(50 + 5 + pntx+ FACING_OFFSIDE, 5, local_info);

            (*local_draw)(58 + 55 + SP_EN_OFFSIDE, 5 + pntx, local_info);
            (*local_draw)(50 + 55 + SP_EN_OFFSIDE, 5 + pntx, local_info);
            (*local_draw)(58 + 55 + 18 + SP_EN_OFFSIDE, 5 + pntx, local_info);
            (*local_draw)(50 + 55 + 18 + SP_EN_OFFSIDE, 5 + pntx, local_info);
            pntx += 2;
        }
        pntx = 0;
        while (pntx < 9)
        {
            (*local_draw)(50 + 55 + pntx+ SP_EN_OFFSIDE, 5, local_info);
            (*local_draw)(50 + 55 + pntx+ SP_EN_OFFSIDE, 45, local_info);
            (*local_draw)(50 + 55 + 18 + pntx+ SP_EN_OFFSIDE, 5, local_info);
            (*local_draw)(50 + 55 + 18 + pntx+ SP_EN_OFFSIDE, 45, local_info);
            pntx += 2;
        }

        hr = 0;

        while (hr < 8)
        {
            n_int	phr = (hr << 5) & 255;
            n_int	pnty = new_sd[(64 + phr) & 255] / 320;
            pntx = new_sd[phr & 255] / 320;
            (void)math_join((25 + 50 + (pntx / 5))+ FACING_OFFSIDE, (25 + (pnty / 5)),
                            (pntx / 17), (pnty / 17), &local_kind);
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
            n_int	ha3 = 0;
            n_byte2	genetic_block = genetics[ha2];
            ha1 = 6;
            while (ha3 < 8)
            {
                n_int four = ( genetic_block >> (ha3 * 2) ) & 3 ;
                if ( four != 0 )
                    (void)math_join(285+GENETICS_X + (ha2 * 10), FH_SCREEN(ha1)+GENETICS_Y, 7, 0, &local_kind);
                ha1++;
                if ( four == 3 )
                    (void)math_join(285+GENETICS_X + (ha2 * 10), FH_SCREEN(ha1)+GENETICS_Y, 7, 0, &local_kind);
                ha1 += 4;
                ha3++;
            }
            ha2++;
        }

        /* draw sex */
        (void)math_join(125+GENDER_X, FH_SCREEN(20)+GENETICS_Y, 5, FH_DELTA(11), &local_kind);
        (void)math_join(130+GENDER_X, FH_SCREEN(31)+GENETICS_Y, 5, FH_DELTA(-11), &local_kind);
        (void)math_join(135+GENDER_X, FH_SCREEN(20)+GENETICS_Y, -10, 0, &local_kind);
        if (FIND_SEX(GET_I(loc_being)) == SEX_FEMALE)
        {
            (void)math_join(130+GENDER_X, FH_SCREEN(30)+GENETICS_Y, 0, FH_DELTA(6), &local_kind);
            (void)math_join(128+GENDER_X, FH_SCREEN(33)+GENETICS_Y, 4, 0, &local_kind);
        }
        else
        {
            (void)math_join(135+GENDER_X, FH_SCREEN(20)+GENETICS_Y, 4, FH_DELTA(-4), &local_kind);
            (void)math_join(139+GENDER_X, FH_SCREEN(16)+GENETICS_Y, -2, 0, &local_kind);
            (void)math_join(139+GENDER_X, FH_SCREEN(16)+GENETICS_Y, 0, FH_DELTA(2), &local_kind);
        }


#ifdef FELINE

        (void)math_join(159+GENDER_X, FH_SCREEN(25)+GENETICS_Y, -9, FH_DELTA(0), &local_kind);
        (void)math_join(181+GENDER_X, FH_SCREEN(25)+GENETICS_Y, 9, FH_DELTA(0), &local_kind);

        (void)math_join(159+GENDER_X, FH_SCREEN(22)+GENETICS_Y, -7, FH_DELTA(-2), &local_kind);
        (void)math_join(181+GENDER_X, FH_SCREEN(28)+GENETICS_Y, 7, FH_DELTA(2), &local_kind);

        (void)math_join(159+GENDER_X, FH_SCREEN(28)+GENETICS_Y, -7, FH_DELTA(2), &local_kind);
        (void)math_join(181+GENDER_X, FH_SCREEN(22)+GENETICS_Y, 7, FH_DELTA(-2), &local_kind);

        (void)math_join(150+GENDER_X, FH_SCREEN(5)+GENETICS_Y, 2, FH_DELTA(8), &local_kind);
        (void)math_join(150+GENDER_X, FH_SCREEN(5)+GENETICS_Y, 8, FH_DELTA(2), &local_kind);

        (void)math_join(190+GENDER_X, FH_SCREEN(5)+GENETICS_Y, -2, FH_DELTA(8), &local_kind);
        (void)math_join(190+GENDER_X, FH_SCREEN(5)+GENETICS_Y, -8, FH_DELTA(2), &local_kind);

#endif

        /* draw direction facing */
        {
            n_vect2 direction_facing;
            being_facing_vector(loc_being, &direction_facing, 63);
            (void)math_join(75+ FACING_OFFSIDE, 25, direction_facing.x, FH_DELTA(direction_facing.y), &local_kind);
        }

        {
            n_int   local_speed = being_speed(loc_being);
            n_int   local_energy = being_energy(loc_being);
            n_int   local_x = being_location_x(loc_being);
            n_int   local_y = being_location_y(loc_being);

            if (local_speed != 0)
            {
                (void)math_join(106 +SP_EN_OFFSIDE, FH_SCREEN(45-local_speed), 6, 0, &local_kind);
            }
            if (local_energy > 127)
            {
                (void)math_join(106 + 18 + SP_EN_OFFSIDE, FH_SCREEN(45-(local_energy >> 7)), 6, 0, &local_kind);

            }
            local_icon = &icns[weather_seven_values(local_sim->land, local_sim->weather, local_x, local_y) << 7];
        }

    }
    else
    {
        /* still give weather even with no Noble Apes */
        local_icon = &icns[weather_seven_values(local_sim->land, local_sim->weather, 0, 0) << 7];
    }

    ha1 = ((((loc_land->date[0]) << 6) / mndivmonth) & 255);
    ha2 = ((((loc_land->date[0]) << 6) / mndivyear) & 255);
    /* draw month meter */
    pntx = (new_sd[(ha1) & 255] / 5440);
    pnty = ( - new_sd[(ha1 + 64) & 255] / 5440);
    (void)math_join(33, 25, pntx, FH_DELTA(pnty), &local_kind);

    /* draw year meter */
    pntx = (new_sd[(ha2) & 255] / 5440);
    pnty = ( - new_sd[(ha2 + 64) & 255] / 5440);
    (void)math_join(17, 25, pntx, FH_DELTA(pnty), &local_kind);

    /* draw clock (ie hours and minutes */
    ha1 = ((((loc_land->time) << 6) / mndivhr) & 255);
    ha2 = ((((loc_land->time) << 6) / mndivmin) & 255);
    pntx = (new_sd[(ha1) & 255] / 2688);
    pnty = ( - new_sd[(ha1 + 64) & 255] / 2688);
    (void)math_join(25, 25, pntx, FH_DELTA(pnty), &local_kind);

    pntx = (new_sd[(ha2) & 255] / 2016);
    pnty = ( - new_sd[(ha2 + 64) & 255] / 2016);
    (void)math_join(25, 25, pntx, FH_DELTA(pnty), &local_kind);

    pnty = 0;
    while (pnty < 32)
    {
        n_uint icon_stripe = (local_icon[(pnty<<2)|3] << 0) | (local_icon[(pnty<<2)|2] << 8)
                             | (local_icon[(pnty<<2)|1] << 16) | (local_icon[(pnty<<2)|0] << 24);
        pntx = 0;
        while ( pntx < 32 )
        {
            if ((icon_stripe >> (31-pntx)) & 1)
            {
                (*local_draw)(5 + pntx, 55 + pnty, local_info);
            }
            pntx++;
        }
        pnty++;
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
static void draw_apeloc(noble_simulation * sim, n_uint reference, n_join * draw)
{
    noble_being  *bei   = &(sim->beings[reference]);
    n_int		  magx = APESPACE_TO_MAPSPACE(being_location_x(bei));
    n_int		  magy = APESPACE_TO_MAPSPACE(being_location_y(bei));
    n_pixel   *local_draw = draw->pixel_draw;
    void	    *local_info = draw->information;
    n_int		  ty;
    n_int		  start = -1, stop = 2;
    n_int		  time_coef = sim->real_time >> 4;
    n_int	    start_point = ((time_coef &3 )) + 3;
    ty = start;
    while (ty < stop)
    {
        n_int tx = start;
        while (tx < stop)
        {
            n_int	scrx = (magx + tx);
            n_int	scry = (magy + ty);
            (*local_draw)(POSITIVE_LAND_COORD(scrx), POSITIVE_LAND_COORD(scry), local_info);
            tx++;
        }
        ty++;
    }
    if(reference == sim->select)
    {
        ty = -1;
        while (ty < 2)
        {
            (*local_draw)(POSITIVE_LAND_COORD(magx + ty), POSITIVE_LAND_COORD(magy - 2 ), local_info);
            (*local_draw)(POSITIVE_LAND_COORD(magx + ty), POSITIVE_LAND_COORD(magy + 2 ), local_info);
            (*local_draw)(POSITIVE_LAND_COORD(magx - 2 ), POSITIVE_LAND_COORD(magy + ty), local_info);
            (*local_draw)(POSITIVE_LAND_COORD(magx + 2 ), POSITIVE_LAND_COORD(magy + ty), local_info);
            ty++;
        }
        start_point++;
    }
    if(being_awake(sim, bei) && bei->speak != 0)
    {
        n_int	local_facing = ((((being_facing(bei))>>2) + 4) & 63) >> 3;
        /* D  C
        G       F

        H       E
           B  A */
        n_color8	*local_col = local_info;
        local_col->color = COLOUR_GREY;
        if(local_facing == 0 || local_facing == 7)
            (*local_draw)(POSITIVE_LAND_COORD(magx + start_point ), POSITIVE_LAND_COORD(magy - 2 ),
                          local_info); /* F */
        if(local_facing == 1 || local_facing == 0)
            (*local_draw)(POSITIVE_LAND_COORD(magx + start_point ), POSITIVE_LAND_COORD(magy + 2 ),
                          local_info); /* E */
        if(local_facing == 2 || local_facing == 1)
            (*local_draw)(POSITIVE_LAND_COORD(magx + 2 ), POSITIVE_LAND_COORD(magy + start_point ),
                          local_info); /* A */
        if(local_facing == 3 || local_facing == 2)
            (*local_draw)(POSITIVE_LAND_COORD(magx - 2 ), POSITIVE_LAND_COORD(magy + start_point ),
                          local_info); /* B */
        if(local_facing == 4 || local_facing == 3)
            (*local_draw)(POSITIVE_LAND_COORD(magx - start_point ), POSITIVE_LAND_COORD(magy + 2 ),
                          local_info); /* H */
        if(local_facing == 5 || local_facing == 4)
            (*local_draw)(POSITIVE_LAND_COORD(magx - start_point ), POSITIVE_LAND_COORD(magy - 2 ),
                          local_info); /* G */
        if(local_facing == 6 || local_facing == 5)
            (*local_draw)(POSITIVE_LAND_COORD(magx - 2 ), POSITIVE_LAND_COORD(magy - start_point ),
                          local_info); /* D */
        if(local_facing == 7 || local_facing == 6)
            (*local_draw)(POSITIVE_LAND_COORD(magx + 2 ), POSITIVE_LAND_COORD(magy - start_point ),
                          local_info); /* C */
    }
}


/*
 * kind = 0, draw normal ape
 * kind = 1, draw selected ape
 * kind = 2, erase normal ape
 * kind = 3, erase selected ape
 */
static void draw_apeloc_hires(noble_simulation * sim, n_uint reference, n_join * draw)
{
    noble_being  *bei   = &(sim->beings[reference]);
    n_int		  magx = APESPACE_TO_HR_MAPSPACE(being_location_x(bei));
    n_int		  magy = APESPACE_TO_HR_MAPSPACE(being_location_y(bei));
    n_pixel     *local_draw = draw->pixel_draw;
    void	    *local_info = draw->information;
    n_int		  ty;
    n_int		  start = -1, stop = 2;
    n_int		  time_coef = sim->real_time >> 4;
    n_int	      start_point = ((time_coef &3 )) + 3;

    ty = start;
    while (ty < stop)
    {
        n_int tx = start;
        while (tx < stop)
        {
            n_int	scrx = (magx + tx);
            n_int	scry = (magy + ty);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(scrx), POSITIVE_LAND_COORD_HIRES(scry), local_info);
            tx++;
        }
        ty++;
    }
    if(reference == sim->select)
    {
        ty = -1;
        while (ty < 2)
        {
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx + ty), POSITIVE_LAND_COORD_HIRES(magy - 2 ), local_info);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx + ty), POSITIVE_LAND_COORD_HIRES(magy + 2 ), local_info);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx - 2 ), POSITIVE_LAND_COORD_HIRES(magy + ty), local_info);
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx + 2 ), POSITIVE_LAND_COORD_HIRES(magy + ty), local_info);
            ty++;
        }
        start_point++;
    }
    if(being_awake(sim, bei) && bei->speak != 0)
    {
        n_int	local_facing = ((((being_facing(bei))>>2) + 4) & 63) >> 3;
        /* D  C
         G       F

         H       E
         B  A */
        n_color8	*local_col = local_info;
        local_col->color = COLOUR_GREY;
        if(local_facing == 0 || local_facing == 7)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx + start_point ), POSITIVE_LAND_COORD_HIRES(magy - 2 ),
                          local_info); /* F */
        if(local_facing == 1 || local_facing == 0)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx + start_point ), POSITIVE_LAND_COORD_HIRES(magy + 2 ),
                          local_info); /* E */
        if(local_facing == 2 || local_facing == 1)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx + 2 ), POSITIVE_LAND_COORD_HIRES(magy + start_point ),
                          local_info); /* A */
        if(local_facing == 3 || local_facing == 2)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx - 2 ), POSITIVE_LAND_COORD_HIRES(magy + start_point ),
                          local_info); /* B */
        if(local_facing == 4 || local_facing == 3)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx - start_point ), POSITIVE_LAND_COORD_HIRES(magy + 2 ),
                          local_info); /* H */
        if(local_facing == 5 || local_facing == 4)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx - start_point ), POSITIVE_LAND_COORD_HIRES(magy - 2 ),
                          local_info); /* G */
        if(local_facing == 6 || local_facing == 5)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx - 2 ), POSITIVE_LAND_COORD_HIRES(magy - start_point ),
                          local_info); /* D */
        if(local_facing == 7 || local_facing == 6)
            (*local_draw)(POSITIVE_LAND_COORD_HIRES(magx + 2 ), POSITIVE_LAND_COORD_HIRES(magy - start_point ),
                          local_info); /* C */
    }
}

static void draw_region(noble_being * local)
{
    n_join	local_draw;
    n_byte * draw = draw_pointer(NUM_VIEW);
    n_int    ly = 0;

    if (draw == 0L) return;

    local_draw.information = draw;
    local_draw.pixel_draw  = &pixel_map;

    while (ly < GRAPH_WINDOW_HEIGHT)
    {
        n_int   lx = 63;
        while (lx < GRAPH_WINDOW_WIDTH)
        {
            draw[ lx | ((ly) << GRAPH_WINDOW_WIDTH_BITS) ] = COLOUR_YELLOW;
            draw[ ly | ((lx) << GRAPH_WINDOW_WIDTH_BITS) ] = COLOUR_YELLOW;
            lx += 64;
        }
        ly += 2;
    }
    ly = 0;
    while (ly < TERRITORY_DIMENSION)
    {
        n_int lx = 0;
        while (lx < TERRITORY_DIMENSION)
        {
            n_string_block string_draw;
            n_int   value = local->territory[lx + (ly * TERRITORY_DIMENSION)].familiarity;
            if (value)
            {
                sprintf(string_draw,"%ld",value);
                draw_string(string_draw, (lx*64)+2 , (ly*64)+5, &local_draw);
            }
            lx++;
        }
        ly++;
    }
}

static void draw_weather(noble_simulation * local_sim)
{
    n_int map_dimensions2 = land_map_dimension(local_sim->land)/2;

    n_weather *wea = local_sim->weather;
    n_color8		local_col;
    n_pixel			*local_draw = &pixel_color8;
    void			  *local_info = &local_col;
    n_int       py = 0;
    
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
            n_int	tmp = weather_pressure(wea, px, py, map_dimensions2);
            if(tmp > WEATHER_CLOUD)
            {
                (*local_draw)(scr_x+1, scr_y+1, local_info);
            }
            if(tmp > WEATHER_RAIN)
            {
                (*local_draw)(scr_x, scr_y , local_info);
            }
            px++;
        }
        py++;
    }
}


static void draw_brain_cyles_per_second(n_uint count, n_join * local_mono)
{
    n_string_block  cycles_per_sec = {' ', ' ', ' ', ' ', ' ', 'X', '.', 'X', ' ',
                                      'B', 'C', 'P', 'S', ' ', ' ', ' ', ' ', ' ', ' ', 0
                                     };
    n_uint	lp = 0, division = 1000000;
    while (lp < 5)
    {
        if ((count + 1) > division)
        {
            if (division != 0)
            {
                cycles_per_sec[lp] = (n_byte)('0' + ((count / division) % 10));
            }
            else
            {
                cycles_per_sec[lp] = (n_byte)('0');
            }

        }
        division /= 10;
        lp++;
    }
    cycles_per_sec[5] = ('0' + ((count / 10) % 10));
    cycles_per_sec[7] = ('0' + ((count / 1) % 10));
    draw_string(cycles_per_sec, terrain_dim_x - 112, 100, local_mono);
}

/* draws the rotating brain, this is always draw and never erase */

static void draw_brain(noble_simulation *local_sim, n_int dim_x, n_int dim_y)
{
    n_byte  draw_big = 1;
    if ((local_sim->select == NO_BEINGS_FOUND) || (number_errors != 0))
    {
        return;
    }

    {
        noble_being * local_being = &(local_sim->beings[local_sim->select]);
        n_byte      * local       = being_brain(local_being);
        n_join	      local_mono;
        n_uint        turn_y = tilt_z;
        n_uint        turn_z = tilt_y;
        n_pixel	    * local_draw_brain = &pixel_overlay;
        void	    * local_info_brain = draw_pointer(NUM_TERRAIN);
        n_int	      lpx  = 0;
        n_int	      loop = 0;
        n_int         draw_brain_size = (draw_big ? (BRAIN_MAGI) : (BRAIN_MAGI/2));


        n_int	      a32 =  (new_sd[(turn_y + 64) & 255] / 105);
        n_int	      a12 =  (new_sd[(turn_y) & 255] / 105);
        n_int	      a21 = ((new_sd[(turn_z + 64) & 255] * draw_brain_size) / NEW_SD_MULTIPLE);
        n_int	      a23 = ((new_sd[(turn_z) & 255] * draw_brain_size) / NEW_SD_MULTIPLE);

        n_int	      a11 = -((a32 * a23) >> 8);
        n_int	      a13 =  ((a32 * a21) >> 8);
        n_int	      a31 =  ((a12 * a23) >> 8);
        n_int	      a33 = -((a12 * a21) >> 8);
        n_int	      act_x2a, term_1a, term_2a;

        n_byte	    * brainptr = local;
        n_byte	    * obrainptr = &local[ BRAIN_OFFSET(32 * 32 * 32) ];

        n_int         center_x = dim_x >> 1;
        n_int         center_y = dim_y >> 1;

        local_mono.pixel_draw  = &pixel_overlay;
        local_mono.information = draw_pointer(NUM_TERRAIN);

        draw_brain_cyles_per_second(local_sim->delta_cycles, &local_mono);

        if (local == 0L)
        {
            return;
        }

        a12 = (a12 * draw_brain_size) >> 8;
        a32 = (a32 * draw_brain_size) >> 8;
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
                    if ((brainptr[BRAIN_OFFSET(loop)] ^ obrainptr[BRAIN_OFFSET(loop)]) >> 5)
                    {
                        n_int	scr_z = 256 + (act_x1 >> 11);
                        n_int	s_x = ((act_x2 * scr_z) >> 16) + center_x;
                        n_int	s_y = ((act_y1 * scr_z) >> 16) + center_y - 120;
                        (*local_draw_brain)(s_x, s_y, local_info_brain);
                        if ((act_x1 > 0) && draw_big)
                        {
                            (*local_draw_brain)(s_x - 1, s_y, local_info_brain);
                            (*local_draw_brain)(s_x + 1, s_y, local_info_brain);
                            (*local_draw_brain)(s_x, s_y - 1, local_info_brain);
                            (*local_draw_brain)(s_x, s_y + 1, local_info_brain);
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
#ifdef BRAIN_HASH
        draw_string(brain_hash_out, 12, 495,  &local_mono);
#endif
    }
}

n_int draw_error(n_constant_string error_text)
{
    n_int	loop = 0;
    n_byte	error_char_copy;

    if(error_text == 0L)
    {
        number_errors = 0;
        return 0;
    }

    SC_DEBUG_STRING(" [ ERROR : ");
    SC_DEBUG_STRING( error_text );
    SC_DEBUG_STRING(" ]");
    SC_DEBUG_NEWLINE;
    SC_DEBUG_OFF;

    if (io_command_line_execution())
    {
        io_console_out(error_text);
        return -1;
    }

    if(number_errors == MAX_NUMBER_ERRORS)
    {
        error_text = " ** Maximum errors reached **";
    }
    do
    {
        error_char_copy = error_array[number_errors][loop] = error_text[loop];
        loop++;
    }
    while((loop< STRING_BLOCK_SIZE) && (error_char_copy != 0));

    error_array[number_errors][loop] = 0;

    if(number_errors != MAX_NUMBER_ERRORS)
    {
        number_errors++;
    }
    return -1;
}


static void draw_tides(n_byte * map, n_byte * screen, n_byte tide)
{
    n_byte	tide_compress[45];
    n_uint  lp = 0;
    n_byte	tide_point = tide - 106;
    n_int	ar = 106;
    n_int	dr = 128;
    n_int	fl = tide_point;
    n_int	fp = 0;
    while(lp<45)
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

static void draw_tides_hi_res(n_byte * data, n_c_uint * block, n_byte tide)
{
    n_byte	tide_compress[45];
    n_uint  lp = 0;
    n_byte	tide_point = tide - 106;
    n_int	ar = 106;
    n_int	dr = 128;
    n_int	fl = tide_point;
    n_int	fp = 0;

    while(lp<45)
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
        n_c_uint  block_group = block[lp >> 5];

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

static void draw_apes(noble_simulation * local_sim, n_byte lores)
{
    n_color8		local_col;

    if (lores == 0) /* set up drawing environ */
    {
        draw_undraw();

        local_col.screen = local_sim->highres;
    }
    else
    {
        local_col.screen = draw_pointer(NUM_VIEW);
        io_copy(local_sim->land->map, local_col.screen, MAP_AREA);
    }

    if (lores == 0)
    {
        static n_byte local_tide;
        if (local_tide != local_sim->land->tide_level)
        {
            local_tide = local_sim->land->tide_level;
            draw_tides_hi_res(local_sim->highres, local_sim->highres_tide, local_tide);
        }
    }
    else
    {
        draw_tides(local_sim->land->map, local_col.screen, local_sim->land->tide_level);
        if (toggle_territory)
        {
            draw_region(&(local_sim->beings[local_sim->select]));
        }
    }

    if (local_sim->select != NO_BEINGS_FOUND)
    {
        n_join			local_8bit;
        n_uint			loop = 0;

        if (lores == 0)
        {
            local_8bit.pixel_draw = &pixel_color8_hires;
        }
        else
        {
            local_8bit.pixel_draw  = &pixel_color8;
        }
        local_8bit.information = &local_col;

        while (loop < local_sim->num)
        {
            noble_being *bei = &(local_sim->beings[loop]);
            if (being_los(local_sim->land, &(local_sim->beings[local_sim->select]), being_location_x(bei), being_location_y(bei)) == 1)
            {
                local_col.color = COLOUR_RED;
            }
            else
            {
                local_col.color = COLOUR_RED_DARK;
            }

            if (lores == 0)
            {
                draw_apeloc_hires(local_sim, loop, &local_8bit);
            }
            else
            {
                draw_apeloc(local_sim, loop, &local_8bit);
            }
            loop++;
        }
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

static void draw_line_braincode(n_string pointer, n_int line)
{
    n_join	local_mono;
    local_mono.pixel_draw  = &pixel_grey;
    local_mono.information = draw_pointer(NUM_TERRAIN);
    draw_string(pointer, 4 + (terrain_dim_x/2) - 256, (line*12) + 246 + (terrain_dim_y/2) - 256, &local_mono);
}

void  draw_terrain_coord(n_int * co_x, n_int * co_y)
{
    *co_x = terrain_dim_x;
    *co_y = terrain_dim_y;
}

void  draw_cycle(n_int window, n_int dim_x, n_int dim_y)
{
    noble_simulation * local_sim = sim_sim();

    if (window == NUM_TERRAIN)
    {
        terrain_dim_x = dim_x;
        terrain_dim_y = dim_y;
    }

    if (check_about) return;

#ifdef THREADED
    sim_draw_thread_start();
#endif

    if (window != NUM_GRAPH)
    {
        draw_apes(local_sim, window);    /* 8 */
    }

    if (window == NUM_TERRAIN)
    {

        draw_terrain(local_sim,dim_x, dim_y);
        draw_meters(local_sim);
        draw_errors(local_sim); /* 12 */

        if (toggle_brain)
        {
            draw_brain(local_sim,dim_x, dim_y);
        }
        if (toggle_braincode)
        {
            console_populate_braincode(local_sim, draw_line_braincode);
        }
    }
    if (window == NUM_VIEW)
    {

#ifdef WEATHER_ON
        if (toggle_weather)
        {
            draw_weather(local_sim); /* 10 */
        }
#endif
        if (draw_drag_on == 1)
        {
            draw_drag();
        }
        
    }
    if (window == NUM_GRAPH)
    {
        graph_draw(local_sim, draw_pointer(NUM_GRAPH), dim_x, dim_y);
    }
#ifdef THREADED
    sim_draw_thread_end();
#endif
}
