/****************************************************************

 sim.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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

#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"
#include "../universe/universe.h"
#include "../entity/entity.h"

#include <stdio.h>

#include "universe_internal.h"

#ifndef    _WIN32

#include <pthread.h>

#endif

static variable_string    apescript_variable_codes[VARIABLE_MAX] =
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

    "brain_value",        /* special input/output */
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

n_byte     *offbuffer = 0L;

/* Twice the minimum number of apes the Simulation will allow to run */
#define MIN_BEINGS        4

static  simulated_group group;
static  simulated_timing timing;

static n_interpret *interpret = 0L;

static n_int        sim_new_progress = 0;

static n_int        sim_new_run = 0;

static n_uint       initial_memory_allocated;

static n_int        sim_desire_output = 0;
static n_int        sim_writing_output = 0;

static n_string_block sim_console_output;

#ifndef    _WIN32

static n_int      sim_quit_value = 0;
static pthread_t  threads[2] = {0};
static n_byte     threads_running[2] = {0};

n_int sim_thread_console_quit( void )
{
    return sim_quit_value;
}

n_int sim_new_run_condition( void )
{
    return sim_new_run;
}

static void sim_console_clean_up( void )
{
    if ( ( io_command_line_execution() == 0 ) || sim_quit_value )
    {
        return;
    }

    sim_quit_value = 1;

    command_quit( 0L, 0L, 0L );

    while ( command_executing() ) {}
}


static n_console_input *input_function = &io_console_entry_clean;
static n_console_output *output_function = &io_console_out;

void sim_set_console_input( n_console_input *local_input_function )
{
    if ( local_input_function != 0L )
    {
        input_function = local_input_function;
    }
}

void sim_set_console_output( n_console_output *local_output_function )
{
    if ( local_output_function != 0L )
    {
        output_function = local_output_function;
    }
}

static void *sim_thread_posix( void *threadid )
{
    n_byte *local = ( n_byte * )threadid;
    if ( io_console( &group, ( simulated_console_command * ) control_commands, *input_function, *output_function ) != 0 )
    {
        sim_console_clean_up();
    }
    local[0] = 0;
    pthread_exit( 0L );
}

void sim_thread_console( void )
{
    if ( io_command_line_execution() == 0 )
    {
        return;
    }

    if ( ( threads_running[0] == 0 ) || ( threads_running[1] == 0 ) )
    {
        n_int loop = 0;
        while ( loop < 2 )
        {
            if ( threads_running[loop] == 0 )
            {
                threads_running[loop] = 1;
                pthread_create( &threads[loop], 0L, sim_thread_posix, &threads_running[loop] );
                return;
            }
            loop++;
        }
    }
}

#endif

void sim_console( n_string simulation_filename, n_uint randomise )
{
    printf( "\n *** %sConsole, %s ***\n", SHORT_VERSION_NAME, FULL_DATE );
    printf( "      For a list of commands type 'help'\n\n" );

    io_command_line_execution_set();
    sim_init( KIND_START_UP, randomise, MAP_AREA, 0 );

    if ( simulation_filename )
    {
        if ( io_disk_check( simulation_filename ) != 0 )
        {
            simulated_group *group = sim_group();
            command_open( group, simulation_filename, io_console_out );
            printf( "Simulation file %s loaded\n", simulation_filename );
        }
    }

#ifndef    _WIN32
    do
    {
        sim_thread_console();
    }
    while ( sim_thread_console_quit() == 0 );
#else
    {
        do
        {}
        while ( io_console( &group,
                            ( simulated_console_command * )control_commands,
                            io_console_entry,
                            io_console_out ) == 0 );
    }
#endif
    sim_close();
}


void sim_set_output( n_int value )
{
    sim_desire_output = value;
}

n_int sim_get_writing_output( void )
{
    return sim_writing_output;
}

n_string sim_output_string( void )
{
    return sim_console_output;
}

n_int sim_new( void )
{
    return sim_new_progress;
}

void sim_realtime( n_uint time )
{
    timing.real_time = time;
}

static simulated_being *sim_being_local( void )
{
    simulated_group *group = sim_group();

    if ( group == 0L )
    {
        return 0L;
    }

    return group->select;
}


void sim_move( n_int rel_vel, n_byte kind )
{
    simulated_being *local;
    if ( ( local = sim_being_local() ) )
    {
        being_move( local, rel_vel, kind );
    }
}

void sim_view_options( n_int px, n_int py )
{
    simulated_being *local;
    if ( ( local = sim_being_local() ) )
    {
        n_byte2    location[2];

        location[0] = APESPACE_CONFINED( MAPSPACE_TO_APESPACE( px ) );
        location[1] = APESPACE_CONFINED( MAPSPACE_TO_APESPACE( py ) );

        being_set_location( local, location );
    }
}

#define MAX_POSSIBLE_CONTROL_CHARACTER_X ((2048 - CHARACTER_WIDTH)/CHARACTER_WIDTH)
#define MAX_POSSIBLE_CONTROL_CHARACTER_Y ((2048 - CHARACTER_WIDTH)/CHARACTER_HEIGHT)

static n_byte sim_control_string[255 * 40];
static n_int  sim_control_string_offset[255];

static n_byte sim_control[MAX_POSSIBLE_CONTROL_CHARACTER_X * MAX_POSSIBLE_CONTROL_CHARACTER_Y];

static n_int  sim_control_max_x;

static n_byte sim_control_previous_character;
static n_byte sim_control_character_location;

simulated_timing *sim_timing( void )
{
    return &timing;
}

simulated_group *sim_group( void )
{
    return &group;
}


n_int sim_control_translate( n_int px, n_int py, n_int *character_x, n_int *character_y )
{
    *character_x = ( px - CHARACTER_WIDTH ) / CHARACTER_WIDTH;
    if ( ( *character_x > -1 ) && ( *character_x < MAX_POSSIBLE_CONTROL_CHARACTER_X ) )
    {
        *character_y = ( py - CHARACTER_WIDTH ) / CHARACTER_HEIGHT;
        if ( ( *character_y > -1 ) && ( *character_y < MAX_POSSIBLE_CONTROL_CHARACTER_Y ) )
        {
            return 1;
        }
    }
    return 0;
}

void sim_control_set( n_int px, n_int py, n_byte value, n_byte character )
{
    n_int character_x, character_y;
    if ( sim_control_translate( px, py, &character_x, &character_y ) )
    {
        sim_control[( character_y * sim_control_max_x ) + character_x] = value;
        if ( value != sim_control_previous_character )
        {
            if ( sim_control_previous_character )
            {
                sim_control_string[sim_control_character_location] = 0;
                sim_control_character_location++;
            }
            sim_control_string_offset[sim_control_previous_character] = sim_control_character_location;
            sim_control_previous_character = value;
        }
        sim_control_string[sim_control_character_location] = character;
        sim_control_character_location++;
    }
}

void sim_control_erase( n_int size_x, n_int size_y, n_int max_characters )
{
    n_int max_y = ( size_y - CHARACTER_WIDTH ) / CHARACTER_HEIGHT;
    sim_control_max_x = max_characters;
    sim_control_previous_character = 0;
    sim_control_character_location = 0;
    memory_erase( sim_control, ( n_uint )( sim_control_max_x * max_y ) );
    memory_erase( sim_control_string, 255 * 40 );
}

static simulated_being *sim_select_name( n_string name )
{
    n_uint name_hash = math_hash( ( n_byte * )name, ( n_uint )io_length( name, STRING_BLOCK_SIZE ) );
    n_uint loop = 0;
    simulated_group *group = sim_group();
    simulated_being *local_beings = group->beings;
    while ( loop < group->num )
    {
        n_uint         being_hash;
        simulated_being   *local_being = &local_beings[loop];
        n_string_block local_name;
        being_name_simple( local_being, local_name );
        being_hash = math_hash( ( n_byte * )local_name, ( n_uint )io_length( local_name, STRING_BLOCK_SIZE ) );
        if ( name_hash == being_hash )
        {
            return local_being;
        }
        loop++;
    }
    return 0L;
}

#undef DEBUG_CONTROL_WINDOW

#ifdef DEBUG_CONTROL_WINDOW

static void sim_control_debug_sim_control( n_int py )
{
    n_int y_loop = 0;
    while ( y_loop < py )
    {
        n_int x_loop = 0;
        while ( x_loop < sim_control_max_x )
        {
            printf( "%d,", sim_control[x_loop + ( y_loop * sim_control_max_x )] );
            x_loop++;
        }
        printf( "\n" );
        y_loop++;
    }
}

#endif



void sim_control_regular( n_int px, n_int py )
{
    n_int character_x, character_y;
    if ( sim_control_translate( px, py, &character_x, &character_y ) )
    {
        n_byte value = sim_control[( character_y * sim_control_max_x ) + character_x];
#ifdef DEBUG_CONTROL_WINDOW
        printf( "p(%ld, %ld) c(%ld, %ld) sim_control_max_x %ld value %d\n", px, py, character_x, character_y, sim_control_max_x, value );
        sim_control_debug_sim_control( character_y );
#endif
        if ( value )
        {
            n_int         offset = sim_control_string_offset[value - 1];
            n_string      contral_name = ( n_string )&sim_control_string[offset];
            simulated_being *select_being = sim_select_name( contral_name );
            if ( select_being )
            {
                sim_set_select( select_being );
            }
        }
    }
}

n_int sim_view_regular( n_int px, n_int py )
{
    simulated_being *local;
    if ( ( local = sim_being_local() ) )
    {
        simulated_group *group = sim_group();
        simulated_being *desired_ape = local;
        n_uint  high_squ = 31;
        n_uint    loop = 0;

        while ( loop < group->num )
        {
            simulated_being     *current_ape = &( group->beings[loop] );

            n_int    screen_x = APESPACE_TO_MAPSPACE( being_location_x( current_ape ) ) - px;
            n_int    screen_y = APESPACE_TO_MAPSPACE( being_location_y( current_ape ) ) - py;
            n_uint   current_squ = ( n_uint )( ( screen_x * screen_x ) + ( screen_y * screen_y ) );

            if ( high_squ > current_squ )
            {
                high_squ = current_squ;
                desired_ape = current_ape;
            }
            loop++;
        }

        if ( local != desired_ape )
        {
            sim_set_select( desired_ape );
            return 0;
        }
        return 1;
    }
    return 0;
}

void sim_terrain( n_int sx )
{
    simulated_being *local;
    if ( sx > 0 )
    {
        if ( ( local = sim_being_local() ) )
        {
            being_wander( local, 1 );
        }
    }
    if ( sx <= 0 )
    {
        if ( ( local = sim_being_local() ) )
        {
            being_wander( local, -1 );
        }
    }
}

void sim_rotate( n_int integer_rotation_256 )
{
    simulated_being *local;
    if ( ( local = sim_being_local() ) )
    {
        being_wander( local, integer_rotation_256 );
    }
}

void sim_change_selected( n_byte forwards )
{
    simulated_group *group = sim_group();
    if ( group )
    {
        command_change_selected( group, forwards );
    }
}

static n_int sim_input( void *vcode, n_byte kind, n_int value )
{
    n_individual_interpret *code = ( n_individual_interpret * )vcode;
    n_int *local_vr = code->variable_references;
    simulated_being    *local_being = 0L;
    n_int temp_select = local_vr[ VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE ];

    if ( temp_select < 0 )
    {
        return APESCRIPT_ERROR( code, AE_SELECTED_ENTITY_OUT_OF_RANGE );
    }
    {
        n_uint local_select = ( n_uint )temp_select;
        simulated_group *group = sim_group();

        if ( local_select >= group->num )
        {
            return APESCRIPT_ERROR( code, AE_SELECTED_ENTITY_OUT_OF_RANGE );
        }
        local_being = &( group->beings[local_select] );
    }

    switch ( kind )
    {
    case VARIABLE_BRAIN_VALUE:
    {
#ifdef BRAIN_ON
        n_int    current_x = local_vr[VARIABLE_BRAIN_X - VARIABLE_VECT_ANGLE];
        n_int    current_y = local_vr[VARIABLE_BRAIN_Y - VARIABLE_VECT_ANGLE];
        n_int    current_z = local_vr[VARIABLE_BRAIN_Z - VARIABLE_VECT_ANGLE];

        if ( ( value < 0 ) || ( value > 255 ) )
        {
            return APESCRIPT_ERROR( code, AE_VALUE_OUT_OF_RANGE );
        }

        if ( ( current_x < 0 ) || ( current_y < 0 ) || ( current_z < 0 ) ||
                ( current_x > 31 ) || ( current_y > 31 ) || ( current_z > 31 ) )
        {
            return APESCRIPT_ERROR( code, AE_COORDINATES_OUT_OF_RANGE );
        }
        {
            simulated_being *local_being = ( simulated_being * )( ( n_individual_interpret * )code )->interpret_data;
            n_byte *local_brain = being_brain( local_being );
            if ( local_brain != 0L )
            {
                TRACK_BRAIN( local_brain, current_x, current_y, current_z ) = ( n_byte ) value;
            }
        }
        /* add brain value */
#endif
        return 0;
    }

    case VARIABLE_HONOR:
        local_being->delta.honor = ( n_byte ) value;
        break;
    case VARIABLE_PARASITES:
        being_set_parasites( local_being, ( n_byte ) value );
        break;
    case VARIABLE_HEIGHT:
        being_set_height( local_being, value );
        break;

#if 0 /* TODO: This should not be done */
    case VARIABLE_FAMILY_NAME_ONE:
        being_set_family_name( local_being,
                               UNPACK_FAMILY_FIRST_NAME( ( n_byte2 ) value ),
                               UNPACK_FAMILY_SECOND_NAME( being_family_name( local_being ) ) );
        break;
    case VARIABLE_FAMILY_NAME_TWO:
        being_set_family_name( local_being,
                               UNPACK_FAMILY_FIRST_NAME( being_family_name( local_being ) ),
                               UNPACK_FAMILY_SECOND_NAME( ( n_byte2 ) value ) );
        break;
#endif
    case VARIABLE_GOAL_TYPE:
        local_being->delta.goal[0] = ( n_byte ) ( value % 3 );
        break;
    case VARIABLE_GOAL_X:
        local_being->delta.goal[1] = ( n_byte2 ) value;
        break;
    case VARIABLE_GOAL_Y:
        local_being->delta.goal[2] = ( n_byte2 ) value;
        break;
    case VARIABLE_POSTURE:
        being_set_posture( local_being, ( n_byte ) value );
        break;

    case VARIABLE_DRIVE_HUNGER:
        local_being->changes.drives[DRIVE_HUNGER] = ( n_byte ) value;
        break;
    case VARIABLE_DRIVE_SOCIAL:
        local_being->changes.drives[DRIVE_SOCIAL] = ( n_byte ) value;
        break;
    case VARIABLE_DRIVE_FATIGUE:
        local_being->changes.drives[DRIVE_FATIGUE] = ( n_byte ) value;
        break;
    case VARIABLE_DRIVE_SEX:
        local_being->changes.drives[DRIVE_SEX] = ( n_byte ) value;
        break;

    case VARIABLE_PREFERENCE_MATE_HEIGHT_MALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_HEIGHT_MALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_MATE_HEIGHT_FEMALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_HEIGHT_FEMALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_MATE_PIGMENTATION_MALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_PIGMENTATION_MALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_MATE_PIGMENTATION_FEMALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_PIGMENTATION_FEMALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_MATE_HAIR_MALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_HAIR_MALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_MATE_HAIR_FEMALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_HAIR_FEMALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_MATE_FRAME_MALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_FRAME_MALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_MATE_FRAME_FEMALE:
        local_being->changes.learned_preference[PREFERENCE_MATE_FRAME_FEMALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_GROOM_MALE:
        local_being->changes.learned_preference[PREFERENCE_GROOM_MALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_GROOM_FEMALE:
        local_being->changes.learned_preference[PREFERENCE_GROOM_FEMALE] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_ANECDOTE_EVENT_MUTATION:
        local_being->changes.learned_preference[PREFERENCE_ANECDOTE_EVENT_MUTATION] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_ANECDOTE_AFFECT_MUTATION:
        local_being->changes.learned_preference[PREFERENCE_ANECDOTE_AFFECT_MUTATION] = ( n_byte ) value;
        break;
    case VARIABLE_PREFERENCE_CHAT:
        local_being->changes.learned_preference[PREFERENCE_CHAT] = ( n_byte ) value;
        break;
    case VARIABLE_ATTENTION_ACTOR_INDEX:
        being_set_attention( local_being, ATTENTION_ACTOR, value % SOCIAL_SIZE );
        break;
    case VARIABLE_ATTENTION_EPISODE_INDEX:
        being_set_attention( local_being, ATTENTION_EPISODE, value % EPISODIC_SIZE );
        break;
    case VARIABLE_ATTENTION_BODY_INDEX:
        being_set_attention( local_being, ATTENTION_BODY, value % INVENTORY_SIZE );
        break;
    }

    if ( kind > VARIABLE_BRAIN_VALUE )
    {
        local_vr[kind - VARIABLE_VECT_ANGLE] = value;
        return 0;
    }
    return -1; /* where this fails is more important than this failure */
}


static n_int sim_output( void *vcode, void *vindividual, n_byte *kind, n_int *number )
{
    simulated_group *group = sim_group();

    n_interpret *code = ( n_interpret * ) vcode;
    n_individual_interpret *individual = ( n_individual_interpret * ) vindividual;
    n_byte    first_value = kind[0];
    n_byte    second_value = kind[1];
    if ( first_value == APESCRIPT_NUMBER )
    {
        *number = code->number_buffer[second_value];
        return 0;
    }
    if ( ( first_value == APESCRIPT_TEXT ) && ( VARIABLE_SPECIAL( second_value, code ) == 0 ) )
    {
        n_int    *local_vr = individual->variable_references;

        if ( ( second_value >= VARIABLE_BIOLOGY_AREA ) && ( second_value <= VARIABLE_BIOLOGY_EAGLE ) )
        {
            *number = second_value - VARIABLE_BIOLOGY_AREA;
            return 0;
        }

        if ( second_value > VARIABLE_BRAIN_VALUE )
        {
            *number = local_vr[ second_value - VARIABLE_VECT_ANGLE ];
            return 0;
        }

        {
            simulated_being *local_current = ( simulated_being * )individual->interpret_data;
            n_int          local_number = 0;
            n_vect2       local_vector;
            vect2_direction( &local_vector, local_vr[0], 32 );
            switch ( second_value )
            {

            case VARIABLE_VECT_X:
                local_number = local_vector.x;
                break;

            case VARIABLE_VECT_Y:
                local_number = local_vector.y;
                break;
            case VARIABLE_RANDOM:
                local_number = being_random( local_current );
                break;
            case VARIABLE_WATER_LEVEL:
                local_number = land_tide_level();
                break;
            case VARIABLE_HUNGRY:
                local_number = BEING_HUNGRY;
                break;
            case VARIABLE_ENERGY:
            {
                simulated_being *local_being = ( simulated_being * )individual->interpret_data;
                local_number = being_energy( local_being );
            }
            break;
            case VARIABLE_LOCATION_Z:
            case VARIABLE_TEST_Z:
            case VARIABLE_IS_VISIBLE:
            case VARIABLE_BIOLOGY_OUTPUT:
            {
                n_int    quick_x;
                n_int    quick_y;

                if ( second_value == VARIABLE_LOCATION_Z )
                {
                    quick_x = being_location_x( local_current );
                    quick_y = being_location_y( local_current );
                }
                else
                {
                    quick_x = local_vr[VARIABLE_TEST_X - VARIABLE_VECT_ANGLE];
                    quick_y = local_vr[VARIABLE_TEST_Y - VARIABLE_VECT_ANGLE];
                    if ( ( quick_x < 0 ) || ( quick_y < 0 ) ||
                            ( quick_x > APESPACE_BOUNDS ) || ( quick_y > APESPACE_BOUNDS ) )
                    {
                        return APESCRIPT_ERROR( individual, AE_COORDINATES_OUT_OF_RANGE );
                    }
                }

                if ( second_value == VARIABLE_IS_VISIBLE )
                {
                    /* range already checked */
                    simulated_being *local_being = ( simulated_being * )individual->interpret_data;
                    n_vect2 location;
                    location.x = ( n_byte2 )quick_x;
                    location.y = ( n_byte2 )quick_y;
                    local_number = being_line_of_sight( local_being, &location );
                }
                else
                {
                    if ( second_value == VARIABLE_BIOLOGY_OUTPUT )
                    {
                        n_int    int_qu_op = local_vr[VARIABLE_BIOLOGY_OPERATOR - VARIABLE_VECT_ANGLE];
                        if ( ( int_qu_op < 0 ) || ( int_qu_op > ( VARIABLE_BIOLOGY_EAGLE - VARIABLE_BIOLOGY_AREA ) ) )
                        {
                            return APESCRIPT_ERROR( individual, AE_VALUE_OUT_OF_RANGE );
                        }
                        local_number = land_operator_interpolated(
                                           ( n_byte )quick_x, ( n_byte )quick_y, ( n_byte * )&operators[int_qu_op - VARIABLE_BIOLOGY_AREA] );
                    }
                    else
                    {
                        local_number = land_location( quick_x, quick_y );
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
                local_number = being_index( group, ( simulated_being * )individual->interpret_data );
                break;
            case VARIABLE_NUMBER_BEINGS:
                local_number = ( n_int )group->num;
                break;

            case VARIABLE_IS_ERROR:
                local_number = -1;
                break;

            case VARIABLE_WEATHER:
            {
                n_int    quick_x;
                n_int    quick_y;

                quick_x = local_vr[VARIABLE_TEST_X - VARIABLE_VECT_ANGLE];
                quick_y = local_vr[VARIABLE_TEST_Y - VARIABLE_VECT_ANGLE];
                if ( ( quick_x < 0 ) || ( quick_y < 0 ) ||
                        ( quick_x > APESPACE_BOUNDS ) || ( quick_y > APESPACE_BOUNDS ) )
                {
                    return APESCRIPT_ERROR( individual, AE_COORDINATES_OUT_OF_RANGE );
                }

                local_number = weather_seven_values( quick_x, quick_y );
            }
            break;
            case VARIABLE_BRAIN_VALUE:
            {
#ifdef BRAIN_ON
                n_int    current_x = local_vr[VARIABLE_BRAIN_X - VARIABLE_VECT_ANGLE];
                n_int    current_y = local_vr[VARIABLE_BRAIN_Y - VARIABLE_VECT_ANGLE];
                n_int    current_z = local_vr[VARIABLE_BRAIN_Z - VARIABLE_VECT_ANGLE];

                if ( ( current_x < 0 ) || ( current_y < 0 ) || ( current_z < 0 ) ||
                        ( current_x > 31 ) || ( current_y > 31 ) || ( current_z > 31 ) )
                {
                    return APESCRIPT_ERROR( individual, AE_COORDINATES_OUT_OF_RANGE );
                }
                {
                    simulated_being *local_being = ( simulated_being * )individual->interpret_data;
                    n_byte *local_brain = being_brain( local_being );
                    if ( local_brain != 0L )
                    {
                        local_number = TRACK_BRAIN( local_brain, current_x, current_y, current_z );
                    }
                }
#endif
            }
            break;
            default:
            {
                n_int             temp_select = local_vr[ VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE ];
                simulated_being        *local_being = 0L;
                simulated_isocial    *local_social_graph = 0L;
                simulated_isocial     social_graph;
                n_vect2          location_vect;
#ifdef EPISODIC_ON
                simulated_iepisodic    *local_episodic = 0L;
                simulated_iepisodic    episodic;
#endif

                if ( temp_select < 0 )
                {
                    return APESCRIPT_ERROR( individual, AE_SELECTED_ENTITY_OUT_OF_RANGE );
                }
                {
                    n_uint local_select = ( n_uint )temp_select;
                    if ( local_select >= group->num )
                    {
                        return APESCRIPT_ERROR( individual, AE_SELECTED_ENTITY_OUT_OF_RANGE );
                    }
                    local_being = &( group->beings[local_select] );
                    if ( local_being != 0L )
                    {
                        local_social_graph = being_social( local_being );
                        if ( local_social_graph != 0L )
                        {
                            social_graph = local_social_graph[being_attention( local_being, ATTENTION_ACTOR )];
                        }
#ifdef EPISODIC_ON
                        local_episodic = being_episodic( local_being );
                        if ( local_episodic != 0L )
                        {
                            episodic = local_episodic[being_attention( local_being, ATTENTION_EPISODE )];
                        }
#endif
                    }
                }
                /* TODO: if the being knows the other being it may be possible to guess some of these */

                /* if the current being can't see the other being, it can't get this information */

                being_space( local_being, &location_vect );

                if ( being_line_of_sight( local_current, &location_vect ) == 0 )
                {
                    local_number = -1;
                }
                else if ( ( local_being != 0L ) && ( local_social_graph != 0L ) )
                {
                    switch ( second_value )
                    {

                    case VARIABLE_HONOR:
                        local_number = being_honor( local_being );
                        break;
                    case VARIABLE_PARASITES:
                        local_number = being_parasites( local_being );
                        break;
                    case VARIABLE_HEIGHT:
                        local_number = being_height( local_being );
                        break;
                    case VARIABLE_FIRST_NAME:
                        local_number = being_gender_name( local_being );
                        break;
#if 0 /* TODO: This should not be done */
                    case VARIABLE_FAMILY_NAME_ONE:
                        local_number = UNPACK_FAMILY_FIRST_NAME( being_family_name( local_being ) );
                        break;
                    case VARIABLE_FAMILY_NAME_TWO:
                        local_number = UNPACK_FAMILY_SECOND_NAME( being_family_name( local_being ) );
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
                        local_number = being_posture( local_being );
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
                        local_number = being_location_x( local_being );
                        break;

                    case    VARIABLE_LOCATION_Y:
                        local_number = being_location_y( local_being );
                        break;

                    case    VARIABLE_ID_NUMBER:
                        local_number = GET_I( local_being );
                        break;

                    case    VARIABLE_DATE_OF_BIRTH:
                        local_number = being_dob( local_being );
                        break;
                    case    VARIABLE_STATE:
                        local_number = being_state( local_being );
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
                        local_number = being_attention( local_being, ATTENTION_ACTOR );
                        break;
                    case    VARIABLE_ATTENTION_EPISODE_INDEX:
                        local_number = being_attention( local_being, ATTENTION_EPISODE );
                        break;
                    case    VARIABLE_ATTENTION_BODY_INDEX:
                        local_number = being_attention( local_being, ATTENTION_BODY );
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
                            ( n_int )social_graph.friend_foe -
                            ( n_int )social_respect_mean( local_being );
                        break;
                    case VARIABLE_SOCIAL_GRAPH_FAMILIARITY:
                        local_number = social_graph.familiarity;
                        break;
                    case VARIABLE_MEMORY_FIRST_NAME:
                        local_number = social_graph.first_name[BEING_MET];
                        break;
#if 0 /* TODO: This should not be done */
                    case VARIABLE_MEMORY_FAMILY_NAME_ONE:
                        local_number = UNPACK_FAMILY_FIRST_NAME( social_graph.family_name[BEING_MET] );
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_TWO:
                        local_number = UNPACK_FAMILY_SECOND_NAME( social_graph.family_name[BEING_MET] );
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
                        local_number = UNPACK_FAMILY_FIRST_NAME( episodic.family_name[0] );
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_TWO0:
                        local_number = UNPACK_FAMILY_SECOND_NAME( episodic.family_name[0] );
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_ONE1:
                        local_number = UNPACK_FAMILY_FIRST_NAME( episodic.family_name[BEING_MET] );
                        break;
                    case VARIABLE_MEMORY_FAMILY_NAME_TWO1:
                        local_number = UNPACK_FAMILY_SECOND_NAME( episodic.family_name[BEING_MET] );
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


n_int     sim_interpret( n_file *input_file )
{
    input_file->size = input_file->location;
    input_file->location = 0;

    interpret = parse_convert( input_file, VARIABLE_BEING, ( variable_string * )apescript_variable_codes );

    if ( interpret == 0L )
    {
        return -1;
    }
    else
    {
        SC_DEBUG_ON( group.select ); /* turn on debugging after script loading */
    }

    interpret->sc_input  = &sim_input;
    interpret->sc_output = &sim_output;

    interpret->input_greater   = VARIABLE_WEATHER;
    interpret->special_less    = VARIABLE_VECT_X;
    return 0;
}

#ifdef BRAIN_ON
static void sim_brain_loop( simulated_group *group, simulated_being *local_being, void *data )
{
    n_byte2 local_brain_state[3];

    if ( being_brainstates( local_being, ( local_being->delta.awake == 0 ), local_brain_state ) )
    {
        n_byte            *local_brain = being_brain( local_being );
        if ( local_brain != 0L )
        {
            being_brain_cycle( local_brain, local_brain_state );
        }
    }
}
#endif


#ifdef BRAINCODE_ON

static void sim_brain_dialogue_loop( simulated_group *group, simulated_being *local_being, void *data )
{
    n_byte     awake = 1;
    n_byte    *local_internal = being_braincode_internal( local_being );
    n_byte    *local_external = being_braincode_external( local_being );
    if ( local_being->delta.awake == 0 )
    {
        awake = 0;
    }
    /* This should be independent of the brainstate/cognitive simulation code */
    brain_dialogue( group, awake, local_being, local_being, local_internal, local_external, being_random( local_being ) % SOCIAL_SIZE );
    brain_dialogue( group, awake, local_being, local_being, local_external, local_internal, being_random( local_being ) % SOCIAL_SIZE );
}

#endif


static void sim_being_awake_loop_no_sim( simulated_being *local_being )
{
    n_byte awake_condition = being_awake( local_being );
    local_being->delta.awake = awake_condition;

#ifdef DEBUG_LACK_OF_MOVEMENT
    being_register_movement( local_being, "awake condition" );
#endif
}

static void sim_being_cycle( simulated_group *group, simulated_being *local_being, void *data )
{
    if ( local_being->delta.awake == 0 )
    {
        return;
    }

    being_cycle_awake( group, local_being );
}

static void sim_start_conditions( void *vindividual, void *structure, void *data )
{
    n_individual_interpret *individual = ( n_individual_interpret * )vindividual;
    n_int        *variables = individual->variable_references;
    simulated_being *local_being = ( simulated_being * )data;

    variables[VARIABLE_FACING - VARIABLE_VECT_ANGLE] = being_facing( local_being );
    variables[VARIABLE_SPEED - VARIABLE_VECT_ANGLE] =  being_speed( local_being );
    variables[VARIABLE_ENERGY_DELTA - VARIABLE_VECT_ANGLE] = 0;

    variables[VARIABLE_SELECT_BEING - VARIABLE_VECT_ANGLE] = being_index( sim_group(), local_being );

    variables[VARIABLE_HEIGHT - VARIABLE_VECT_ANGLE] = being_height( local_being );
    variables[VARIABLE_GOAL_TYPE - VARIABLE_VECT_ANGLE] = local_being->delta.goal[0];
    variables[VARIABLE_GOAL_X - VARIABLE_VECT_ANGLE] = local_being->delta.goal[1];
    variables[VARIABLE_GOAL_Y - VARIABLE_VECT_ANGLE] = local_being->delta.goal[2];
    variables[VARIABLE_DRIVE_HUNGER - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_HUNGER];
    variables[VARIABLE_DRIVE_SOCIAL - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_SOCIAL];
    variables[VARIABLE_DRIVE_FATIGUE - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_FATIGUE];
    variables[VARIABLE_DRIVE_SEX - VARIABLE_VECT_ANGLE] = local_being->changes.drives[DRIVE_SEX];
    variables[VARIABLE_FAMILY_NAME_ONE - VARIABLE_VECT_ANGLE] = being_family_first_name( local_being );
    variables[VARIABLE_FAMILY_NAME_TWO - VARIABLE_VECT_ANGLE] = being_family_second_name( local_being );
    variables[VARIABLE_HONOR - VARIABLE_VECT_ANGLE] = being_honor( local_being );
    variables[VARIABLE_PARASITES - VARIABLE_VECT_ANGLE] = being_parasites( local_being );
}

static void sim_end_conditions( void *vindividual, void *structure, void *data )
{
    n_individual_interpret *individual = ( n_individual_interpret * )vindividual;
    n_int        *variables = individual->variable_references;
    simulated_being *local_being = ( simulated_being * )data;

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

    if ( local_facing < 0 )
    {
        local_facing = 255 - ( ( 0 - local_facing ) & 255 );
    }
    else
    {
        local_facing = local_facing & 255;
    }

    if ( local_speed > 39 )
    {
        local_speed = 39;
    }
    if ( local_speed < 0 )
    {
        local_speed = 0;
    }

    being_wander( local_being, local_facing - being_facing( local_being ) );

    being_set_speed( local_being, ( n_byte ) local_speed );
    being_energy_delta( local_being, local_energy_delta );

    being_set_height( local_being, local_height );

    if ( local_goal_type != GOAL_NONE )
    {
        local_being->delta.goal[0]  = ( n_byte2 )local_goal_type;
        local_being->delta.goal[1]  = ( n_byte2 )local_goal_x;
        local_being->delta.goal[2]  = ( n_byte2 )local_goal_y;
        local_being->delta.goal[3]  = GOAL_TIMEOUT;

        local_being->braindata.script_overrides |= OVERRIDE_GOAL;
    }
    if ( local_drive_hunger > -1 )
    {
        local_being->changes.drives[DRIVE_HUNGER]  = ( n_byte )local_drive_hunger;
    }
    if ( local_drive_social > -1 )
    {
        local_being->changes.drives[DRIVE_SOCIAL]  = ( n_byte )local_drive_social;
    }
    if ( local_drive_fatigue > -1 )
    {
        local_being->changes.drives[DRIVE_FATIGUE]  = ( n_byte )local_drive_fatigue;
    }
    if ( local_drive_sex > -1 )
    {
        local_being->changes.drives[DRIVE_SEX]  = ( n_byte )local_drive_sex;
    }
    being_set_family_name( local_being, ( n_byte )local_family_name1, ( n_byte )local_family_name2 );

    local_being->delta.honor = ( n_byte )local_honor;
    being_set_parasites( local_being, ( n_byte )local_parasites );
}



static void sim_being_interpret( simulated_group *group, simulated_being *local_being, void *data )
{
    n_individual_interpret individual;

    interpret_individual( &individual );

    if ( local_being->delta.awake == 0 )
    {
        return;
    }

    if ( interpret == 0L )
    {
        return;
    }

    if ( interpret_cycle( interpret, &individual, -1,
                          group->beings, local_being,
                          &sim_start_conditions, &sim_end_conditions ) == -1 )
    {
        interpret_cleanup( &interpret );
    }
}


static void sim_time( simulated_group *group )
{
    simulated_timing *timing = sim_timing();

    timing->count_cycles += group->num;

    timing->count_frames ++;

    if ( ( timing->real_time - timing->last_time ) > 60 )
    {
        timing->last_time = timing->real_time;
        timing->delta_cycles = timing->count_cycles;
        timing->delta_frames = timing->count_frames;
        timing->count_cycles = 0;
        timing->count_frames = 0;
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

static void sim_being_remove_final( simulated_group *group, being_remove_loop2_struct **brls )
{
    group->num = ( *brls )->count;
    if ( ( *brls )->selected_died )
    {
        if ( ( *brls )->count )
        {
            sim_set_select( group->beings );
        }
        else
        {
            sim_set_select( 0L );
        }
    }

    if ( ( *brls )->count == 0 )
    {
        if ( sim_new_run == 0 )
        {
            ( void )SHOW_ERROR( "No Apes remain start new run" );
            sim_new_run = 1;
        }
    }
    being_remove_internal_clear();
    memory_free( ( void ** )brls );
}


void sim_update_output( void )
{
    if ( sim_desire_output == 0 )
    {
        return;
    }

    if ( group.select == 0L )
    {
        return;
    }
    sim_writing_output = 1;
    memory_erase( ( n_byte * )sim_console_output, STRING_BLOCK_SIZE );
    watch_control( &group, being_get_select_name( &group ), group.select, sim_console_output );
    sim_writing_output = 0;
}

static KIND_OF_USE local_execution = KIND_PRE_STARTUP;

void sim_cycle( void )
{
    if ( local_execution == KIND_PRE_STARTUP )
    {
        return;
    }
    if ( local_execution != KIND_NOTHING_TO_RUN )
    {
        if ( local_execution != KIND_MEMORY_SETUP )
        {
            if ( local_execution != KIND_NEW_APES )
            {
                land_clear( local_execution, AGE_OF_MATURITY );
#ifdef LAND_ON
                land_init();
                land_init_high_def( 1 );
                land_tide();
#endif
            }
            if ( local_execution != KIND_LOAD_FILE )
            {
                n_byte2 local_random[2];
                n_byte2 *genetics = land_genetics();
                local_random[0] = genetics[0];
                local_random[1] = genetics[1];

                math_random3( local_random );

#ifdef WEATHER_ON
                weather_init();
#endif
                /* Sets the number of Simulated Apes initially created, and creates them */
                group.num = being_init_group( group.beings, local_random, group.max >> 1, group.max );
            }
        }

        sim_set_select( group.beings );

        sim_new_progress = 0;
        local_execution = KIND_NOTHING_TO_RUN;
    }


    n_int       max_honor = 0;

    land_cycle();
#ifdef WEATHER_ON
    weather_cycle();
#endif

    loop_being_no_sim_no_data( group.beings, group.num, sim_being_awake_loop_no_sim );
    loop_being_no_sim_no_data( group.beings, group.num, being_cycle_universal );

    if ( interpret )
    {
        loop_being( &group, sim_being_interpret, PROCESSING_WELTER_WEIGHT );
    }
    else
    {
        /** Listen for any shouts */
        loop_being( &group, being_listen, PROCESSING_FEATHER_WEIGHT );
#ifdef EPISODIC_ON
        loop_being_no_sim_no_data( group.beings, group.num, episodic_cycle_no_sim );
#endif
        loop_being( &group, sim_being_cycle, PROCESSING_MIDDLE_WEIGHT );
        loop_being( &group, drives_cycle, PROCESSING_LIGHT_WEIGHT );
    }

    if ( land_time() & 1 )
    {
#ifdef BRAIN_ON
        loop_being( &group, sim_brain_loop, PROCESSING_WELTER_WEIGHT );
#endif
    }
#ifdef BRAINCODE_ON
    else
    {
        loop_being( &group, sim_brain_dialogue_loop, PROCESSING_MIDDLE_WEIGHT );
    }
#endif

    loop_being_no_sim( group.beings, group.num, being_tidy_loop_no_sim, &max_honor );

    loop_being( &group, social_initial_loop, PROCESSING_LIGHT_WEIGHT );

    if ( max_honor )
    {
        loop_being_no_sim_no_data( group.beings, group.num, being_recalibrate_honor_loop_no_sim );
    }

    loop_being_no_sim_no_data( group.beings, group.num, social_secondary_loop_no_sim );

    {
        n_string_block selected_name = {0};
        n_int          selected_lives = 1;
        being_remove_loop2_struct *brls = being_remove_initial( &group );
        if ( group.select )
        {
            being_name_simple( group.select, selected_name );
        }
        if ( group.ext_death != 0L )
        {
            loop_no_thread( &group, 0L, being_remove_loop1, 0L );
        }
        loop_no_thread( &group, 0L, being_remove_loop2, brls );
        selected_lives = brls->selected_died == 0;
        sim_being_remove_final( &group, &brls );

        loop_being_no_sim_no_data( group.beings, group.num, being_speed_advance );

        if ( selected_lives )
        {
            simulated_being *new_select = sim_select_name( selected_name );
            if ( new_select != group.select )
            {
                sim_set_select( new_select );
            }
        }
    }

    sim_time( &group );
}

#define    MINIMAL_ALLOCATION    ((512*512)+(TERRAIN_WINDOW_AREA)+(CONTROL_WINDOW_AREA)+(sizeof(simulated_being) * MIN_BEINGS)+sizeof(simulated_remains)+1)

#define MAXIMUM_ALLOCATION  (MINIMAL_ALLOCATION + (sizeof(simulated_being) * 400))

n_uint sim_memory_allocated( n_int max )
{
    if ( max )
    {
        return MAXIMUM_ALLOCATION;
    }
    else
    {
        return initial_memory_allocated;
    }
}

static n_int being_memory( simulated_group *group, n_byte *buffer, n_uint *location, n_uint memory_available )
{
    n_uint  lpx = 0;

    group->max = LARGE_SIM;

    while ( lpx < group->max )
    {
        simulated_being *local_being = &( group->beings[ lpx ] );
        memory_erase( ( n_byte * )local_being, sizeof( simulated_being ) );
        lpx ++;
    }
    return 0;
}

static n_int sim_memory( n_uint offscreen_size )
{
    n_uint    current_location = 0;
    n_uint  memory_allocated = MAXIMUM_ALLOCATION;

    offbuffer = memory_new_range( offscreen_size + MINIMAL_ALLOCATION, &memory_allocated );

    if ( offbuffer == 0L )
    {
        return SHOW_ERROR( "Memory not available" );
    }

    memory_erase( offbuffer, memory_allocated );
    memory_erase( ( void * )&group.remains, sizeof( simulated_remains ) );

    return being_memory( &group, offbuffer, &current_location, memory_allocated );
}

static void debug_birth_event( simulated_being *born, simulated_being *mother, void *sim )
{
    n_string_block name, mother_name, father_name;
    being_name_simple( born, name );
    being_name_simple( mother, mother_name );
    being_name_byte2( mother->changes.father_name[0], mother->changes.father_name[1], father_name );
    printf( "*** Born: %s (Mother: %s Father: %s)\n", name, mother_name, father_name );
}

static void debug_death_event( simulated_being *deceased, void *sim )
{
    n_string_block name;
    being_name_simple( deceased, name );
    printf( "*** Dead: %s\n", name );
}

void *sim_init( KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size )
{
    n_byte2    local_random[2];

    sim_writing_output = 1;
    sim_new_progress = 1;

    if ( kind == KIND_NEW_SIMULATION )
    {
        if ( interpret )
        {
            interpret_cleanup( &interpret );
            interpret = 0L;
        }
        memory_execute(io_command_line_execution_set);
    }
    timing.real_time = randomise;
    timing.last_time = randomise;

#ifdef FIXED_RANDOM_SIM
    randomise = FIXED_RANDOM_SIM;
#endif

    timing.delta_cycles = 0;
    timing.count_cycles = 0;
    timing.delta_frames = 0;
    timing.count_frames = 0;

#if 1
    group.ext_birth = &debug_birth_event;
    group.ext_death = &debug_death_event;
#else
    group.ext_birth = 0L;
    group.ext_death = 0L;
#endif

    if ( ( kind == KIND_START_UP ) || ( kind == KIND_MEMORY_SETUP ) )
    {
        if ( sim_memory( offscreen_size ) != 0 )
        {
            return 0L;
        }
    }

    local_random[0] = ( n_byte2 )( randomise >> 16 ) & 0xffff;
    local_random[1] = ( n_byte2 )( randomise & 0xffff );

    math_random3( local_random );

    if ( ( kind != KIND_LOAD_FILE ) && ( kind != KIND_MEMORY_SETUP ) )
    {
        land_seed_genetics( local_random );
    }

    being_remains_init( &( group.remains ) ); /* Eventually this should be captured through the file handling and moved into the code below */

    local_execution = kind;
    return ( ( void * ) offbuffer );
}

void sim_close( void )
{
    command_quit( 0L, 0L, 0L );
    io_console_quit();
#ifndef _WIN32
    sim_console_clean_up();
#endif
    interpret_cleanup( &interpret );
    memory_free( ( void ** ) &offbuffer );
    /*death_record_file_cleanup();*/
}

void sim_set_select( simulated_being *select )
{
    group.select = select;
}

static void sim_flood_loop( simulated_group *group, simulated_being *local, void *data )
{
    n_vect2 location;

    being_space( local, &location );
    spacetime_convert_to_map( &location );

    if ( land_location_vect( &location ) < 160 )
    {
        being_dead( local );
    }
}

void sim_flood( void )
{
    loop_no_thread( &group, 0L, sim_flood_loop, 0L );
}

void sim_healthy_carrier( void )
{
    n_uint  loop = ( group.num >> 2 );

    while ( loop < group.num )
    {
        simulated_being *local = &group.beings[loop];
        being_dead( local );
        loop++;
    }
}
