/****************************************************************

 graph.h

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

/*! \file   graph.c
 *  \brief  This covers the primary graphics of Simulated Ape.
 */

#include "../toolkit/toolkit.h"

void graph_init( n_int four_byte_factory );
void graph_init_three(void);

void graph_erase( n_byte *buffer, n_vect2 *img, n_rgba32 *color );

/* draws a line */
void graph_line( n_byte *buffer,
                 n_vect2 *img,
                 n_vect2 *previous,
                 n_vect2 *current,
                 n_rgba32 *color,
                 n_byte thickness );

void graph_curve( n_byte *buffer,
                  n_vect2 *img,
                  n_vect2 *pt0,
                  n_vect2 *pt1,
                  n_vect2 *pt2,
                  n_rgba32 *color,
                  n_byte radius_percent,
                  n_uint start_thickness,
                  n_uint end_thickness );

void graph_fill_polygon( n_vect2 *points, n_int no_of_points,
                         n_rgba32 *color, n_byte transparency,
                         n_byte *buffer, n_vect2 *img );

