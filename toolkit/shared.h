/****************************************************************

 shared.h

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

#ifndef SIMULATEDAPE_SHARED_H
#define SIMULATEDAPE_SHARED_H

#ifndef	_WIN32

#include "toolkit.h"

#else

#include "toolkit.h"

#endif

typedef enum
{
    SHARED_CYCLE_OK = 0,
    SHARED_CYCLE_QUIT,
    SHARED_CYCLE_DEBUG_OUTPUT,
    SHARED_CYCLE_NEW_APES
} shared_cycle_state;


void shared_color_8_bit_to_48_bit(n_byte2 * fit);

void shared_dimensions(n_int * dimensions);

n_int shared_init(n_int view, n_uint random);

void shared_close(void);

n_int shared_menu(n_int menuValue);

n_uint shared_max_fps(void);

void shared_rotate(n_double num, n_int wwind);
void shared_delta(n_double delta_x, n_double delta_y, n_int wwind);
void shared_zoom(n_double num, n_int wwind);

void shared_keyReceived(n_int value, n_int localIdentification);
void shared_keyUp(void);

void shared_mouseOption(n_byte option);
void shared_mouseReceived(n_double valX, n_double valY, n_int localIdentification);
void shared_mouseUp(void);

void shared_about(void);

void shared_draw(n_byte * outputBuffer, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed);
void shared_draw_ios(n_byte4 * outputBuffer, n_int dim_x, n_int dim_y);

shared_cycle_state shared_cycle(n_uint ticks, n_int localIdentification);
shared_cycle_state shared_cycle_ios(n_uint ticks);

n_byte * shared_legacy_draw(n_byte fIdentification, n_int dim_x, n_int dim_y);

n_int shared_new(n_uint seed);
n_int shared_new_agents(n_uint seed);

n_byte shared_openFileName(n_constant_string cStringFileName, n_int isScript);

void shared_saveFileName(n_constant_string cStringFileName);

void shared_script_debug_handle(n_constant_string cStringFileName);

#ifndef	_WIN32

n_int sim_thread_console_quit(void);

#endif

#endif /* SIMULATEDAPE_SHARED_H */
