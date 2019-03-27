/****************************************************************

 sim.c

 =============================================================

 Copyright 1996-2019 Tom Barbalet. All rights reserved.

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

#include "../noble/noble.h"

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

static n_int        sim_new_run = 0;

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

n_int sim_new_run_condition(void)
{
    return sim_new_run;
}

static void sim_console_clean_up(void)
{
    if ((io_command_line_execution() == 0) || sim_quit_value)
    {
        return;
    }

    sim_quit_value = 1;

    command_quit(0L,0L,0L);

    while (command_executing()) {}
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

static n_int sim_input(void *vcode, n_byte kind, n_int value)
{
    noble_simulation * local_sim = sim_sim();
    n_individual_interpret * code = (n_individual_interpret *)vcode;
    n_int *local_vr = code->variable_references;
    noble_being    *local_being = 0L;
    n_int temp_select = local_vr[ VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE ];
    
    if( temp_select < 0 )
    {
        return APESCRIPT_ERROR(code, AE_SELECTED_ENTITY_OUT_OF_RANGE);
    }
    {
        n_uint local_select = temp_select;
        if( local_select >= local_sim->num)
        {
            return APESCRIPT_ERROR(code, AE_SELECTED_ENTITY_OUT_OF_RANGE);
        }
        local_being = &(local_sim->beings[local_select]);
    }
    
    switch(kind)
    {
        case VARIABLE_BRAIN_VALUE:
        {
#ifdef BRAIN_ON
            n_int    current_x = local_vr[VARIABLE_BRAIN_X - VARIABLE_VECT_ANGLE];
            n_int    current_y = local_vr[VARIABLE_BRAIN_Y - VARIABLE_VECT_ANGLE];
            n_int    current_z = local_vr[VARIABLE_BRAIN_Z - VARIABLE_VECT_ANGLE];
            
            if((value < 0) || (value > 255))
            {
                return APESCRIPT_ERROR(code, AE_VALUE_OUT_OF_RANGE);
            }
            
            if( (current_x < 0) || (current_y < 0) || (current_z < 0) ||
               (current_x > 31) || (current_y > 31) || (current_z > 31))
            {
                return APESCRIPT_ERROR(code, AE_COORDINATES_OUT_OF_RANGE);
            }
            {
                noble_being * local_being = (noble_being *)((n_individual_interpret *)code)->interpret_data;
                n_byte *local_brain = being_brain(local_being);
                if (local_brain != 0L)
                {
                    TRACK_BRAIN(local_brain, current_x, current_y, current_z) = (n_byte) value;
                }
            }
            /* add brain value */
#endif
            return 0;
        }
            
        case VARIABLE_HONOR:
            local_being->delta.honor = (n_byte) value;
            break;
        case VARIABLE_PARASITES:
            being_set_parasites(local_being, (n_byte) value);
            break;
        case VARIABLE_HEIGHT:
            being_set_height(local_being, value);
            break;
            
#if 0 /* TODO: This should not be done */
        case VARIABLE_FAMILY_NAME_ONE:
            being_set_family_name(local_being,
                                  UNPACK_FAMILY_FIRST_NAME((n_byte2) value),
                                  UNPACK_FAMILY_SECOND_NAME(being_family_name(local_being)));
            break;
        case VARIABLE_FAMILY_NAME_TWO:
            being_set_family_name(local_being,
                                  UNPACK_FAMILY_FIRST_NAME(being_family_name(local_being)),
                                  UNPACK_FAMILY_SECOND_NAME((n_byte2) value));
            break;
#endif
        case VARIABLE_GOAL_TYPE:
            local_being->delta.goal[0] = (n_byte) (value % 3);
            break;
        case VARIABLE_GOAL_X:
            local_being->delta.goal[1] = (n_byte2) value;
            break;
        case VARIABLE_GOAL_Y:
            local_being->delta.goal[2] = (n_byte2) value;
            break;
        case VARIABLE_POSTURE:
            being_set_posture(local_being, (n_byte) value);
            break;
            
        case VARIABLE_DRIVE_HUNGER:
            local_being->changes.drives[DRIVE_HUNGER] = (n_byte) value;
            break;
        case VARIABLE_DRIVE_SOCIAL:
            local_being->changes.drives[DRIVE_SOCIAL] = (n_byte) value;
            break;
        case VARIABLE_DRIVE_FATIGUE:
            local_being->changes.drives[DRIVE_FATIGUE] = (n_byte) value;
            break;
        case VARIABLE_DRIVE_SEX:
            local_being->changes.drives[DRIVE_SEX] = (n_byte) value;
            break;
            
        case VARIABLE_PREFERENCE_MATE_HEIGHT_MALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_HEIGHT_MALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_MATE_HEIGHT_FEMALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_HEIGHT_FEMALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_MATE_PIGMENTATION_MALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_PIGMENTATION_MALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_MATE_PIGMENTATION_FEMALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_PIGMENTATION_FEMALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_MATE_HAIR_MALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_HAIR_MALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_MATE_HAIR_FEMALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_HAIR_FEMALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_MATE_FRAME_MALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_FRAME_MALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_MATE_FRAME_FEMALE:
            local_being->changes.learned_preference[PREFERENCE_MATE_FRAME_FEMALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_GROOM_MALE:
            local_being->changes.learned_preference[PREFERENCE_GROOM_MALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_GROOM_FEMALE:
            local_being->changes.learned_preference[PREFERENCE_GROOM_FEMALE] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_ANECDOTE_EVENT_MUTATION:
            local_being->changes.learned_preference[PREFERENCE_ANECDOTE_EVENT_MUTATION] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_ANECDOTE_AFFECT_MUTATION:
            local_being->changes.learned_preference[PREFERENCE_ANECDOTE_AFFECT_MUTATION] = (n_byte) value;
            break;
        case VARIABLE_PREFERENCE_CHAT:
            local_being->changes.learned_preference[PREFERENCE_CHAT] = (n_byte) value;
            break;
        case VARIABLE_ATTENTION_ACTOR_INDEX:
            being_set_attention(local_being,ATTENTION_ACTOR, value % SOCIAL_SIZE);
            break;
        case VARIABLE_ATTENTION_EPISODE_INDEX:
            being_set_attention(local_being,ATTENTION_EPISODE, value % EPISODIC_SIZE);
            break;
        case VARIABLE_ATTENTION_BODY_INDEX:
            being_set_attention(local_being,ATTENTION_BODY, value % INVENTORY_SIZE);
            break;
    }
    
    if (kind>VARIABLE_BRAIN_VALUE)
    {
        local_vr[kind-VARIABLE_VECT_ANGLE] = value;
        return 0;
    }
    return -1; /* where this fails is more important than this failure */
}


static n_int sim_output(void * vcode, void * vindividual, n_byte * kind, n_int * number)
{
    noble_simulation * local_sim = sim_sim();
    n_interpret * code = (n_interpret *) vcode;
    n_individual_interpret * individual = (n_individual_interpret *) vindividual;
    n_byte    first_value = kind[0];
    n_byte    second_value = kind[1];
    if(first_value == APESCRIPT_NUMBER)
    {
        *number = code->number_buffer[second_value];
        return 0;
    }
    if((first_value == APESCRIPT_TEXT) && (VARIABLE_SPECIAL(second_value, code) == 0))
    {
        n_int    *local_vr = individual->variable_references;
        
        if( (second_value >= VARIABLE_BIOLOGY_AREA) && (second_value <= VARIABLE_BIOLOGY_EAGLE))
        {
            *number = second_value - VARIABLE_BIOLOGY_AREA;
            return 0;
        }
        
        if(second_value>VARIABLE_BRAIN_VALUE)
        {
            *number = local_vr[ second_value - VARIABLE_VECT_ANGLE ];
            return 0;
        }
        
        {
            noble_being * local_current = (noble_being *)individual->interpret_data;
            n_int          local_number = 0;
            n_vect2       local_vector;
            vect2_direction(&local_vector, local_vr[0], 32);
            switch(second_value)
            {
                    
                case VARIABLE_VECT_X:
                    local_number = local_vector.x;
                    break;
                    
                case VARIABLE_VECT_Y:
                    local_number = local_vector.y;
                    break;
                case VARIABLE_RANDOM:
                    local_number = being_random(local_current);
                    break;
                case VARIABLE_WATER_LEVEL:
                    local_number = land_tide_level();
                    break;
                case VARIABLE_HUNGRY:
                    local_number = BEING_HUNGRY;
                    break;
                case VARIABLE_ENERGY:
                {
                    noble_being * local_being = (noble_being *)individual->interpret_data;
                    local_number = being_energy(local_being);
                }
                    break;
                case VARIABLE_LOCATION_Z:
                case VARIABLE_TEST_Z:
                case VARIABLE_IS_VISIBLE:
                case VARIABLE_BIOLOGY_OUTPUT:
                {
                    n_int    quick_x;
                    n_int    quick_y;
                    
                    if(second_value == VARIABLE_LOCATION_Z)
                    {
                        quick_x = being_location_x(local_current);
                        quick_y = being_location_y(local_current);
                    }
                    else
                    {
                        quick_x = local_vr[VARIABLE_TEST_X-VARIABLE_VECT_ANGLE];
                        quick_y = local_vr[VARIABLE_TEST_Y-VARIABLE_VECT_ANGLE];
                        if( (quick_x < 0) || (quick_y < 0) ||
                           (quick_x > APESPACE_BOUNDS ) || (quick_y > APESPACE_BOUNDS) )
                        {
                            return APESCRIPT_ERROR(individual, AE_COORDINATES_OUT_OF_RANGE);
                        }
                    }
                    
                    if ( second_value == VARIABLE_IS_VISIBLE )
                    {
                        /* range already checked */
                        noble_being * local_being = (noble_being *)individual->interpret_data;
                        n_vect2 location;
                        location.x = (n_byte2)quick_x;
                        location.y = (n_byte2)quick_y;
                        local_number = being_line_of_sight(local_being, &location);
                    }
                    else
                    {
                        if(second_value == VARIABLE_BIOLOGY_OUTPUT)
                        {
                            n_int    int_qu_op = local_vr[VARIABLE_BIOLOGY_OPERATOR-VARIABLE_VECT_ANGLE];
                            if((int_qu_op < 0 ) || (int_qu_op > (VARIABLE_BIOLOGY_EAGLE - VARIABLE_BIOLOGY_AREA)))
                            {
                                return APESCRIPT_ERROR(individual, AE_VALUE_OUT_OF_RANGE);
                            }
                            local_number = land_operator_interpolated(
                                                                      (n_byte)quick_x, (n_byte)quick_y, (n_byte*)&operators[int_qu_op-VARIABLE_BIOLOGY_AREA]);
                        }
                        else
                        {
                            local_number = land_location(quick_x, quick_y);
                        }
                    }
                }
                    break;
                    
                case VARIABLE_TIME:
                    local_number = land_time();
                    break;
                case VARIABLE_DATE:
                    local_number = land_date();
                    break;
                case VARIABLE_CURRENT_BEING:
                    local_number = being_index(local_sim, (noble_being *)individual->interpret_data);
                    break;
                case VARIABLE_NUMBER_BEINGS:
                    local_number = local_sim->num;
                    break;
                    
                case VARIABLE_IS_ERROR:
                    local_number = -1;
                    break;
                    
                case VARIABLE_WEATHER:
                {
                    n_int    quick_x;
                    n_int    quick_y;
                    
                    quick_x = local_vr[VARIABLE_TEST_X-VARIABLE_VECT_ANGLE];
                    quick_y = local_vr[VARIABLE_TEST_Y-VARIABLE_VECT_ANGLE];
                    if( (quick_x < 0) || (quick_y < 0) ||
                       (quick_x > APESPACE_BOUNDS ) || (quick_y > APESPACE_BOUNDS) )
                    {
                        return APESCRIPT_ERROR(individual, AE_COORDINATES_OUT_OF_RANGE);
                    }
                    
                    local_number = weather_seven_values(quick_x, quick_y);
                }
                    break;
                case VARIABLE_BRAIN_VALUE:
                {
#ifdef BRAIN_ON
                    n_int    current_x = local_vr[VARIABLE_BRAIN_X - VARIABLE_VECT_ANGLE];
                    n_int    current_y = local_vr[VARIABLE_BRAIN_Y - VARIABLE_VECT_ANGLE];
                    n_int    current_z = local_vr[VARIABLE_BRAIN_Z - VARIABLE_VECT_ANGLE];
                    
                    if( (current_x < 0) || (current_y < 0) || (current_z < 0) ||
                       (current_x > 31) || (current_y > 31) || (current_z > 31))
                    {
                        return APESCRIPT_ERROR(individual, AE_COORDINATES_OUT_OF_RANGE);
                    }
                    {
                        noble_being * local_being = (noble_being *)individual->interpret_data;
                        n_byte *local_brain = being_brain(local_being);
                        if (local_brain != 0L)
                        {
                            local_number = TRACK_BRAIN(local_brain, current_x, current_y, current_z);
                        }
                    }
#endif
                }
                    break;
                default:
                {
                    n_int             temp_select = local_vr[ VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE ];
                    noble_being        *local_being = 0L;
                    noble_social    *local_social_graph = 0L;
                    noble_social     social_graph;
                    n_vect2          location_vect;
#ifdef EPISODIC_ON
                    noble_episodic    *local_episodic = 0L;
                    noble_episodic    episodic;
#endif
                    
                    if( temp_select < 0 )
                    {
                        return APESCRIPT_ERROR(individual, AE_SELECTED_ENTITY_OUT_OF_RANGE);
                    }
                    {
                        n_uint local_select = temp_select;
                        if( local_select >= local_sim->num)
                        {
                            return APESCRIPT_ERROR(individual, AE_SELECTED_ENTITY_OUT_OF_RANGE);
                        }
                        local_being = &(local_sim->beings[local_select]);
                        if (local_being != 0L)
                        {
                            local_social_graph = being_social(local_being);
                            if (local_social_graph!=0L)
                            {
                                social_graph = local_social_graph[being_attention(local_being,ATTENTION_ACTOR)];
                            }
#ifdef EPISODIC_ON
                            local_episodic = being_episodic(local_being);
                            if (local_episodic != 0L)
                            {
                                episodic = local_episodic[being_attention(local_being,ATTENTION_EPISODE)];
                            }
#endif
                        }
                    }
                    /* TODO: if the being knows the other being it may be possible to guess some of these */
                    
                    /* if the current being can't see the other being, it can't get this information */
                    
                    being_space(local_being, &location_vect);
                    
                    if (being_line_of_sight(local_current, &location_vect) == 0)
                    {
                        local_number = -1;
                    }
                    else if ((local_being != 0L) && (local_social_graph != 0L))
                    {
                        switch(second_value)
                        {
                                
                            case VARIABLE_HONOR:
                                local_number = being_honor(local_being);
                                break;
                            case VARIABLE_PARASITES:
                                local_number = being_parasites(local_being);
                                break;
                            case VARIABLE_HEIGHT:
                                local_number = being_height(local_being);
                                break;
                            case VARIABLE_FIRST_NAME:
                                local_number = being_gender_name(local_being);
                                break;
#if 0 /* TODO: This should not be done */
                            case VARIABLE_FAMILY_NAME_ONE:
                                local_number = UNPACK_FAMILY_FIRST_NAME(being_family_name(local_being));
                                break;
                            case VARIABLE_FAMILY_NAME_TWO:
                                local_number = UNPACK_FAMILY_SECOND_NAME(being_family_name(local_being));
                                break;
#endif
                            case VARIABLE_GOAL_TYPE:
                                local_number = local_being->delta.goal[0];
                                break;
                            case VARIABLE_GOAL_X:
                                local_number = local_being->delta.goal[1];
                                break;
                            case VARIABLE_GOAL_Y:
                                local_number = local_being->delta.goal[2];
                                break;
                            case VARIABLE_POSTURE:
                                local_number = being_posture(local_being);
                                break;
                                
                            case VARIABLE_DRIVE_HUNGER:
                                local_number = local_being->changes.drives[DRIVE_HUNGER];
                                break;
                            case VARIABLE_DRIVE_SOCIAL:
                                local_number = local_being->changes.drives[DRIVE_SOCIAL];
                                break;
                            case VARIABLE_DRIVE_FATIGUE:
                                local_number = local_being->changes.drives[DRIVE_FATIGUE];
                                break;
                            case VARIABLE_DRIVE_SEX:
                                local_number = local_being->changes.drives[DRIVE_SEX];
                                break;
                                
                            case    VARIABLE_LOCATION_X:
                                local_number = being_location_x(local_being);
                                break;
                                
                            case    VARIABLE_LOCATION_Y:
                                local_number = being_location_y(local_being);
                                break;
                                
                            case    VARIABLE_ID_NUMBER:
                                local_number = GET_I(local_being);
                                break;
                                
                            case    VARIABLE_DATE_OF_BIRTH:
                                local_number = being_dob(local_being);
                                break;
                            case    VARIABLE_STATE:
                                local_number = being_state(local_being);
                                break;
                            case    VARIABLE_PREFERENCE_MATE_HEIGHT_MALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_HEIGHT_MALE];
                                break;
                            case    VARIABLE_PREFERENCE_MATE_HEIGHT_FEMALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_HEIGHT_FEMALE];
                                break;
                            case    VARIABLE_PREFERENCE_MATE_PIGMENTATION_MALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_PIGMENTATION_MALE];
                                break;
                            case    VARIABLE_PREFERENCE_MATE_PIGMENTATION_FEMALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_PIGMENTATION_FEMALE];
                                break;
                            case    VARIABLE_PREFERENCE_MATE_HAIR_MALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_HAIR_MALE];
                                break;
                            case    VARIABLE_PREFERENCE_MATE_HAIR_FEMALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_HAIR_FEMALE];
                                break;
                            case    VARIABLE_PREFERENCE_MATE_FRAME_MALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_FRAME_MALE];
                                break;
                            case    VARIABLE_PREFERENCE_MATE_FRAME_FEMALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_MATE_FRAME_FEMALE];
                                break;
                            case    VARIABLE_PREFERENCE_GROOM_MALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_GROOM_MALE];
                                break;
                            case    VARIABLE_PREFERENCE_GROOM_FEMALE:
                                local_number = local_being->changes.learned_preference[PREFERENCE_GROOM_FEMALE];
                                break;
                            case    VARIABLE_PREFERENCE_ANECDOTE_EVENT_MUTATION:
                                local_number = local_being->changes.learned_preference[PREFERENCE_ANECDOTE_EVENT_MUTATION];
                                break;
                            case    VARIABLE_PREFERENCE_ANECDOTE_AFFECT_MUTATION:
                                local_number = local_being->changes.learned_preference[PREFERENCE_ANECDOTE_AFFECT_MUTATION];
                                break;
                            case    VARIABLE_PREFERENCE_CHAT:
                                local_number = local_being->changes.learned_preference[PREFERENCE_CHAT];
                                break;
                            case    VARIABLE_ATTENTION_ACTOR_INDEX:
                                local_number = being_attention(local_being,ATTENTION_ACTOR);
                                break;
                            case    VARIABLE_ATTENTION_EPISODE_INDEX:
                                local_number = being_attention(local_being,ATTENTION_EPISODE);
                                break;
                            case    VARIABLE_ATTENTION_BODY_INDEX:
                                local_number = being_attention(local_being,ATTENTION_BODY);
                                break;
                            case    VARIABLE_SHOUT_CONTENT:
                                local_number = local_being->changes.shout[SHOUT_CONTENT];
                                break;
                            case    VARIABLE_SHOUT_HEARD:
                                local_number = local_being->changes.shout[SHOUT_HEARD];
                                break;
                            case    VARIABLE_SHOUT_CTR:
                                local_number = local_being->changes.shout[SHOUT_CTR];
                                break;
                            case    VARIABLE_SHOUT_VOLUME:
                                local_number = local_being->changes.shout[SHOUT_VOLUME];
                                break;
                            case    VARIABLE_SHOUT_FAMILY0:
                                local_number = local_being->changes.shout[SHOUT_FAMILY0];
                                break;
                            case    VARIABLE_SHOUT_FAMILY1:
                                local_number = local_being->changes.shout[SHOUT_FAMILY1];
                                break;
                                
                            case VARIABLE_SOCIAL_GRAPH_LOCATION_X:
                                local_number = social_graph.space_time.location[0];
                                break;
                            case VARIABLE_SOCIAL_GRAPH_LOCATION_Y:
                                local_number = social_graph.space_time.location[1];
                                break;
                            case VARIABLE_SOCIAL_GRAPH_TIME:
                                local_number = social_graph.space_time.time;
                                break;
                            case VARIABLE_SOCIAL_GRAPH_DATE:
                                local_number = social_graph.space_time.date;
                                break;
                            case VARIABLE_SOCIAL_GRAPH_ATTRACTION:
                                local_number = social_graph.attraction;
                                break;
                            case VARIABLE_SOCIAL_GRAPH_FOF:
                                local_number =
                                (n_int)social_graph.friend_foe -
                                (n_int)social_respect_mean(local_being);
                                break;
                            case VARIABLE_SOCIAL_GRAPH_FAMILIARITY:
                                local_number = social_graph.familiarity;
                                break;
                            case VARIABLE_MEMORY_FIRST_NAME:
                                local_number = social_graph.first_name[BEING_MET];
                                break;
#if 0 /* TODO: This should not be done */
                            case VARIABLE_MEMORY_FAMILY_NAME_ONE:
                                local_number = UNPACK_FAMILY_FIRST_NAME(social_graph.family_name[BEING_MET]);
                                break;
                            case VARIABLE_MEMORY_FAMILY_NAME_TWO:
                                local_number = UNPACK_FAMILY_SECOND_NAME(social_graph.family_name[BEING_MET]);
                                break;
#endif
#ifdef EPISODIC_ON
                            case VARIABLE_MEMORY_LOCATION_X:
                                local_number = episodic.space_time.location[0];
                                break;
                            case VARIABLE_MEMORY_LOCATION_Y:
                                local_number = episodic.space_time.location[1];
                                break;
                            case VARIABLE_MEMORY_TIME:
                                local_number = episodic.space_time.time;
                                break;
                            case VARIABLE_MEMORY_DATE:
                                local_number = episodic.space_time.date;
                                break;
                            case VARIABLE_MEMORY_FIRST_NAME0:
                                local_number = episodic.first_name[0];
                                break;
                            case VARIABLE_MEMORY_FIRST_NAME1:
                                local_number = episodic.first_name[BEING_MET];
                                break;
#if 0 /* TODO: This should not be done */
                            case VARIABLE_MEMORY_FAMILY_NAME_ONE0:
                                local_number = UNPACK_FAMILY_FIRST_NAME(episodic.family_name[0]);
                                break;
                            case VARIABLE_MEMORY_FAMILY_NAME_TWO0:
                                local_number = UNPACK_FAMILY_SECOND_NAME(episodic.family_name[0]);
                                break;
                            case VARIABLE_MEMORY_FAMILY_NAME_ONE1:
                                local_number = UNPACK_FAMILY_FIRST_NAME(episodic.family_name[BEING_MET]);
                                break;
                            case VARIABLE_MEMORY_FAMILY_NAME_TWO1:
                                local_number = UNPACK_FAMILY_SECOND_NAME(episodic.family_name[BEING_MET]);
                                break;
#endif
                            case VARIABLE_MEMORY_EVENT:
                                local_number = episodic.event;
                                break;
                            case VARIABLE_MEMORY_AFFECT:
                                local_number = episodic.affect - EPISODIC_AFFECT_ZERO;
                                break;
#endif
                        }
                    }
                }
                    
                    break;
            }
            /* put variable cross here */
            *number = local_number;
            return 0;
        }
    }
    return -1; /* where this fails is more important than this failure */
}


n_int     sim_interpret(n_file * input_file)
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

    interpret->sc_input  = &sim_input;
    interpret->sc_output = &sim_output;

    interpret->input_greater   = VARIABLE_WEATHER;
    interpret->special_less    = VARIABLE_VECT_X;
    return 0;
}

#ifdef BRAIN_ON
static void sim_brain_loop(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    n_byte2 local_brain_state[3];

    if(being_brainstates(local_being, (local_being->delta.awake == 0), local_brain_state))
    {
        n_byte			*local_brain = being_brain(local_being);
        if (local_brain != 0L)
        {
            being_brain_cycle(local_brain, local_brain_state);
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
    if(local_being->delta.awake == 0)
    {
        awake=0;
    }
    /* This should be independent of the brainstate/cognitive simulation code */
    brain_dialogue(local_sim, awake, local_being, local_being, local_internal, local_external, being_random(local_being)%SOCIAL_SIZE);
    brain_dialogue(local_sim, awake, local_being, local_being, local_external, local_internal, being_random(local_being)%SOCIAL_SIZE);
}

#endif


static void sim_being_awake_loop_no_sim(noble_being * local_being, void * data)
{
    n_byte awake_condition = being_awake(local_being);
    local_being->delta.awake = awake_condition;
    
#ifdef DEBUG_LACK_OF_MOVEMENT
    being_register_movement(local_being, "awake condition");
#endif
}

static void sim_being_universal_loop_no_sim(noble_being * local_being, void * data)
{
    being_cycle_universal(local_being);
}

static void sim_being_cycle(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    if (local_being->delta.awake == 0) return;

    being_cycle_awake(local_sim, local_being);
}

static void sim_start_conditions(void * vindividual, void * structure, void * data)
{
    n_individual_interpret * individual = (n_individual_interpret *)vindividual;
    n_int       * variables = individual->variable_references;
    noble_being * local_being = (noble_being*)data;
    
    variables[VARIABLE_FACING - VARIABLE_VECT_ANGLE] = being_facing(local_being);
    variables[VARIABLE_SPEED - VARIABLE_VECT_ANGLE] =  being_speed(local_being);
    variables[VARIABLE_ENERGY_DELTA - VARIABLE_VECT_ANGLE] = 0;
    
    variables[VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE] = being_index(sim_sim(), local_being);
    
    variables[VARIABLE_HEIGHT - VARIABLE_VECT_ANGLE] = being_height(local_being);
    variables[VARIABLE_GOAL_TYPE - VARIABLE_VECT_ANGLE] = local_being->delta.goal[0];
    variables[VARIABLE_GOAL_X - VARIABLE_VECT_ANGLE] = local_being->delta.goal[1];
    variables[VARIABLE_GOAL_Y - VARIABLE_VECT_ANGLE] = local_being->delta.goal[2];
    variables[VARIABLE_DRIVE_HUNGER - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_HUNGER];
    variables[VARIABLE_DRIVE_SOCIAL - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_SOCIAL];
    variables[VARIABLE_DRIVE_FATIGUE - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_FATIGUE];
    variables[VARIABLE_DRIVE_SEX - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_SEX];
    variables[VARIABLE_FAMILY_NAME_ONE - VARIABLE_VECT_ANGLE] = being_family_first_name(local_being);
    variables[VARIABLE_FAMILY_NAME_TWO - VARIABLE_VECT_ANGLE] = being_family_second_name(local_being);
    variables[VARIABLE_HONOR - VARIABLE_VECT_ANGLE] = being_honor(local_being);
    variables[VARIABLE_PARASITES - VARIABLE_VECT_ANGLE] = being_parasites(local_being);
}

static void sim_end_conditions(void * vindividual, void * structure, void * data)
{
    n_individual_interpret * individual = (n_individual_interpret *)vindividual;
    n_int       * variables = individual->variable_references;
    noble_being * local_being = (noble_being*)data;
    
    n_int    local_facing = variables[VARIABLE_FACING - VARIABLE_VECT_ANGLE];
    n_int    local_speed  = variables[VARIABLE_SPEED  - VARIABLE_VECT_ANGLE];
    n_int    local_energy_delta = variables[VARIABLE_ENERGY_DELTA - VARIABLE_VECT_ANGLE];
    n_int    local_height = variables[VARIABLE_HEIGHT - VARIABLE_VECT_ANGLE];
    n_int    local_goal_type = variables[VARIABLE_GOAL_TYPE - VARIABLE_VECT_ANGLE];
    n_int    local_goal_x = variables[VARIABLE_GOAL_X - VARIABLE_VECT_ANGLE];
    n_int    local_goal_y = variables[VARIABLE_GOAL_Y - VARIABLE_VECT_ANGLE];
    n_int    local_drive_hunger = variables[VARIABLE_DRIVE_HUNGER - VARIABLE_VECT_ANGLE];
    n_int    local_drive_social = variables[VARIABLE_DRIVE_SOCIAL - VARIABLE_VECT_ANGLE];
    n_int    local_drive_fatigue = variables[VARIABLE_DRIVE_FATIGUE - VARIABLE_VECT_ANGLE];
    n_int    local_drive_sex = variables[VARIABLE_DRIVE_SEX - VARIABLE_VECT_ANGLE];
    n_int    local_family_name1 = variables[VARIABLE_FAMILY_NAME_ONE - VARIABLE_VECT_ANGLE];
    n_int    local_family_name2 = variables[VARIABLE_FAMILY_NAME_TWO - VARIABLE_VECT_ANGLE];
    
    n_int    local_honor = variables[VARIABLE_HONOR - VARIABLE_VECT_ANGLE];
    n_int    local_parasites = variables[VARIABLE_PARASITES - VARIABLE_VECT_ANGLE];
    
    if(local_facing< 0)
    {
        local_facing = 255 - ( (0 - local_facing) & 255 );
    }
    else
    {
        local_facing = local_facing & 255;
    }
    
    if (local_speed > 39) local_speed = 39;
    if (local_speed < 0)  local_speed = 0;
    
    being_wander(local_being, local_facing - being_facing(local_being));
    
    being_set_speed(local_being, (n_byte) local_speed);
    being_energy_delta(local_being, local_energy_delta);
    
    being_set_height(local_being, local_height);
    
    if (local_goal_type!=GOAL_NONE)
    {
        local_being->delta.goal[0]  = (n_byte2)local_goal_type;
        local_being->delta.goal[1]  = (n_byte2)local_goal_x;
        local_being->delta.goal[2]  = (n_byte2)local_goal_y;
        local_being->delta.goal[3]  = GOAL_TIMEOUT;
        
        local_being->braindata.script_overrides |= OVERRIDE_GOAL;
    }
    if (local_drive_hunger>-1)
    {
        local_being->changes.drives[DRIVE_HUNGER]  = (n_byte)local_drive_hunger;
    }
    if (local_drive_social>-1)
    {
        local_being->changes.drives[DRIVE_SOCIAL]  = (n_byte)local_drive_social;
    }
    if (local_drive_fatigue>-1)
    {
        local_being->changes.drives[DRIVE_FATIGUE]  = (n_byte)local_drive_fatigue;
    }
    if (local_drive_sex>-1)
    {
        local_being->changes.drives[DRIVE_SEX]  = (n_byte)local_drive_sex;
    }
    being_set_family_name(local_being,(n_byte)local_family_name1,(n_byte)local_family_name2);
    
    local_being->delta.honor = (n_byte)local_honor;
    being_set_parasites(local_being, (n_byte)local_parasites);
}



static void sim_being_interpret(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    n_individual_interpret individual;

    interpret_individual(&individual);

    if (local_being->delta.awake == 0) return;

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

#if 1

#define PROCESSING_HEAVY_WEIGHT    (4)
#define PROCESSING_MIDDLE_WEIGHT   (8)
#define PROCESSING_WELTER_WEIGHT  (16)
#define PROCESSING_LIGHT_WEIGHT   (32)
#define PROCESSING_FEATHER_WEIGHT (64)

#else

#define PROCESSING_HEAVY_WEIGHT    (3)
#define PROCESSING_MIDDLE_WEIGHT   (7)
#define PROCESSING_WELTER_WEIGHT  (17)
#define PROCESSING_LIGHT_WEIGHT   (31)
#define PROCESSING_FEATHER_WEIGHT (67)

#endif

static void sim_being_remove_final(noble_simulation * local_sim, being_remove_loop2_struct ** brls)
{
    local_sim->num = (*brls)->count;
    if ((*brls)->selected_died)
    {
        if ((*brls)->count)
        {
            sim_set_select(local_sim->beings);
        }
        else
        {
            sim_set_select(0L);
        }
    }
    
    if ((*brls)->count == 0)
    {
        if (sim_new_run == 0)
        {
            (void)SHOW_ERROR("No Apes remain start new run");
            sim_new_run = 1;
        }
    }
    being_remove_internal_clear();
    memory_free((void **)brls);
}

void sim_cycle(void)
{
    n_int       max_honor = 0;

    land_cycle();
#ifdef WEATHER_ON
    weather_cycle();
#endif

    loop_being_no_sim(sim.beings, sim.num, sim_being_awake_loop_no_sim, 0L);
    loop_being_no_sim(sim.beings, sim.num, sim_being_universal_loop_no_sim, 0L);

    if (interpret)
    {
        loop_being(&sim, sim_being_interpret, PROCESSING_WELTER_WEIGHT);
    }
    else
    {
        /** Listen for any shouts */
        loop_being(&sim, being_listen, PROCESSING_FEATHER_WEIGHT);
#ifdef EPISODIC_ON
        loop_being_no_sim(sim.beings, sim.num, episodic_cycle_no_sim, 0L);
#endif
        loop_being(&sim, sim_being_cycle, PROCESSING_MIDDLE_WEIGHT);
        loop_being(&sim, drives_cycle, PROCESSING_LIGHT_WEIGHT);
    }

    if (land_time() & 1)
    {
#ifdef BRAIN_ON
        loop_being(&sim, sim_brain_loop, PROCESSING_WELTER_WEIGHT);
#endif
    }
#ifdef BRAINCODE_ON
    else
    {
        loop_being(&sim, sim_brain_dialogue_loop, PROCESSING_MIDDLE_WEIGHT);
    }
#endif

    loop_being_no_sim(sim.beings, sim.num, being_tidy_loop_no_sim, &max_honor);

    loop_being(&sim, social_initial_loop, PROCESSING_LIGHT_WEIGHT);

    if (max_honor)
    {
        loop_being_no_sim(sim.beings, sim.num, being_recalibrate_honor_loop_no_sim, 0L);
    }

    loop_being_no_sim(sim.beings, sim.num, social_secondary_loop_no_sim, 0L);

    {
        being_remove_loop2_struct * brls = being_remove_initial(&sim);
        if (sim.ext_death != 0L)
        {
            loop_no_thread(&sim, 0L, being_remove_loop1, 0L);
        }
        loop_no_thread(&sim, 0L, being_remove_loop2, brls);
        sim_being_remove_final(&sim, &brls);
    }

    sim_time(&sim);
}

#define	MINIMAL_ALLOCATION	((512*512)+(TERRAIN_WINDOW_AREA)+(sizeof(noble_being) * MIN_BEINGS)+sizeof(noble_remains)+1+(sizeof(noble_simulation)))

#define MAXIMUM_ALLOCATION  (MINIMAL_ALLOCATION + (sizeof(noble_being) * 400))

static void sim_memory_remains(noble_simulation * local, n_byte * buffer, n_uint * location)
{
    local->remains = (noble_remains *) & buffer[ *location ];
    *location += sizeof(noble_remains);
}

static n_int being_memory(noble_simulation * local, n_byte * buffer, n_uint * location, n_uint memory_available)
{
    n_uint  lpx = 0;
    n_uint  number_apes = 0;
    
    if (memory_available < 1)
    {
        return SHOW_ERROR("Memory not available");
    }
    
    number_apes = (memory_available / sizeof(noble_being)) - 1;
    
    if (number_apes < 1)
    {
        return SHOW_ERROR("Not enough memory for an ape");
    }
    
#ifdef LARGE_SIM
    local->max = LARGE_SIM;
#else
    local->max = (memory_available / sizeof(noble_being)) - 1;
#endif
    local->beings = (noble_being *) & buffer[  * location ];
    * location += sizeof(noble_being) * local->max ;
    
    while (lpx < local->max)
    {
        noble_being * local_being = &(local->beings[ lpx ]);
        memory_erase((n_byte *)local_being, sizeof(noble_being));
        lpx ++;
    }
    return 0;
}

static n_int sim_memory(n_uint offscreen_size)
{
    n_uint	current_location = 0;
    n_uint  memory_allocated = MAXIMUM_ALLOCATION;
        
    offbuffer = memory_new_range(offscreen_size + MINIMAL_ALLOCATION, &memory_allocated);
    
    current_location = offscreen_size;
    
    sim_memory_remains(&sim, offbuffer, &current_location);

    memory_allocated = memory_allocated - current_location;
    
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
    sim.real_time = randomise;
    sim.last_time = randomise;
    
#ifdef FIXED_RANDOM_SIM
    randomise = FIXED_RANDOM_SIM;
#endif
    
    sim.delta_cycles = 0;
    sim.count_cycles = 0;
    sim.delta_frames = 0;
    sim.count_frames = 0;

    sim.ext_birth = 0L;
    sim.ext_death = 0L;
    
    if ((kind == KIND_START_UP) || (kind == KIND_MEMORY_SETUP))
    {
        if (sim_memory(offscreen_size) != 0)
        {
            return 0L;
        }
    }

    local_random[0] = (n_byte2)(randomise >> 16) & 0xffff;
    local_random[1] = (n_byte2)(randomise & 0xffff);

    
    if ((kind != KIND_LOAD_FILE) && (kind != KIND_MEMORY_SETUP))
    {
        land_seed_genetics(local_random);
    }
    
    being_remains_init(sim.remains); /* Eventually this should be captured through the file handling and moved into the code below */

    if (kind != KIND_MEMORY_SETUP)
    {
        if (kind != KIND_NEW_APES)
        {
            land_clear(kind, AGE_OF_MATURITY);
#ifdef LAND_ON
            land_init();
            land_init_high_def(1);
            land_tide();
#endif
        }
        if (kind != KIND_LOAD_FILE)
        {
#ifdef WEATHER_ON
            weather_init();
#endif
            /* Sets the number of Noble Apes initiaally created, and creates them */
            sim.num = being_init_group(sim.beings, local_random, sim.max >> 1, sim.max);
        }
    }

    sim_set_select(sim.beings);

    sim_new_progress = 0;

    return ((void *) offbuffer);
}

void sim_close(void)
{
    command_quit(0L, 0L, 0L);
    io_console_quit();
#ifndef _WIN32
    sim_console_clean_up();
#endif
    interpret_cleanup(&interpret);
    memory_free((void **) &offbuffer);
    /*death_record_file_cleanup();*/
}

void sim_set_select(noble_being * select)
{
    sim.select = select;
    command_external_watch();
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
    loop_no_thread(&sim, 0L, sim_flood_loop, 0L);
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
