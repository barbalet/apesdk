/****************************************************************

 gldraw.c

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

/*! \file   gldraw.c
 *  \brief  Handles OpenGL graphics
 */

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#import <OpenGL/OpenGL.h>

#include "noble.h"
#include "gldraw.h"

static n_int draw_scene_not_done = 0;
static GLuint  terrain_display_list = 0;
static GLuint  active_display_list = 0;


static void gldraw_character_line(n_int px, n_int py, n_int dx, n_int dy)
{
    n_vect2 start;
    n_vect2 end;
    start.x = px;
    start.y = py;
    end.x = px + dx;
    end.y = py + dy;
    gldraw_line(&start, &end);
}

/* draws a string starting at point (off_x,off_y) */

#define    glledfir(x, y, dx, dy, c)    if(((val >> c)&1)) gldraw_character_line((0- x) + off_x - offset,y + off_y,  0 - dx, dy)

/**
 This is used to produce letter LED style letters through the generic
 draw function specified.
 @param str The string to be drawn.
 @param off_x The starting x location for the string to be drawn.
 @param off_y The starting y location for the string to be drawn.
 @param draw The generic draw function used to draw the character.
 */
void gldraw_string(n_constant_string str, n_int off_x, n_int off_y)
{
    n_int    char_loop = 0;
    while (str[char_loop] > 31)
    {
        n_int    val = math_seg14(str[char_loop] - 32);
        n_int    offset = char_loop << 3;
        /* draw the character as a 14-segment LCD/LED output */
        
        glledfir(3, 8, 0, 0, 15);
        
        glledfir(3, 2, 0, 0, 14);
        
        glledfir(1, 0, 4, 0, 13);
        
        glledfir(6, 1, 0, 2, 12);
        
        glledfir(6, 5, 0, 2, 11);
        
        glledfir(1, 8, 4, 0, 10);
        
        glledfir(0, 5, 0, 2, 9);
        
        glledfir(0, 1, 0, 2, 8);
        
        glledfir(4, 4, 1, 0, 7);
        
        glledfir(1, 4, 1, 0, 6);
        
        glledfir(3, 5, 0, 2, 5);
        
        glledfir(4, 6, 0, 1, 4);
        
        glledfir(2, 6, -1, 1, 3);
        
        glledfir(4, 2, 1, -1, 2);
        
        glledfir(1, 1, 1, 1, 1);
        
        glledfir(3, 1, 0, 2, 0);
        char_loop ++;
    }
}

n_int gldraw_scene_done()
{
    return (draw_scene_not_done < 2);
}

void gldraw_background_green(void)
{
    glClearColor(0, 0.15, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gldraw_background_black(void)
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gldraw_start_display_list(void)
{
    terrain_display_list = glGenLists(1);
    glNewList(terrain_display_list, GL_COMPILE);
}

void gldraw_end_display_list(void)
{
    glEndList();
    draw_scene_not_done++;
}

void gldraw_display_list(void)
{
    glCallList(terrain_display_list);
}

void gldraw_start_active_list(void)
{
    active_display_list = glGenLists(1);
    glNewList(active_display_list, GL_COMPILE);
}

void gldraw_end_active_list(void)
{
    glEndList();
}

void gldraw_active_list(void)
{
    glCallList(active_display_list);
}

void gldraw_wide_line()
{
    glLineWidth(2);
}

void gldraw_thin_line()
{
    glLineWidth(1);
}

void gldraw_red(void)
{
    glColor3f(1, 0, 0);
}

void gldraw_green(void)
{
    glColor3f(0.2, 0.4, 0.2);
}

void gldraw_lightgreen(void)
{
    glColor3f(0.4, 0.8, 0.4);
}

void gldraw_orange()
{
    glColor3f(0.5, 0.2, 0.0);
}

void gldraw_blue()
{
    glColor3f(0.0, 0.0, 0.5);
}

void gldraw_blue_clear()
{
    glColor4f(0.0, 0.0, 0.5, 0.5);
}

void gldraw_darkgrey()
{
    glColor3f(0.5, 0.5, 0.5);
}

void gldraw_grey()
{
    glColor3f(0.7, 0.7, 0.7);
}

void gldraw_lightgrey()
{
    glColor3f(0.9, 0.9, 0.9);
}

void gldraw_lightgrey_clear()
{
    glColor4f(0.9, 0.9, 0.9, 0.5);
}

void gldraw_black()
{
    glColor3f(0, 0, 0);
}

void gldraw_vertex(n_vect2 * point)
{
    glVertex2i((GLint)point->x, (GLint)point->y);
}

void gldraw_line(n_vect2 * start, n_vect2 * end)
{
    glBegin(GL_LINES);
    
    gldraw_vertex(start);
    gldraw_vertex(end);
    
    glEnd();
}

void gldraw_quads(n_vect2 * quads, n_byte filled)
{
    if (filled)
    {
        glBegin(GL_QUADS);
    }
    else
    {
        glBegin(GL_LINE_LOOP);
    }
    gldraw_vertex(&quads[0]);
    gldraw_vertex(&quads[1]);
    gldraw_vertex(&quads[2]);
    gldraw_vertex(&quads[3]);
    glEnd();
}

void gldraw_delta_move(n_vect2 * center, n_vect2 * location, n_int turn, n_int scale)
{
    GLfloat floating_scale = ((GLfloat)(100+scale))/100;
    glTranslatef(0-center->x , 0-center->y, 0);
    glRotatef(1.40625 * turn, 0, 0, 1);
    glScaled(floating_scale, floating_scale, 1);
    glTranslatef(location->x + center->x, location->y + center->y, 0);
}

void gldraw_start_points(void)
{
    glBegin(GL_POINTS);
}

void gldraw_end_points(void)
{
    glEnd();
}
