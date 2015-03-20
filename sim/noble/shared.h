/****************************************************************

 shared.h

 =============================================================

 Copyright 1996-2015 Tom Barbalet. All rights reserved.

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

#ifndef NOBLEAPE_SHARED_H
#define NOBLEAPE_SHARED_H

#ifndef	_WIN32

#include "../noble/noble.h"

#else

#include "..\noble\noble.h"

#endif

typedef enum
{
    SHARED_CYCLE_OK = 0,
    SHARED_CYCLE_QUIT,
    SHARED_CYCLE_DEBUG_OUTPUT
} shared_cycle_state;


typedef enum
{
    SSS_OPEN = 0,
    SSS_OPEN_SCRIPT,
    SSS_SAVED
} shared_saved_string_type;

void shared_saved_string(shared_saved_string_type ssst, n_string value);

shared_cycle_state shared_cycle(n_uint ticks, n_byte fIdentification, n_int dim_x, n_int dim_y);

n_int shared_init(n_byte view, n_uint random);

void shared_close(void);

n_int shared_menu(n_int menuValue);

n_uint shared_max_fps(void);

void shared_rotate(n_double num, n_byte wwind);

void shared_keyReceived(n_byte2 value, n_byte fIdentification);
void shared_keyUp(void);

void shared_mouseOption(n_byte option);
void shared_mouseReceived(n_int valX, n_int valY, n_byte fIdentification);
void shared_mouseUp(void);

void shared_about(n_constant_string value);

void shared_draw(n_byte * outputBuffer, n_byte fIdentification, n_int dim_x, n_int dim_y);

n_int shared_new(n_uint seed);

n_byte shared_openFileName(n_string cStringFileName,n_byte isScript);

void shared_saveFileName(n_string cStringFileName);

void shared_script_debug_handle(n_string cStringFileName);

#ifndef	_WIN32

n_int sim_thread_console_quit(void);

#endif

#endif /* NOBLEAPE_SHARED_H */
