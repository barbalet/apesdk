/****************************************************************
 
glrender.c
 
 =============================================================
 
 Copyright 1996-2025 Tom Barbalet. All rights reserved.
 
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

/*! \file   glrender.c
 *  \brief  Handles low level graphics
 */

#include "glrender.h"

#include <stdio.h>

typedef struct
{
    n_vect2 start;
    n_vect2 end;
    GLR_COLOR  color;
    n_byte  thickness;
} glr_line;

typedef struct
{
    n_vect2 points[4];
    GLR_COLOR  color;
} glr_quad;

typedef struct
{
    n_byte color;
    n_byte thickness;
} glr_line_data;

#define MULTIPLE_CHECK (1000)

typedef enum
{
    GRAPHICS_CASE_ACTIVE = 0,
    GRAPHICS_CASE_DISPLAY = 1,
    GRAPHICS_CASE_TEXT = 2
} graphics_case;

static graphics_case current_case;

static memory_list * display_quads = 0L;
static memory_list * display_lines = 0L;

static memory_list * active_lines = 0L;

static memory_list * text_lines = 0L;

static GLR_COLOR current_color = GLR_GREEN;
static n_byte    current_thickness = 1;

static n_vect2 current_center = {0};
static n_vect2 current_location = {0};
static n_int   current_turn = 0;
static n_int   current_scale = 1;

static n_vect2 graph_size = {800, 600};

#define GLR_RGB(r,g,b) {0, (n_byte)(255*r), (n_byte)(255*g) , (n_byte)(255*b) }

static n_rgba32 color_map[ 8 ] = {
    /* green */      GLR_RGB(0.2, 0.5, 0.2),
    /* lightgreen */ GLR_RGB(0.4, 0.8, 0.4),
    /* red */        GLR_RGB(1.0, 0.0, 0.0),
    /* orange */     GLR_RGB(0.5, 0.2, 0.0),
    /* lightgrey */  GLR_RGB(0.9, 0.9, 0.9),
    /* grey */       GLR_RGB(0.7, 0.7, 0.7),
    /* darkgrey */   GLR_RGB(0.6, 0.6, 0.6),
    /* black */      GLR_RGB(0.0, 0.0, 0.0)
};

void glrender_set_size(n_int size_x, n_int size_y)
{
    graph_size.x = size_x;
    graph_size.y = size_y;
}

static n_int draw_scene_not_done = 0;

static void glrender_character_line(n_int px, n_int py, n_int dx, n_int dy)
{
    n_vect2 start;
    n_vect2 end;
    start.x = px;
    start.y = py;
    end.x = px + dx;
    end.y = py + dy;
    glrender_line(&start, &end);
}

/* draws a string starting at point (off_x,off_y) */

#define  led_fire(x, y, dx, dy, c)  if(((val >> c)&1)) glrender_character_line((0- x) + off_x - offset,y + off_y,  0 - dx, dy)

/**
 This is used to produce letter LED style letters through the generic
 draw function specified.
 @param str The string to be drawn.
 @param off_x The starting x location for the string to be drawn.
 @param off_y The starting y location for the string to be drawn.
 */
void glrender_string(n_constant_string str, n_int off_x, n_int off_y)
{
    n_int    char_loop = 0;
        
    while (str[char_loop] > 31)
    {
        n_int    val = math_seg14(str[char_loop] - 32);
        n_int    offset = char_loop << 3;
        /* draw the character as a 14-segment LCD/LED output */
        
        led_fire(3, 8, 0, 0, 15);
        led_fire(3, 2, 0, 0, 14);
        led_fire(1, 0, 4, 0, 13);
        led_fire(6, 1, 0, 2, 12);
        led_fire(6, 5, 0, 2, 11);
        led_fire(1, 8, 4, 0, 10);
        led_fire(0, 5, 0, 2, 9);
        led_fire(0, 1, 0, 2, 8);
        led_fire(4, 4, 1, 0, 7);
        led_fire(1, 4, 1, 0, 6);
        led_fire(3, 5, 0, 2, 5);
        led_fire(4, 6, 0, 1, 4);
        led_fire(2, 6, -1, 1, 3);
        led_fire(4, 2, 1, -1, 2);
        led_fire(1, 1, 1, 1, 1);
        led_fire(3, 1, 0, 2, 0);
        char_loop ++;
    }
}

void glrender_color_map_replace(n_byte4 * replace)
{
    n_int loop = 0;
    while (loop < 8)
    {
        color_map[loop].thirtytwo = replace[loop];
        loop++;
    }
}

static n_byte4 glrender_color_switch(n_int value)
{
    n_int loop = 0;
    while (loop < 7)
    {
        if (color_map[loop].rgba.b == value)
        {
            return color_map[loop].thirtytwo;
        }
        loop++;
    }
    return color_map[loop].rgba.b;
}


void glrender_color_map(n_byte2 * value)
{
    n_int loop = 0;
    n_int loop3 = 0;
    while (loop < 256)
    {
        n_byte4 value32 = glrender_color_switch(loop);
        value[loop3++] = ((value32 >> 0) & 255) << 8;
        value[loop3++] = ((value32 >> 8) & 255) << 8;
        value[loop3++] = ((value32 >> 16) & 255) << 8;
        loop++;
    }
}

n_int glrender_scene_done(void)
{
    return (draw_scene_not_done < 2);
}

void glrender_color(const GLR_COLOR color)
{
    current_color = color;
}

void glrender_translate(n_vect2 * input, n_vect2 * output, n_vect2 * direction_vector)
{
    vect2_subtract(output, input, &current_location);
    vect2_add(output, output, &current_center);
    vect2_scalar_multiply(output, current_scale);
    vect2_scalar_bitshiftdown(output, 7);
    vect2_rotation_bitshift(output, direction_vector);
    vect2_subtract(output, output, &current_center);
}

void glrender_render_erase(n_byte * output)
{
    graph_erase(output, &graph_size, (n_rgba32*)&color_map[GLR_GREEN]);
}

void glrender_render_quads(n_byte * output, memory_list * quads)
{
    n_vect2 direction_vector;
    n_int   loop = 0;
    glr_quad * display_quad_list = (glr_quad *)quads->data;
    vect2_direction(&direction_vector, 255-current_turn, 1);
    
    while (loop < quads->count)
    {
        glr_quad *quad_from_array = &display_quad_list[loop];
        n_vect2  local_coordinate_quad[4];
        n_rgba32 *local_color = ( n_rgba32 *)&color_map[quad_from_array->color];

        glrender_translate(&quad_from_array->points[0], &local_coordinate_quad[0], &direction_vector);
        glrender_translate(&quad_from_array->points[1], &local_coordinate_quad[1], &direction_vector);
        glrender_translate(&quad_from_array->points[2], &local_coordinate_quad[2], &direction_vector);
        glrender_translate(&quad_from_array->points[3], &local_coordinate_quad[3], &direction_vector);
        
        graph_fill_polygon((n_vect2 *)&local_coordinate_quad, 4, local_color, 0, output, &graph_size);
        
        graph_line(output, &graph_size, &local_coordinate_quad[0], &local_coordinate_quad[1], local_color, 3);
        graph_line(output, &graph_size, &local_coordinate_quad[1], &local_coordinate_quad[2], local_color, 3);
        graph_line(output, &graph_size, &local_coordinate_quad[2], &local_coordinate_quad[3], local_color, 3);
        graph_line(output, &graph_size, &local_coordinate_quad[3], &local_coordinate_quad[0], local_color, 3);

        loop++;
    }
}

void glrender_render_lines(n_byte * output, memory_list *lines)
{
    n_vect2 direction_vector;
    n_byte4 loop = 0;
    n_byte4 count = 0;
    glr_line * active_list = (glr_line *)lines->data;
    vect2_direction(&direction_vector, 255-current_turn, 1);
    
    while (loop < lines->count)
    {
        n_vect2 reset_start, reset_end;
        
        glr_line_data local_data;
        
        glrender_translate(&active_list[loop].start, &reset_start, &direction_vector);
        glrender_translate(&active_list[loop].end, &reset_end, &direction_vector);
        
        local_data.color = active_list[loop].color;
        local_data.thickness = (n_byte)active_list[loop].thickness;
        
        loop++;
                
        if ((reset_start.x < 0) && (reset_end.x < 0))
        {
            continue;
        }
        if ((reset_start.y < 0) && (reset_end.y < 0))
        {
            continue;
        }
        if ((reset_start.x > (graph_size.x - 1)) && (reset_end.x > (graph_size.x - 1)))
        {
            continue;
        }
        if ((reset_start.y > (graph_size.y - 1)) && (reset_end.y > (graph_size.y - 1)))
        {
            continue;
        }

        printf("graph_line( %ld %ld ) ( %ld %ld ) ( %ld %ld )\n", graph_size.x,graph_size.y, reset_start.x, reset_start.x, reset_end.x, reset_end.y);
        
        //graph_line(output, &graph_size, &reset_start, &reset_end, (n_rgba32*)&color_map[local_data.color], local_data.thickness);

        count ++;
    }
}

void glrender_render_text(n_byte * output)
{
    glrender_render_lines(output, text_lines);
}

void glrender_render_active(n_byte * output)
{
    glrender_render_lines(output, active_lines);
}

void glrender_render_display(n_byte * output)
{
    glrender_render_erase(output);
    glrender_render_quads(output, display_quads);
    //glrender_render_lines(output, display_lines); // bug resides
}

void glrender_background_green(void)
{
}

void glrender_start_display_list(void)
{
    current_case = GRAPHICS_CASE_DISPLAY;
    if (display_quads == 0L)
    {
        display_quads = memory_list_new(sizeof(glr_quad), 500 * MULTIPLE_CHECK);
    }
    if (display_lines == 0L)
    {
        display_lines = memory_list_new(sizeof(glr_line), 2000 * MULTIPLE_CHECK);
    }
}

void glrender_end_display_list(void)
{
    draw_scene_not_done++;
}

void glrender_start_active_list(void)
{
    current_case = GRAPHICS_CASE_ACTIVE;
    if (active_lines == 0L)
    {
        active_lines  = memory_list_new(sizeof(glr_line), 64 * MULTIPLE_CHECK);
    }
    else
    {
        active_lines->count = 0;
    }
}

void glrender_end_active_list(void)
{
    
}

void glrender_start_text_list(void)
{
    current_case = GRAPHICS_CASE_TEXT;
    if (text_lines == 0L)
    {
        text_lines  = memory_list_new(sizeof(glr_line), 64 * MULTIPLE_CHECK);
    }
    else
    {
        text_lines->count = 0;
    }
}

void glrender_end_text_list(void)
{
    
}

void glrender_wide_line(void)
{
    current_thickness = 12;
}

void glrender_thin_line(void)
{
    current_thickness = 6;
}

#define ENCODE_RGB(r,g,b) (((const n_int)(255*r)<<16)|((n_int)(255*g)<<8)|(n_int)(255*b))

#define DECODE_R(rgb) ((float)((rgb>>16)&255)/(float)255.0)
#define DECODE_G(rgb) ((float)((rgb>> 8)&255)/(float)255.0)
#define DECODE_B(rgb) ((float)((rgb>> 0)&255)/(float)255.0)

void glrender_line(n_vect2 * start, n_vect2 * end)
{
    glr_line new_line;
    
    new_line.color = current_color;
    new_line.thickness = current_thickness;
    new_line.start.x = start->x;
    new_line.start.y = start->y;
    new_line.end.x = end->x;
    new_line.end.y = end->y;

    if (current_case == GRAPHICS_CASE_ACTIVE)
    {
        memory_list_copy(active_lines, (n_byte*)&new_line, sizeof(new_line));
    }
    if (current_case == GRAPHICS_CASE_DISPLAY)
    {
        memory_list_copy(display_lines, (n_byte*)&new_line, sizeof(new_line));
    }
    if (current_case == GRAPHICS_CASE_TEXT)
    {
        memory_list_copy(text_lines, (n_byte*)&new_line, sizeof(new_line));
    }
}

void glrender_fill(n_vect2 * quads)
{
    glr_quad new_quad;
    new_quad.color = current_color;
    memory_copy((n_byte*)quads, (n_byte*)new_quad.points, sizeof(n_vect2) * 4);

    if (current_case == GRAPHICS_CASE_ACTIVE)
    {
        //NO ACTIVE QUADS
    }
    if (current_case == GRAPHICS_CASE_DISPLAY)
    {
        memory_list_copy(display_quads, (n_byte*)&new_quad, sizeof(glr_quad));
    }
}

void glrender_quads(n_vect2 * quads, n_byte filled)
{
    if (filled)
    {
        glrender_fill(quads);
    }
    glrender_wide_line();
    glrender_line(&quads[0], &quads[1]);
    glrender_line(&quads[1], &quads[2]);
    glrender_line(&quads[2], &quads[3]);
    glrender_line(&quads[3], &quads[0]);
}

void glrender_delta_move(n_vect2 * center, n_vect2 * location, n_int turn, n_int scale)
{
    current_center.x = center->x;
    current_center.y = center->y;
    current_location.x = location->x;
    current_location.y = location->y;
    current_turn = turn;
    current_scale = 100 + scale;
}

void glrender_init(void)
{
#ifndef _WIN32
    graph_init(1);
#endif
}

void glrender_reset(void)
{
    if (display_quads)
    {
        display_quads->count = 0;
    }
    if (display_lines)
    {
        display_lines->count = 0;
    }
    if (active_lines)
    {
        active_lines->count = 0;
    }
    if (text_lines)
    {
        text_lines->count = 0;
    }
}

void glrender_close(void)
{
    if (display_quads)
    {
        memory_list_free(&display_quads);
    }
    if (display_lines)
    {
        memory_list_free(&display_lines);
    }
    if (active_lines)
    {
        memory_list_free(&active_lines);
    }
    if (text_lines)
    {
        memory_list_free(&text_lines);
    }
}
