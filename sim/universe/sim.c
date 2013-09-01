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
    /* 0 */ /* special "variables" */
    "function",
    "run",

    /* 2 */
    "while",
    "if",

    /* output only */
    "vector_x",
    "vector_y",
    /* 6 */
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
    /* 10 */
    "test_z",
    "is_visible",

    "time",
    "date",
    /* 14 */
    "current_being",
    "number_beings",

    "location_x",
    "location_y",

    "state",  /* new ! */
    "listen", /* new ! */

    /* 18 */
    "id_number",
    "date_of_birth",

    "weather",
    "brain_value",		/* special input/output */
    /* actual variables start here */
    /* 22 */
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

    "speak", /* new ! */

    /* 26 */
    "brain_x",
    "brain_y",

    "brain_z",
    "select_being",
    /* 30 */
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

#ifdef BRAIN_HASH

n_byte	brain_hash_out[12] = {0};
n_uint	brain_hash_count;

#endif

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

#ifdef BRAIN_HASH
        sim_brain_hash(&sim)
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
    if (io_console(sim_sim(), (noble_console_command *) control_commands, io_console_entry_clean, io_console_out) != 0)
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

n_int	sim_interpret(n_byte * buff, n_uint len)
{
    n_file	local;

    local . size = len;
    local . location = 0;
    local . data = buff;

    interpret = parse_convert(&local, VARIABLE_BEING, (variable_string *)apescript_variable_codes);

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
    brain_dialogue(local_sim, awake, local_being, local_being, local_internal, local_external, math_random(local_being->seed)%SOCIAL_SIZE);
    brain_dialogue(local_sim, awake, local_being, local_being, local_external, local_internal, math_random(local_being->seed)%SOCIAL_SIZE);
}

static void sim_brain_dialogue(noble_simulation * local_sim)
{
    being_loop_no_return(local_sim, sim_brain_dialogue_no_return);
}
#endif


#ifdef BRAIN_HASH
static void sim_brain_hash(noble_simulation * local_sim)
{
    brain_hash_count++;
    if((brain_hash_count & 63) == 0)
    {
        n_byte * hash_brain = being_brain(&(sim.beings[sim.select]));

        brain_hash_count = 0;

        if ((sim.select != NO_BEINGS_FOUND) && (hash_brain != 0L))
        {
            brain_hash(hash_brain, brain_hash_out);
        }
    }
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
                interpret_cleanup(interpret);
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

static void sim_indicators(noble_simulation * sim)
{
    n_uint b;
    n_int i,n,diff;
    n_uint current_date;
    n_uint average_cohesion=0;
    n_uint average_amorousness=0;
    n_uint average_familiarity=0;
    n_uint average_energy=0;
    n_uint social_links=0;
    n_byte x,y;
    social_link * local_social_graph;
    n_uint mean_genome[8*CHROMOSOMES];
    noble_being * local_being;
    n_uint local_dob;
    FILE * fp;
    n_string_block filename;
    n_uint drives[DRIVES];
    n_uint family[2],sd;
    n_uint positive_affect=0,negative_affect=0;
    n_uint average_first_person=0;
    n_uint average_intentions=0;
#ifdef IMMUNE_ON
    n_uint average_antigens=0,average_antibodies=0;
    noble_immune_system * immune;
#endif
#ifdef BRAINCODE_ON
    n_uint mean_braincode[BRAINCODE_SIZE*2];
#endif

    if (sim->land->time%INDICATORS_FREQUENCY!=0) return;

    INDICATOR_SET(sim, IT_POPULATION, sim->num);

    if (sim->num==0)
    {
        return;
    }

    current_date = TIME_IN_DAYS(sim->land->date);

    for (i=0; i<8*8; i++)
    {
        INDICATOR_SET(sim, IT_POPULATION_DENSITY + i, 0);
    }

    for (n=0; n<8*CHROMOSOMES; n++)
    {
        mean_genome[n]=0;
    }
    for (n=0; n<DRIVES; n++)
    {
        drives[n]=0;
    }
    for (n=0; n<2; n++)
    {
        family[n]=0;
    }

#ifdef BRAINCODE_ON
    for (n=0; n<BRAINCODE_SIZE*2; n++)
    {
        mean_braincode[n]=0;
    }
#endif

    for (b=0; b<sim->num; b++)
    {
        enum drives_definition dd;
        local_being = &(sim->beings[b]);
        local_dob = being_dob(local_being);

        INDICATOR_ADD(sim, IT_AVERAGE_AGE_DAYS, current_date - local_dob);

        average_energy += (n_uint)being_energy(local_being);
#ifdef EPISODIC_ON
        average_first_person += (n_uint)episodic_first_person_memories_percent(sim,local_being,0);
        average_intentions += (n_uint)episodic_first_person_memories_percent(sim,local_being,1);
#endif
#ifdef PARASITES_ON
        INDICATOR_ADD(sim, IT_PARASITES, local_being->parasites);
#endif

        /* family */
        family[0]+=being_family_first_name(local_being);
        family[1]+=being_family_second_name(local_being);

        /* drives */
        for (dd=DRIVE_HUNGER; dd<DRIVES; dd++)
        {
            drives[dd] += being_drive(local_being, dd);
        }

        /* population density */
        x = (n_byte)(APESPACE_TO_MAPSPACE(being_location_x(local_being)) * 8 / MAP_DIMENSION);
        y = (n_byte)(APESPACE_TO_MAPSPACE(being_location_y(local_being)) * 8 / MAP_DIMENSION);

        INDICATOR_INC(sim, IT_POPULATION_DENSITY + (y * 8) + x);

        /* affect */
        positive_affect += being_affect(sim,local_being,1);
        negative_affect += being_affect(sim,local_being,0);

        /* social graph indicators */
        local_social_graph = being_social(local_being);
        for (i=0; i<SOCIAL_SIZE; i++)
        {
            if (!SOCIAL_GRAPH_ENTRY_EMPTY(local_social_graph,i))
            {
                social_links++;
                average_cohesion += (n_uint)(local_social_graph[i].friend_foe);
                average_familiarity += (n_uint)(local_social_graph[i].familiarity);
                average_amorousness += (n_uint)(local_social_graph[i].attraction);
            }
        }

        /* average genome */
        for (n=0; n<8*CHROMOSOMES; n++)
        {
            mean_genome[n] += GET_NUCLEOTIDE(being_genetics(local_being),n);
        }

#ifdef BRAINCODE_ON
        /* average braincode */
        for (n=0; n<BRAINCODE_SIZE; n++)
        {
            mean_braincode[n] += (n_uint)being_braincode_internal(local_being)[n];
            mean_braincode[n+BRAINCODE_SIZE] += (n_uint)being_braincode_external(local_being)[n];
        }
#endif

#ifdef IMMUNE_ON
        /* immune system */
        immune = &(local_being->immune_system);
        for (n=0; n<IMMUNE_ANTIGENS; n++)
        {
            average_antigens += (n_uint)immune->antigens[n];
        }
        for (n=0; n<IMMUNE_POPULATION; n++)
        {
            average_antibodies += (n_uint)immune->antibodies[n];
        }
#endif
    }

    INDICATOR_SET(sim, IT_AVERAGE_ANTIGENS, average_antigens);
    INDICATOR_SET(sim, IT_AVERAGE_ANTIBODIES, average_antibodies);

    INDICATOR_SET(sim, IT_AVERAGE_SOCIAL_LINKS, social_links);

    INDICATOR_SET(sim, IT_AVERAGE_POSITIVE_AFFECT, positive_affect);
    INDICATOR_SET(sim, IT_AVERAGE_NEGATIVE_AFFECT, negative_affect);
    INDICATOR_SET(sim, IT_AVERAGE_ENERGY, average_energy);

    INDICATOR_SET(sim, IT_AVERAGE_FIRST_PERSON, average_first_person);
    INDICATOR_SET(sim, IT_AVERAGE_INTENTIONS, average_intentions);

    INDICATOR_MULTIPLY(sim, IT_AVERAGE_PARASITE_MOBILITY, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_GROOMING, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_SHOUTS, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_LISTENS, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_CHAT, 100);

    INDICATOR_MULTIPLY(sim, IT_AVERAGE_SOCIAL_LINKS, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_POSITIVE_AFFECT, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_NEGATIVE_AFFECT, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_ENERGY, 100);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_FIRST_PERSON, 10);
    INDICATOR_MULTIPLY(sim, IT_AVERAGE_INTENTIONS, 10);

    INDICATOR_NORMALIZE(sim, IT_AVERAGE_PARASITE_MOBILITY);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_GROOMING);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_SHOUTS);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_LISTENS);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_CHAT);

    INDICATOR_NORMALIZE(sim, IT_AVERAGE_SOCIAL_LINKS);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_POSITIVE_AFFECT);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_NEGATIVE_AFFECT);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_ENERGY);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_FIRST_PERSON);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_INTENTIONS);

    INDICATOR_NORMALIZE(sim, IT_AVERAGE_BRAINPROBE_ACTIVITY);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_AGE_DAYS);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_ENERGY_INPUT);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_ENERGY_OUTPUT);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_MOBILITY);

#ifdef BRAINCODE_ON
    braincode_statistics(sim);
#endif

    if (social_links > 0)
    {
        INDICATOR_SET(sim, IT_AVERAGE_COHESION, average_cohesion);
        INDICATOR_MULTIPLY(sim, IT_AVERAGE_COHESION, 100);

        INDICATOR_SET(sim, IT_AVERAGE_FAMILIARITY, average_familiarity);
        INDICATOR_SET(sim, IT_AVERAGE_AMOROUSNESS, average_amorousness);

        INDICATOR_DIVIDE(sim, IT_AVERAGE_COHESION, social_links);
        INDICATOR_DIVIDE(sim, IT_AVERAGE_FAMILIARITY, social_links);
        INDICATOR_DIVIDE(sim, IT_AVERAGE_AMOROUSNESS, social_links);
    }
    else
    {
        INDICATOR_SET(sim, IT_AVERAGE_COHESION, SOCIAL_RESPECT_NORMAL);
        INDICATOR_MULTIPLY(sim, IT_AVERAGE_COHESION, 100);
    }

    /* family name variance */
    for (i=0; i<2; i++)
    {
        family[i]/=sim->num;
    }

    sd = 0;

    for (b=0; b<sim->num; b++)
    {
        local_being = &(sim->beings[b]);

        diff = (n_int)ABS(being_family_first_name(local_being) - (n_int)family[0]);
        sd += (n_uint)diff;
        diff = (n_int)ABS(being_family_second_name(local_being) - (n_int)family[1]);
        sd += (n_uint)diff;
    }

    INDICATOR_SET(sim, IT_FAMILY_NAME_SD, sd);
    INDICATOR_MULTIPLY(sim, IT_FAMILY_NAME_SD, 100);
    INDICATOR_NORMALIZE(sim, IT_FAMILY_NAME_SD);

    /* drives */
    for (i=0; i<DRIVES; i++)
    {
        INDICATOR_SET(sim, IT_DRIVES + i, drives[i]);
        INDICATOR_MULTIPLY(sim, IT_DRIVES + i, 100);
        INDICATOR_NORMALIZE(sim, IT_DRIVES + i);
    }

    /* genetics variance */
    sd=0;
    for (n=0; n<8*CHROMOSOMES; n++)
    {
        mean_genome[n] /= sim->num;
    }
    for (b=0; b<sim->num; b++)
    {
        n_genetics  * genetics;
        local_being = &(sim->beings[b]);
        genetics = being_genetics(local_being);
        for (n=0; n<8*CHROMOSOMES; n++)
        {
            diff = (n_int)ABS(GET_NUCLEOTIDE(genetics,n) - (n_int)mean_genome[n]);
            sd += (n_uint)diff;
        }
    }

    INDICATOR_SET(sim, IT_GENETICS_SD, sd);
    INDICATOR_MULTIPLY(sim, IT_GENETICS_SD, 100);
    INDICATOR_NORMALIZE(sim, IT_IDEOLOGY_SD);

    sd = 0;
#ifdef BRAINCODE_ON
    /* ideology variance */
    for (n=0; n<BRAINCODE_SIZE; n++)
    {
        mean_braincode[n] /= (n_uint)sim->num;
        mean_braincode[n+BRAINCODE_SIZE] /= (n_uint)sim->num;
    }
    for (b=0; b<sim->num; b++)
    {
        local_being = &(sim->beings[b]);
        for (n=0; n<BRAINCODE_SIZE; n++)
        {
            diff = ABS((n_int)(being_braincode_internal(local_being)[n]) - (n_int)mean_braincode[n]);
            sd += (n_uint)diff;

            diff = ABS((n_int)(being_braincode_external(local_being)[n]) - (n_int)mean_braincode[n+BRAINCODE_SIZE]);
            sd += (n_uint)diff;
        }
    }

    INDICATOR_SET(sim, IT_IDEOLOGY_SD, sd);
    INDICATOR_NORMALIZE(sim, IT_IDEOLOGY_SD);

#endif

#ifdef IMMUNE_ON

    INDICATOR_NORMALIZE(sim, IT_AVERAGE_ANTIBODIES);
    INDICATOR_NORMALIZE(sim, IT_AVERAGE_ANTIGENS);

#endif

    if (sim->indicators_logging!=0)
    {
        sprintf((char*)filename,"indicators%u.csv",(unsigned int)sim->indicators_logging);
        fp = fopen(filename,"r");
        if (fp==NULL)
        {
            fp = fopen(filename,"w");
            if (fp!=NULL)
            {

                n_int loop = 0;
                while (loop < IT_NUMBER_ENTRIES)
                {
                    n_string indicator_name = INDICATOR_NAME(sim, loop);
                    if (indicator_name)
                    {
                        if (loop < (IT_NUMBER_ENTRIES-1))
                        {
                            fprintf(fp,"%s,",indicator_name);
                        }
                        else
                        {
                            fprintf(fp,"%s\n",indicator_name);
                        }
                    }
                    loop++;
                }
            }
        }
        else
        {
            fclose(fp);
            fp  =fopen(filename,"a");
        }
        if (fp!=NULL)
        {
            n_int loop = 0;
            while (loop < IT_NUMBER_ENTRIES)
            {
                if (INDICATOR_NAME(sim, loop))
                {
                    if (loop < (IT_NUMBER_ENTRIES-1))
                    {
                        fprintf(fp,"%lu,",INDICATOR_ACCESS(sim, loop));
                    }
                    else
                    {
                        fprintf(fp,"%lu\n",INDICATOR_ACCESS(sim, loop));
                    }
                }
                loop++;
            }
            fclose(fp);
        }
    }

    /* increment index within the buffer */
    sim->indicator_index++;
    if (sim->indicator_index>=INDICATORS_BUFFER_SIZE)
    {
        sim->indicator_index = 0;
    }

    io_erase((n_byte *) &(sim->indicators_base[sim->indicator_index * IT_NUMBER_ENTRIES]), IT_NUMBER_ENTRIES*sizeof(n_uint));
}

/* this is a protoype for the order of these functions it is not used here explicitly */

void sim_cycle(void)
{
#ifndef THREADED

    land_cycle(sim.land);
    sim_being(&sim);    /* 2 */
#ifdef WEATHER_ON
    weather_cycle(sim.land, sim.weather);
#endif
    sim_brain(&sim);    /* 4 */

#ifdef BRAINCODE_ON
    sim_brain_dialogue(&sim);
#endif

#ifdef BRAIN_HASH
    sim_brain_hash(&sim)
#endif
    being_tidy(&sim);
    being_remove(&sim); /* 6 */
    sim_social(&sim);
    sim_indicators(&sim);
    sim_time(&sim);

#endif
}


#define MAXIMUM_ALLOCATION  ( 60 * 1024 * 1024 )


#define	MINIMAL_ALLOCATION	(sizeof(n_land)+(MAP_AREA)+(2*HI_RES_MAP_AREA)+(HI_RES_MAP_AREA/8)+(512*512)+(TERRAIN_WINDOW_AREA)+((sizeof(noble_being) + DOUBLE_BRAIN) * MIN_BEINGS)+1+(sizeof(n_uint)*2))


static void sim_memory_land(noble_simulation * local, n_byte * buffer, n_uint * location)
{
    local->land = (n_land *) & buffer[ *location ];
    *location += sizeof(n_land);
    
    local->land -> map = &buffer[ *location ];
    *location += (MAP_AREA);
    
    local->highres = &buffer[ *location ];
    *location += (2 * HI_RES_MAP_AREA);
    
    local->highres_tide = (n_c_uint *) &buffer[ *location ];
    *location += (HI_RES_MAP_AREA/8);
    
    local->weather = (n_weather *) &buffer[ *location ];
    *location += sizeof(n_weather);
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
    
    io_erase((n_byte *)sim.indicators_base, INDICATORS_BUFFER_SIZE * IT_NUMBER_ENTRIES);
    sim.indicator_index = 0;
    sim.indicators_logging=0;

    io_erase((n_byte *)sim.indicators_name, sizeof(n_string) * IT_NUMBER_ENTRIES);

    /* By setting these names, these indicate the values to be outputted. Set more to get more! */

    INDICATOR_INIT_NAME(sim, IT_POPULATION,"Population");
    INDICATOR_INIT_NAME(sim, IT_DROWNINGS,"Drownings");
    INDICATOR_INIT_NAME(sim, IT_PARASITES,"Parasites");

    INDICATOR_INIT_NAME(sim, IT_AVERAGE_PARASITE_MOBILITY,"Average Parasite Mobility (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_CHAT,"Average Chat (x100)");

    INDICATOR_INIT_NAME(sim, IT_AVERAGE_AGE_DAYS,"Average Age (days)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_MOBILITY,"Average Mobility");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_ENERGY,"Average Energy (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_ENERGY_INPUT,"Average Energy Input");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_ENERGY_OUTPUT,"Average Energy Output");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_AMOROUSNESS,"Average Amorousness");

    INDICATOR_INIT_NAME(sim, IT_AVERAGE_FAMILIARITY,"Average Familiarity");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_COHESION,"Average Cohesion (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_SOCIAL_LINKS,"Average Social Links (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_POSITIVE_AFFECT,"Average Positive Affect (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_NEGATIVE_AFFECT,"Average Negative Affect (x100)");

    INDICATOR_INIT_NAME(sim, IT_AVERAGE_ANTIGENS,"Average Antigens");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_ANTIBODIES,"Average Antibodies");

    INDICATOR_INIT_NAME(sim, IT_IDEOLOGY_SD,"Ideological Variation");
    INDICATOR_INIT_NAME(sim, IT_GENETICS_SD,"Genetic Variation (x100)");
    INDICATOR_INIT_NAME(sim, IT_FAMILY_NAME_SD,"Family Name Variation (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_SHOUTS,"Average Shouts (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_LISTENS,"Average Listens (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_GROOMING,"Average Grooming (x100)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_BRAINPROBE_ACTIVITY,"Average Brainprobe Activity (x100)");

    INDICATOR_INIT_NAME(sim, IT_DRIVES + DRIVE_HUNGER,"Average Hunger (x100)");
    INDICATOR_INIT_NAME(sim, IT_DRIVES + DRIVE_SOCIAL,"Average Social Drive (x100)");
    INDICATOR_INIT_NAME(sim, IT_DRIVES + DRIVE_FATIGUE,"Average Fatigue (x100)");
    INDICATOR_INIT_NAME(sim, IT_DRIVES + DRIVE_SEX,"Average Sex Drive (x100)");

    INDICATOR_INIT_NAME(sim, IT_FOOD + FOOD_VEGETABLE,"Food (Vegetable)");
    INDICATOR_INIT_NAME(sim, IT_FOOD + FOOD_FRUIT,"Food (Fruit)");
    INDICATOR_INIT_NAME(sim, IT_FOOD + FOOD_SHELLFISH,"Food (Shellfish)");
    INDICATOR_INIT_NAME(sim, IT_FOOD + FOOD_SEAWEED,"Food (Seaweed)");

    INDICATOR_INIT_NAME(sim, IT_AVERAGE_FIRST_PERSON,"Average First Person (x10)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_INTENTIONS,"Average Intentions (x10)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_SENSORS,"Average Braincode Sensors (x10)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_ACTUATORS,"Average Braincode Actuators (x10)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_OPERATORS,"Average Braincode Operators (x10)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_CONDITIONALS,"Average Braincode Conditionals (x10)");
    INDICATOR_INIT_NAME(sim, IT_AVERAGE_DATA,"Average Braincode Data (x10)");
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
    sim.delta_cycles = 0;
    sim.count_cycles = 0;
    sim.real_time = randomise;
    sim.last_time = randomise;

    sim.ext_birth = 0L;
    sim.ext_death = 0L;
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
        sim_tide_block(sim.land->map, sim.highres, sim.highres_tide);
#endif
        if (kind != KIND_LOAD_FILE)
        {
            n_uint count_to = sim.max >> 2;
#ifdef WEATHER_ON
            weather_init(sim.weather, sim.land);
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
    io_free((void *) offbuffer);
    interpret_cleanup(interpret);
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
