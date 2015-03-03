/****************************************************************

 sim.c

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

#define CONSOLE_REQUIRED
#define CONSOLE_ONLY

#include <stdio.h>

#ifndef	_WIN32
#include "../entity/entity.h"
#else
#include "..\entity\entity.h"
#endif

#include "universe_internal.h"

#ifndef	_WIN32

#include <pthread.h>

#endif

static variable_string	apescript_variable_codes[VARIABLE_MAX]=
{
    /* special "variables" */
    "function",
    "run",

    "while",
    "if",

    /* output only */
    "vector_x",
    "vector_y",

    "random",
    "water_level",

    "biology_area",
    "biology_height",

    "biology_water",
    "biology_moving_sun",

    "biology_total_sun",
    "biology_salt",
    "biology_bush",

    "biology_grass",
    "biology_tree",

    "biology_seaweed",
    "biology_rockpool",
    "biology_beach",

    "biology_insect",
    "biology_mouse",

    "biology_parrot",
    "biology_lizard",

    "biology_eagle",
    "biology_output",

    "hungry",
    "energy",
    "location_z",

    "test_z",
    "is_visible",

    "time",
    "date",

    "current_being",
    "number_beings",

    "location_x",
    "location_y",

    "state",

    "id_number",
    "date_of_birth",

    "is_error",    
    "weather",
    
    "brain_value",		/* special input/output */
    /* actual variables start here */

    "vector_angle",
    "facing",

    "speed",
    "energy_delta",

    "honor",
    "parasites",
    "height",

    "first_name",
    "family_name_one",
    "family_name_two",

    "goal_type",
    "goal_x",
    "goal_y",

    "drive_hunger",
    "drive_social",
    "drive_fatigue",
    "drive_sex",

    "brain_x",
    "brain_y",

    "brain_z",
    "select_being",

    "test_x",
    "test_y",

    "biology_operator",

    "posture",

    "preference_mate_height_male",
    "preference_mate_height_female",
    "preference_mate_pigment_male",
    "preference_mate_pigment_female",
    "preference_mate_hair_male",
    "preference_mate_hair_female",
    "preference_mate_frame_male",
    "preference_mate_frame_female",
    "preference_groom_male",
    "preference_groom_female",
    "preference_anecdote_event",
    "preference_anecdote_affect",
    "preference_chat",

    "attention_actor_index",
    "attention_episode_index",
    "attention_body_index",

    "shout_content",
    "shout_heard",
    "shout_counter",
    "shout_volume",
    "shout_family_first",
    "shout_family_last",

    "social_graph_location_x",
    "social_graph_location_y",
    "social_graph_time",
    "social_graph_date",
    "social_graph_attraction",
    "social_graph_fof",
    "social_graph_familiarity",
    "social_graph_first_name",
    "social_graph_family_first",
    "social_graph_family_last",

    "memory_location_x",
    "memory_location_y",
    "memory_time",
    "memory_date",
    "memory_first_name_zero",
    "memory_family_first_zero",
    "memory_family_last_zero",
    "memory_first_name_one",
    "memory_family_first_one",
    "memory_family_last_one",
    "memory_event",
    "memory_affect",
            
    "being"
};

n_byte	* offbuffer = 0L;

/* Twice the minimum number of apes the Simulation will allow to run */
#define MIN_BEINGS		4

static noble_simulation	sim;

static n_interpret *interpret = 0L;

static n_int        sim_new_progress = 0;

n_int sim_new(void)
{
    return sim_new_progress;
}

#ifndef	_WIN32

static n_int      sim_quit_value = 0;
static pthread_t  threads[2] = {0};
static n_byte     threads_running[2] = {0};

n_int sim_thread_console_quit(void)
{
    return sim_quit_value;
}

static void sim_console_clean_up(void)
{
    if ((io_command_line_execution() == 0) || sim_quit_value)
    {
        return;
    }

    sim_quit_value = 1;

    console_quit(0L,0L,0L);

    while (console_executing()) {}
}



static void *sim_thread_posix(void *threadid)
{
    n_byte *local = (n_byte *)threadid;
    if (io_console(&sim, (noble_console_command *) control_commands, io_console_entry_clean, io_console_out) != 0)
    {
        sim_console_clean_up();
    }
    local[0] = 0;
    pthread_exit(0L);
}

void sim_thread_console(void)
{
    if (io_command_line_execution() == 0)
    {
        return;
    }

    if ((threads_running[0] == 0)||(threads_running[1] == 0))
    {
        n_int loop = 0;
        while (loop < 2)
        {
            if (threads_running[loop] == 0)
            {
                threads_running[loop] = 1;
                pthread_create(&threads[loop], 0L, sim_thread_posix, &threads_running[loop]);
                return;
            }
            loop++;
        }
    }
}

#endif

noble_simulation * sim_sim(void)
{
    return &sim;
}

void sim_realtime(n_uint time)
{
    sim.real_time = time;
}

n_int     file_interpret(n_file * input_file)
{
    input_file->size = input_file->location;
    input_file->location = 0;

    interpret = parse_convert(input_file, VARIABLE_BEING, (variable_string *)apescript_variable_codes);

    if(interpret == 0L)
    {
        return -1;
    }
    else
    {
        SC_DEBUG_ON(sim.select); /* turn on debugging after script loading */
    }

    interpret->sc_input  = &sketch_input;
    interpret->sc_output = &sketch_output;

    interpret->input_greater   = VARIABLE_WEATHER;
    interpret->special_less    = VARIABLE_VECT_X;
    return 0;
}

#ifdef BRAIN_ON
static void sim_brain_loop(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    n_byte2 local_brain_state[3];

    if(being_brainstates(local_being, (being_awake(&sim, local_being) == 0), local_brain_state))
    {
        n_byte			*local_brain = being_brain(local_being);
        if (local_brain != 0L)
        {
            brain_cycle(local_brain, local_brain_state);
        }
    }
}
#endif


#ifdef BRAINCODE_ON

static void sim_brain_dialogue_loop(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    n_byte     awake = 1;
    n_byte    *local_internal = being_braincode_internal(local_being);
    n_byte    *local_external = being_braincode_external(local_being);
    if(being_awake(&sim, local_being) == 0)
    {
        awake=0;
    }
    /* This should be independent of the brainstate/cognitive simulation code */
    brain_dialogue(local_sim, awake, local_being, local_being, local_internal, local_external, being_random(local_being)%SOCIAL_SIZE);
    brain_dialogue(local_sim, awake, local_being, local_being, local_external, local_internal, being_random(local_being)%SOCIAL_SIZE);
}

#endif


static void sim_being_universal_loop(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    being_cycle_universal(local_sim,local_being, (being_awake(local_sim, local_being) != 0));
}

static void sim_being_cycle(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    if (being_awake(local_sim, local_being) == 0) return;
    
    being_cycle_awake(local_sim, local_being);
}

static void sim_being_interpret(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    n_individual_interpret individual;
    
    interpret_individual(&individual);
    
    if (being_awake(local_sim, local_being) == 0) return;

    if (interpret == 0L) return;
    
    if(interpret_cycle(interpret, &individual, -1,
                       local_sim->beings, local_being,
                       &sim_start_conditions, &sim_end_conditions) == -1)
    {
        interpret_cleanup(&interpret);
    }
}


static void sim_time(noble_simulation * local_sim)
{
    local_sim->count_cycles += local_sim->num;
    
    local_sim->count_frames ++;
    
    if ((local_sim->real_time - local_sim->last_time) > 60)
    {
        local_sim->last_time = local_sim->real_time;
        local_sim->delta_cycles = local_sim->count_cycles;
        local_sim->delta_frames = local_sim->count_frames;
        local_sim->count_cycles = 0;
        local_sim->count_frames = 0;
    }
}

void sim_cycle(void)
{
    n_int       max_honor = 0;

    land_cycle();
#ifdef WEATHER_ON
    weather_cycle();
#endif
    
    being_loop(&sim, sim_being_universal_loop, 32);

    if (interpret)
    {        
        being_loop(&sim, sim_being_interpret, 16);
    }
    else
    {
        /** Listen for any shouts */
        being_loop(&sim, being_listen, 64);
#ifdef EPISODIC_ON
        being_loop(&sim, episodic_cycle, 64);
#endif
        being_loop(&sim, sim_being_cycle, 8);
        being_loop(&sim, drives_cycle, 32);
    }
    
    if (land_time() & 1)
    {
#ifdef BRAIN_ON
        being_loop(&sim, sim_brain_loop, 16);
#endif
    }
#ifdef BRAINCODE_ON
    else
    {
        being_loop(&sim, sim_brain_dialogue_loop, 8);
    }
#endif
        
    being_loop_no_thread(&sim, 0L, being_tidy_loop, &max_honor);
    
    being_loop(&sim, social_initial_loop, 32);
    
    if (max_honor)
    {
        being_loop(&sim, being_recalibrate_honor_loop, 64);
    }
    
    being_loop(&sim, social_secondary_loop, 64);

    {
        being_remove_loop2_struct * brls = being_remove_initial(&sim);        
        if (sim.ext_death != 0L)
        {
            being_loop_no_thread(&sim, 0L, being_remove_loop1, 0L);
        }
        being_loop_no_thread(&sim, 0L, being_remove_loop2, brls);
        being_remove_final(&sim, &brls);
    }
    
    sim_time(&sim);
}

#define	MINIMAL_ALLOCATION	((512*512)+(TERRAIN_WINDOW_AREA)+(sizeof(noble_being) * MIN_BEINGS)+1+(sizeof(noble_simulation)))

#define MAXIMUM_ALLOCATION  (MINIMAL_ALLOCATION + (sizeof(noble_being) * 200))

static void sim_memory_remains(noble_simulation * local, n_byte * buffer, n_uint * location)
{
    local->remains = (noble_remains *) & buffer[ *location ];
    *location += sizeof(noble_remains);
}

static n_int sim_memory(n_uint offscreen_size)
{
    n_uint	current_location = 0;
    n_uint  memory_allocated = MAXIMUM_ALLOCATION;
    
    if (memory_allocated < MINIMAL_ALLOCATION)
    {
        return SHOW_ERROR("Not enough memory to run");
    }
    
    offbuffer = io_new_range(offscreen_size + MINIMAL_ALLOCATION, &memory_allocated);

    current_location = offscreen_size;
    
    sim_memory_remains(&sim, offbuffer, &current_location);
    
    memory_allocated = memory_allocated - offscreen_size - current_location;
    
    return being_memory(&sim, offbuffer, &current_location, memory_allocated);
}

void * sim_init(KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size)
{
    n_byte2	local_random[2];
    
    sim_new_progress = 1;
    
    if (kind == KIND_NEW_SIMULATION)
    {
        if(interpret)
        {
            interpret_cleanup(&interpret);
            interpret = 0L;
        }
    }
    sim.delta_cycles = 0;
    sim.count_cycles = 0;
    sim.delta_frames = 0;
    sim.count_frames = 0;
    sim.real_time = randomise;
    sim.last_time = randomise;

    sim.ext_birth = 0L;
    sim.ext_death = 0L; /*&console_capture_death; */
#ifdef FIXED_RANDOM_SIM
    randomise = FIXED_RANDOM_SIM;
#endif
    if ((kind == KIND_START_UP) || (kind == KIND_MEMORY_SETUP))
    {
        if (sim_memory(offscreen_size) != 0)
        {
            return 0L;
        }
        execute_init();
    }
    if ((kind != KIND_LOAD_FILE) && (kind != KIND_MEMORY_SETUP))
    {
        n_byte2 local_genetics[2];
        local_random[0] = (n_byte2)(randomise >> 16) & 0xffff;
        local_random[1] = (n_byte2)(randomise & 0xffff);

        local_genetics[0] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
        local_genetics[1] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
        land_set_genetics(local_genetics);

    }

    being_remains_init(&sim); /* Eventually this should be captured through the file handling and moved into the code below */
    
    if (kind != KIND_MEMORY_SETUP)
    {
        land_clear(kind, AGE_OF_MATURITY);
#ifdef LAND_ON
#ifdef EXECUTE_THREADED
        land_init(&offbuffer[landbuffer_size], execute_add);
#else
        land_init(&offbuffer[landbuffer_size], 0L);
#endif
        land_init_high_def(1);
        land_tide();
#endif
        if (kind != KIND_LOAD_FILE)
        {
#if (MAPBITS == 8)
            n_uint count_to = sim.max >> 2;
#else
            n_uint count_to = sim.max >> 4;
#endif
#ifdef WEATHER_ON
            weather_init();
#endif       
            sim.num = 0;
            while (sim.num < count_to)
            {
                math_random3(local_random);
                if((sim.num + 1) < sim.max)
                {
                    if (being_init(sim.beings, sim.num, &sim.beings[sim.num], 0L, local_random) != 0)
                    {                        
                        being_erase(&sim.beings[sim.num]);
                        break;
                    }
                    else
                    {
                        sim.num++;
                    }
                }
            }
        }
    }

    sim_set_select(sim.beings);

    sim_new_progress = 0;
    
    return ((void *) offbuffer);
}

void sim_close(void)
{
    console_quit(0L, 0L, 0L);
    io_console_quit();
    execute_close();
#ifndef _WIN32
    sim_console_clean_up();
#endif
    interpret_cleanup(&interpret);
    io_free((void **) &offbuffer);
    /*death_record_file_cleanup();*/
}

void sim_set_select(noble_being * select)
{
    sim.select = select;
    console_external_watch();
}

static void sim_flood_loop(noble_simulation * sim, noble_being * local, void * data)
{
    n_vect2 location;
    
    being_space(local, &location);
    land_convert_to_map(&location);

    if (land_location_vect(&location) < 160)
    {
        being_dead(local);
    }
}

void sim_flood(void)
{
    being_loop_no_thread(&sim, 0L, sim_flood_loop, 0L);
}

void sim_healthy_carrier(void)
{
    n_uint  loop = (sim.num >> 2);

    while (loop < sim.num)
    {
        noble_being * local = &sim.beings[loop];
        being_dead(local);
        loop++;
    }
}
