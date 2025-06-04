/****************************************************************

 command.c

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

#define _CRT_SECURE_NO_WARNINGS

#define REMOVE_BOLDING_TEXT

#include "../entity/entity.h"

#include "universe_internal.h"

#include <stdio.h>

static n_int simulation_running = 1;
static n_int simulation_executing = 0;

const n_string RUN_STEP_CONST = "RSC";

/** The type of watch */
static n_int watch_type = WATCH_NONE;

/** keeps a running count of the total string length */
static n_int watch_string_length = 0;

/** enable or disable logging to file */
static n_int nolog = 0;

/** number of steps to periodically log simulation indicators */
static n_int indicator_index = 1;

/** How many steps at which to periodically save the simulation */
static n_uint save_interval_steps = 60;

static n_int          command_file_interaction = 0;
static n_string_block command_file_name;


static void command_offset( n_byte *start, n_byte *point, n_string text )
{
    printf( "%s %ld\n", text, ( n_int )( point - start ) );
}

#define FILE_CHECK(value) command_offset((n_byte*)&here, (n_byte*)value, #value)

static void command_audit( void )
{
    {
        simulated_being    here;

        FILE_CHECK( &here.delta.location[0] );
        FILE_CHECK( &here.delta.direction_facing );
        FILE_CHECK( &here.delta.velocity );
        FILE_CHECK( &here.delta.stored_energy );
        FILE_CHECK( &here.constant.date_of_birth );
        FILE_CHECK( &here.delta.random_seed[0] );
        FILE_CHECK( &here.delta.macro_state );
        FILE_CHECK( &here.braindata.brain_state[0] );
        FILE_CHECK( &here.delta.height );
        FILE_CHECK( &here.delta.mass );
        FILE_CHECK( &here.braindata.script_overrides );
        FILE_CHECK( &here.changes.shout[0] );
        FILE_CHECK( &here.delta.crowding );
        FILE_CHECK( &here.delta.posture );
        FILE_CHECK( &here.changes.inventory[0] );

        FILE_CHECK( &here.delta.parasites );
        FILE_CHECK( &here.delta.honor );

        FILE_CHECK( &here.changes.date_of_conception ); /* constant */
        FILE_CHECK( &here.braindata.attention[0] );
        FILE_CHECK( &here.constant.genetics[0] );         /* constant */
        FILE_CHECK( &here.changes.fetal_genetics[0] );         /* constant */
        FILE_CHECK( &here.changes.father_name[0] );                /* why is this needed? */
        FILE_CHECK( &here.delta.social_coord_x );
        FILE_CHECK( &here.delta.social_coord_y );
        FILE_CHECK( &here.delta.social_coord_nx ); /* why is this needed? */
        FILE_CHECK( &here.delta.social_coord_ny ); /* why is this needed? */
        FILE_CHECK( &here.changes.drives[0] );
        FILE_CHECK( &here.delta.goal[0] );
        FILE_CHECK( &here.changes.learned_preference[0] );
        FILE_CHECK( &here.constant.generation_min );
        FILE_CHECK( &here.constant.generation_max );
        FILE_CHECK( &here.changes.child_generation_min );
        FILE_CHECK( &here.changes.child_generation_max );
#ifdef TERRITORY_ON
        FILE_CHECK( &here.events.territory[0] );
#endif
#ifdef IMMUNE_ON
        FILE_CHECK( &here.immune_system );
#endif
#ifdef BRAINCODE_ON
        FILE_CHECK( &here.braindata.braincode_register[0] );
        FILE_CHECK( &here.braindata.brainprobe[0] );
#endif
#ifdef BRAIN_ON
        FILE_CHECK( &here.braindata.brain[0] );
#endif
        FILE_CHECK( &here.events.social[0] );
        FILE_CHECK( &here.events.episodic[0] );
    }
    {
        simulated_isocial here;

        FILE_CHECK( &here.space_time.location[0] );
        FILE_CHECK( &here.space_time.time );
        FILE_CHECK( &here.space_time.date );
        FILE_CHECK( &here.first_name[0] );
        FILE_CHECK( &here.family_name[0] );

        FILE_CHECK( &here.attraction );
        FILE_CHECK( &here.friend_foe );
        FILE_CHECK( &here.belief );
        FILE_CHECK( &here.familiarity );
        FILE_CHECK( &here.relationship );
        FILE_CHECK( &here.entity_type );

#ifdef FEATURE_SET
        FILE_CHECK( &here.classification );
#endif

#ifdef BRAINCODE_ON
        FILE_CHECK( &here.braincode[0] );
#endif

    }
    {
        simulated_iepisodic here;

        FILE_CHECK( &here.space_time.location[0] );
        FILE_CHECK( &here.space_time.time );
        FILE_CHECK( &here.space_time.date );
        FILE_CHECK( &here.first_name[0] );
        FILE_CHECK( &here.family_name[0] );
        FILE_CHECK( &here.event );
        FILE_CHECK( &here.food );
        FILE_CHECK( &here.affect );
        FILE_CHECK( &here.arg );
    }
}

n_int command_executing( void )
{
    return simulation_executing;
}

/**
 *
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function the output function
 */
n_int command_being( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;
    output_function( being_get_select_name( group ) );
    return 0;
}

static n_byte2 command_friends_first[SOCIAL_SIZE_BEINGS];
static n_byte2 command_friends_family[SOCIAL_SIZE_BEINGS];
static n_int   command_friends_count;

static n_byte2 command_enemies_first[SOCIAL_SIZE_BEINGS];
static n_byte2 command_enemies_family[SOCIAL_SIZE_BEINGS];
static n_int   command_enemies_count;

static n_byte2 command_attract_first[SOCIAL_SIZE_BEINGS];
static n_byte2 command_attract_family[SOCIAL_SIZE_BEINGS];
static n_int   command_attract_count;

n_int command_relationship_count( n_int friend_type )
{
    switch ( friend_type )
    {
    case 0:   /**< friends */
        return command_friends_count;
    case 1:   /**< enemies */
        return command_enemies_count;
    case 2:   /**< attraction */
        return command_attract_count;
    }
    return 0;
}

simulated_being *command_relationship_being( simulated_group *group, n_int friend_type, n_int location )
{
    n_byte2 first_name_look_up;
    n_byte2 family_name_look_up;
    n_uint loop = 0;
    switch ( friend_type )
    {
    case 0:   /**< friends */
        first_name_look_up = command_friends_first[location];
        family_name_look_up = command_friends_family[location];
        break;
    case 1:   /**< enemies */
        first_name_look_up = command_enemies_first[location];
        family_name_look_up = command_enemies_family[location];
        break;
    case 2:   /**< attraction */
        first_name_look_up = command_friends_first[location];
        family_name_look_up = command_friends_family[location];
        break;
    default:
        return 0L;
    }
    while ( loop < group->num )
    {
        simulated_being *local = &group->beings[loop];
        if ( ( being_gender_name( local ) == first_name_look_up ) && ( being_family_name( local ) == family_name_look_up ) )
        {
            return local;
        }
        loop++;
    }
    return 0L;
}

static void command_show_friends_being( void *ptr, simulated_being *local_being,
                                        n_int friend_type, n_string result, n_string four_characters )
{
    simulated_group *group = ( simulated_group * ) ptr;
    n_int i, found, attraction;
    simulated_isocial *local_social_graph;
    n_int first = 1;

    if ( local_being == 0 )
    {
        return;
    }

    /** Get the social graph for the being */
    local_social_graph = being_social( local_being );

    if ( local_social_graph == 0L )
    {
        return;
    }

    switch ( friend_type )
    {
    case 0:   /**< friends */
        command_friends_count = 0;
        break;
    case 1:   /**< enemies */
        command_enemies_count = 0;
        break;
    case 2:   /**< attraction */
        command_attract_count = 0;
        break;
    }
    /** For each entry in the social graph */
    for ( i = 1; i < SOCIAL_SIZE_BEINGS; i++ )
    {
        /** Skip empty entries */
        if ( SOCIAL_GRAPH_ENTRY_EMPTY( local_social_graph, i ) )
        {
            continue;
        }

        found = 0;
        attraction = 0;
        switch ( friend_type )
        {
        case 0:   /**< friends */
        {
            if ( local_social_graph[i].friend_foe >= social_respect_mean( local_being ) )
            {
                if ( local_social_graph[i].entity_type == ENTITY_BEING )
                {
                    if ( local_social_graph[i].attraction > 0 )
                    {
                        attraction = 1;

                    }
                    command_friends_first[command_friends_count] = local_social_graph[i].first_name[BEING_MET];
                    command_friends_family[command_friends_count] = local_social_graph[i].family_name[BEING_MET];
                    command_friends_count++;
                }
                found = 1;
            }
            break;
        }
        case 1:   /**< enemies */
        {
            if ( local_social_graph[i].friend_foe < social_respect_mean( local_being ) )
            {
                if ( local_social_graph[i].entity_type == ENTITY_BEING )
                {
                    if ( local_social_graph[i].attraction > 0 )
                    {
                        attraction = 1;
                    }
                    command_enemies_first[command_enemies_count] = local_social_graph[i].first_name[BEING_MET];
                    command_enemies_family[command_enemies_count] = local_social_graph[i].family_name[BEING_MET];
                    command_enemies_count++;
                }
                found = 1;
            }
            break;
        }
        case 2:   /**< attraction */
        {
            if ( local_social_graph[i].attraction > 0 )
            {
                if ( local_social_graph[i].entity_type == ENTITY_BEING )
                {
                    command_attract_first[command_attract_count] = local_social_graph[i].first_name[BEING_MET];
                    command_attract_family[command_attract_count] = local_social_graph[i].family_name[BEING_MET];
                    command_attract_count++;
                }
                found = 1;
            }
            break;
        }
        }

        if ( found == 1 )
        {
            n_int relationship_index;
            n_string_block relationship_str1;
            n_string_block relationship_str2;
            n_string_block met_being_name;
            n_string_block result_str;

            /** Print the name and familiarity */
            social_graph_link_name( group, local_being, i, BEING_MET, met_being_name );

            /** type of relationship */

            relationship_index = local_social_graph[i].relationship;

            if ( relationship_index > RELATIONSHIP_SELF )
            {
                being_relationship_description( relationship_index, relationship_str1 );

                if ( IS_FAMILY_MEMBER( local_social_graph, i ) )
                {
                    io_three_strings( relationship_str2, " (", relationship_str1, ")", 0 );
                }
                else
                {
                    n_string_block meeter_being_name;
                    n_string_block string_of_strings;
                    io_three_strings( meeter_being_name, " ", "", "", 0 );
                    social_graph_link_name( group, local_being, i, BEING_MEETER, meeter_being_name );
                    io_three_strings( string_of_strings, relationship_str1, " of *", meeter_being_name, 0 );
#ifdef REMOVE_BOLDING_TEXT
                    io_three_strings( relationship_str2, " ", string_of_strings, "*", 0 );
#else
                    io_three_strings( relationship_str2, " <", string_of_strings, "*>", 0 );
#endif
                }
            }
            else
            {
                io_three_strings( relationship_str2, " ", "", "", 0 );
            }

            if ( i != being_attention( local_being, ATTENTION_ACTOR ) )
            {
                /** Not the current focus of attention */

                if ( first )
                {
                    sprintf( result_str, "%s%05d  *%s*%s %ld\n", four_characters, ( int )local_social_graph[i].familiarity, met_being_name, relationship_str2, attraction );
                    first = 0;
                }
                else
                {
                    sprintf( result_str, "    %05d  *%s*%s %ld\n", ( int )local_social_graph[i].familiarity, met_being_name, relationship_str2, attraction );
                }
            }
            else
            {
                if ( first )
                {
                    /** The current focus of attention */
#ifdef REMOVE_BOLDING_TEXT
                    sprintf( result_str, "%s %05d *%s*%s %ld\n", four_characters, ( int )local_social_graph[i].familiarity, met_being_name, relationship_str2, attraction );
#else
                    sprintf( result_str, "%s %05d <*%s*>%s %ld\n", four_characters, ( int )local_social_graph[i].familiarity, met_being_name, relationship_str2, attraction );
#endif
                    first = 0;
                }
                else
                {
#ifdef REMOVE_BOLDING_TEXT
                    sprintf( result_str, "    %05d *%s*%s %ld\n", ( int )local_social_graph[i].familiarity, met_being_name, relationship_str2, attraction );
#else
                    sprintf( result_str, "    %05d <*%s*>%s %ld\n", ( int )local_social_graph[i].familiarity, met_being_name, relationship_str2, attraction );
#endif
                }
            }
            /** Write to the final result string */
            io_string_write( result, result_str, &watch_string_length );
        }
    }
}

/**
 * Show the friends of the given being
 * @param ptr pointer to simulated_group object
 * @param beingname name of the being
 * @param friend_type 0=friends, 1=enemies, 2=mates
 * @param result returned text
 */
static void command_show_friends( void *ptr, n_string beingname, n_int friend_type, n_string result )
{
    simulated_group *group = ( simulated_group * ) ptr;
    simulated_being *local_being;
    n_string local_being_name = io_string_copy( beingname );
    /** Get the being object from its name */
    local_being = being_from_name( group, local_being_name );
    memory_free( ( void ** )&local_being_name );

    if ( local_being == 0 )
    {
        return;
    }

    command_show_friends_being( ptr, local_being, friend_type, result, "        " );
}

/**
 * gets the number of mins/hours/days/months/years
 * @param str text to be processed
 * @param number the number of the time units
 * @param interval the time units
 * @return number of mins/hours/days/months/years
 */
n_int get_time_interval( n_string str, n_int *number, n_int *interval )
{
    n_int i, index = 0, ctr = 0, result = 0, divisor = 0;
    char c;
    n_string_block buf;
    n_int retval = -1;
    n_int length = io_length( str, 256 );

    for ( i = 0; i < length; i++ )
    {
        if ( str[i] != ' ' )
        {
            buf[ctr++] = str[i];
        }

        if ( ( str[i] == ' ' ) || ( i == ( length - 1 ) ) )
        {
            buf[ctr] = 0;

            switch ( index )
            {
            case 0:
            {
                io_number( ( n_string )buf, &result, &divisor );
                *number = result;
                retval = 0;
                break;
            }
            case 1:
            {
                if ( ctr == 1 )
                {
                    char lower_c;
                    lower_c = c = buf[0];
                    IO_LOWER_CHAR( lower_c );
                    if ( c == 'm' )
                    {
                        *interval = INTERVAL_MINS;
                    }
                    if ( lower_c == 'h' )
                    {
                        *interval = INTERVAL_HOURS;
                    }
                    if ( lower_c == 'd' )
                    {
                        *interval = INTERVAL_DAYS;
                    }
                    if ( c == 'M' )
                    {
                        *interval = INTERVAL_MONTHS;
                    }
                    if ( lower_c == 'y' )
                    {
                        *interval = INTERVAL_YEARS;
                    }
                }
                else
                {
                    IO_LOWER_CHAR( buf[0] );
                    if ( io_find( ( n_string )buf, 0, ctr, "min", 3 ) > -1 )
                    {
                        *interval = INTERVAL_MINS;
                    }
                    if ( ( io_find( ( n_string )buf, 0, ctr, "hour", 4 ) > -1 ) ||
                            ( io_find( ( n_string )buf, 0, ctr, "hr", 2 ) > -1 ) )
                    {
                        *interval = INTERVAL_HOURS;
                    }
                    if ( io_find( ( n_string )buf, 0, ctr, "day", 3 ) > -1 )
                    {
                        *interval = INTERVAL_DAYS;
                    }
                    if ( io_find( ( n_string )buf, 0, ctr, "mon", 3 ) > -1 )
                    {
                        *interval = INTERVAL_MONTHS;
                    }
                }

                break;
            }
            }

            index++;
            ctr = 0;
        }
    }
    return retval;
}

static void command_simulation_loop( simulated_group *group, simulated_being *local_being, void *data )
{
    n_int   *int_data = data;
    if ( FIND_SEX( GET_I( local_being ) ) == SEX_FEMALE )
    {
        int_data[0]++;
    }
    if ( ( land_date() - being_dob( local_being ) ) < AGE_OF_MATURITY )
    {
        int_data[1]++;
    }
}

/**
 * Show details of the overall simulation
 * @param ptr pointer to simulated_group object
 * @param response parameters of the command
 * @param output_function function to be used to display the result
 * @return 0
 */
n_int command_simulation( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;
    simulated_timing *timing = sim_timing();
    n_string_block beingstr, time;
    n_int int_data[2];
    n_byte2 *local_land_genetics = land_genetics();
    n_string_block land_dimension, land_genetics0, land_genetics1, genetics, population;
    n_string_block adults, juveniles, tide_level;

    loop_no_thread( group, 0L, command_simulation_loop, int_data );

    io_number_to_string( land_dimension, ( n_uint )land_map_dimension() );
    io_number_to_string( land_genetics0, local_land_genetics[0] );
    io_number_to_string( land_genetics1, local_land_genetics[1] );
    io_number_to_string( population, group->num );

    io_number_to_string( adults, ( n_uint )( ( n_int )( group->num ) - int_data[1] ) );
    io_number_to_string( juveniles, ( n_uint )int_data[1] );

    io_number_to_string( tide_level, land_tide_level() );

    io_three_strings( beingstr, "Map dimension: ", land_dimension, "", 1 );

    io_three_strings( genetics, land_genetics0, " ", land_genetics1, 0 );

    io_three_strings( beingstr, beingstr, "Land seed: ", genetics, 1 );
    io_three_strings( beingstr, beingstr, "Population: ", population, 1 );
    io_three_strings( beingstr, beingstr, "Adults: ", adults, 0 );
    io_three_strings( beingstr, beingstr, "   Juveniles: ", juveniles, 1 );


    if ( group->num > 0 )
    {
        n_string_block males, females, males_percent, females_percent;
        io_number_to_string( males, ( group->num - int_data[0] ) );
        io_number_to_string( females, int_data[0] );

        io_number_to_string( males_percent, ( ( group->num - int_data[0] ) * 100 ) / group->num );
        io_number_to_string( females_percent, ( int_data[0] * 100 ) / group->num );

        io_three_strings( beingstr, beingstr, "Females: ", females, 0 );
        io_three_strings( beingstr, beingstr, " (", females_percent, 0 );
        io_three_strings( beingstr, beingstr, "%)   Males: ", males, 0 );
        io_three_strings( beingstr, beingstr, " (", males_percent, 0 );
        io_three_strings( beingstr, beingstr, "%)", "", 1 );
    }

    io_three_strings( beingstr, beingstr, "Tide level: ", tide_level, 1 );

    spacetime_to_string( time );

    if ( timing->delta_cycles )
    {
        n_string_block delta_cycles;

        io_number_to_string( delta_cycles, timing->delta_cycles );

        io_three_strings( beingstr, beingstr, "Brain Cycles Per Second: ", delta_cycles, 1 );
    }

    if ( simulation_executing )
    {
        io_three_strings( beingstr, beingstr, time, " Simulation running", 0 );
    }
    else
    {
        io_three_strings( beingstr, beingstr, time, " Simulation not running", 0 );
    }

    output_function( beingstr );

    return 0;
}

/**
 * Shows the names of all beings
 * @param ptr pointer to simulated_group
 * @param response parameters of the command
 * @param output_function function used to display the result
 * @return 0
 */
n_int command_list( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;

    simulated_being *local_being;
    n_string_block line_text;
    n_int          location = 0;
    n_uint          loop = 0;

    if ( group->num == 0 )
    {
        output_function( "No apes present. Trying (re)running the Simulation" );
        return 0;
    }

    /** show names in index order */
    while ( loop < group->num )
    {
        n_string_block name;
        n_int          length;
        /** get the being */
        local_being = &group->beings[loop];

        /** get the name of the being */
        being_name_simple( local_being, name );

        io_string_write( line_text, name, &location );

        length = io_length( name, STRING_BLOCK_SIZE );

        while ( length < 24 )
        {
            io_string_write( line_text, " ", &location );
            length++;
        }

        if ( ( loop % 3 ) == 2 )
        {
            output_function( line_text );
            location = 0;
        }
        loop++;
    }

    if ( location != 0 )
    {
        output_function( line_text );
    }

    return 0;
}

void command_change_selected( simulated_group *group, n_byte forwards )
{
    simulated_being *local_select = group->select;
    simulated_being *first = group->beings;
    simulated_being *last = &( group->beings[group->num - 1] );
    if ( forwards )
    {
        if ( group->select != last )
        {
            local_select++;
        }
        else
        {
            local_select = first;
        }
    }
    else
    {
        if ( group->select != first )
        {
            local_select--;
        }
        else
        {
            local_select = last;
        }
    }
    sim_set_select( local_select );
}

static n_int command_check_ape_present( void *ptr, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;

    if ( group->select )
    {
        return 1;
    }
    if ( group->num )
    {
        output_function( "No apes selected." );
    }
    else
    {
        output_function( "No apes selected. Trying (re)running the Simulation" );
    }
    return 0;
}

n_int command_next( void *ptr, n_string response, n_console_output output_function )
{
    if ( command_check_ape_present( ptr, output_function ) )
    {
        command_change_selected( ( simulated_group * ) ptr, 1 );
    }
    return 0;
}

n_int command_previous( void *ptr, n_string response, n_console_output output_function )
{
    if ( command_check_ape_present( ptr, output_function ) )
    {
        command_change_selected( ( simulated_group * ) ptr, 0 );
    }
    return 0;
}

#ifdef BRAINCODE_ON
void command_populate_braincode( simulated_group *group, line_braincode function )
{
    if ( group->select )
    {
        simulated_being *local_being = group->select;
        n_byte       *internal_bc = being_braincode_internal( local_being );
        n_byte       *external_bc = being_braincode_external( local_being );
        n_int         loop = 0;

        n_string_block  initial_information;
        n_int           position = 0;

        io_string_write( initial_information, "EXT                                                         INT", &position );

        ( *function )( initial_information, -1 );

        while ( loop < 22 )
        {
            n_string_block command_information;

            n_string_block first_internal;
            n_string_block first_external;

            position = 0;

            brain_three_byte_command( ( n_string )first_internal, &internal_bc[loop * BRAINCODE_BYTES_PER_INSTRUCTION] );
            brain_three_byte_command( ( n_string )first_external, &external_bc[loop * BRAINCODE_BYTES_PER_INSTRUCTION] );

            if ( loop == 21 )
            {
                io_string_write( command_information, first_external, &position );
                io_string_write( command_information, "                   ", &position );
                io_string_write( command_information, first_internal, &position );
            }
            else
            {
                n_string_block second_internal;
                n_string_block second_external;

                brain_three_byte_command( ( n_string )second_internal, &internal_bc[( loop + 22 )*BRAINCODE_BYTES_PER_INSTRUCTION] );
                brain_three_byte_command( ( n_string )second_external, &external_bc[( loop + 22 )*BRAINCODE_BYTES_PER_INSTRUCTION] );

                io_string_write( command_information, first_external, &position );
                io_string_write( command_information, "  ", &position );
                io_string_write( command_information, second_external, &position );
                io_string_write( command_information, "   ", &position );
                io_string_write( command_information, first_internal, &position );
                io_string_write( command_information, "  ", &position );
                io_string_write( command_information, second_internal, &position );
            }
            ( *function )( command_information, loop );
            loop++;
        }
    }

}
#endif

/**
 * Show the appearance parameters for a being
 * @param ptr pointer to simulated_group object
 * @param beingname name of the being
 * @param local_being being to be shown
 * @param result resulting text containing appearance
 */
static void watch_appearance( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
    n_string_block str;
    n_genetics *genetics = being_genetics( local_being );

    sprintf( str, "Height: %.3f m\n", ( int )GET_BEING_HEIGHT( local_being ) / 1000.0f );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Mass: %.2f Kg\n", ( float )GET_M( local_being ) / 100.0f );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Body fat: %.2f Kg\n", ( float )GET_BODY_FAT( local_being ) / 100.0f );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Hair length: %.1f mm\n", ( float )( GENE_HAIR( genetics ) * 100.0f / 160.0f ) );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Pigmentation: %02d\n", ( int )( GENE_PIGMENTATION( genetics ) ) );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Body frame: %02d\n", ( int )( GENE_FRAME( genetics ) ) );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Eye separation: %.1f mm\n",
             80.0f + ( ( float )( GENE_EYE_SEPARATION( genetics ) ) ) );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Eye color: %02d       Eye shape: %02d\n",
             ( int )( GENE_EYE_COLOR( genetics ) ),
             ( int )( GENE_EYE_SHAPE( genetics ) ) );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Nose shape: %02d      Ear shape: %02d\n",
             ( int )( GENE_NOSE_SHAPE( genetics ) ),
             ( int )( GENE_EAR_SHAPE( genetics ) ) );
    io_string_write( result, str, &watch_string_length );

    sprintf( str, "Eyebrow shape: %02d   Mouth shape: %02d\n",
             ( int )( GENE_EYEBROW_SHAPE( genetics ) ),
             ( int )( GENE_MOUTH_SHAPE( genetics ) ) );
    io_string_write( result, str, &watch_string_length );
}


#ifdef BRAINCODE_ON

static n_string static_result;

static void watch_line_braincode( n_string string, n_int line )
{
    io_string_write( static_result, string, &watch_string_length );
    io_string_write( static_result, "\n", &watch_string_length );
}

#endif

/**
 * Shows braincode for the given being
 * @param ptr pointer to simulated_group object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_braincode( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
#ifdef BRAINCODE_ON
    n_int i;
    io_string_write( result, "\nRegisters:\n", &watch_string_length );
    for ( i = 0; i < BRAINCODE_PSPACE_REGISTERS; i++ )
    {
        result[watch_string_length++] = ( n_char )( 65 + ( local_being->braindata.braincode_register[i] % 60 ) );
    }
    result[watch_string_length++] = '\n';
    result[watch_string_length++] = '\n';

    static_result = result;

    command_populate_braincode( ptr, watch_line_braincode );

    static_result = 0L;
    result[watch_string_length++] = '\n';
#endif
}

static void watch_speech( void *ptr, n_string beingname, simulated_being *local, n_string result )
{
#ifdef BRAINCODE_ON
    n_int loop;
    n_byte *external_bc = being_braincode_external( local );
    for ( loop = 0; loop < BRAINCODE_SIZE / BRAINCODE_BYTES_PER_INSTRUCTION; loop++ )
    {
        n_string_block sentence;

        brain_sentence( ( n_string )sentence, &external_bc[loop * 3] );

        io_string_write( result, sentence, &watch_string_length );
        if ( ( loop & 3 ) == 3 )
        {
            result[watch_string_length++] = '.';
        }
        if ( loop < BRAINCODE_SIZE / BRAINCODE_BYTES_PER_INSTRUCTION - 1 )
        {
            result[watch_string_length++] = ' ';
        }
    }
    result[watch_string_length++] = '.';
    result[watch_string_length++] = '\n';
#endif
}



/**
 * Shows the social graph for the given being
 * @param ptr pointer to simulated_group object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_social_graph( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
    io_string_write( result, "\nFriends:\n", &watch_string_length );
    command_show_friends( ptr, beingname, 0, result );
    io_string_write( result, "\nEnemies:\n", &watch_string_length );
    command_show_friends( ptr, beingname, 1, result );
}

static void watch_pathogen_graph( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
#ifdef IMMUNE_ON
    n_uint j;
    n_uint antibodies[256] = {0};
    n_uint antigens[256] = {0};

    simulated_immune_system *immune = &( local_being->immune_system );
    for ( j = 0; j < IMMUNE_POPULATION; j++ )
    {
        antibodies[immune->shape_antibody[j]]++;
    }
    for ( j = 0; j < IMMUNE_ANTIGENS; j++ )
    {
        antigens[immune->shape_antigen[j]]++;
    }

    for ( j = 0; j < 256; j++ )
    {
        if ( antibodies[j] != 0 )
        {
            n_string_block result_str;
            sprintf( result_str, "AB( %ld ) = %ld\n", j,  antibodies[j] );
            io_string_write( result, result_str, &watch_string_length );
        }
        if ( antigens[j] != 0 )
        {
            n_string_block result_str;
            sprintf( result_str, "AG( %ld ) = %ld\n", j,  antigens[j] );
            io_string_write( result, result_str, &watch_string_length );
        }
    }
#endif
}

/**
 * Shows the episodic memory for the given being
 * @param ptr pointer to simulated_group object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_episodic( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
    n_uint i;
    for ( i = 0; i < EPISODIC_SIZE; i++ )
    {
        n_string_block str = {0};
        n_string_block description = {0};
        n_int          position = 0;
        ( void )episode_description( local_being, i, str );
        if ( io_length( str, STRING_BLOCK_SIZE ) > 0 )
        {
#ifdef REMOVE_BOLDING_TEXT
            io_string_write( description, "  ", &position );
            io_string_write( description, str, &position );
            io_string_write( description, "\n", &position );
#else
            if ( being_attention( local_being, ATTENTION_EPISODE ) != i )
            {
                io_string_write( description, "  ", &position );
                io_string_write( description, str, &position );
                io_string_write( description, "\n", &position );
            }
            else
            {
                io_string_write( description, " <", &position );
                io_string_write( description, str, &position );
                io_string_write( description, ">\n", &position );
            }
#endif
            io_string_write( result, description, &watch_string_length );
        }
    }
}

/**
 * Shows the genome for the given being
 * @param ptr pointer to simulated_group object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_genome( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
    n_int i, j;
    n_byte genome[CHROMOSOMES * 8 + 1];

    for ( i = 0; i < 2; i++ )
    {
        body_genome( ( n_byte )i, being_genetics( local_being ), genome );
        for ( j = 0; j < CHROMOSOMES * 8; j++ )
        {
            if ( ( j > 0 ) && ( j % 8 == 0 ) )
            {
                io_string_write( result, "\t", &watch_string_length );
            }
            result[watch_string_length++] = genome[j];
        }
        io_string_write( ( n_string )result, "\n", &watch_string_length );
    }
}

/**
 * Shows brainprobes for the given being
 * @param ptr pointer to simulated_group object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_brainprobes( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
#ifdef BRAINCODE_ON
    n_int i;
    n_string_block str2;
    n_string_block type_str;
    io_string_write( result, "\n  Type    Posn  Freq Offset Addr State\n  ", &watch_string_length );
    for ( i = 0; i < 36; i++ )
    {
        io_string_write( result, "-", &watch_string_length );
    }
    io_string_write( result, "\n", &watch_string_length );

    io_three_strings( type_str, "Input ", "", "", 0 );
    for ( i = 0; i < BRAINCODE_PROBES; i++ )
    {
        if ( local_being->braindata.brainprobe[i].type == INPUT_SENSOR )
        {
            sprintf( ( n_string )str2, "  %s  %03d   %02d   %03d    %03d  %d\n",
                     type_str,
                     local_being->braindata.brainprobe[i].position,
                     local_being->braindata.brainprobe[i].frequency,
                     local_being->braindata.brainprobe[i].offset,
                     local_being->braindata.brainprobe[i].address,
                     local_being->braindata.brainprobe[i].state );
            io_string_write( result, ( n_string )str2, &watch_string_length );
        }
    }

    io_three_strings( type_str, "Output ", "", "", 0 );

    for ( i = 0; i < BRAINCODE_PROBES; i++ )
    {
        if ( local_being->braindata.brainprobe[i].type == OUTPUT_ACTUATOR )
        {
            sprintf( ( n_string )str2, "  %s  %03d   %02d   %03d    %03d  %d\n",
                     type_str,
                     local_being->braindata.brainprobe[i].position,
                     local_being->braindata.brainprobe[i].frequency,
                     local_being->braindata.brainprobe[i].offset,
                     local_being->braindata.brainprobe[i].address,
                     local_being->braindata.brainprobe[i].state );
            io_string_write( result, ( n_string )str2, &watch_string_length );
        }
    }
#endif
}

static n_string watch_resolve_drives( n_byte *drives )
{
    n_byte max_value = 0;
    n_int loop = 0;
    n_int dominant = -1;
    while ( loop < DRIVES )
    {
        if ( drives[loop] > max_value )
        {
            max_value = drives[loop];
            dominant = loop;
        }
        loop++;
    }
    loop = 0;
    while ( loop < DRIVES )
    {
        if ( ( drives[loop] > ( max_value / 2 ) ) && ( loop != dominant ) )
        {
            return "Mixed drives";
        }
        loop++;
    }
    if ( dominant == DRIVE_SEX )
    {

        return "Find mate";
    }
    if ( dominant == DRIVE_HUNGER )
    {

        return "Find food";
    }
    if ( dominant == DRIVE_SOCIAL )
    {
        return "Find friends";
    }
    if ( dominant == DRIVE_FATIGUE )
    {

        return "Find rest";
    }
    return "No Drive";
}

static n_string sixteen_wind[16] =
{
    "E  ",
    "ESE",
    "SE ",
    "SSE",

    "S  ",
    "SSW",
    "SW ",
    "WSW",

    "W  ",
    "WNW",
    "NW ",
    "NNW",

    "N  ",
    "NNE",
    "NE ",
    "ENE",
};

static n_string watch_sixteenth_wind( n_byte facing )
{
    n_int   round_facing = facing + 7;
    return sixteen_wind[( round_facing >> 4 ) & 15];
}
/**
 * Shows the main parameters for the given being
 * @param ptr pointer to simulated_group object
 * @param beingname Name of the being
 * @param local_being being to be viewed
 * @param result returned text
 */
static void watch_stats( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
    n_string_block     str;
    n_string_block     relationship_str;
    n_string_block     status;

    if ( local_being == 0L )
    {
        ( void )SHOW_ERROR( "No being for stats" );
        return;
    }

    being_state_description( being_state( local_being ), status );
    being_relationship_description( being_attention( local_being, ATTENTION_RELATIONSHIP ), relationship_str );

    sprintf( str, "%s (%ld %ld) %s\n%s: %s\nGen %lu:%lu  %s  ERG:%ld SPD:%ld\nHonor:%d  HEI:%ld  Days Old:%ld\n",
             beingname,
             being_location_x( local_being ) / 16, being_location_y( local_being ) / 16,
             watch_sixteenth_wind( being_facing( local_being ) ),
             watch_resolve_drives( being_drives( local_being ) ),
             status,
             ( n_uint )local_being->constant.generation_min,
             ( n_uint )local_being->constant.generation_max,
             ( ( FIND_SEX( GET_I( local_being ) ) == SEX_FEMALE ) ? "Female" : "Male  " ),
             being_energy( local_being ),
             being_ten_minute_distance( local_being ),

             being_honor( local_being ),
             GET_BEING_HEIGHT( local_being ),
             land_date() - being_dob( local_being )
           );

    io_string_write( result, str, &watch_string_length );

    if ( being_pregnant( local_being ) )
    {
        sprintf( str, "Days Preg: %ld\n", land_date() - being_pregnant( local_being ) );
        io_string_write( result, str, &watch_string_length );
    }
    sprintf( str, "Aware Body: %s Link: %s\n", being_body_inventory_description( being_attention( local_being, ATTENTION_BODY ) ),
             relationship_str );

    io_string_write( result, str, &watch_string_length );

    io_string_write( result, "Friend\n", &watch_string_length );
    command_show_friends_being( ptr, local_being, 0, result, "    " );
    io_string_write( result, "Enemy\n", &watch_string_length );
    command_show_friends_being( ptr, local_being, 1, result, "    " );
    io_string_write( result, "\n", &watch_string_length );
    watch_episodic( ptr, beingname, local_being, result );
}


void watch_control( void *ptr, n_string beingname, simulated_being *local_being, n_string result )
{
    watch_string_length = 0;
    watch_stats( ptr, beingname, local_being, result );
}

/**
 * This should duplicate all console for watch functions
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to show the output
 * @param title title
 * @param watch_function watch function
 * @return 0
 */
static n_int command_duplicate( void *ptr, n_string response, n_console_output output_function, n_string title, console_generic watch_function )
{
    simulated_group *group = ( simulated_group * ) ptr;

    simulated_being *local_being = 0L;
    n_string_block beingstr;

    watch_string_length = 0;

    if ( ( response == 0 ) && ( group->select ) )
    {
        response = being_get_select_name( group );
        if ( title != 0L )
        {
            io_string_write( ( n_string )beingstr, "\n", &watch_string_length );
            io_string_write( ( n_string )beingstr, title, &watch_string_length );
            io_string_write( ( n_string )beingstr, " for ", &watch_string_length );
            io_string_write( ( n_string )beingstr, response, &watch_string_length );
            io_string_write( ( n_string )beingstr, "\n", &watch_string_length );
        }
    }

    if ( response != 0L )
    {
        local_being = being_from_name( group, response );
        if ( local_being == 0L )
        {
            ( void )SHOW_ERROR( "Being not found" );
            return 0;
        }
        being_set_select_name( group, response );

        watch_function( ptr, being_get_select_name( group ), local_being, ( n_string )beingstr );
        beingstr[watch_string_length] = 0;
        output_function( beingstr );
        return 0;
    }

    ( void )SHOW_ERROR( "No being was specified" );
    return 0;
}

/**
 *
 * @param ptr pointer to simulated_group object
 * @param response response string
 * @param output_function output function
 * @return 0
 */
n_int command_genome( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Genome", watch_genome );
}

/**
 *
 * @param ptr pointer to simulated_group object
 * @param response response string
 * @param output_function output function
 * @return 0
 */
n_int command_stats( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, 0L, watch_stats );
}

/**
 *
 * @param ptr pointer to simulated_group object
 * @param response response string
 * @param output_function output function
 * @return 0
 */
n_int command_probes( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Brain probes", watch_brainprobes );
}

/**
 * Show the episodic memory
 * @param ptr pointer to simulated_group object
 * @param response response string
 * @param output_function output function
 * @return 0
 */
n_int command_episodic( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Episodic memory", watch_episodic );
}


n_int command_pathogen_graph( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Pathogen graph", watch_pathogen_graph );
}

/**
 * Show the social graph
 * @param ptr pointer to simulated_group object
 * @param response response string
 * @param output_function output function
 * @return 0
 */
n_int command_social_graph( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Social graph", watch_social_graph );
}

/**
 * Show the braincode
 * @param ptr pointer to simulated_group object
 * @param response response string
 * @param output_function output function
 * @return 0
 */
n_int command_braincode( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Braincode", watch_braincode );
}

n_int command_speech( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Speech", watch_speech );
}


/**
 * Show appearance values
 * @param ptr pointer to simulated_group object
 * @param response response string
 * @param output_function output function
 * @return 0
 */
n_int command_appearance( void *ptr, n_string response, n_console_output output_function )
{
    return command_duplicate( ptr, response, output_function, "Appearance", watch_appearance );
}

static void histogram_being_state_loop( simulated_group *group, simulated_being *local_being, void *data )
{
    n_uint *histogram = data;
    n_uint n = 2;
    if ( being_state( local_being ) == BEING_STATE_ASLEEP )
    {
        histogram[0]++;
    }
    else
    {
        while ( n < BEING_STATES )
        {
            if ( being_state( local_being ) & ( 1 << ( n - 1 ) ) )
            {
                histogram[n]++;
            }
            n++;
        }
    }
}

/**
 * Update a histogram of being states
 * @param group pointer to simulated_group object
 * @param histogram histogram array to be updated
 * @param normalize whether to normalize the histogram
 */
static void histogram_being_state( simulated_group *group, n_uint *histogram, n_byte normalize )
{
    n_uint i;

    for ( i = 0; i < BEING_STATES; i++ )
    {
        histogram[i] = 0;
    }

    loop_no_thread( group, 0L, histogram_being_state_loop, histogram );

    if ( normalize )
    {
        n_uint tot = 0;
        for ( i = 0; i < BEING_STATES; i++ )
        {
            tot += histogram[i];
        }
        if ( tot > 0 )
        {
            for ( i = 0; i < BEING_STATES; i++ )
            {
                histogram[i] = histogram[i] * 1000 / tot;
            }
        }
    }
}

/**
 * Watch a particular being
 * @param ptr pointer to simulated_group object
 * @param output_function output function to be used
 */
static void watch_being( void *ptr, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;
    simulated_being *local_being;
    n_string_block beingstr;
    n_uint i;
    n_int  j;
    n_byte2 state;


    if ( being_remove_internal() )
    {
        do {}
        while ( being_remove_internal() );
    }

    being_remove_external_set( 1 );

    if ( watch_type == WATCH_STATES )
    {
        n_uint histogram[16];
        n_string_block str;

        watch_string_length = 0;

        spacetime_to_string( str );
        io_string_write( beingstr, str, &watch_string_length );
        histogram_being_state( group, ( n_uint * )histogram, 1 );
        for ( i = 0; i < BEING_STATES; i++ )
        {
            if ( i == 1 )
            {
                continue;    /**< skip the awake state */
            }

            if ( i == 0 )
            {
                state = 0;
            }
            else
            {
                state = ( n_byte2 )( 1 << ( i - 1 ) );
            }

            being_state_description( state, ( n_string )str );
            io_string_write( beingstr, ( n_string )str, &watch_string_length );
            io_string_write( beingstr, ":", &watch_string_length );
            for ( j = 0; j < 12 - io_length( ( n_string )str, STRING_BLOCK_SIZE ); j++ )
            {
                io_string_write( beingstr, " ", &watch_string_length );
            }

            if ( histogram[i] > 0 )
            {
                sprintf( ( n_string )str, "%.1f\n", histogram[i] / 10.0f );
                io_string_write( beingstr, str, &watch_string_length );
            }
            else
            {
                io_string_write( beingstr, "----\n", &watch_string_length );
            }
        }
        output_function( beingstr );
        return;
    }

    if ( group->select )
    {
        local_being = group->select;

        watch_string_length = 0;

        switch ( watch_type )
        {
        case WATCH_ALL:
        {
            watch_stats( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }
        case WATCH_SOCIAL_GRAPH:
        {
            watch_social_graph( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }
        case WATCH_EPISODIC:
        {
            watch_episodic( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }
        case WATCH_BRAINCODE:
        {
            watch_braincode( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }
        case WATCH_BRAINPROBES:
        {
            watch_brainprobes( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }

        case WATCH_APPEARANCE:
        {
            watch_appearance( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }
        case WATCH_SPEECH:
        {
            watch_speech( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }
        case WATCH_PATHOGENS:
        {
            watch_pathogen_graph( ptr, being_get_select_name( group ), local_being, beingstr );
            break;
        }
        }

        if ( watch_type != WATCH_NONE )
        {
            output_function( beingstr );
        }
    }
    being_remove_external_set( 0 );
}

static n_int command_on_off( n_string response )
{
    n_int length;

    if ( response == 0 )
    {
        return -1;
    }

    length = io_length( response, STRING_BLOCK_SIZE );
    if ( ( io_find( response, 0, length, "off", 3 ) > -1 ) ||
            ( io_find( response, 0, length, "0", 1 ) > -1 ) ||
            ( io_find( response, 0, length, "false", 5 ) > -1 ) ||
            ( io_find( response, 0, length, "no", 2 ) > -1 ) )
    {
        return 0;
    }

    if ( ( io_find( response, 0, length, "on", 2 ) > -1 ) ||
            ( io_find( response, 0, length, "1", 1 ) > -1 ) ||
            ( io_find( response, 0, length, "true", 4 ) > -1 ) ||
            ( io_find( response, 0, length, "yes", 3 ) > -1 ) )
    {
        return 1;
    }

    return -1;
}

n_int command_event( void *ptr, n_string response, n_console_output output_function )
{
#ifdef EPISODIC_ON
    n_int return_response = command_on_off( response );

    if ( ( return_response == -1 ) && response )
    {
        if ( io_find( response, 0, io_length( response, STRING_BLOCK_SIZE ), "social", 6 ) > -1 )
        {
            episodic_logging( output_function, 1 );
            output_function( "Event output for social turned on" );
        }
        return 0;
    }

    if ( return_response == 0 )
    {
        episodic_logging( 0L, 0 );
        output_function( "Event output turned off" );
    }
    else
    {
        episodic_logging( output_function, 0 );
        output_function( "Event output turned on" );
    }
#else
    output_function( "Episodic not supported in this build" );
#endif
    return 0;
}

n_int command_memory( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;
    n_string_block str2;
    sprintf( str2, "maximum memory %ld\nallocated memory %ld\nmaximum apes %ld",
             sim_memory_allocated( 1 ), sim_memory_allocated( 0 ), group->max );
    output_function( str2 );
    return 0;
}

/**
 * Enable or disable logging
 * @param ptr pointer to simulated_group object
 * @param response command parameters - off/on/0/1/yes/no
 * @param output_function function to be used to display output
 * @return 0
 */
n_int command_logging( void *ptr, n_string response, n_console_output output_function )
{
    n_int return_response = command_on_off( response );

    if ( return_response == -1 )
    {
        return 0;
    }
    if ( return_response == 0 )
    {
        nolog = 1;
        indicator_index = 0;
        watch_type = WATCH_NONE;
        output_function( "Logging turned off" );
    }
    else
    {
        nolog = 0;
        indicator_index = 1;
        output_function( "Logging turned on" );
    }
    return 0;
}

#ifdef BRAINCODE_ON

/**
 * Compare two braincode arrays
 * @param braincode0 Braincode array for the first being
 * @param braincode1 Braincode byte array for the second being
 * @param block_size The number of instructions to compare within a block
 * @returns Location of the first match within the first braincode array
 */
static n_int command_compare_brain( n_byte *braincode0, n_byte *braincode1, n_int block_size )
{
    n_int block_size_bytes = block_size * BRAINCODE_BYTES_PER_INSTRUCTION;
    n_int loop = 0;
    while ( loop < ( BRAINCODE_SIZE - block_size_bytes ) )
    {
        n_int loop2 = 0;
        while ( loop2 < ( BRAINCODE_SIZE - block_size_bytes ) )
        {
            n_int block_step = 0;
            while ( block_step < block_size )
            {
                if ( braincode0[loop + block_step * BRAINCODE_BYTES_PER_INSTRUCTION] ==
                        braincode1[loop2 + block_step * BRAINCODE_BYTES_PER_INSTRUCTION] )
                {
                    block_step++;
                    if ( block_step == block_size )
                    {
                        return loop;
                    }
                }
                else
                {
                    break;
                }
            }
            loop2 += BRAINCODE_BYTES_PER_INSTRUCTION;
        }
        loop += BRAINCODE_BYTES_PER_INSTRUCTION;
    }
    return -1;
}

#endif

/**
 * Shows repeated sections of braincode
 * @param ptr pointer to simulated_group
 * @param response response string
 * @param output_function output function
 * @returns 0
 */
n_int command_idea( void *ptr, n_string response, n_console_output output_function )
{
#ifdef BRAINCODE_ON
#ifndef CONSOLE_IDEA_MIN_BLOCK_SIZE
#define CONSOLE_IDEA_MIN_BLOCK_SIZE 3
#define CONSOLE_IDEA_MAX_BLOCK_SIZE 8
#endif
    const n_int min_block_size = 3;
    const n_int max_block_size = 8;
    n_uint i, total_matches = 0, total_tests = 0;
    n_uint histogram[5 + 1];
    simulated_group *group = ( simulated_group * ) ptr;
    /* clear the histogram */
    for ( i = 0; i <= ( n_uint )( max_block_size - min_block_size ); i++ )
    {
        histogram[i] = 0;
    }

    if ( group->select )
    {
        n_uint loop = 0;
        while ( loop < group->num )
        {
            simulated_being *local_being = &( group->beings[loop] );
            n_byte *bc_external = being_braincode_external( local_being );
            if ( bc_external )
            {

                n_uint loop2 = loop + 1;
                while ( loop2 < group->num )
                {
                    simulated_being *local_being2 = &( group->beings[loop2] );
                    n_byte *bc_external2 = being_braincode_external( local_being2 );

                    if ( bc_external2 )
                    {
                        n_int   location = 0;
                        n_int   block_size = min_block_size;

                        while ( block_size <= max_block_size )
                        {
                            location = command_compare_brain( bc_external,
                                                              bc_external2,
                                                              block_size );

                            if ( location != -1 )
                            {
                                histogram[block_size - min_block_size]++;
                                total_matches++;
                            }
                            total_tests++;
                            block_size++;
                        }
                    }
                    loop2++;
                }

            }
            loop++;
        }
    }

    if ( total_tests > 0 )
    {
        n_string_block output;

        sprintf( output, "Matches %03u.%04u percent\n",
                 ( n_c_int )( total_matches * 100 / total_tests ),
                 ( n_c_int )( total_matches * 1000000 / total_tests ) % 10000 );
        output_function( output );


        output_function( "Block Percent   Instances" );
        output_function( "-------------------------" );

        for ( i = 0; i <= ( n_uint )( max_block_size - min_block_size ); i++ )
        {
            sprintf( output, "%02u    %03u.%04u  %04u",
                     ( n_c_int )( i + min_block_size ),
                     ( n_c_int )( histogram[i] * 100 / total_tests ),
                     ( n_c_int )( ( histogram[i] * 1000000 / total_tests ) % 10000 ),
                     ( n_c_int )histogram[i] );
            output_function( output );
        }
    }

#endif
    return 0;
}

/**
 * Watch a particular being
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function to be used to display output
 * @return 0
 */
n_int command_watch( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;
    n_int          length;
    n_string_block output;
    n_int          position = 0;

    if ( command_check_ape_present( ptr, output_function ) == 0 )
    {
        return 0;
    }

    if ( response == 0L )
    {
        return 0;
    }
    else
    {
        length = io_length( response, STRING_BLOCK_SIZE );
    }

    if ( ( length < 5 ) && ( io_find( response, 0, length, "off", 3 ) > -1 ) )
    {
        output_function( "Stopped watching" );
        watch_type = WATCH_NONE;
        return 0;
    }

    if ( ( length < 10 ) && ( io_find( response, 0, length, "state", 5 ) > -1 ) )
    {
        watch_type = WATCH_STATES;
        output_function( "Watching being states" );
        return 0;
    }

    if ( being_from_name( group, response ) != 0 )
    {
        being_set_select_name( group, response );
        io_string_write( output, "Watching ", &position );
        io_string_write( output, being_get_select_name( group ), &position );
        output_function( output );
        position = 0;
        watch_type = WATCH_ALL;
    }
    else
    {
        if ( group->select )
        {
            if ( io_find( response, 0, length, "braincode", 9 ) > -1 )
            {
                watch_type = WATCH_BRAINCODE;
                io_string_write( output, "Watching braincode for ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }
            if ( ( io_find( response, 0, length, "brainprobe", 10 ) > -1 ) ||
                    ( io_find( response, 0, length, "brain probe", 11 ) > -1 ) ||
                    ( io_find( response, 0, length, "probes", 6 ) > -1 ) )
            {
                watch_type = WATCH_BRAINPROBES;
                io_string_write( output, "Watching brain probes for ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }
            if ( ( io_find( response, 0, length, "graph", 5 ) > -1 ) ||
                    ( io_find( response, 0, length, "friend", 6 ) > -1 ) )
            {
                watch_type = WATCH_SOCIAL_GRAPH;

                io_string_write( output, "Watching social graph for ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }
            if ( ( io_find( response, 0, length, "episodic", 8 ) > -1 ) ||
                    ( io_find( response, 0, length, "episodic memory", 15 ) > -1 ) ||
                    ( io_find( response, 0, length, "memory", 6 ) > -1 ) )
            {
                watch_type = WATCH_EPISODIC;

                io_string_write( output, "Watching episodic memory for ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }
            if ( io_find( response, 0, length, "pathogen", 8 ) > -1 )
            {
                watch_type = WATCH_PATHOGENS;

                io_string_write( output, "Watching pathogen for ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }
            if ( io_find( response, 0, length, "speech", 6 ) > -1 )
            {
                watch_type = WATCH_SPEECH;

                io_string_write( output, "Watching speech for ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }
            if ( ( length < 5 ) && ( io_find( response, 0, length, "all", 3 ) > -1 ) )
            {
                watch_type = WATCH_ALL;

                io_string_write( output, "Watching ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }

            if ( io_find( response, 0, length, "appear", 6 ) > -1 )
            {
                watch_type = WATCH_APPEARANCE;
                io_string_write( output, "Watching appearance for ", &position );
                io_string_write( output, being_get_select_name( group ), &position );
                output_function( output );
                return 0;
            }
        }

        output_function( "Being not found\n" );
    }
    return 0;
}

/**
 * Set the time interval for simulation
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int command_interval( void *ptr, n_string response, n_console_output output_function )
{
    n_int number = 1, interval = INTERVAL_DAYS, interval_set = 0;

    if ( response != 0 )
    {
        if ( io_length( response, STRING_BLOCK_SIZE ) > 0 )
        {
            if ( get_time_interval( response, &number, &interval ) > -1 )
            {
                if ( number > 0 )
                {
                    n_string_block  output, number_string;
                    save_interval_steps = number * interval_steps[interval];
                    io_number_to_string( number_string, number );
                    io_three_strings( output, "Logging interval set to ", number_string, ( n_string )interval_description[interval], 0 );
                    output_function( output );
                    interval_set = 1;
                }
            }
        }
    }

    if ( interval_set == 0 )
    {
        if ( save_interval_steps < 60 )
        {
            n_string_block  output, number_string;
            io_number_to_string( number_string, save_interval_steps );
            io_three_strings( output, "Current time interval is ", number_string, " min(s)", 0 );
            output_function( output );
        }
        else
        {
            if ( save_interval_steps < 60 * 24 )
            {
                n_string_block  output, number_string;
                io_number_to_string( number_string, save_interval_steps / 60 );
                io_three_strings( output, "Current time interval is ", number_string, " hour(s)", 0 );
                output_function( output );
            }
            else
            {
                n_string_block  output, number_string;
                io_number_to_string( number_string, save_interval_steps / ( 60 * 24 ) );
                io_three_strings( output, "Current time interval is ", number_string, " day(s)", 0 );
                output_function( output );
            }
        }
    }
    return 0;
}

n_int command_stop( void *ptr, n_string response, n_console_output output_function )
{
    simulation_running = 0;
    if ( output_function )
    {
        output_function( "Simulation stopped" );
    }
    return 0;
}

/**
 *
 * @param ptr pointer to simulated_group
 * @param response response string
 * @param output_function output function
 */
n_int command_file( void *ptr, n_string response, n_console_output output_function )
{
    io_search_file_format( simulated_file_format, response );
    return 0;
}

/**
 * Run the simulation for a single time interval
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int command_step( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;
    n_uint loop = 0;

    if ( response != RUN_STEP_CONST )
    {
        if ( simulation_executing == 1 )
        {
            output_function( "Simulation already running" );
            return 0;
        }

        if ( command_file_interaction )
        {
            output_function( "File interaction in use: step" );
            return 0;
        }

        simulation_executing = 1;
    }
    simulation_running = 1;

    while ( ( loop < save_interval_steps ) && simulation_running )
    {
        sim_cycle();
        if ( group->num == 0 )
        {
            simulation_running = 0;
        }
        loop++;
    }
    if ( response != RUN_STEP_CONST )
    {
        watch_being( group, output_function );
    }

    if ( response != RUN_STEP_CONST )
    {
        simulation_executing = 0;
    }

    return 0;
}

/**
 * Run the simulation
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int command_run( void *ptr, n_string response, n_console_output output_function )
{
    n_uint run = 0;
    n_int  number = 0, interval = INTERVAL_DAYS;
    n_int  forever = 0;

    if ( simulation_executing == 1 )
    {
        output_function( "Simulation already running" );
        return 0;
    }

    if ( command_file_interaction )
    {
        output_function( "File interaction in use: run" );
        return 0;
    }

    simulation_executing = 1;

    simulation_running = 1;

    if ( response != 0L )
    {
        n_int length = io_length( response, STRING_BLOCK_SIZE );
        if ( length > 0 )
        {
            if ( ( io_find( response, 0, length, "forever", 7 ) > -1 ) )
            {
                forever = 1;
                number = 1;
            }
            else if ( get_time_interval( response, &number, &interval ) <= -1 )
            {
                number = -1;
            }

            if ( number > 0 )
            {
                n_uint i = 0;
                n_string_block  output;
                n_uint end_point = ( number * interval_steps[interval] );
                n_uint temp_save_interval_steps = save_interval_steps;
                n_uint count = 0;
                save_interval_steps = 1;

                if ( forever )
                {
                    io_three_strings( output, "Running forever (type \"stop\" to end)", "", "", 0 );
                }
                else
                {
                    n_string_block  number_string;
                    io_number_to_string( number_string, number );
                    io_three_strings( output, "Running for ", number_string, ( n_string )interval_description[interval], 0 );
                }

                output_function( output );

                while ( ( i < end_point ) && simulation_running )
                {
                    command_step( ptr, RUN_STEP_CONST, output_function );

                    if ( temp_save_interval_steps )
                    {
                        if ( ( count % temp_save_interval_steps ) == 0 )
                        {
                            watch_being( ptr, output_function );
                        }
                    }
                    count++;
                    if ( !forever )
                    {
                        i++;
                    }
                }

                if ( temp_save_interval_steps )
                {
                    if ( ( count % temp_save_interval_steps ) != 1 )
                    {
                        watch_being( ptr, output_function );
                    }
                }

                save_interval_steps = temp_save_interval_steps;
                run = 1;
            }
        }
    }

    simulation_executing = 0;

    if ( run == 0 )
    {
        ( void )SHOW_ERROR( "Time not specified, examples: run 2 days, run 6 hours" );
    }

    return 0;
}

/**
 * Reset the simulation
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int command_reset( void *ptr, n_string response, n_console_output output_function )
{
    n_byte2 seed[2];
    n_byte2 *local_land_genetics = land_genetics();

    seed[0] = local_land_genetics[0];
    seed[1] = local_land_genetics[1];

    math_random3( seed );

    if ( sim_init( KIND_NEW_SIMULATION, ( n_uint )( ( seed[0] << 16 ) | seed[1] ), MAP_AREA, 0 ) )
    {
        output_function( "Simulation reset" );
        return 0;
    }
    output_function( "Simulation has not enough memory" );
    return 1;
}

/**
 * Returns a mode value used by epic and top commands
 * @param response command parameter: female/male/juvenile
 * @return 0
 */
static n_byte command_get_response_mode( n_string response )
{
    n_int length;
    if ( response == 0L )
    {
        return 0;
    }
    length = io_length( response, STRING_BLOCK_SIZE );
    if ( response != 0 )
    {
        /** females */
        if ( io_find( response, 0, length, "fem", 3 ) > -1 )
        {
            return 1;
        }
        /** males */
        if ( io_find( response, 0, length, "male", 4 ) > -1 )
        {
            return 2;
        }
        /** juveniles */
        if ( ( io_find( response, 0, length, "juv", 3 ) > -1 ) ||
                ( io_find( response, 0, length, "chil", 4 ) > -1 ) )
        {
            return 3;
        }
    }
    return 0;
}

n_int command_speak( void *ptr, n_string response, n_console_output output_function )
{
    n_string_block paragraph = {0};
    simulated_group *group = ( simulated_group * ) ptr;
    simulated_being *local = group->select;
    watch_speech( ptr, 0L, local, paragraph );
    watch_string_length = 0;
    speak_out( response, paragraph );
    return 0;
}

n_int command_alphabet( void *ptr, n_string response, n_console_output output_function )
{
    speak_out( response, " aeio a e i o vfstpbjm abefijmopstv a b e f i j m o p s t v.  .  \n" );
    return 0;
}


n_int command_save( void *ptr, n_string response, n_console_output output_function )
{
    n_file *file_opened;
    n_string_block output_string;

    if ( response == 0L )
    {
        return 0;
    }

    if ( command_file_interaction )
    {
        if ( output_function )
        {
            output_function( "File interaction in use: save" );
        }
        return 0;
    }

    command_stop( ptr, "", output_function );

    file_opened = tranfer_out_json();
    if ( file_opened == 0L )
    {
        return SHOW_ERROR( "Failed to generate output contents" );
    }

    command_file_interaction = 1;

    io_disk_write( file_opened, response );
    io_file_free( &file_opened );

    if ( output_function )
    {
        n_int position = 0;
        io_string_write( command_file_name, response, &position );
        position = 0;

        io_string_write( output_string, "Simulation file ", &position );
        io_string_write( output_string, response, &position );
        io_string_write( output_string, " saved\n", &position );
        output_function( output_string );
    }

    command_file_interaction = 0;

    return 0;
}

/* load simulation-data/script */
static n_int command_base_open( void *ptr, n_string response, n_console_output output_function, n_byte script )
{
    if ( response == 0L )
    {
        return 0;
    }

    if ( command_file_interaction )
    {
        if ( output_function )
        {
            output_function( "File interaction in use: open" );
        }
        return 0;
    }

    command_stop( ptr, "", output_function );
    command_file_interaction = 1;

    if ( io_disk_check( response ) != 0 )
    {
        n_file *file_opened = io_file_new();
        n_string_block output_string;

        if ( io_disk_read( file_opened, response ) != FILE_OKAY )
        {
            io_file_free( &file_opened );
            command_file_interaction = 0;
            return SHOW_ERROR( "Failed to open file" );
        }

        if ( script )
        {
            if ( sim_interpret( file_opened ) != 0 )
            {
                io_file_free( &file_opened );
                command_file_interaction = 0;
                return SHOW_ERROR( "Failed to interpret file" );
            }
        }
        else
        {
            if ( tranfer_in( file_opened ) != 0 )
            {
                io_file_free( &file_opened );
                command_file_interaction = 0;
                return SHOW_ERROR( "Failed to read in file" );
            }
            if ( sim_init( KIND_LOAD_FILE, 0, MAP_AREA, 0 ) == 0L )
            {
                return SHOW_ERROR( "Not enough memory to load file" );
            }
            io_file_free( &file_opened );
        }
        command_file_interaction = 0;
        if ( output_function )
        {
            n_int position = 0;
            io_string_write( command_file_name, response, &position );

            position = 0;

            io_string_write( output_string, "Simulation file ", &position );
            io_string_write( output_string, response, &position );
            io_string_write( output_string, " open\n", &position );

            output_function( output_string );
        }
    }
    return 0;
}

/* load simulation data */
n_int command_open( void *ptr, n_string response, n_console_output output_function )
{
    return command_base_open( ptr, response, output_function, 0 );
}

/* load apescript file */
n_int command_script( void *ptr, n_string response, n_console_output output_function )
{
    return command_base_open( ptr, response, output_function, 1 );
}

/**
 * Displays beings in descending order of honor value
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int command_top( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;

    n_uint i, j;
    n_int  k;
    n_uint max = 10;
    n_byte *eliminated;
    n_uint current_date, local_dob, age_in_years, age_in_months, age_in_days;
    n_string_block str;
    simulated_being *b;
    n_byte mode = command_get_response_mode( response );

    output_function( "Honor Name                     Sex\tAge" );
    output_function( "-----------------------------------------------------------------" );

    eliminated = ( n_byte * )memory_new( group->num * sizeof( n_byte ) );
    for ( i = 0; i < group->num; i++ )
    {
        eliminated[i] = 0;
    }

    if ( group->num < max )
    {
        max = group->num;
    }
    for ( i = 0; i < max; i++ )
    {
        n_int winner = -1;
        n_int max_honor = 0;
        n_byte passed;
        n_string_block output_value;

        for ( j = 0; j < group->num; j++ )
        {
            if ( eliminated[j] == 0 )
            {
                n_int honor;
                b = &group->beings[j];

                honor = being_honor( b );

                if ( honor >= max_honor )
                {
                    passed = 0;
                    switch ( mode )
                    {
                    case 0:
                    {
                        passed = 1;
                        break;
                    }
                    case 1:
                    {
                        if ( FIND_SEX( GET_I( b ) ) == SEX_FEMALE )
                        {
                            passed = 1;
                        }
                        break;
                    }
                    case 2:
                    {
                        if ( FIND_SEX( GET_I( b ) ) != SEX_FEMALE )
                        {
                            passed = 1;
                        }
                        break;
                    }
                    case 3:
                    {
                        if ( AGE_IN_DAYS( b ) < AGE_OF_MATURITY )
                        {
                            passed = 1;
                        }
                        break;
                    }
                    }

                    if ( passed != 0 )
                    {
                        winner = ( n_int )j;
                        max_honor = honor;
                    }
                }
            }
        }

        if ( winner == -1 )
        {
            break;
        }

        eliminated[winner] = 1;
        b = &group->beings[winner];

        sprintf( output_value, "%03d   ", ( int )( being_honor( b ) ) );

        being_name_simple( b, str );

        io_three_strings( output_value, output_value, str, "", 0 );

        for ( k = 0; k < 25 - io_length( str, STRING_BLOCK_SIZE ); k++ )
        {
            io_three_strings( output_value, output_value, " ", "", 0 );
        }
        if ( FIND_SEX( GET_I( b ) ) == SEX_FEMALE )
        {
            io_three_strings( output_value, output_value, "Female\t", "", 0 );
        }
        else
        {
            io_three_strings( output_value, output_value, "Male\t", "", 0 );
        }

        current_date = land_date();
        local_dob = ( n_uint )being_dob( b );
        age_in_years = ( n_uint )AGE_IN_YEARS( b );
        age_in_months = ( ( current_date - local_dob ) - ( age_in_years * TIME_YEAR_DAYS ) ) / ( TIME_YEAR_DAYS / 12 );
        age_in_days = ( current_date - local_dob ) - ( ( TIME_YEAR_DAYS / 12 ) * age_in_months ) - ( age_in_years * TIME_YEAR_DAYS );

        if ( age_in_years > 0 )
        {
            n_string_block number;
            io_number_to_string( number, age_in_years );
            io_three_strings( output_value, output_value, number, " yrs ", 0 );
        }
        if ( age_in_months > 0 )
        {
            n_string_block number;
            io_number_to_string( number, age_in_months );
            io_three_strings( output_value, output_value, number, " mnths ", 0 );
        }
        {
            n_string_block number;
            io_number_to_string( number, age_in_days );
            io_three_strings( output_value, output_value, number, "  days", 0 );
        }
        output_function( output_value );

    }

    memory_free( ( void ** )&eliminated );

    return 0;
}

n_int command_debug( void *ptr, n_string response, n_console_output output_function )
{
    command_audit();
    return 0;
}

/**
 * Lists the most talked about beings, based upon episodic memories
 * @param ptr pointer to simulated_group object
 * @param response command parameters
 * @param output_function function used to display the output
 * @return 0
 */
n_int command_epic( void *ptr, n_string response, n_console_output output_function )
{
    simulated_group *group = ( simulated_group * ) ptr;

    n_uint i, j, k, e;
    simulated_being *local_being;
    simulated_iepisodic *local_episodic;
    const n_uint max = 1024;
    n_byte2 *first_name = ( n_byte2 * )memory_new( max * sizeof( n_byte2 ) );
    n_byte2 *family_name = ( n_byte2 * )memory_new( max * sizeof( n_byte2 ) );
    n_byte2 *hits = ( n_byte2 * )memory_new( max * sizeof( n_byte2 ) );
    n_byte2 temp;
    n_string_block name;
    n_byte passed, mode = command_get_response_mode( response );

    /** clear the list of beings and their hit scores */
    for ( i = 0; i < max; i++ )
    {
        first_name[i] = 0;
        family_name[i] = 0;
        hits[i] = 0;
    }

    for ( i = 0; i < group->num; i++ )
    {
        /** get the being */
        local_being = &group->beings[i];

        /** get the episodic memories for the being */
        local_episodic = being_episodic( local_being );

        /** skip is no memories were retrieved */
        if ( local_episodic == 0L )
        {
            continue;
        }

        /** for all memories */
        for ( e = 0; e < EPISODIC_SIZE; e++ )
        {
            /** non-empty slot */
            if ( local_episodic[e].event > 0 )
            {
                /** j = 0 is the being having the memory
                     j = 1 is the being who is the subject of the memory */
                for ( j = BEING_MEETER; j <= BEING_MET; j++ )
                {
                    /** name should be non-zero */
                    if ( local_episodic[e].first_name[j] +
                            local_episodic[e].family_name[j] > 0 )
                    {
                        passed = 0;
                        switch ( mode )
                        {
                        case 0:
                        {
                            passed = 1;
                            break;
                        }
                        case 1:
                        {
                            if ( ( local_episodic[e].first_name[j] >> 8 ) == SEX_FEMALE )
                            {
                                passed = 1;
                            }
                            break;
                        }
                        case 2:
                        {
                            if ( ( local_episodic[e].first_name[j] >> 8 ) != SEX_FEMALE )
                            {
                                passed = 1;
                            }
                            break;
                        }
                        case 3:
                        {
                            simulated_being *b = 0L;
                            n_string_block name;

                            being_name_byte2( local_episodic[e].first_name[j], local_episodic[e].family_name[j], name );

                            b = being_from_name( group, name );
                            if ( b != 0L )
                            {
                                if ( AGE_IN_DAYS( b ) < AGE_OF_MATURITY )
                                {
                                    passed = 1;
                                }
                            }
                            break;
                        }
                        }

                        if ( passed != 0 )
                        {
                            /** Avoid memories about yourself, since we're interested
                               in gossip about other beings */

                            if ( being_name_comparison( local_being, local_episodic[e].first_name[j], local_episodic[e].family_name[j] ) )
                            {
                                if ( ( ( j == BEING_MET ) &&
                                        ( local_episodic[e].event != EVENT_SEEK_MATE ) &&
                                        ( local_episodic[e].event != EVENT_EAT ) ) ||
                                        ( j == BEING_MEETER ) )
                                {
                                    /** add this being to the list, or increment its hit score */
                                    for ( k = 0; k < max; k++ )
                                    {
                                        if ( hits[k] == 0 ) /**< last entry in the list */
                                        {
                                            first_name[k] = local_episodic[e].first_name[j];
                                            family_name[k] = local_episodic[e].family_name[j];
                                            break;
                                        }
                                        if ( first_name[k] == local_episodic[e].first_name[j] )
                                        {
                                            if ( family_name[k] == local_episodic[e].family_name[j] )
                                            {
                                                /** being found in the list */
                                                break;
                                            }
                                        }
                                    }
                                    /** increment the hit score for the being */
                                    if ( k < max )
                                    {
                                        hits[k]++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /** top 10 most epic beings */
    for ( i = 0; i < 10; i++ )
    {
        /** search the rest of the list */
        for ( j = i + 1; j < max; j++ )
        {
            if ( hits[j] == 0 )
            {
                break;    /**< end of the list */
            }

            /** if this being has more hits then swap list entries, so that
             the most popular beings are at the top of the list */
            if ( hits[j] > hits[i] )
            {
                /** swap */
                temp = first_name[j];
                first_name[j] = first_name[i];
                first_name[i] = temp;

                temp = family_name[j];
                family_name[j] = family_name[i];
                family_name[i] = temp;

                temp = hits[j];
                hits[j] = hits[i];
                hits[i] = temp;
            }
        }
        if ( hits[i] > 0 )
        {
            n_string_block output_value;
            /** get the name of the being */

            being_name_byte2( first_name[i], family_name[i], name );

            sprintf( output_value, "%06d %s", ( int )hits[i], name );
            output_function( output_value );
        }
    }

    /** free list memory */
    memory_free( ( void ** )&first_name );
    memory_free( ( void ** )&family_name );
    memory_free( ( void ** )&hits );

    return 0;
}

n_int command_quit( void *ptr, n_string response, n_console_output output_function )
{
    simulation_executing = 0;

    ( void )command_stop( ptr, response, output_function );
    return io_quit( ptr, response, output_function );
}

