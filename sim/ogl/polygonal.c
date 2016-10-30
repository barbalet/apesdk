/****************************************************************

 polygonal.c

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

/*! \file   polygonal.c
 *  \brief  Provides the polygonal rendering of the Noble Ape Simulation
    environment and also should produce a similar (and refining) interface
    to draw.c/gui.h.
 */

/*NOBLEMAKE DEL=""*/

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <OpenGL/OpenGL.h>

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../universe/universe_internal.h"
#include "../gui/gui.h"

#else

#include "..\noble\noble.h"
#include "..\universe\universe.h"
#include "..\universe\universe_internal.h"
#include "..\gui\gui.h"

#endif

#include "ogl.h"


#define RESOLUTION (1024)

static n_byte polygonal_terrain_first_run = 1;

static GLdouble color[256*3] = {0.0f};

#define LAND_STEP_AREA 2

/* TODO: step through the direction facing
   The sections that project forward should be calculated accordingly

 +-------+
 |       |
 |       |
 |   *   |
 +-------+

 +-------+
 |       |
 |       |
 |  *    |
 +-------+

 +-------+
 |       |
 | *     |
 |       |
 +-------+


   */

static void polygonal_render_terrain(n_byte * local_land, n_int co_x, n_int co_y)
{
    n_int       y0 = local_land[POSITIVE_LAND_COORD_HIRES(co_x) |
                                (POSITIVE_LAND_COORD_HIRES(co_y) * HI_RES_MAP_DIMENSION)];
    GLdouble    point[3];
    n_int       loop_y = 0 - RESOLUTION;
    while (loop_y < RESOLUTION)
    {
        n_int   loop_x = 0 - RESOLUTION;
        n_int   squarey =    loop_y;
        n_int   offset_y0 = (POSITIVE_LAND_COORD_HIRES(loop_y + co_y) * HI_RES_MAP_DIMENSION);
        n_int   offset_y1 = (POSITIVE_LAND_COORD_HIRES(loop_y + co_y + LAND_STEP_AREA) * HI_RES_MAP_DIMENSION);

        squarey *= squarey;

        glBegin (GL_QUAD_STRIP);

        while (loop_x < RESOLUTION)
        {
            n_int   y1 = local_land[POSITIVE_LAND_COORD_HIRES(loop_x + co_x) | offset_y0];
            n_int   squarex = loop_x;

            squarex *= squarex;

            point[0] = loop_x;
            point[1] = y1 - y0;
            point[2] = loop_y;

            if ((squarey + squarex) < 26)
            {
                glColor3d (1.0f, 0.0f, 0.0f);
            }
            else
            {
                glColor3dv (&color[(y1*3)]);
            }
            glVertex3dv(point);
            point[2] += (LAND_STEP_AREA * 1.0f);

            y1 = local_land[POSITIVE_LAND_COORD_HIRES(loop_x + co_x) | offset_y1];

            point[1]  = y1 - y0;
            if ((squarey + squarex) < 26)
            {
                glColor3d (1.0f, 0.0f, 0.0f);
            }
            else
            {
                glColor3dv (&color[(y1*3)]);
            }
            glVertex3dv(point);
            loop_x += LAND_STEP_AREA;
        }
        glEnd ();
        loop_y += LAND_STEP_AREA;
    }
}


static void polygonal_terrain_init()
{
    n_int   loop = 0;
    n_byte2 points[256*3];
    
    glShadeModel (GL_SMOOTH);	// Enable Smooth Shading

    glClearDepth (1.0f);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


    glViewport (0, 0, 512, 512);
    glMatrixMode (GL_PROJECTION);
    // glLoadIdentity ();
    gluPerspective (45.0f, (GLfloat)1.0f, 0.1f, 2000.0f);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    glEnable ( GL_CULL_FACE );

    glClearColor (0.1f, 0.1f, 0.5f, 0.5f);

    draw_fit(land_points, points);

    while( loop < (256 * 3))
    {
        color[loop] = ((GLdouble)points[loop])/65536.f;
        loop++;
    }

    polygonal_terrain_first_run = 0;
}




static void polygonal_terrain(void)
{
    noble_simulation * local_sim = sim_sim();
    noble_being * loc_being = local_sim->select;
    n_int turn = being_facing(loc_being);
    n_vect2 co;
    n_int modified_turn = ((32+64+8) + turn) & 255;
    GLdouble rotating_angle = (modified_turn * 360.0)/256.0;

    being_high_res(loc_being, &co);
    
    if (polygonal_terrain_first_run)
    {
        polygonal_terrain_init();
    }

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity ();

    gluLookAt (256, 256, 256,
               0, 0, 0,
               0, 1, 0);

    glRotated(rotating_angle, 0.0f, 1.0f, 0.0f);

    polygonal_render_terrain(land_topology_highdef(), co.x, co.y);
}

void polygonal_close(void)
{
}


n_int polygonal_entry(n_int value)
{
    if (value == NUM_VIEW)
    {
        return 1;
    }
    polygonal_terrain();
    return 0;
}


