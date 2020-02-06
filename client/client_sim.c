/****************************************************************

 client_sim.c

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

#define CONSOLE_REQUIRED
#define CONSOLE_ONLY

#include "client.h"

n_int command_save(void * ptr, n_string response, n_console_output output_function)
{
    return 0;
}

n_int command_open(void * ptr, n_string response, n_console_output output_function)
{
    return 0;
}

n_int command_script(void * ptr, n_string response, n_console_output output_function)
{
    return 0;
}

#ifdef BRAINCODE_ON
void command_populate_braincode(noble_simulation * local_sim, line_braincode function)
{

}
#endif

n_int command_stop(void * ptr, n_string response, n_console_output output_function)
{
    return 0;
}

void sim_change_selected(n_byte forwards)
{
    
}

void      sim_close(void)
{
    
}

void *    sim_init(KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size)
{
    return 0L;
}

void      sim_cycle(void)
{
    
}

void sim_flood(void)
{
    
}

void sim_healthy_carrier(void)
{
    
}

void sim_realtime(n_uint time)
{
    
}

n_int sim_new(void)
{
    return 0;
}

n_int sim_new_run_condition(void)
{
    return 0;
}

void sim_view_options(n_int px, n_int py)
{
    
}

void sim_rotate(n_int integer_rotation_256)
{
    
}

void sim_move(n_int rel_vel, n_byte kind)
{
    
}

n_int sim_view_regular(n_int px, n_int py)
{
    return 0;
}

void sim_terrain(n_int sx)
{
    
}

noble_simulation * sim_sim(void)
{
    return 0L;
}

#ifndef    _WIN32
n_int sim_thread_console_quit(void)
{
    return 0;
}

void  sim_thread_console(void)
{
    
}
#endif
