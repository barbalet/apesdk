/****************************************************************

 client.h

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

#ifndef NOBLEAPE_CLIENT_H
#define NOBLEAPE_CLIENT_H

#include "../noble/noble.h"

#define CHROMOSOMES                 4

void transfer_parameters(void);
void transfer_sim_start(void);

n_file * transfer_out_json(void);

#ifdef NOBLE_APE_CLIENT

typedef n_uint n_genetics;

typedef struct {
    n_int signature, version_number;
    n_string copyright, date;
} noble_simulation_identifier;

typedef struct {
    n_vect2 location;
    n_int direction_facing, velocity, stored_energy, identification, state;
    n_int genetics[4];
    n_string name;
} noble_being;

typedef struct {
    n_vect2 genetics;
    n_int date, time;
} noble_land;

typedef struct {
    noble_land land;
    noble_simulation_identifier id;
    noble_being * beings;
    noble_being * select;
    n_int  count;
    n_int  max;
    n_uint being_selected_id;
    n_uint real_time;
} noble_simulation;



#ifdef BRAIN_ON
n_byte * being_brain(noble_being * value);
#endif

n_int being_energy(noble_being * value);

n_byte being_facing(noble_being * value);

n_genetics * being_genetics(noble_being * value);

void being_high_res(noble_being * value, n_vect2 * vector);

n_byte being_line_of_sight(noble_being * local, n_vect2 * location);

n_int being_location_x(noble_being * value);

n_int being_location_y(noble_being * value);

void being_space(noble_being * value, n_vect2 * vector);

n_byte being_speed(noble_being * value);

n_byte2 being_state(noble_being * value);

n_int being_female(noble_being * value);
n_int being_speaking(noble_being * value);

n_int command_save(void * ptr, n_string response, n_console_output output_function);
n_int command_open(void * ptr, n_string response, n_console_output output_function);
n_int command_script(void * ptr, n_string response, n_console_output output_function);

#ifdef BRAINCODE_ON
void command_populate_braincode(noble_simulation * local_sim, line_braincode function);
#endif

n_int command_stop(void * ptr, n_string response, n_console_output output_function);

typedef void (loop_fn)(noble_simulation * sim, noble_being * actual, void * data);

void loop_no_thread(noble_simulation * sim, noble_being * being_not, loop_fn bf_func, void * data);

void      sim_close(void);

void *    sim_init(KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size);
void      sim_cycle(void);

void sim_flood(void);

void sim_healthy_carrier(void);

void sim_realtime(n_uint time);

n_int sim_new(void);

n_int sim_new_run_condition(void);

void sim_view_options(n_int px, n_int py);
void sim_rotate(n_int integer_rotation_256);
void sim_move(n_int rel_vel, n_byte kind);
void sim_change_selected(n_byte forwards);

n_int sim_view_regular(n_int px, n_int py);

void sim_terrain(n_int sx);

noble_simulation * sim_sim(void);

#ifndef    _WIN32
n_int sim_thread_console_quit(void);
void  sim_thread_console(void);
#endif

#endif


#endif /* NOBLEAPE_CLIENT_H */
