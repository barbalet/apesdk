/****************************************************************

 sim.c

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

/*NOBLEMAKE DEL=""*/

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
#include <unistd.h>

#endif

/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

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
    "location_z",

    "test_z",
    "is_visible",

    "time",
    "date",

    "current_being",
    "number_beings",

    "location_x",
    "location_y",

    "state",  /* new ! */

    /* 18 */
    "id_number",
    "date_of_birth",

    "weather",
    "brain_value",		/* special input/output */
    /* actual variables start here */

    "vector_angle",
    "facing",

    "speed",
    "energy",

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
\
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
    "social_graph_century",
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
    "memory_century",
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
#define MIN_BEINGS		40

static noble_simulation	sim;

static n_interpret *interpret = 0L;

/*NOBLEMAKE END=""*/

static n_int            sim_new_progress = 0;

n_int sim_new(void)
{
    return sim_new_progress;
}


#ifdef THREADED

static void sim_indicators(noble_simulation * sim);
static void sim_brain(noble_simulation * local_sim);
static void sim_brain_dialogue(noble_simulation * local_sim);
static void sim_being(noble_simulation * local_sim);
static void sim_time(noble_simulation * local_sim);

static n_int            thread_on = 0;

static pthread_t        land_thread;
static pthread_t        brain_thread;
static pthread_t        being_thread;

static n_int            sim_done = 3;

static pthread_cond_t   sim_cond;
static pthread_mutex_t  quit_mtx;
static n_int            thread_quit;

static pthread_mutex_t  draw_mtx;
static pthread_cond_t   draw_cond;

static n_int            sim_draw_thread = 0;

void sim_draw_thread_on(void)
{
    sim_draw_thread = 1;
}

void sim_draw_thread_start(void)
{
    pthread_mutex_lock(&draw_mtx);
    while(sim_done < 3)pthread_cond_wait(&draw_cond,&draw_mtx);
    pthread_cond_broadcast(&sim_cond);
    pthread_mutex_unlock(&draw_mtx);
}

void sim_draw_thread_end(void)
{
    n_int local_quit;
    pthread_mutex_lock(&quit_mtx);
    local_quit = thread_quit;
    pthread_mutex_unlock(&quit_mtx);
    if(local_quit==1) pthread_exit(NULL);
}

static void * sim_thread_land(void * id)
{
    n_int local_quit;
    do
    {
        pthread_mutex_lock(&draw_mtx);
        if(thread_on ==1 ) pthread_cond_wait(&sim_cond, &draw_mtx);
        sim_done--;
        pthread_mutex_unlock(&draw_mtx);

        land_cycle(sim.land);
#ifdef WEATHER_ON
        weather_cycle(sim.land, sim.weather);
#endif
        pthread_mutex_lock(&draw_mtx);
        sim_done++;
        pthread_mutex_unlock(&draw_mtx);

        pthread_mutex_lock(&quit_mtx);
        local_quit = thread_quit;
        pthread_mutex_unlock(&quit_mtx);
    }
    while (local_quit == 0);
    pthread_exit(NULL);
}

static void * sim_thread_being(void * id)
{
    n_int local_quit;
    do
    {
        pthread_mutex_lock(&draw_mtx);
        if(thread_on ==1 ) pthread_cond_wait(&sim_cond, &draw_mtx);
        sim_done--;
        pthread_mutex_unlock(&draw_mtx);

        sim_being(&sim);    /* 2 */
        being_tidy(&sim);
        being_remove(&sim); /* 6 */
        sim_social(&sim);
        sim_indicators(&sim);
        sim_time(&sim);

        pthread_mutex_lock(&draw_mtx);
        sim_done++;
        pthread_mutex_unlock(&draw_mtx);

        pthread_mutex_lock(&quit_mtx);
        local_quit = thread_quit;
        pthread_mutex_unlock(&quit_mtx);
    }
    while (local_quit == 0);
    pthread_exit(NULL);
}

static void * sim_thread_brain(void * id)
{
    n_int local_quit;
    do
    {
        pthread_mutex_lock(&draw_mtx);
        if(thread_on ==1 ) pthread_cond_wait(&sim_cond, &draw_mtx);
        sim_done--;
        pthread_mutex_unlock(&draw_mtx);

        sim_brain(&sim);    /* 4 */

#ifdef BRAINCODE_ON
        sim_brain_dialogue(&sim);
#endif
        pthread_mutex_lock(&draw_mtx);
        sim_done++;

        if(sim_done == 3)
        {
            pthread_cond_signal(&draw_cond);
        }
        pthread_mutex_unlock(&draw_mtx);

        pthread_mutex_lock(&quit_mtx);
        local_quit = thread_quit;
        pthread_mutex_unlock(&quit_mtx);
    }
    while (local_quit == 0);

    pthread_exit(NULL);
}
static void sim_thread_init(void)
{
    pthread_mutex_init(&quit_mtx, NULL);
    pthread_mutex_init(&draw_mtx, NULL);

    pthread_cond_init(&draw_cond, NULL);
    pthread_cond_init(&sim_cond, NULL);
    pthread_create(&land_thread, NULL, sim_thread_land, NULL);
    pthread_create(&being_thread, NULL, sim_thread_being, NULL);
    pthread_create(&brain_thread, NULL, sim_thread_brain, NULL);
}

static void sim_thread_close(void)
{
    pthread_mutex_lock(&quit_mtx);
    thread_quit = 1;
    pthread_mutex_unlock(&quit_mtx);

}
#endif

#ifndef	_WIN32

#include <pthread.h>

static n_int      sim_quit_value = 0;
static pthread_t  threads[2] = {0};
static n_byte     threads_running[2] = {0};

n_int sim_thread_console_quit(void)
{
    return sim_quit_value;
}

static void sim_console_clean_up(void)
{
    n_int loop = 0;

    if ((io_command_line_execution() == 0) || sim_quit_value)
    {
        return;
    }

    sim_quit_value = 1;

    console_quit(0L,0L,0L);

    while (console_executing()) {}

    while (loop < 2)
    {
        if (threads_running[loop] != 0)
        {
            pthread_cancel(threads[loop]);
        }
        loop++;
    }
}

static void *sim_thread(void *threadid)
{
    n_byte *local = (n_byte *)threadid;
    if (io_console(&sim, (noble_console_command *) control_commands, io_console_entry_clean, io_console_out) != 0)
    {
        sim_console_clean_up();
    }
    local[0] = 0;
    pthread_exit(NULL);
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
                pthread_create(&threads[loop], 0L, sim_thread, &threads_running[loop]);
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
        SC_DEBUG_ON; /* turn on debugging after script loading */
    }

    interpret->sc_input  = &sketch_input;
    interpret->sc_output = &sketch_output;

    interpret->input_greater   = VARIABLE_WEATHER;
    interpret->special_less    = VARIABLE_VECT_X;

    interpret->location = 0;
    interpret->leave = 0;
    interpret->localized_leave = 0;

    return 0;
}

static void sim_brain_no_return(noble_simulation * local_sim, noble_being * local_being)
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

static void sim_brain(noble_simulation * local_sim)
{
    being_loop_no_return(local_sim, sim_brain_no_return);
}

#ifdef BRAINCODE_ON

static void sim_brain_dialogue_no_return(noble_simulation * local_sim, noble_being * local_being)
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

static void sim_brain_dialogue(noble_simulation * local_sim)
{
    being_loop_no_return(local_sim, sim_brain_dialogue_no_return);
}
#endif

static void sim_being(noble_simulation * local_sim)
{
    n_uint loop = 0;

    local_sim->someone_speaking = 0;

    while (loop < local_sim->num)
    {
        noble_being * local_being = &(local_sim->beings[loop]);

        n_byte awake = (being_awake(local_sim, local_being) != 0);

        being_cycle_universal(local_sim,local_being, awake);

        if (awake)
        {
            if(interpret_cycle(interpret, -1, local_sim->beings, loop, &sim_start_conditions, &sim_end_conditions) == -1)
            {
                interpret_cleanup(&interpret);
            }
            if(interpret == 0L)
            {
                being_cycle_awake(local_sim, loop);
            }
        }

        loop++;
    }
}

static void sim_time(noble_simulation * local_sim)
{
    local_sim->count_cycles += local_sim->num;

    if ((local_sim->real_time - local_sim->last_time) > 600)
    {
        local_sim->last_time = local_sim->real_time;
        local_sim->delta_cycles = local_sim->count_cycles;
        local_sim->count_cycles = 0;
    }
}

/* this is a protoype for the order of these functions it is not used here explicitly */

void sim_cycle(void)
{
#ifndef THREADED

    land_cycle(sim.land);
    sim_being(&sim);    /* 2 */
#ifdef WEATHER_ON
    weather_cycle(sim.land);
#endif
    sim_brain(&sim);    /* 4 */

#ifdef BRAINCODE_ON
    sim_brain_dialogue(&sim);
#endif
    
    being_tidy(&sim);
    being_remove(&sim); /* 6 */
    sim_social(&sim);
    /*sim_indicators(&sim);*/
    sim_time(&sim);

#endif
}


#define MAXIMUM_ALLOCATION  ( 60 * 1024 * 1024 )


#define	MINIMAL_ALLOCATION	(sizeof(n_land)+(MAP_AREA)+(2*HI_RES_MAP_AREA)+(HI_RES_MAP_AREA/8)+(512*512)+(TERRAIN_WINDOW_AREA)+((sizeof(noble_being) + DOUBLE_BRAIN) * MIN_BEINGS)+1+(sizeof(n_uint)*2))


static void sim_memory_land(noble_simulation * local, n_byte * buffer, n_uint * location)
{
    local->land = (n_land *) & buffer[ *location ];
    *location += sizeof(n_land);
}


static void sim_memory(n_uint offscreen_size)
{
    n_uint	current_location = 0;
    n_uint  memory_allocated = MAXIMUM_ALLOCATION;

    offbuffer = io_new_range(offscreen_size + MINIMAL_ALLOCATION, &memory_allocated);

    current_location = offscreen_size;

    sim_memory_land(&sim, offbuffer, &current_location);
    
    memory_allocated = memory_allocated - offscreen_size - current_location;
    
    being_memory(&sim, offbuffer, &current_location, memory_allocated);
}

void sim_tide_block(n_byte * small_map, n_byte * map, n_c_uint * tide_block)
{
    n_uint  lp = 0;
    math_bilinear_512_4096(small_map, map);

    while (lp < (HI_RES_MAP_AREA/32))
    {
        tide_block[lp++] = 0;
    }
    lp = 0;
    while (lp < HI_RES_MAP_AREA)
    {
        n_byte val = map[lp<<1];
        if ((val > 105) && (val < 151))
        {
            tide_block[lp>>5] |= 1 << (lp & 31);
        }
        lp++;
    }
}

void * sim_init(KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size)
{
    n_byte2	local_random[2];
    
    sim_new_progress = 1;
    
    if ((kind == KIND_NEW_SIMULATION) && (interpret))
    {
        interpret_cleanup(&interpret);
        interpret = 0L;
    }
    sim.delta_cycles = 0;
    sim.count_cycles = 0;
    sim.real_time = randomise;
    sim.last_time = randomise;

    sim.ext_birth = 0L;
    sim.ext_death = 0L; /*&console_capture_death; */
#ifdef FIXED_RANDOM_SIM
    randomise = FIXED_RANDOM_SIM;
#endif
    if ((kind == KIND_START_UP) || (kind == KIND_MEMORY_SETUP))
    {
        sim_memory(offscreen_size);
    }
    if ((kind != KIND_LOAD_FILE) && (kind != KIND_MEMORY_SETUP))
    {
        local_random[0] = (n_byte2)(randomise >> 16) & 0xffff;
        local_random[1] = (n_byte2)(randomise & 0xffff);

        sim.land->genetics[0] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
        sim.land->genetics[1] = (n_byte2)(((math_random(local_random) & 255) << 8) | (math_random(local_random) & 255));
    }

    if (kind != KIND_MEMORY_SETUP)
    {
        land_clear(sim.land, kind, AGE_OF_MATURITY);
#ifdef LAND_ON
        land_init(sim.land , &offbuffer[landbuffer_size]);
        sim_tide_block(sim.land->map, sim.land->highres, sim.land->highres_tide);
#endif
        if (kind != KIND_LOAD_FILE)
        {
            n_uint count_to = sim.max >> 2;
#ifdef WEATHER_ON
            weather_init(sim.land);
#endif       
            sim.num = 0;
            while (sim.num < count_to)
            {
                math_random3(local_random);
                if((sim.num + 1) < sim.max)
                {
                    if (being_init(sim.land, sim.beings, sim.num, &sim.beings[sim.num], 0L, local_random) != 0)
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

#ifdef THREADED
    if (thread_on == 0)
    {
        thread_on = 1;
        sim_thread_init();
    }
#endif

    sim_set_select(0);

    sim_new_progress = 0;
    
    return ((void *) offbuffer);
}

void sim_close(void)
{
    io_console_quit();
#ifndef _WIN32
    sim_console_clean_up();
#endif
#ifdef THREADED
    sim_thread_close();
#endif
    interpret_cleanup(&interpret);
    io_free((void **) &offbuffer);
    /*death_record_file_cleanup();*/
}

void sim_set_select(n_uint number)
{
    sim.select = number;
    console_external_watch();
}

void sim_populations(n_uint	*total, n_uint * female, n_uint * male)
{
    n_uint  loop = 0;
    n_uint  local_female = 0;
    n_uint  local_male = 0;
    while (loop < sim.num)
    {
        noble_being * local = &sim.beings[loop];
        if (FIND_SEX(GET_I(local))==SEX_FEMALE)
        {
            local_female++;
        }
        else
        {
            local_male++;
        }
        loop++;
    }
    *total = sim.num;
    *female = local_female;
    *male = local_male;
}

void sim_flood(void)
{
    n_uint  loop = 0;

    while (loop < sim.num)
    {
        noble_being * local = &sim.beings[loop];
        n_int         local_x = APESPACE_TO_MAPSPACE(being_location_x(local));
        n_int         local_y = APESPACE_TO_MAPSPACE(being_location_y(local));
        n_int         local_z = QUICK_LAND(sim.land, local_x, local_y);

        if (local_z < 160)
        {
            being_set_energy(local, BEING_DEAD);
        }

        loop++;
    }
}

void sim_healthy_carrier(void)
{
    n_uint  loop = (sim.num >> 2);

    while (loop < sim.num)
    {
        noble_being * local = &sim.beings[loop];
        being_set_energy(local, BEING_DEAD);
        loop++;
    }
}
