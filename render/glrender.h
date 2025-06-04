/****************************************************************

 glrender.h

 =============================================================

 Copyright 1996-2024 Tom Barbalet. All rights reserved.

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

/*! \file   glrender.h
 *  \brief  This is specifically for the shared GL interface
 */

#ifndef _glrender_h_
#define _glrender_h_

#include "../toolkit/toolkit.h"
#include "graph.h"

typedef enum{
    GLR_GREEN = 0,
    GLR_LIGHT_GREEN = 1,
    GLR_RED = 2,
    GLR_ORANGE = 3,
    GLR_LIGHT_GREY = 4,
    GLR_GREY = 5,
    GLR_DARK_GREY = 6,
    GLR_BLACK = 7
} GLR_COLOR;


void glrender_set_size(n_int size_x, n_int size_y);

void glrender_string(n_constant_string str, n_int off_x, n_int off_y);

void glrender_color_map(n_byte2 * value);

void glrender_color_map_replace(n_byte4 * replace);

void glrender_color(const GLR_COLOR color);

void glrender_background_green(void); /* */

void glrender_start_text_list(void); /* */
void glrender_end_text_list(void); /* */

void glrender_start_display_list(void); /* */
void glrender_end_display_list(void); /* */

void glrender_start_active_list(void); /* */
void glrender_end_active_list(void); /* */

n_int glrender_scene_done(void); /* */

void glrender_wide_line(void); /* */
void glrender_thin_line(void); /* */

void glrender_line(n_vect2 * start, n_vect2 * end); /* */
void glrender_quads(n_vect2 * quads, n_byte filled); /* */

void glrender_delta_move(n_vect2 * center, n_vect2 * location, n_int turn, n_int scale); /* */

void glrender_render_lines(n_byte * output, memory_list *lines);

void glrender_render_text(n_byte * output);
void glrender_render_display(n_byte * output);
void glrender_render_active(n_byte * output);

void glrender_init(void);
void glrender_reset(void);
void glrender_close(void);

#endif /* _glrender_h_ */

