/****************************************************************

 graph.c

 =============================================================

 Copyright 1996-2019 Tom Barbalet. All rights reserved.

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

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../entity/entity.h"
#include "../gui/gui.h"

#else

#include "..\noble\noble.h"
#include "..\universe.h\universe.h"
#include "..\entity\entity.h"
#include "..\gui\gui.h"

#endif

#include <math.h>

#include "graph.h"

/**
 * @brief Returns an array of 2D points used for drawing diagrams
 * @param source_points Array of 2D points which is the template
 * @param no_of_source_points Number of 2D points in the template
 * @param extra_points The number of points to be returned via the extra parameters
 * @param x The starting x coordinate
 * @param y The starting y coordinate
 * @param mirror Flip in the vertical axis
 * @param scale_width Horizontal scaling factor x1000
 * @param scale_length Length (vertical) scaling factor x1000
 * @param angle Rotation angle of the result
 * @param points Returned 2D points
 * @param no_of_points Number of returned 2D points
 * @param max_points The maximum number of points which may be returned
 */

void outline_points(const n_vect2 * source_points,
                           n_int no_of_source_points, n_int extra_points,
                           n_int x, n_int y,
                           n_byte mirror,
                           n_vect2 * scale,
                           n_int angle,
                           n_vect2 *axis,
                           n_vect2 *extra_1,
                           n_vect2 *extra_2,
                           n_vect2 *extra_3,
                           n_vect2 *extra_4,
                           n_points * collection)
{
    n_vect2  ds, location, vector;
    n_int    i, axis_length,point_length;
    n_double axis_angle, point_angle;
    n_double ang = angle*TWO_PI/7200;
    
    vect2_populate(&location, x, y);
    vect2_subtract(&ds, (n_vect2 *)&source_points[1], (n_vect2 *)source_points);
    vect2_multiplier(&ds, &ds, scale, 1, 1000);
    
    /** length of the object */
    axis_length = (n_int)math_root(vect2_dot(&ds, &ds, 1, 1));
    if (axis_length < 1) axis_length=1;
    
    /** invert around the vertical axis if needed */
    if (mirror != 0)
    {
        ds.x = -ds.x;
    }
    
    /** find the orientation angle of the axis */
    axis_angle = (float)acos(ds.x/(float)axis_length);
    
    if (ds.y < 0)
    {
        axis_angle = TWO_PI-axis_angle;
    }
    
    vect2_populate(&vector, (n_int)(axis_length*sin(ang+(TWO_PI/4)-axis_angle)),
                   (n_int)(axis_length*cos(ang+(TWO_PI/4)-axis_angle)));
    
    /** calculate the position of the end point of the axis */
    
    vect2_add(axis, &location, &vector);
    
    /** draw lines between each point */
    for (i = 2; i < no_of_source_points + 2 + extra_points; i++)
    {
        n_vect2 point;
        vect2_subtract(&ds, (n_vect2 *)&source_points[i], (n_vect2 *)source_points);
        vect2_multiplier(&ds, &ds, scale, 1, 1000);
        point_length = (n_int)math_root(vect2_dot(&ds, &ds, 1, 1));
        if (point_length < 1)
        {
            point_length = 1;
        }
        
        /** invert the line around the vertical axis if necessary */
        if (mirror != 0)
        {
            ds.x = -ds.x;
        }
        
        /** angle of the line */
        point_angle = (float)acos(ds.x/(float)point_length);
        if (ds.y < 0)
        {
            point_angle = (TWO_PI)-point_angle;
        }
        
        /** position of the end of the line */
        vect2_populate(&vector, (n_int)(point_length*sin(ang+point_angle-axis_angle)),
                                (n_int)(point_length*cos(ang+point_angle-axis_angle)));
        
        vect2_add(&point, &location, &vector);
        
        /** store the calculated point positions in an array */
        if (collection->no_of_points < collection->max_points)
        {
            if (i < no_of_source_points + 2)
            {
                vect2_copy(&collection->points[collection->no_of_points], &point);
                collection->no_of_points++;
            }
        }
        /* TODO
        else
        {
            (void)SHOW_ERROR("Maximum number of skeleton points reached");
        }
        */
        /** This is a crude way of keeping track of the last few points
         so that they can be returned by the function */
        vect2_copy(extra_1, extra_2);
        vect2_copy(extra_2, extra_3);
        vect2_copy(extra_3, extra_4);
        vect2_copy(extra_4, &point);
    }
    
    if (collection->no_of_points > -1 && collection->no_of_points < collection->max_points)
    {
        collection->points[collection->no_of_points].x = 9999;
        collection->points[collection->no_of_points].y = 9999;
        collection->no_of_points++;
    }
    else
    {
        SHOW_ERROR("Outside point range for drawing");
    }
}



