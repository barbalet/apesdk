/****************************************************************

 loop.c

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

#include "../entity/entity.h"

#include "universe.h"

#include <stdio.h>


void loop_no_thread( simulated_group *group, simulated_being *being_not, loop_fn bf_func, void *data )
{
    n_uint loop = 0;
    while ( loop < group->num )
    {
        simulated_being *output = &( group->beings[loop] );
        if ( output != being_not )
        {
            bf_func( group, output, data );
        }
        loop++;
    }
}


void loop_being_no_sim( simulated_being *beings, n_uint number_beings, loop_no_sim_fn bf_func, void *data )
{
    n_uint loop  = 0;
    while ( loop < number_beings )
    {
        simulated_being *output = &( beings[loop] );
        bf_func( output, data );
        loop++;
    }
}

void loop_being_no_sim_no_data( simulated_being *beings, n_uint number_beings, loop_no_sim_no_data_fn bf_func )
{
    n_uint loop  = 0;
    while ( loop < number_beings )
    {
        simulated_being *output = &( beings[loop] );
        bf_func( output );
        loop++;
    }
}


static void loop_add_generic( execute_function *function, void *general_data, void *read_data, void *write_data, n_int count, n_int size )
{
    if ( size )
    {
        n_byte *location = ( n_byte * )read_data;
        n_int   loop = 0;
        while ( loop < count )
        {
            if ( function( general_data, ( void * )&location[loop * size], 0L ) == -1 )
            {
                break;
            }
            loop++;
        }
    }
    else
    {
        function( general_data, read_data, write_data );
    }
}

void loop_being( simulated_group *group, loop_fn bf_func, n_int beings_per_thread )
{
    n_uint loop  = 0;
    n_uint count = ( n_uint )beings_per_thread;
    n_uint beings_per_thread_uint = ( n_uint )beings_per_thread;
    while ( loop < group->num )
    {
        simulated_being *output = &( group->beings[loop] );

        if ( ( beings_per_thread_uint + loop ) >= group->num )
        {
            count = group->num - loop;
        }

        loop_add_generic( ( execute_function * )bf_func, ( void * )group, ( void * )output, 0L, ( n_int )count, sizeof( simulated_being ) );

        if ( count != beings_per_thread_uint )
        {
            break;
        }
        else
        {
            loop += count;
        }
    }
}


typedef struct
{
    n_string name;
    simulated_being *being_from_name;
} being_from_name_loop_struct;

static void being_from_name_loop( simulated_group *group, simulated_being *local, void *data )
{
    being_from_name_loop_struct *bfns = ( being_from_name_loop_struct * )data;
    n_string_block str;

    if ( bfns->being_from_name )
    {
        return;
    }

    being_name_simple( local, str );

    io_lower( str, io_length( str, STRING_BLOCK_SIZE ) );

    if ( io_find( str, 0, io_length( str, STRING_BLOCK_SIZE ), bfns->name, io_length( bfns->name, STRING_BLOCK_SIZE ) ) > -1 )
    {
        bfns->being_from_name = local;
    }
}

/**
 * @brief return the being array index with the given name
 * @param group Pointer to the simulated_group object
 * @param name Name of the being
 * @return Array index of the being within the simulation object
 */
simulated_being *being_from_name( simulated_group *group, n_string name )
{
    being_from_name_loop_struct bfns;
    bfns.being_from_name = 0L;
    io_lower( name, io_length( name, STRING_BLOCK_SIZE ) );
    bfns.name = name;
    loop_no_thread( group, 0L, being_from_name_loop, &bfns );
    return bfns.being_from_name;
}

void being_set_select_name( simulated_group *group, n_string name )
{
    simulated_being *response = being_from_name( group, name );
    if ( response == 0L )
    {
        ( void )SHOW_ERROR( "Ape not found" );
        return;
    }
    group->select = response;
}

n_string being_get_select_name( simulated_group *group )
{
    static n_string_block name;
    n_int  position = 0;
    if ( group->select == 0L )
    {
        io_string_write( name, "*** ALL APES DEAD ***", &position );
    }
    else
    {
        being_name_simple( group->select, name );
    }
    return ( n_string )name;
}

typedef struct
{
    n_int         parse_requirements;
    n_int         older;
    n_uint        comparison_best;
    n_int         actual_age;
    n_genetics   *genetics;
    simulated_being *return_value;
} being_find_closest_struct;

static void being_find_closest_loop( simulated_group *group, simulated_being *local, void *data )
{
    being_find_closest_struct *bfcs = ( being_find_closest_struct * )data;

    n_byte success = 0;
    n_int  local_dob = being_dob( local );

    if ( bfcs->older == 0 )
    {
        success = 1;
    }

    if ( ( bfcs->older == 1 ) && ( ( local_dob - AGE_OF_MATURITY ) > bfcs->actual_age ) )
    {
        success = 1;
    }

    if ( ( bfcs->older == -1 ) && ( ( bfcs->actual_age - AGE_OF_MATURITY ) > local_dob ) )
    {
        success = 1;
    }

    if ( success )
    {
        n_uint comparison = being_genetic_comparison( bfcs->genetics, being_genetics( local ), bfcs->parse_requirements );
        if ( comparison > bfcs->comparison_best )
        {
            bfcs->comparison_best = comparison;
            bfcs->return_value = local;
        }
    }

}

static simulated_being *being_find_closest( simulated_group *group, simulated_being *actual, n_int parse_requirements, n_int older )
{
    being_find_closest_struct bfcs;
    bfcs.parse_requirements = parse_requirements;
    bfcs.older = older;
    /** comparison must be better than average */
    bfcs.comparison_best = 3 * sizeof( n_genetics ) * CHROMOSOMES;
    bfcs.return_value = 0L;
    bfcs.genetics = being_genetics( actual );
    bfcs.actual_age = being_dob( actual );

    loop_no_thread( group, actual, being_find_closest_loop, &bfcs );

    return bfcs.return_value;
}

typedef struct
{
    n_uint        comparison_best;
    n_int         max_age;
    n_genetics   *genetics;
    simulated_being *return_value;
} being_find_child_struct;

static void being_find_child_loop( simulated_group *group, simulated_being *local, void *data )
{
    being_find_child_struct *bfcs = ( being_find_child_struct * ) data;
    n_uint  comparison = being_genetic_comparison( bfcs->genetics, being_genetics( local ), -1 );
    if ( ( comparison > bfcs->comparison_best ) &&
            ( ( land_date() - being_dob( local ) ) < bfcs->max_age ) )
    {
        bfcs->comparison_best = comparison;
        bfcs->return_value = local;
    }
}

static simulated_being *being_find_child( simulated_group *group, n_genetics *genetics, n_int max_age )
{
    being_find_child_struct bfcs;
    bfcs.comparison_best = 0;
    bfcs.max_age = max_age;
    bfcs.genetics = genetics;
    bfcs.return_value = 0L;
    loop_no_thread( group, 0L, being_find_child_loop, &bfcs );
    return bfcs.return_value;
}

typedef struct
{
    n_byte2 first_gender;
    n_byte2 family;
    simulated_being *local;
} being_find_name_struct;

static void being_find_name_loop( simulated_group *group, simulated_being *local, void *data )
{
    being_find_name_struct *bfns = ( being_find_name_struct * )data;
    if ( bfns->local == 0L )
    {
        if ( being_name_comparison( local, bfns->first_gender, bfns->family ) )
        {
            bfns->local = local;
        }
    }
}

simulated_being *being_find_name( simulated_group *group, n_byte2 first_gender, n_byte2 family )
{
    being_find_name_struct bfns;
    bfns.first_gender = first_gender;
    bfns.family = family;
    bfns.local = 0L;
    loop_no_thread( group, 0L, being_find_name_loop, &bfns );
    return bfns.local;
}

/** returns the total positive and negative affect within memory */
n_uint being_affect( simulated_being *local, n_byte is_positive )
{
    n_uint affect = 0;
#ifdef EPISODIC_ON
    n_uint i;
    simulated_iepisodic *local_episodic = being_episodic( local );
    if ( !local_episodic )
    {
        return affect;
    }

    for ( i = 0; i < EPISODIC_SIZE; i++ )
    {
        if ( is_positive != 0 )
        {
            if ( local_episodic[i].affect > EPISODIC_AFFECT_ZERO )
            {
                affect += ( n_uint )( local_episodic[i].affect ) - EPISODIC_AFFECT_ZERO;
            }
        }
        else
        {
            if ( local_episodic[i].affect < EPISODIC_AFFECT_ZERO )
            {
                affect += EPISODIC_AFFECT_ZERO - ( n_uint )( local_episodic[i].affect );
            }
        }
    }
#endif
    return affect;
}

const n_string body_inventory_description[INVENTORY_SIZE] =
{
    "Head", "Teeth", "Back", "Front", "Left hand", "Right hand", "Left foot", "Right foot"
};

n_string being_body_inventory_description( n_int index )
{
    return body_inventory_description[index % INVENTORY_SIZE];
}

const n_string relationship_description[RELATIONSHIPS] =
{
    "Associate", "Self", "Mother", "Father", "Daughter",
    "Son", "Granddaughter", "Grandson", "Sister", "Brother",
    "Maternal Grandmother", "Maternal Grandfather", "Paternal Grandmother", "Paternal Grandson", "Mother",
    "Father", "Daughter", "Son", "Granddaughter", "Grandson",
    "Sister", "Brother", "Maternal Grandmother", "Maternal Grandfather", "Paternal Grandmother",
    "Paternal Grandson"
};

void being_relationship_description( n_int index, n_string description )
{
    n_int position = 0;
    if ( index >= RELATIONSHIPS )
    {
        n_string_block index_string;
        io_number_to_string( index_string, ( n_uint )index );
        io_three_strings( description, "ERROR: relationship out of range ", index_string, "", 1 );
        return;
    }
    io_string_write( description, relationship_description[index], &position );
}

static void being_inventory_string( n_string string, n_int *location, n_int item )
{
    switch ( item )
    {
    case INVENTORY_BRANCH:
        io_string_write( string, "branch", location );
        break;
    case INVENTORY_ROCK:
        io_string_write( string, "rock", location );
        break;
    case INVENTORY_SHELL:
        io_string_write( string, "shell", location );
        break;
    case INVENTORY_TWIG:
        io_string_write( string, "twig", location );
        break;
    case INVENTORY_NUT_CRACKED:
        io_string_write( string, "cracked nut", location );
        break;
    case INVENTORY_GRASS:
        io_string_write( string, "piece of grass", location );
        break;
    case INVENTORY_SCRAPER:
        io_string_write( string, "scraper", location );
        break;
    case INVENTORY_SPEAR:
        io_string_write( string, "spear", location );
        break;
    case INVENTORY_FISH:
        io_string_write( string, "fish", location );
        break;
    case INVENTORY_BIRD_EGGS:
        io_string_write( string, "bird eggs", location );
        break;
    case INVENTORY_LIZARD_EGGS:
        io_string_write( string, "lizard eggs", location );
        break;
    case INVENTORY_CHILD:
    case INVENTORY_WOUND:
    case INVENTORY_GROOMED:
    default:
        io_string_write( string, "thing being carried", location );
        break;
    }
}

static void being_social_event_string( n_string string, n_int *location, n_int event_type, n_string name_str )
{
    switch ( event_type )
    {
    case EVENT_MATE:
        io_string_write( string, "Mated with ", location );
        break;
    case EVENT_SEEK_MATE:
        io_string_write( string, "Searched for ", location );
        break;
    /*case EVENT_GROOM:
         io_string_write(string,"Groomed ",location);
         break; */
    case EVENT_GROOMED:
        io_string_write( string, "Groomed by ", location );
        break;
    case EVENT_CHAT:
        io_string_write( string, "Chatted with ", location );
        break;
    case EVENT_BIRTH:
        io_string_write( string, "Gave birth to ", location );
        break;
    case EVENT_HURLED:
        io_string_write( string, "Hurled a rock at ", location );
        break;
    case EVENT_HURLED_BY:
        io_string_write( string, "Hit by a rock hurled by ", location );
        break;

    case EVENT_HIT:
        io_string_write( string, "Hit ", location );
        break;
    case EVENT_HIT_BY:
        io_string_write( string, "Hit by ", location );
        break;
    case EVENT_CARRIED:
        io_string_write( string, "Carried ", location );
        break;
    case EVENT_CARRIED_BY:
        io_string_write( string, "Carried by ", location );
        break;
    case EVENT_SUCKLED:
        io_string_write( string, "Suckled ", location );
        break;
    case EVENT_SUCKLED_BY:
        io_string_write( string, "Suckled by ", location );
        break;
    case EVENT_WHACKED:
        io_string_write( string, "Whacked ", location );
        break;
    case EVENT_WHACKED_BY:
        io_string_write( string, "Whacked by ", location );
        break;
    case EVENT_HUGGED:
        io_string_write( string, "Hugged ", location );
        break;
    case EVENT_HUGGED_BY:
        io_string_write( string, "Hugged by ", location );
        break;
    case EVENT_PRODDED:
        io_string_write( string, "Prodded ", location );
        break;
    case EVENT_PRODDED_BY:
        io_string_write( string, "Prodded by ", location );
        break;
    case EVENT_GIVEN:
        io_string_write( string, "Given ", location );
        break;
    case EVENT_GIVEN_BY:
        io_string_write( string, "Given by ", location );
        break;
    case EVENT_POINT:
        io_string_write( string, "Pointed to ", location );
        break;
    case EVENT_POINTED:
        io_string_write( string, "Pointed to by ", location );
        break;
    case EVENT_SMILED:
        io_string_write( string, "Smiled at ", location );
        break;
    case EVENT_SMILED_BY:
        io_string_write( string, "Smiled at by ", location );
        break;
    case EVENT_TICKLED:
        io_string_write( string, "Tickled ", location );
        break;
    case EVENT_TICKLED_BY:
        io_string_write( string, "Tickled by ", location );
        break;
    case EVENT_GLOWERED:
        io_string_write( string, "Glowered at ", location );
        break;
    case EVENT_GLOWERED_BY:
        io_string_write( string, "Glowered at by ", location );
        break;
    case EVENT_PATTED:
        io_string_write( string, "Patted ", location );
        break;
    case EVENT_PATTED_BY:
        io_string_write( string, "Patted by ", location );
        break;
    default:
    {
        n_string_block  number_str;
        io_number_to_string( number_str, ( n_uint )event_type );

        io_string_write( string, "Erroneous action (", location );
        io_string_write( string, number_str, location );
        io_string_write( string, ") with ", location );
        break;
    }
    }
    io_string_write( string, "*", location );
    io_string_write( string, name_str, location );
    io_string_write( string, "*", location );
}

void being_remains_init( simulated_remains *remains )
{
    remains->count = 0;
    remains->location = 0;
}

static void being_remains( simulated_group *group, simulated_being *dead )
{
    simulated_remains *remains  = &( group->remains );
    n_byte2         location = remains->location;

    remains->bodies[location].location[0] = dead->delta.location[0];
    remains->bodies[location].location[1] = dead->delta.location[1];
    remains->location = ( remains->location + 1 ) % NUMBER_OF_BODIES;

    if ( remains->count <= NUMBER_OF_BODIES )
    {
        remains->count++;
    }
}

n_int episode_description( simulated_being *local_being,
                           n_int index,
                           n_string description )
{
    n_string_block str = {0};
    n_int string_index = 0;
    n_int social = 0;
#ifdef EPISODIC_ON
    n_string_block str2, name_str;
    simulated_iepisodic *local_episodic;
    n_uint days_elapsed, time_elapsed;

    local_episodic = being_episodic( local_being );

    if ( local_episodic == 0L )
    {
        return SHOW_ERROR( "No episodic description" );
    }

    if ( ( local_episodic[index].event > 0 ) &&
            being_name_comparison( local_being, local_episodic[index].first_name[0], local_episodic[index].family_name[0] ) )
    {
        being_name_byte2( local_episodic[index].first_name[BEING_MET], local_episodic[index].family_name[BEING_MET], name_str );

        if ( local_episodic[index].event & ( EVENT_INTENTION ) )
        {
            io_string_write( str, "Intends ", &string_index );
        }

        switch ( local_episodic[index].event & ( EVENT_INTENTION - 1 ) )
        {
        case EVENT_EAT:
        {
            io_string_write( str, "Was eating ", &string_index );
            switch ( local_episodic[index].food )
            {
            case FOOD_VEGETABLE:
            {
                io_string_write( str, "vegetation", &string_index );
                break;
            }
            case FOOD_FRUIT:
            {
                io_string_write( str, "fruit", &string_index );
                break;
            }
            case FOOD_SHELLFISH:
            {
                io_string_write( str, "shellfish", &string_index );
                break;
            }
            case FOOD_SEAWEED:
            {
                io_string_write( str, "seaweed", &string_index );
                break;
            }
            case FOOD_BIRD_EGGS:
            {
                io_string_write( str, "bird eggs", &string_index );
                break;
            }
            case FOOD_LIZARD_EGGS:
            {
                io_string_write( str, "lizard eggs", &string_index );
                break;
            }
            }
            break;
        }
        case EVENT_SWIM:
        {
            io_string_write( str, "Went swimming", &string_index );
            break;
        }
        case EVENT_GROOM: // this appears to be a duplicate
        {
            io_string_write( str, "Groomed *", &string_index );
            io_string_write( str, name_str, &string_index );
            io_string_write( str, "*'s ", &string_index );
            io_string_write( str, being_body_inventory_description( local_episodic[index].arg ), &string_index );

            social = 1;
            break;
        }
        case EVENT_SHOUT:
        {
            io_string_write( str, "Shouted ", &string_index );
            break;
        }
        case EVENT_FISH:
        {
            io_string_write( str, "Caught a fish ", &string_index );
            break;
        }
        case EVENT_CHEW:
        {
            io_string_write( str, "Chewing ", &string_index );
            if ( local_episodic[index].arg & INVENTORY_GRASS )
            {
                io_string_write( str, "grass ", &string_index );
            }
            else
            {
                if ( local_episodic[index].arg & INVENTORY_TWIG )
                {
                    io_string_write( str, "twig ", &string_index );
                }
                else
                {
                    if ( local_episodic[index].arg & INVENTORY_FISH )
                    {
                        io_string_write( str, "fish ", &string_index );
                    }
                    else
                    {
                        if ( local_episodic[index].arg & INVENTORY_NUT_CRACKED )
                        {
                            io_string_write( str, "a cracked nut ", &string_index );
                        }
                        else
                        {
                            {
                                if ( local_episodic[index].arg & INVENTORY_BIRD_EGGS )
                                {
                                    io_string_write( str, "birds eggs ", &string_index );
                                }
                                else
                                {
                                    if ( local_episodic[index].arg & INVENTORY_LIZARD_EGGS )
                                    {
                                        io_string_write( str, "lizard eggs ", &string_index );
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if ( local_episodic[index].arg & 1 )
            {
                io_string_write( str, "held in right hand ", &string_index );
            }
            else
            {
                io_string_write( str, "held in left hand ", &string_index );
            }
            break;
        }
        case EVENT_DRAG:
            io_string_write( str, "Dragged a ", &string_index );
            being_inventory_string( str, &string_index, local_episodic[index].arg );
            break;
        case EVENT_BRANDISH:
            io_string_write( str, "Waved a ", &string_index );
            being_inventory_string( str, &string_index, local_episodic[index].arg );
            break;
        case EVENT_DROP:
            io_string_write( str, "Dropped a ", &string_index );
            being_inventory_string( str, &string_index, local_episodic[index].arg );
            break;
        case EVENT_PICKUP:
            io_string_write( str, "Picked up a ", &string_index );
            being_inventory_string( str, &string_index, local_episodic[index].arg );
            break;
        default:
            being_social_event_string( str, &string_index, local_episodic[index].event & ( EVENT_INTENTION - 1 ), name_str );
            social = 1;
            break;
        }

        if ( string_index == 0 )
        {
            return SHOW_ERROR( "No string in episodic description" );
        }

        days_elapsed = land_date() - local_episodic[index].space_time.date;
        if ( days_elapsed == 0 )
        {
            time_elapsed = land_time() - local_episodic[index].space_time.time;

            if ( time_elapsed < 60 )
            {
                if ( time_elapsed == 0 )
                {
                    io_string_write( str, " now", &string_index );
                }
                else if ( time_elapsed == 1 )
                {
                    io_string_write( str, " a minute ago", &string_index );
                }
                else if ( time_elapsed < 5 )
                {
                    io_string_write( str, " a few minutes ago", &string_index );
                }
                else
                {
                    n_string_block time_elapsed_string;
                    io_number_to_string( time_elapsed_string, time_elapsed );
                    io_three_strings( str2, " ", time_elapsed_string, " minutes ago", 0 );
                    io_string_write( str, str2, &string_index );
                }
            }
            else
            {
                if ( time_elapsed < 120 )
                {
                    io_string_write( str, " an hour ago", &string_index );
                }
                else
                {
                    n_string_block time_elapsed_string;
                    io_number_to_string( time_elapsed_string, time_elapsed / 60 );
                    io_three_strings( str2, " ", time_elapsed_string, " hours ago", 0 );
                    io_string_write( str, str2, &string_index );
                }
            }
        }
        else
        {
            if ( days_elapsed == 1 )
            {
                io_string_write( str, " yesterday", &string_index );
            }
            else
            {
                n_string_block days_elapsed_string;
                io_number_to_string( days_elapsed_string, days_elapsed );
                io_three_strings( str2, " ", days_elapsed_string, " days ago", 0 );
                io_string_write( str, str2, &string_index );
            }
        }
    }
#endif
    str[string_index] = 0;

    string_index = 0;

    io_string_write( description, str, &string_index );

    return social;
}


/**
 This checks to see if the Simulated Ape is awake
 @param local The simulation pointer
 @return 2 is fully awake, 1 is slightly awake to eat, 0 is asleep
 */
n_byte being_awake( simulated_being *local )
{
    if ( local )
    {
        if ( being_energy_less_than( local, BEING_DEAD + 1 ) )
        {
            return FULLY_ASLEEP;
        }

        /** if it is not night, the being is fully awake */
        if ( IS_NIGHT( land_time() ) == 0 )
        {
            return FULLY_AWAKE;
        }

        /** if it  night the being is... */

        /** ... fully awake to swim */
        {
            n_vect2 location;

            being_space( local, &location );
            spacetime_convert_to_map( &location );

            if ( WATER_TEST( land_location_vect( &location ), land_tide_level() ) )
            {
                return FULLY_AWAKE;
            }
        }
        /** ... slightly awake to eat */

        if ( being_energy_less_than( local, BEING_HUNGRY + 1 ) )
        {
#ifdef DEBUG_LACK_OF_MOVEMENT
            being_register_movement( local, "set slightly awake due to energy" );
#endif
            return SLIGHTLY_AWAKE;
        }

        /** ... slightly awake to slow down */

        if ( being_speed( local ) > 0 )
        {
#ifdef DEBUG_LACK_OF_MOVEMENT
            being_register_movement( local, "set slightly awake due to speed" );
#endif
            return SLIGHTLY_AWAKE;
        }

        /** ... asleep */
    }
    return FULLY_ASLEEP;
}

/*

 Rough sketch on brain compression method

 Hypthesis:

 From a zeroed brain,

 After n OUTPUT_ACTUATOR firings into the brain, and,

 After m brain_cycle occurrences with the states recorded,

 There should be a near identical brain to a standard run brain.

 n and m are independent but an acceptable level needs to be recorded.

 Data set:

 Type (either output actuator or brain cycle)

 For output actuator, record position in brain (n_byte2) and set value (n_byte)

 For brain cycle, record awake or asleep.

 Position in brain is on the 32k boundry, the delimiting could be:

 Position in brain - if 32k-1 then actuator set. If 64k-1, then brain cycle

 Set value is set value if actuator set, 0 is asleep, 1 is awake if brain cycle

 */


#ifdef BRAINCODE_ON
#ifdef BRAIN_ON

static void being_brain_probe( simulated_being *local )
{
    n_byte *local_brain = being_brain( local );
    n_int    i = 0;
    n_int    count[NUMBER_BRAINPROBE_TYPES] = {0};


    if ( local_brain == 0L )
    {
        return;
    }

    while ( i < BRAINCODE_PROBES )
    {
        count[local->braindata.brainprobe[i++].type]++;
    }

    /** check to ensure that there are a minimum number of sensors and actuators */
    if ( count[INPUT_SENSOR] < ( BRAINCODE_PROBES >> 2 ) )
    {
        local->braindata.brainprobe[0].type = INPUT_SENSOR;
    }
    else if ( count[OUTPUT_ACTUATOR] < ( BRAINCODE_PROBES >> 2 ) )
    {
        local->braindata.brainprobe[0].type = OUTPUT_ACTUATOR;
    }

    /** update each probe */
    i = 0;

    while ( i < BRAINCODE_PROBES )
    {
        local->braindata.brainprobe[i].state++;
        if ( local->braindata.brainprobe[i].state >= local->braindata.brainprobe[i].frequency )
        {
            n_byte *local_braincode = being_braincode_internal( local );
            /** position within the brain */
            n_int position_in_brain = ( ( local->braindata.brainprobe[i].position * ( SINGLE_BRAIN >> 8 ) ) ) & ( SINGLE_BRAIN - 1 );
            n_int position_in_braincode = local->braindata.brainprobe[i].address % BRAINCODE_SIZE;

            local->braindata.brainprobe[i].state = 0;

            if ( local->braindata.brainprobe[i].type == INPUT_SENSOR )
            {
                /** address within braincode */
                n_int set_value = ( local_brain[position_in_brain] + local->braindata.brainprobe[i].offset ) & 255;
                /** read from brain */
                local_braincode[position_in_braincode] = ( n_byte )set_value;
            }
            else
            {
                /** address within braincode */
                n_int set_value = ( local_braincode[position_in_braincode] + local->braindata.brainprobe[i].offset ) & 255;
                /** write to brain */
                local_brain[position_in_brain] = ( n_byte )set_value;
            }
        }
        i++;
    }
}
#endif
#endif

/** stuff still goes on during sleep */
void being_cycle_universal( simulated_being *local )
{
    n_int immune_energy_used = immune_response( &local->immune_system, being_honor_immune( local ), being_energy( local ) );

    NA_ASSERT( ( immune_energy_used >= 0 ), "Positive energy added back from immune response" );

    if ( immune_energy_used > 0 )
    {
        being_energy_delta( local, 0 - immune_energy_used );
    }

#ifdef BRAINCODE_ON
#ifdef BRAIN_ON
    /** may need to add external probe linking too */
    being_brain_probe( local );
#endif
#endif

    if ( ( local->delta.awake == 0 ) && local )
    {
        being_set_state( local, BEING_STATE_ASLEEP );
        being_reset_drive( local, DRIVE_FATIGUE );
    }
}

/* For a new child this populates the social graph with family relationships */
static void being_create_family_links( simulated_being *mother,
                                       simulated_being *child,
                                       simulated_group *group )
{
    n_int i, j, index;
    simulated_being *parent[6] = {0L};
    simulated_being *sibling;
    n_byte parent_relation[6];
    n_byte child_relation[6];
    n_byte sibling_relation;
    simulated_isocial *parent_social_graph;

    if ( mother == 0L )
    {
        return;
    }

    /** First tow entries in the array are parents.
     Subsequent entries are grandparents */
    parent[0] = mother;
    parent[1] = being_find_name( group, mother->changes.father_name[0], mother->changes.father_name[1] );

    parent_relation[0] = RELATIONSHIP_DAUGHTER;
    parent_relation[1] = RELATIONSHIP_DAUGHTER;
    parent_relation[2] = RELATIONSHIP_GRANDDAUGHTER;
    parent_relation[3] = RELATIONSHIP_GRANDDAUGHTER;
    parent_relation[4] = RELATIONSHIP_GRANDDAUGHTER;
    parent_relation[5] = RELATIONSHIP_GRANDDAUGHTER;

    child_relation[0] = RELATIONSHIP_MOTHER;
    child_relation[1] = RELATIONSHIP_MOTHER;
    child_relation[2] = RELATIONSHIP_MATERNAL_GRANDMOTHER;
    child_relation[3] = RELATIONSHIP_MATERNAL_GRANDMOTHER;
    child_relation[4] = RELATIONSHIP_PATERNAL_GRANDMOTHER;
    child_relation[5] = RELATIONSHIP_PATERNAL_GRANDMOTHER;

    /** grandparents */
    for ( j = 0; j < 2; j++ ) /** maternal or paternal */
    {
        if ( parent[j] )
        {
            /** social graph for mother or father */
            parent_social_graph = being_social( parent[j] );
            if ( parent_social_graph )
            {
                for ( i = 0; i < 2; i++ ) /** grandmother or grandfather */
                {
                    parent[2 + ( j * 2 ) + i] = 0L;
                    /** graph index for parent's mother or father */
                    index = social_get_relationship( parent[j], ( n_byte )( RELATIONSHIP_MOTHER + i ) );
                    if ( ( index > -1 ) && ( parent_social_graph != 0L ) )
                    {
                        /** store the grandparent reference if still living */
                        parent[2 + ( j * 2 ) + i] =
                            being_find_name( group,
                                             parent_social_graph[index].first_name[BEING_MET],
                                             parent_social_graph[index].family_name[BEING_MET] );
                    }
                }
            }
        }
    }

    /** brothers and sisters */
    sibling_relation = RELATIONSHIP_BROTHER;
    if ( FIND_SEX( GET_I( child ) ) == SEX_FEMALE )
    {
        sibling_relation = RELATIONSHIP_SISTER;
    }

    for ( j = 0; j < 2; j++ )
    {
        /** social graph for mother or father */
        if ( parent[j] )
        {
            parent_social_graph = being_social( parent[j] );
            if ( parent_social_graph )
            {
                for ( i = 1; i < SOCIAL_SIZE_BEINGS; i++ )
                {
                    if ( ( parent_social_graph[i].relationship == RELATIONSHIP_SON ) ||
                            ( parent_social_graph[i].relationship == RELATIONSHIP_DAUGHTER ) )
                    {
                        sibling = being_find_name( group, parent_social_graph[i].first_name[BEING_MET], parent_social_graph[i].family_name[BEING_MET] );
                        if ( sibling != 0L )
                        {
                            if ( parent_social_graph[i].relationship == RELATIONSHIP_SON )
                            {
                                social_set_relationship( group, child, RELATIONSHIP_BROTHER, sibling );
                            }
                            else
                            {
                                social_set_relationship( group, child, RELATIONSHIP_SISTER, sibling );
                            }
                            social_set_relationship( group, sibling, sibling_relation, child );
                        }
                    }
                }
            }
        }
    }

    /** set relationships */
    for ( i = 0; i < 6; i++ )
    {
        if ( parent[i] == 0L )
        {
            continue;
        }

        /** create the parent/child social graph relation */
        if ( FIND_SEX( GET_I( child ) ) == SEX_FEMALE )
        {
            social_set_relationship( group, parent[i], parent_relation[i], child );
        }
        else
        {
            social_set_relationship( group, parent[i], parent_relation[i] + 1, child );
        }

        if ( i % 2 == 0 )
        {
            social_set_relationship( group, child, child_relation[i], parent[i] );
        }
        else
        {
            social_set_relationship( group, child, child_relation[i] + 1, parent[i] );
        }
    }

}

void being_set_goal_mate( simulated_being *local, n_byte2 first_name, n_byte2 family_name )
{
    local->delta.goal[0] = GOAL_MATE;
    local->delta.goal[1] = first_name;
    local->delta.goal[2] = family_name;
    local->delta.goal[3] = GOAL_TIMEOUT;
}

void being_set_goal_none( simulated_being *local )
{
    local->delta.goal[0] = GOAL_NONE;
}

void being_set_goal_location( simulated_being *local, n_byte2 lx, n_byte2 ly )
{
    local->delta.goal[0] = GOAL_LOCATION;
    local->delta.goal[1] = lx;
    local->delta.goal[2] = ly;
    local->delta.goal[3] = GOAL_TIMEOUT;
}

n_int being_check_goal( simulated_being *local, goal_types goal )
{
    return ( local->delta.goal[0] == goal );
}

void being_goal_cycle( simulated_being *local )
{
    /** decrement the goal counter */
    if ( local->delta.goal[3] > 0 )
    {
        local->delta.goal[3]--;
    }
    else
    {
        /** timed out */
        being_set_goal_none( local );
    }
}

static void being_follow_loop1( simulated_group *group, simulated_being *other, void *data )
{
    being_nearest *nearest = ( being_nearest * )data;
    n_vect2        difference_vector;

    /** is this the same as the name of the being to which we are paying attention? */
    if ( ( FIND_SEX( GET_I( other ) ) != FIND_SEX( GET_I( nearest->local ) ) ) &&
            being_name_comparison( other, nearest->local->delta.goal[1], nearest->local->delta.goal[2] ) )
    {
        n_vect2 other_location;
        being_delta( nearest->local, other, &difference_vector );
        being_space( other, &other_location );

        if ( being_line_of_sight( nearest->local, &other_location ) ) /* incorrect use of los */
        {
            n_uint compare_distance = ( n_uint )vect2_dot( &difference_vector, &difference_vector, 1, 1 );
            if ( compare_distance < nearest->opposite_sex_distance )
            {
                nearest->opposite_sex = other;
                nearest->opposite_sex_distance = compare_distance;
            }
        }
    }
}

static void being_follow_loop2( simulated_group *group, simulated_being *other, void *data )
{
    being_nearest *nearest = ( being_nearest * )data;
    n_vect2        difference_vector;

    /** is this the same as the name of the being to which we are paying attention? */
    if ( being_name_comparison( other,
                                nearest->local_social->first_name[BEING_MET],
                                nearest->local_social->family_name[BEING_MET] ) )
    {
        /** Is this being within sight? */

        n_vect2 other_location;
        being_delta( nearest->local, other, &difference_vector );
        being_space( other, &other_location );
        if ( being_line_of_sight( nearest->local, &other_location ) )
        {
            n_uint compare_distance = ( n_uint )vect2_dot( &difference_vector, &difference_vector, 1, 1 );
            if ( FIND_SEX( GET_I( other ) ) != FIND_SEX( GET_I( nearest->local ) ) )
            {
                if ( compare_distance < nearest->opposite_sex_distance )
                {
                    nearest->opposite_sex = other;
                    nearest->opposite_sex_distance = compare_distance;
                }
            }
            else
            {
                if ( compare_distance < nearest->same_sex_distance )
                {
                    nearest->same_sex = other;
                    nearest->same_sex_distance = compare_distance;
                }
            }
        }
    }

}

/**
 * Follow a being to which we are paying attention
 * @param group Pointer to the simulated_group
 * @param local The current being
 * @param nearest The nearest being structure
 */
static void being_follow( simulated_group *group,
                          simulated_being *local,
                          being_nearest *nearest )
{
    /* There is a bug here where same_sex and opposite_sex appears to never be set */
    simulated_isocial *local_social_graph;
    n_int          social_graph_index;

    nearest->local = local;
    nearest->opposite_sex_distance = 0xffffffff;
    nearest->same_sex_distance = 0xffffffff;
    nearest->opposite_sex = 0L;
    nearest->same_sex = 0L;

    /** is a mate in view? */
    if ( being_check_goal( local, GOAL_MATE ) )
    {
        loop_no_thread( group, local, being_follow_loop1, nearest );
        if ( nearest->opposite_sex != 0L )
        {
            return;
        }
    }

    local_social_graph = being_social( local );
    if ( local_social_graph == 0L )
    {
        return;
    }

    /** which entry in the social graph are we paying attention to? */
    social_graph_index = being_attention( local, ATTENTION_ACTOR );

    nearest->local_social = &local_social_graph[social_graph_index];

    /** Does this entry correspond to another being? */
    if ( ( social_graph_index > 0 ) &&
            ( local_social_graph[social_graph_index].entity_type == ENTITY_BEING ) &&
            ( !SOCIAL_GRAPH_ENTRY_EMPTY( local_social_graph, social_graph_index ) ) )
    {
        loop_no_thread( group, local, being_follow_loop2, nearest );
    }
}

void being_listen_loop_no_sim( simulated_being *other, void *data )
{
    being_listen_struct *bls = ( being_listen_struct * )data;
    n_vect2       difference_vector;
    n_uint        compare_distance;

    being_delta( bls->local, other, &difference_vector );
    compare_distance = ( n_uint )vect2_dot( &difference_vector, &difference_vector, 1, 1 );
    /** listen for the nearest shout out */
    if ( ( being_state( other )&BEING_STATE_SHOUTING ) &&
            ( compare_distance < SHOUT_RANGE ) &&
            ( other->changes.shout[SHOUT_VOLUME] > bls->max_shout_volume ) )
    {
        bls->max_shout_volume = other->changes.shout[SHOUT_VOLUME];
        bls->local->changes.shout[SHOUT_HEARD] = other->changes.shout[SHOUT_CONTENT];
        bls->local->changes.shout[SHOUT_FAMILY0] = being_family_first_name( other );
        bls->local->changes.shout[SHOUT_FAMILY1] = being_family_second_name( other );
    }
}

/**
 * Listen for shouts
 * @param group Pointer to the simulated_group
 * @param local_being Array index of the current being
 * @param data Unused here
 */
void being_listen( simulated_group *group, simulated_being *local_being, void *data )
{
    being_listen_struct bls;

    if ( local_being->delta.awake == 0 )
    {
        return;
    }

    bls.max_shout_volume = 127;
    bls.local = local_being;
    /** clear shout values */
    if ( local_being->changes.shout[SHOUT_CTR] > 0 )
    {
        local_being->changes.shout[SHOUT_CTR]--;
    }
    loop_being_no_sim( group->beings, group->num, being_listen_loop_no_sim, &bls );
}

static void being_closest_loop( simulated_group *group, simulated_being *test_being, void *data )
{
    being_nearest *nearest = ( being_nearest * )data;
    n_vect2       difference_vector;
    n_uint        compare_distance;
    n_vect2       location_test;
    being_delta( nearest->local, test_being, &difference_vector );
    compare_distance = ( n_uint )vect2_dot( &difference_vector, &difference_vector, 1, 1 );

    if ( FIND_SEX( GET_I( test_being ) ) != FIND_SEX( GET_I( nearest->local ) ) )
    {
        if ( compare_distance < nearest->opposite_sex_distance )
        {
            being_space( test_being, &location_test );
            /* 'function' : conversion from 'n_int' to 'n_byte2', possible loss of data x 2 */
            if ( being_line_of_sight( nearest->local, &location_test ) )
            {
                nearest->opposite_sex_distance = compare_distance;
                nearest->opposite_sex = test_being;
            }
        }
    }
    else
    {
        if ( compare_distance < nearest->same_sex_distance )
        {
            being_space( test_being, &location_test );

            if ( being_line_of_sight( nearest->local, &location_test ) )
            {
                nearest->same_sex_distance = compare_distance;
                nearest->same_sex = test_being;
            }
        }
    }
}

/**
 * Returns the closest beings
 * @param group Pointer to the simulated_group
 * @param local Array index of the current being
 */
static void being_closest( simulated_group *group,
                           simulated_being *local,
                           being_nearest *nearest )
{
    nearest->local = local;
    nearest->opposite_sex_distance = 0xffffffff;
    nearest->same_sex_distance = 0xffffffff;
    nearest->opposite_sex = 0L;
    nearest->same_sex = 0L;
    loop_no_thread( group, local, being_closest_loop, nearest );
}

/**
 * One being interacts with another
 * @param group Pointer to the simulated_group
 * @param local Array index of the being
 * @param other_being Array index of the other being
 * @param other_being_distance Distance to the other being
 * @param awake Whether the being is awake
 * @param state The state of the being
 * @param speed The speed of the being
 * @param opposite_sex Non zero if the other being is the opposite sex
 */
static void being_interact( simulated_group *group,
                            simulated_being *local,
                            simulated_being     *other_being,
                            n_uint   other_being_distance,
                            n_int   *awake,
                            n_byte2 *state,
                            n_int   *speed,
                            n_byte   opposite_sex )
{
    if ( other_being != 0L )
    {
        n_int         today_days   = land_date();
        n_int         birth_days   = being_dob( local );
        n_int         local_is_female = FIND_SEX( GET_I( local ) );

        n_vect2 delta_vector;

        /** social networking */
        n_byte2 familiarity = 0;
        n_int   being_index = social_network( group, local, other_being, other_being_distance );

        being_delta( local, other_being, &delta_vector );

        if ( being_index > -1 )
        {
            simulated_isocial *local_social_graph = being_social( local );
            if ( local_social_graph )
            {
                familiarity = local_social_graph[being_index].familiarity;
            }
        }

        being_facing_towards( local, &delta_vector );

        if ( ( birth_days + AGE_OF_MATURITY ) < today_days )
        {
            if ( social_groom( group, local, other_being, other_being_distance, *awake, familiarity ) )
            {
                *state |= BEING_STATE_GROOMING;

                /* both beings stop */
                *speed = 0;
                being_set_speed( other_being, 0 );
            }
            else
            {
                /* squabbling between adults */
                if ( ( other_being_distance < SQUABBLE_RANGE ) && ( ( being_dob( other_being ) + AGE_OF_MATURITY ) < today_days ) )
                {
                    n_byte2 squabble_val;
                    being_set_speed( local, ( n_byte )*speed );
#ifdef DEBUG_LACK_OF_MOVEMENT
                    if ( *speed == 0 )
                    {
                        being_register_movement( local, "speed is zero" );
                    }
#endif
                    squabble_val = social_squabble( local, other_being, other_being_distance, local_is_female, group );
                    if ( squabble_val != 0 )
                    {
                        *state |= squabble_val;
                        *speed = being_speed( local );
                    }
                }
            }

        }
        if ( ( other_being_distance < SOCIAL_RANGE ) && ( being_index > -1 ) )
        {
            /* attraction and mating */
            if ( opposite_sex != 0 )
            {
                *state |= social_mate( local, other_being, being_index, other_being_distance, group );
            }

            /* chat */
            *state |= social_chat( local, other_being, being_index, group );
        }
    }
}

typedef struct
{
    n_int counter;
    n_int return_value;
    simulated_being *being;
} being_index_loop_struct;

static void being_index_loop( simulated_group *group, simulated_being *local_being, void *data )
{
    being_index_loop_struct *bils = ( being_index_loop_struct * ) data;

    if ( bils->return_value != -1 )
    {
        return;
    }

    if ( local_being == bils->being )
    {
        bils->return_value = bils->counter;
    }
    else
    {
        bils->counter++;
    }
}

n_int being_index( simulated_group *group, simulated_being *local )
{
    being_index_loop_struct value;

    value.return_value = -1;
    value.being = local;
    value.counter = 0;

    loop_no_thread( group, 0L, being_index_loop, &value );
    return value.return_value;
}

void being_territory_index( simulated_being *local )
{
    n_uint territory_index =
        APESPACE_TO_TERRITORY( being_location_y( local ) ) * TERRITORY_DIMENSION +
        APESPACE_TO_TERRITORY( being_location_x( local ) );

    if ( local->events.territory[territory_index].familiarity < 65534 )
    {
        local->events.territory[territory_index].familiarity++;
    }
    else
    {
        /** rescale familiarity values */
        for ( territory_index = 0; territory_index < TERRITORY_AREA; territory_index++ )
        {
            local->events.territory[territory_index].familiarity >>= 2;
        }
    }
}

static n_int being_temporary_speed( simulated_being *local, n_int *test_land, n_int *az )

{
    n_vect2 location_vector;
    n_vect2 facing_vector;
    n_vect2 slope_vector;
    n_vect2 looking_vector;

    being_space( local, &location_vector );

    being_facing_vector( local, &facing_vector, 4 );
    land_vect2( &slope_vector, az, &location_vector );
    vect2_add( &looking_vector, &location_vector, &facing_vector );
    spacetime_convert_to_map( &looking_vector );

    *test_land = ( WATER_TEST( land_location_vect( &looking_vector ), land_tide_level() ) != 0 );

    {
        n_int delta_z = vect2_dot( &slope_vector, &facing_vector, 1, 24 );
        n_int tmp_speed = ( ( delta_z + 280 ) >> 4 );
#ifdef DEBUG_LACK_OF_MOVEMENT
        if ( tmp_speed == 0 )
        {
            being_register_movement( local, "temp speed zero in setting temp speed" );
        }
#endif
        return tmp_speed;
    }
}

static n_int being_conception_child_mass( simulated_group *group, simulated_being *local, n_byte2 loc_state )
{
    n_int         birth_days = being_dob( local );
    n_int         today_days = land_date();
    n_int         child_mass = 0;
    n_int         carrying_child = 0;
    n_genetics   *genetics = being_genetics( local );

    /** a certain time after giving birth females become receptive again */
    if ( ( being_pregnant( local ) != 0 ) &&
            ( ( being_pregnant( local ) + GESTATION_DAYS + CONCEPTION_INHIBITION_DAYS ) < today_days ) )
    {
        /** zero value indicates ready to conceive */
        local->changes.date_of_conception = 0;
    }

    if ( ( loc_state & ( BEING_STATE_AWAKE | BEING_STATE_SWIMMING ) ) == BEING_STATE_AWAKE )
    {
        n_int conception_days = being_pregnant( local ) ;
        if ( conception_days > 0 )
        {
            n_int gestation_days = conception_days + GESTATION_DAYS;
            if ( today_days > gestation_days )
            {
                /** A mother could have multiple children, so only find the youngest */
                simulated_being *being_child = being_find_child( group, genetics, CARRYING_DAYS );

                /** Birth */
                if ( being_child == 0L )
                {
                    if ( ( group->num + 1 ) < group->max )
                    {
                        being_child = &( group->beings[group->num] );

                        if ( being_init( group->beings, ( n_int )group->num, being_child, local, 0L ) == 0 )
                        {
#ifdef EPISODIC_ON
                            episodic_close( local, being_child, EVENT_BIRTH, AFFECT_BIRTH, 0 );
#endif
                            being_create_family_links( local, being_child, group );
                            if ( group->ext_birth != 0 )
                            {
                                group->ext_birth( being_child, local, group );
                            }
                            group->num++;
                        }
                    }
                }
                else
                {
                    /** mother carries the child */
                    n_int carrying_days = conception_days + GESTATION_DAYS + CARRYING_DAYS;
                    if ( today_days < carrying_days )
                    {
                        if ( !( ( local->changes.inventory[BODY_FRONT] & INVENTORY_CHILD ) ||
                                ( local->changes.inventory[BODY_BACK] & INVENTORY_CHILD ) ) )
                        {
                            local->changes.inventory[BODY_BACK] |= INVENTORY_CHILD;
                            being_set_attention( local, ATTENTION_BODY, BODY_BACK );
                        }
                        carrying_child = 1;

                        being_set_location( being_child, being_location( local ) );

                        child_mass = GET_M( being_child );
#ifdef EPISODIC_ON
                        episodic_close( local, being_child, EVENT_CARRIED, AFFECT_CARRYING, 0 );
                        episodic_close( being_child, local, EVENT_CARRIED_BY, AFFECT_CARRIED, 0 );
#endif
                    }
                }
            }
            else
            {
                /** Compute the mass of the unborn child.
                 This will be added to the mass of the mother */
                child_mass = ( today_days - conception_days ) * BIRTH_MASS / GESTATION_DAYS;
            }
        }

        /** child follows the mother */
        if ( ( birth_days + WEANING_DAYS ) > today_days )
        {
            simulated_being *mother = being_find_closest( group, local, 1, 1 );
            if ( mother != 0L )
            {
                /** orient towards the mother */
                n_vect2    mother_vector;

                being_delta( mother, local, &mother_vector );

                being_facing_towards( local, &mother_vector );

                /** suckling */
                if ( ( loc_state & BEING_STATE_HUNGRY ) != 0 )
                {
                    n_int distance = vect2_dot( &mother_vector, &mother_vector, 1, 1 );
                    if ( distance < SUCKLING_MAX_SEPARATION )
                    {
                        /** child moves from back to front */
                        if ( mother->changes.inventory[BODY_BACK] & INVENTORY_CHILD )
                        {
                            mother->changes.inventory[BODY_BACK] -= INVENTORY_CHILD;
                        }
                        mother->changes.inventory[BODY_FRONT] |= INVENTORY_CHILD;
                        being_set_attention( mother, ATTENTION_BODY, BODY_FRONT );
                        /** sucking causes loss of grooming */
                        if ( mother->changes.inventory[BODY_FRONT] & INVENTORY_GROOMED )
                        {
                            mother->changes.inventory[BODY_FRONT] -= INVENTORY_GROOMED;
                        }
                        /** hungry mothers stop producing milk */
                        if ( being_energy_less_than( mother, BEING_HUNGRY ) == 0 )
                        {
                            /** mother loses energy */
                            being_energy_delta( mother, 0 - SUCKLING_ENERGY );
                            /** child gains energy */

                            being_energy_delta( local, SUCKLING_ENERGY );

                            /** set child state to suckling */
                            loc_state |= BEING_STATE_SUCKLING;
                            /** child acquires immunity from mother */
                            immune_seed( &mother->immune_system, &local->immune_system );
#ifdef EPISODIC_ON
                            episodic_close( mother, local, EVENT_SUCKLED, AFFECT_SUCKLING, 0 );
                            episodic_close( local, mother, EVENT_SUCKLED_BY, AFFECT_SUCKLING, 0 );
#endif
                        }
                    }
                }
            }
        }
    }

    /** no longer carrying the child */
    if ( ( carrying_child == 0 ) && ( FIND_SEX( GET_I( local ) ) == SEX_FEMALE ) )
    {
        if ( local->changes.inventory[BODY_FRONT] & INVENTORY_CHILD )
        {
            local->changes.inventory[BODY_FRONT] -= INVENTORY_CHILD;
        }
        if ( local->changes.inventory[BODY_BACK] & INVENTORY_CHILD )
        {
            local->changes.inventory[BODY_BACK] -= INVENTORY_CHILD;
        }
    }
    return child_mass;
}

static n_byte2 being_state_find( simulated_being *local, n_int az, n_int loc_s )
{
    n_byte2        loc_state  = BEING_STATE_ASLEEP;
    n_int         awake = local->delta.awake;

#ifdef LAND_ON
    /* TODO why not test_land here? */
    if ( WATER_TEST( az, land_tide_level() ) != 0 )
    {
        loc_state |= BEING_STATE_SWIMMING;
    }
#endif

    if ( awake != FULLY_ASLEEP )
    {
        loc_state |= BEING_STATE_AWAKE;
    }

    if ( loc_s != 0 )
    {
        loc_state |= BEING_STATE_MOVING;
    }

    {
        n_int   hungry = being_energy_less_than( local, BEING_HUNGRY );

        if ( ( loc_state & ( BEING_STATE_AWAKE | BEING_STATE_SWIMMING | BEING_STATE_MOVING ) ) == BEING_STATE_AWAKE )
        {
            hungry = being_energy_less_than( local, BEING_FULL );
#ifdef DEBUG_LACK_OF_MOVEMENT
            being_register_movement( local, "is eating path" );
#endif
        }

        if ( hungry != 0 )
        {
#ifdef DEBUG_LACK_OF_MOVEMENT
            being_register_movement( local, "is hungry" );
#endif
            loc_state |= BEING_STATE_HUNGRY;
        }
    }
    return loc_state;
}

static void being_not_swimming( simulated_group *group, simulated_being *local, n_int *tmp_speed, being_nearest *nearest, n_int *loc_s, n_byte2 *loc_state )
{
    n_genetics   *genetics = being_genetics( local );

    /** adjust speed using genetics */
    *tmp_speed = ( *tmp_speed * ( GENE_SPEED( genetics ) + 8 ) ) >> 3;

    /** is the being to which we are paying attention within view? */
    being_follow( group, local, nearest );
    if ( nearest->opposite_sex == 0L )
    {
        /** Find the closest beings */
        being_closest( group, local, nearest );
    }

    /* TODO: SOCIAL_THRESHOLD should not be a macro, it should be a function returning n_int */
    if ( being_drive( local, DRIVE_SOCIAL ) > SOCIAL_THRESHOLD( local ) )
    {
        n_int  awake = local->delta.awake;

        being_interact( group,
                        local,
                        nearest->same_sex, nearest->same_sex_distance,
                        &awake, loc_state,
                        loc_s, 0 );

        being_interact( group,
                        local,
                        nearest->opposite_sex, nearest->opposite_sex_distance,
                        &awake, loc_state,
                        loc_s, 1 );
    }
}

static void being_swimming( simulated_group *group, simulated_being *local, n_int *tmp_speed )
{
    n_uint          loop;
    n_genetics   *genetics = being_genetics( local );

    being_turn_away_from_water( local );

    /** horizontally oriented posture */
    being_set_posture( local, 0 );

    /** When swimming drop everything except what's on your head or back.
     Note that the groomed flag is also cleared */

    for ( loop = 0; loop < INVENTORY_SIZE; loop++ )
    {
        if ( !( ( loop == BODY_HEAD ) || ( loop == BODY_BACK ) ) )
        {
            local->changes.inventory[loop] = 0;
        }
    }
    /** swimming proficiency */
    *tmp_speed = ( *tmp_speed * ( GENE_SWIM( genetics ) + 8 ) ) >> 4;

    /* TODO: affect_type should probably be used rather than energy? */
#ifdef EPISODIC_ON
    episodic_self( local, EVENT_SWIM, AFFECT_GROOM, ( n_byte2 )being_energy( local ) );
#endif
    /** bathing removes parasites */
    being_remove_parasites( local, 1 );
}

static void being_mass_calculation( simulated_group *group, simulated_being *local, n_byte2  loc_state )
{
    n_int          loc_h      = being_height( local );
    n_int child_mass = being_conception_child_mass( group, local, loc_state );
    /** amount of body fat in kg */
    n_int fat_mass = GET_BODY_FAT( local );
    if ( fat_mass > BEING_MAX_MASS_FAT_G )
    {
        fat_mass = BEING_MAX_MASS_FAT_G;
    }
    GET_M( local ) = ( n_byte2 )( ( BEING_MAX_MASS_G * loc_h / BEING_MAX_HEIGHT ) + fat_mass + child_mass );
}

void being_genetic_wandering( simulated_being *local, being_nearest *nearest )
{
    n_genetics   *genetics = being_genetics( local );

    if ( being_check_goal( local, GOAL_NONE ) &&
            ( nearest->opposite_sex == 0L ) &&
            ( nearest->same_sex == 0L ) &&
            ( being_random( local ) < 1000 + 3600 * GENE_STAGGER( genetics ) ) )
    {
        n_int     wander = math_spread_byte( being_random( local ) & 7 );
        being_wander( local, wander );
    }
}

void being_calculate_speed( simulated_being *local, n_int tmp_speed, n_byte2 loc_state )
{
    n_int          loc_s      = being_speed( local );

    if ( tmp_speed > 39 )
    {
        tmp_speed = 39;
    }
    if ( tmp_speed < 0 )
    {
        tmp_speed = 0;
    }

    if ( ( local->delta.awake != FULLY_AWAKE ) || ( ( loc_state & ( BEING_STATE_HUNGRY | BEING_STATE_NO_FOOD ) ) == BEING_STATE_HUNGRY ) )
    {
        if ( ( loc_state & BEING_STATE_SWIMMING ) != 0 )
        {
            tmp_speed = ( being_energy( local ) >> 7 );
        }
        else
        {
            if ( ( loc_state & BEING_STATE_NO_FOOD ) != BEING_STATE_NO_FOOD )
            {
                tmp_speed = 0;
#ifdef DEBUG_LACK_OF_MOVEMENT
                being_register_movement( local, "not fully awake and hungry not swimming" );
#endif
            }
        }
    }

    if ( tmp_speed > loc_s )
    {
        loc_s++;
    }
    if ( tmp_speed < loc_s )
    {
        loc_s--;
    }
    if ( tmp_speed < loc_s )
    {
        loc_s--;
    }
    if ( tmp_speed < loc_s )
    {
        loc_s--;
    }

    being_set_speed( local, ( n_byte )loc_s );
}

void being_cycle_awake( simulated_group *group, simulated_being *local )
{
    n_int          loc_s      = being_speed( local );
    n_int          loc_h      = being_height( local );
    n_int         birth_days = being_dob( local );
    n_int         today_days = land_date();

    /** tmp_speed is the optimum speed based on the gradient */
    /** delta_energy is the energy required for movement */
    n_int    az;
    being_nearest nearest;
    n_int   test_land = 1;
    n_int   tmp_speed = being_temporary_speed( local, &test_land, &az );
    n_byte2  loc_state = being_state_find( local, az, loc_s );

    nearest.opposite_sex = 0L;
    nearest.same_sex = 0L;

    /** If it sees water in the distance then turn */
    if ( ( ( loc_state & BEING_STATE_SWIMMING ) != 0 ) || test_land )
    {
        being_swimming( group, local, &tmp_speed );
    }
    else
    {
        being_not_swimming( group, local, &tmp_speed, &nearest, &loc_s, &loc_state );
    }
#ifdef DEBUG_LACK_OF_MOVEMENT
    if ( tmp_speed == 0 )
    {
        being_register_movement( local, "temp speed zero" );
    }
#endif
    if ( ( loc_state & ( BEING_STATE_SWIMMING | BEING_STATE_GROOMING | BEING_STATE_ATTACK | BEING_STATE_SHOWFORCE ) ) == 0 )
    {
        if ( ( loc_state & BEING_STATE_HUNGRY ) == BEING_STATE_HUNGRY )
        {
            if ( loc_s == 0 )
            {
                /** eating when stopped */
                n_byte  food_type;
                n_int   energy = food_eat( being_location_x( local ), being_location_y( local ), az, &food_type, local );

#ifdef DEBUG_LACK_OF_MOVEMENT
                {
                    n_string_block energy_string;
                    sprintf( energy_string, "energy delta is %ld tmp_speed is %ld", energy, tmp_speed );
                    being_register_movement( local, energy_string );
                }
#endif
                if ( energy != 0 )
                {
#ifdef EPISODIC_ON
                    /** remember eating */
                    episodic_food( local, energy, food_type );
#endif

                    being_energy_delta( local, energy );
                    being_reset_drive( local, DRIVE_HUNGER );
                    loc_state |= BEING_STATE_EATING;
                    /** grow */
                    if ( loc_h < BEING_MAX_HEIGHT )
                    {
                        if ( ( birth_days + AGE_OF_MATURITY ) > today_days )
                        {
                            loc_h += ENERGY_TO_GROWTH( local, energy );
                        }
                    }
                }
                else
                {
                    loc_state |= BEING_STATE_NO_FOOD;
#ifdef DEBUG_LACK_OF_MOVEMENT
                    {
                        being_register_movement( local, "no food state set" );
                    }
#endif
                }
            }
        }
        else
        {
            /** orient towards a goal */
            social_goals( local );
            if ( loc_s == 0 )
            {
                loc_s = 10;
            }
        }
    }

    being_set_height( local, loc_h );
    being_set_state( local, loc_state );
    being_calculate_speed( local, tmp_speed, loc_state );
    being_genetic_wandering( local, &nearest );
#ifdef TERRITORY_ON
    being_territory_index( local );
#endif
    being_mass_calculation( group, local, loc_state );
}

void being_tidy_loop_no_sim( simulated_being *local_being, void *data )
{
    n_genetics  *genetics = being_genetics( local_being );
    n_int        local_honor = being_honor( local_being );
    n_int        delta_e = 0;
    n_int        conductance = 5;
    n_int       *max_honor = data;
    if ( local_honor >= 254 )
    {
        max_honor[0] = 1;
    }
    if ( local_being->delta.awake != FULLY_ASLEEP )
    {
        delta_e = being_move_energy( local_being, &conductance );
    }
    else
    {
        being_set_speed( local_being, 0 );
#ifdef DEBUG_LACK_OF_MOVEMENT
        being_register_movement( local_being, "not fully awake" );
#endif
        delta_e += ( 7 ) >> 2;
    }

    if ( delta_e > 0 )
    {
        /** hairy creatures are better insulated */
        delta_e -= ( ( GENE_HAIR( genetics ) * delta_e ) >> conductance );
        if ( delta_e < 1 )
        {
            delta_e = 1;
        }
    }

    being_energy_delta( local_being, 0 - delta_e );

    if ( land_time() == 0 )
    {
        n_int age_in_years = AGE_IN_YEARS( local_being );
        /** this simulates natural death or at least some trauma the ape may or may not be able to recover from */
        if ( age_in_years > 29 )
        {
            if ( being_random( local_being ) < ( age_in_years - 29 ) )
            {
                being_energy_delta( local_being, 0 - BEING_HUNGRY );
            }
        }
    }
}

void  being_recalibrate_honor_loop_no_sim( simulated_being *value )
{
    value->delta.honor = ( n_byte )( ( ( n_int )( value->delta.honor ) * 220 ) / 255 );
}

static n_int being_remove_internal_value = 0;
static n_int being_remove_external_value = 0;

n_int being_remove_internal( void )
{
    return being_remove_internal_value;
}

void being_remove_external_set( n_int value )
{
    being_remove_external_value = value;
}

void being_remove_loop1( simulated_group *group, simulated_being *local_being, void *data )
{
    if ( being_energy_less_than( local_being, BEING_DEAD + 1 ) )
    {
        group->ext_death( local_being, group );
    }
}

void being_remove_loop2( simulated_group *group, simulated_being *local, void *data )
{
    being_remove_loop2_struct *brls = ( being_remove_loop2_struct * )data;

    if ( being_energy_less_than( local, BEING_DEAD + 1 ) == 0 )
    {
        if ( local != brls->being_count )
        {
            memory_copy( ( n_byte * )local, ( n_byte * )brls->being_count, sizeof( simulated_being ) );
        }
        brls->being_count++;
        brls->count++;
    }
    else
    {
        being_remains( group, local );
        if ( local == brls->reference )
        {
            brls->selected_died = 1;
        }
    }
}

being_remove_loop2_struct *being_remove_initial( simulated_group *group )
{
    being_remove_loop2_struct *brls = ( being_remove_loop2_struct * )memory_new( sizeof( being_remove_loop2_struct ) );

    brls->reference = group->select;
    brls->being_count = group->beings;
    brls->selected_died = 0;
    brls->count = 0;

    if ( being_remove_external_value )
    {
        do {}
        while ( being_remove_external_value );
    }

    being_remove_internal_value = 1;
    return brls;
}

void being_remove_internal_clear( void )
{
    being_remove_internal_value = 0;
}

