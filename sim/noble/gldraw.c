/****************************************************************

 gldraw.c

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

/*! \file   gldraw.c
 *  \brief  Handles OpenGL graphics
 */

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#import <OpenGL/OpenGL.h>

#include "noble.h"

static n_int draw_scene_not_done = 0;
static GLuint  terrain_display_list = 0;

n_int gldraw_scene_done()
{
    return (draw_scene_not_done < 2);
}

void gldraw_background_green(void)
{
    glClearColor(0, 0.15, 0, 0);
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

void gldraw_wide_line()
{
    glLineWidth(100);
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
    glColor3f(0.3, 0.3, 1);
}

void gldraw_orange()
{
    glColor3f(0.5, 0.2, 0.0);
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

void gldraw_delta_move(n_vect2 * center, n_vect2 * location, n_int turn)
{
    glTranslatef(0-center->x , 0-center->y, 0);
    glRotatef(1.40625 * turn, 0, 0, 1);
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
