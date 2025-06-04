/****************************************************************

 graph.c

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

/*! \file   graph.c
 *  \brief  This covers the primary graphics of Simulated Ape.
 */

#include "stdio.h"
#include "graph.h"

typedef void ( graph_func_set_color )( n_byte *buffer, n_rgba32 *color, n_int number );
typedef void ( graph_func_set_color_transparency )( n_byte *buffer, n_rgba32 *color, n_int number, n_byte transparency );
typedef n_int ( graph_func_bytes_per_unit )( void );

void graph_four_set_color_transparency( n_byte *buffer, n_rgba32 *color, n_int number, n_byte transparency )
{
    n_int n4 = number * 4;
    buffer[n4]   = ( ( color->rgba.b * ( 255 - transparency ) ) + ( buffer[n4] * transparency ) ) / 256;
    buffer[n4 + 1] = ( ( color->rgba.g * ( 255 - transparency ) ) + ( buffer[n4 + 1] * transparency ) ) / 256;
    buffer[n4 + 2] = ( ( color->rgba.r * ( 255 - transparency ) ) + ( buffer[n4 + 2] * transparency ) ) / 256;
    buffer[n4 + 3] = 0;
}

void graph_four_set_color( n_byte *buffer, n_rgba32 *color, n_int number )
{
    n_byte4 *convert = ( n_byte4 * )buffer;
    convert[number] = color->thirtytwo;
}

n_int graph_four_bytes_per_unit( void )
{
    return 4;
}

void graph_three_set_color_transparency( n_byte *buffer, n_rgba32 *color, n_int number, n_byte transparency )
{
    n_int n3 = number * 3;
    buffer[n3]   = ( ( color->rgba.b * ( 255 - transparency ) ) + ( buffer[n3] * transparency ) ) / 256;
    buffer[n3 + 1] = ( ( color->rgba.g * ( 255 - transparency ) ) + ( buffer[n3 + 1] * transparency ) ) / 256;
    buffer[n3 + 2] = ( ( color->rgba.r * ( 255 - transparency ) ) + ( buffer[n3 + 2] * transparency ) ) / 256;
}

void graph_three_set_color( n_byte *buffer, n_rgba32 *color, n_int number )
{
    n_int n3 = number * 3;
    buffer[n3]   = color->rgba.b;
    buffer[n3 + 1] = color->rgba.g;
    buffer[n3 + 2] = color->rgba.r;

}

n_int graph_three_bytes_per_unit( void )
{
    return 3;
}

void graph_one_set_color_transparency( n_byte *buffer, n_rgba32 *color, n_int number, n_byte transparency )
{
    buffer[number]   = ( ( color->rgba.b * ( 255 - transparency ) ) + ( buffer[number] * transparency ) ) / 256;
}

void graph_one_set_color( n_byte *buffer, n_rgba32 *color, n_int number )
{
    buffer[number] = color->rgba.b;
}

n_int graph_one_bytes_per_unit( void )
{
    return 1;
}

static graph_func_set_color       *graph_local_set_color = &graph_one_set_color;
static graph_func_bytes_per_unit *graph_local_bytes_per_unit = &graph_one_bytes_per_unit;
static graph_func_set_color_transparency *graph_local_set_color_transparency = &graph_one_set_color_transparency;


void graph_init( n_int four_byte_factory )
{
    if ( four_byte_factory )
    {
        graph_local_set_color = &graph_four_set_color;
        graph_local_bytes_per_unit = &graph_four_bytes_per_unit;
        graph_local_set_color_transparency = &graph_four_set_color_transparency;
    }
}

void graph_init_three(void)
{
    graph_local_set_color = &graph_three_set_color;
    graph_local_bytes_per_unit = &graph_three_bytes_per_unit;
    graph_local_set_color_transparency = &graph_three_set_color_transparency;
}

void graph_erase( n_byte *buffer, n_vect2 *img, n_rgba32 *color )
{
    n_int i = 0;
    while ( i < img->x )
    {
        graph_local_set_color( buffer, color, i++ );
    }
    i = 1;
    while ( i < img->y )
    {
        n_int bytes_per_unit = graph_local_bytes_per_unit();
        memory_copy( buffer, &buffer[ i++ * img->x * bytes_per_unit], ( n_uint )( img->x * bytes_per_unit ) );
    }
}

typedef struct{
    n_vect2  * img;
    n_rgba32 * color;
    n_byte   * buffer;
}graph_point_for_line;

n_byte ( graph_line_to_point )( n_int px, n_int py, n_int dx, n_int dy, void *information )
{
    graph_point_for_line * local_data = (graph_point_for_line *)information;
    n_vect2              * local_img_dimensions = local_data->img;
    if ((px >= 0) && (px < local_img_dimensions->x))
    {
        if ((py >= 0) && (py < local_img_dimensions->y))
        {
            n_int number = ( py * local_img_dimensions->x + px );
            graph_local_set_color( local_data->buffer, local_data->color, number );
        }
    }
    
    return 1;
}


void graph_line2( n_byte   *buffer,
                 n_vect2  *img,
                 n_vect2  *previous,
                 n_vect2  *current,
                 n_rgba32 *color,
                 n_byte thickness )
{
    n_join  graph_line_join;
    
    graph_point_for_line data;
    data.img = img;
    data.color = color;
    data.buffer = buffer;

    graph_line_join.information = &data;
    graph_line_join.pixel_draw =  &graph_line_to_point;
    
    math_line_vect( previous, current, &graph_line_join );
    if ( thickness > 2 )
    {
        previous->x --;
        current->x --;
        math_line_vect( previous, current, &graph_line_join );
        previous->x += 2;
        current->x += 2;
        math_line_vect( previous, current, &graph_line_join );
        previous->x --;
        current->x --;

        previous->y --;
        current->y --;
        math_line_vect( previous, current, &graph_line_join );
        previous->y += 2;
        current->y += 2;
        math_line_vect( previous, current, &graph_line_join );
        previous->y --;
        current->y --;
    }
}


/* draws a line */
void graph_line( n_byte *buffer,
                 n_vect2 *img,
                 n_vect2 *previous,
                 n_vect2 *current,
                 n_rgba32 *color,
                 n_byte thickness )
{
    n_int i, max;
    n_vect2 delta;
    n_vect2 absdelta;

    vect2_subtract( &delta, current, previous );

    vect2_copy( &absdelta, &delta );

    if ( absdelta.x < 0 )
    {
        absdelta.x = -delta.x;
    }
    if ( absdelta.y < 0 )
    {
        absdelta.y = -delta.y;
    }

    max = absdelta.x;
    if ( absdelta.y > max )
    {
        max = absdelta.y;
    }

    for ( i = 0; i < max; i++ )
    {
        n_int xx = previous->x + ( i * ( current->x - previous->x ) / max );
        if ( ( xx > -1 ) && ( xx < img->x ) )
        {
            n_int yy = previous->y + ( i * ( current->y - previous->y ) / max );
            if ( ( yy > -1 ) && ( yy < img->y ) )
            {
                n_int n = ( yy * img->x + xx );
                graph_local_set_color( buffer, color, n );

                if ( thickness > 2 )
                {
                    if ( yy > 0 )
                    {
                        n_int n = ( yy - 1 ) * img->x + xx;
                        graph_local_set_color( buffer, color, n );
                    }
                    if ( xx > 0 )
                    {
                        n_int n = ( yy * img->x + xx - 1 );
                        graph_local_set_color( buffer, color, n );
                    }
                    if ( ( yy + 1 ) < img->y )
                    {
                        n_int n = ( ( yy + 1 ) * img->x + xx );
                        graph_local_set_color( buffer, color, n );
                    }
                    if ( ( xx + 1 ) < img->x )
                    {
                        n_int n = ( yy * img->x + xx + 1 );
                        graph_local_set_color( buffer, color, n );
                    }
                }

            }
        }
    }
}


/**
 * @brief Draws a curve using three points
 * @param buffer Image buffer (three bytes per pixel)
 * @param img Vector size of the image
 * @param pt0 the start point
 * @param pt1 the middle point
 * @param pt2 the end point
 * @param color color
 * @param radius_percent Radius of the curve as a percentage
 * @param start_thickness Thickness of the curve at the start point
 * @param end_thickness Thickness of the curve at the end point
 */
void graph_curve( n_byte *buffer,
                  n_vect2 *img,
                  n_vect2 *pt0,
                  n_vect2 *pt1,
                  n_vect2 *pt2,
                  n_rgba32 *color,
                  n_byte radius_percent,
                  n_uint start_thickness,
                  n_uint end_thickness )
{
    n_int pts[8];

    n_vect2 current;
    n_vect2 previous = {0, 0};

    n_uint i;
    const n_uint divisions = 20;
    n_double c[5], d[5], f;

    /** turn three points into four using the curve radius */
    pts[0] = pt0->x;
    pts[1] = pt0->y;

    pts[2] = pt1->x + ( ( pt0->x - pt1->x ) * radius_percent / 100 );
    pts[3] = pt1->y + ( ( pt0->y - pt1->y ) * radius_percent / 100 );

    pts[4] = pt1->x + ( ( pt2->x - pt1->x ) * radius_percent / 100 );
    pts[5] = pt1->y + ( ( pt2->y - pt1->y ) * radius_percent / 100 );

    pts[6] = pt2->x;
    pts[7] = pt2->y;

    c[0] = ( -pts[0 * 2] + 3 * pts[1 * 2] - 3 * pts[2 * 2] + pts[3 * 2] ) / 6.0;
    c[1] = ( 3 * pts[0 * 2] - 6 * pts[1 * 2] + 3 * pts[2 * 2] ) / 6.0;
    c[2] = ( -3 * pts[0 * 2] + 3 * pts[2 * 2] ) / 6.0;
    c[3] = ( pts[0 * 2] + 4 * pts[1 * 2] + pts[2 * 2] ) / 6.0;

    d[0] = ( -pts[( 0 * 2 ) + 1] + 3 * pts[( 1 * 2 ) + 1] - 3 * pts[( 2 * 2 ) + 1] + pts[( 3 * 2 ) + 1] ) / 6.0;
    d[1] = ( 3 * pts[( 0 * 2 ) + 1] - 6 * pts[( 1 * 2 ) + 1] + 3 * pts[( 2 * 2 ) + 1] ) / 6.0;
    d[2] = ( -3 * pts[( 0 * 2 ) + 1] + 3 * pts[( 2 * 2 ) + 1] ) / 6.0;
    d[3] = ( pts[( 0 * 2 ) + 1] + 4 * pts[( 1 * 2 ) + 1] + pts[( 2 * 2 ) + 1] ) / 6.0;

    for ( i = 0; i < divisions; i++ )
    {
        f = ( n_double )i / ( n_double )divisions;
        current.x = ( n_int )( ( c[2] + f * ( c[1] + f * c[0] ) ) * f + c[3] );
        current.y = ( n_int )( ( d[2] + f * ( d[1] + f * d[0] ) ) * f + d[3] );

        if ( i > 0 )
        {
            graph_line( buffer, img,
                        &previous, &current,
                        color,
                        ( n_byte )( start_thickness +
                                    ( ( end_thickness - start_thickness ) * i / divisions ) ) );
        }
        vect2_copy( &previous, &current );
    }
}

#define  MAX_POLYGON_CORNERS 1000

/**
 * @brief Draw a filled polygon
 * @param points Array containing 2D points
 * @param no_of_points The number of 2D points
 * @param color color of polygon
 * @param transparency Degree of transparency
 * @param buffer Image buffer (3 bytes per pixel)
 * @param img image vector size
 */
void graph_fill_polygon( n_vect2 *points, n_int no_of_points,
                         n_rgba32 *color, n_byte transparency,
                         n_byte *buffer, n_vect2 *img )
{
    n_int nodes, nodeX[MAX_POLYGON_CORNERS] = {0}, i, j, swap, n, x, y;
    n_int min_x = 99999, min_y = 99999;
    n_int max_x = -99999, max_y = -99999;

    for ( i = 0; i < no_of_points; i++ )
    {
        x = points[i].x;
        y = points[i].y;
        if ( ( x == 9999 ) || ( y == 9999 ) )
        {
            continue;
        }
        if ( x < min_x )
        {
            min_x = x;
        }
        if ( y < min_y )
        {
            min_y = y;
        }
        if ( x > max_x )
        {
            max_x = x;
        }
        if ( y > max_y )
        {
            max_y = y;
        }
    }

    if ( min_x < 0 )
    {
        min_x = 0;
    }
    if ( min_y < 0 )
    {
        min_y = 0;
    }
    if ( max_x >= img->x )
    {
        max_x = img->x - 1;
    }
    if ( max_y >= img->y )
    {
        max_y = img->y - 1;
    }

    for ( y = min_y; y <= max_y; y++ )
    {
        /**  Build a list of nodes */
        nodes = 0;
        j = no_of_points - 1;
        for ( i = 0; i < no_of_points; i++ )
        {
            if ( ( ( points[i].y < y ) && ( points[j].y >= y ) ) ||
                    ( ( points[j].y < y ) && ( points[i].y >= y ) ) )
            {
                if ( points[j].y - points[i].y )
                {
                    nodeX[nodes++] =
                        points[i].x + ( y - points[i].y ) *
                        ( points[j].x - points[i].x ) /
                        ( points[j].y - points[i].y );
                }
            }
            j = i;
            if ( nodes == MAX_POLYGON_CORNERS )
            {
                break;
            }
        }

        /**  Sort the nodes, via a simple “Bubble” sort */
        i = 0;
        while ( i < nodes - 1 )
        {
            if ( nodeX[i] > nodeX[i + 1] )
            {
                swap = nodeX[i];
                nodeX[i] = nodeX[i + 1];
                nodeX[i + 1] = swap;
                if ( i )
                {
                    i--;
                }
            }
            else
            {
                i++;
            }
        }

        /**  Fill the pixels between node pairs */
        for ( i = 0; i < nodes; i += 2 )
        {
            if ( nodeX[i] >= max_x )
            {
                break;
            }
            if ( nodeX[i + 1] > min_x )
            {
                /** range check */
                if ( nodeX[i] <= min_x )
                {
                    nodeX[i] = min_x + 1;
                }
                if ( nodeX[i + 1] >= max_x )
                {
                    nodeX[i + 1] = max_x - 1;
                }

                for ( x = nodeX[i]; x < nodeX[i + 1]; x++ )
                {
                    n = ( ( y * img->x ) + x );

                    if ( transparency == 0 )
                    {
                        graph_local_set_color( buffer, color, n );
                    }
                    else
                    {
                        graph_local_set_color_transparency( buffer, color, n, transparency );
                    }

                }
            }
        }
    }
}
