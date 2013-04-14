/****************************************************************

 shared.h

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

/*NOBLEMAKE VAR=""*/

#ifndef NOBLEAPE_SHARED_H
#define NOBLEAPE_SHARED_H

void shared_cycle(n_uint ticks, n_int fIdentification);

void shared_cycle_no_draw(n_uint ticks, n_int fIdentification);

void shared_cycle_really_no_draw(n_uint ticks, n_int fIdentification);

void shared_cycle_really_draw(n_int fIdentification, n_int dim_x, n_int dim_y);


n_int shared_init(n_byte view, n_uint random);

void shared_close(void);

void shared_notPause(void);

void shared_notWeather(void);
void shared_notBrain(void);
void shared_notBrainCode(void);
void shared_notTerritory(void);

void shared_flood(void);
void shared_healthy_carrier(void);

void shared_keyReceived(n_byte2 value, n_byte fIdentification);

void shared_keyUp(void);

void shared_mouseOption(n_byte option);

void shared_mouseReceived(n_int valX, n_int valY, n_byte fIdentification);

void shared_mouseUp(void);

void shared_about(n_string value);

n_byte * shared_draw(n_byte fIdentification);

void shared_timeForColor(n_byte2 * fit, n_int fIdentification);

void shared_previousApe(void);

void shared_nextApe(void);

void shared_clearErrors(void);

void shared_new(n_uint seed);

void shared_brainDisplay(n_byte value);

n_byte shared_openFileName(n_string cStringFileName,n_byte isScript);

void shared_saveFileName(n_string cStringFileName);

n_int shared_script_debug_ready(void);

void shared_script_debug_handle(n_string cStringFileName);

void shared_graph_command(n_int gc_used);

#ifndef	_WIN32

n_int sim_thread_console_quit(void);

#endif

#endif /* NOBLEAPE_SHARED_H */

/*NOBLEMAKE END=""*/


