/****************************************************************

 gldraw.h

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

/*! \file   gldraw.h
 *  \brief  This is specifically for the shared GL interface
 */

#ifndef _NOBLEAPE_GLDRAW_H_
#define _NOBLEAPE_GLDRAW_H_

void gldraw_string(n_constant_string str, n_int off_x, n_int off_y);

void gldraw_background_green(void);
void gldraw_background_black(void);

void gldraw_start_display_list(void);
void gldraw_end_display_list(void);
void gldraw_display_list(void);

void gldraw_start_active_list(void);
void gldraw_end_active_list(void);
void gldraw_active_list(void);

n_int gldraw_scene_done(void);

void gldraw_wide_line(void);
void gldraw_thin_line(void);

void gldraw_orange(void);
void gldraw_blue(void);
void gldraw_blue_clear(void);

void gldraw_darkgrey(void);
void gldraw_grey(void);
void gldraw_lightgrey(void);
void gldraw_lightgrey_clear(void);

void gldraw_black(void);

void gldraw_red(void);
void gldraw_green(void);
void gldraw_lightgreen(void);

void gldraw_vertex(n_vect2 * point);
void gldraw_line(n_vect2 * start, n_vect2 * end);
void gldraw_quads(n_vect2 * quads, n_byte filled);

void gldraw_delta_move(n_vect2 * center, n_vect2 * location, n_int turn, n_int scale);

void gldraw_start_points(void);
void gldraw_end_points(void);

#endif /* _NOBLEAPE_GLDRAW_H_ */

