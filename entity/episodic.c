/****************************************************************

 episodic.c

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

/*! \file   episodic.c
 *  \brief  This handles episodic memory
 */

#include "entity.h"
#include "entity_internal.h"

/*! /file episodic.c
 * /brief Functions relating to updates of episodic memories and intentions.
 */

#ifdef EPISODIC_ON

static n_console_output *local_logging;
static n_int local_social;

void episodic_logging( n_console_output *output_function, n_int social )
{
    local_logging = output_function;
    local_social = social;
}

/**
 * @brief If the given episodic memory is an intention, as defined by the event type,
 * then update the learned preferences based upon the intention type.
 * For example, if the ape intends to chat then the chatting preference
 * may be increased which makes chatting more likely.
 * @param local pointer to the particular ape
 * @param episode_index array index of the episodic memory representing the intention
 */
static void episodic_intention_update( simulated_being *local, n_int episode_index )
{
    simulated_iepisodic *local_episodic = being_episodic( local );
    n_byte event;
    n_int learned_preference_index = -1;
    if ( local_episodic == 0L )
    {
        return;
    }
    event = local_episodic[episode_index].event - EVENT_INTENTION;
    switch ( event )
    {
    case EVENT_CHAT:
    {
        learned_preference_index = PREFERENCE_CHAT;
        break;
    }
    case EVENT_GROOM:
    {
        if ( ( local_episodic[episode_index].arg & 2 ) != 0 )
        {
            learned_preference_index = PREFERENCE_GROOM_MALE;
        }
        else
        {
            learned_preference_index = PREFERENCE_GROOM_FEMALE;
        }
        break;
    }
    }

    /** alter preferences */
    if ( learned_preference_index > -1 )
    {
        if ( ( local_episodic[episode_index].arg & 1 ) != 0 )
        {
            if ( local->changes.learned_preference[learned_preference_index] < 255 )
            {
                local->changes.learned_preference[learned_preference_index]++;
            }
        }
        else
        {
            if ( local->changes.learned_preference[learned_preference_index] > 0 )
            {
                local->changes.learned_preference[learned_preference_index]--;
            }
        }
    }
}

/**
 * @brief Update the episodic memories for a given ape.
 * This is based upon a fading memory model in which older memories
 * are replaced by newer ones.  Each memory has an associated affect
 * value indicating its emotional impact, and this fades over time.
 *
 * The rate of fading is genetically regulated, with different rates
 * for memories with positive and negative affect.
 * This facilitates optimistic/pessimistic and forgetful/memorable
 * type personalities.
 *
 * The fading memory model may not be strictly realistic, and might
 * be replaced by something else in future.
 * @param local_being pointer to the ape
 */
void episodic_cycle_no_sim( simulated_being *local_being )
{
    if ( local_being->delta.awake == 0 )
    {
        return;
    }
    {
        n_int i;
        simulated_iepisodic *local_episodic = being_episodic( local_being );
        n_genetics *genetics = being_genetics( local_being );

        if ( !local_episodic )
        {
            return;
        }

        for ( i = 0; i < EPISODIC_SIZE; i++ )
        {
            if ( local_episodic[i].event == 0 )
            {
                continue;
            }

            /** remove intentions which are outdated */
            if ( local_episodic[i].event >= EVENT_INTENTION )
            {
                /** is this my intention, or someone else's? */
                if ( being_name_comparison( local_being, local_episodic[i].first_name[BEING_MEETER], local_episodic[i].family_name[BEING_MEETER] ) )
                {
                    if ( spacetime_before_now( &local_episodic[i].space_time ) )
                    {
                        local_episodic[i].event = 0;
                        continue;
                    }
                }
                episodic_intention_update( local_being, i );
            }

            /** fade towards EPISODIC_AFFECT_ZERO */
            if ( local_episodic[i].affect < EPISODIC_AFFECT_ZERO )
            {
                /** negative memories fade */
                if ( EPISODIC_AFFECT_ZERO - local_episodic[i].affect > 16 )
                {
                    local_episodic[i].affect += ( 1 + GENE_NEGATIVE_AFFECT_FADE( genetics ) );
                }
                else
                {
                    local_episodic[i].affect++;
                }
            }
            else
            {
                if ( local_episodic[i].affect > EPISODIC_AFFECT_ZERO )
                {
                    /** positive memories fade */
                    if ( local_episodic[i].affect - EPISODIC_AFFECT_ZERO > 16 )
                    {
                        local_episodic[i].affect -= ( 1 + GENE_POSITIVE_AFFECT_FADE( genetics ) );
                    }
                    else
                    {
                        local_episodic[i].affect--;
                    }
                }
            }
        }
    }
}

/**
 * @brief Returns a celebrity factor based upon how many apes within
 * the episodic memory of the given ape have a similar name to the
 * met ape, and their friend or foe values.
 * This means that initial beliefs about other apes are partly
 * a form of stereotyping
 * @param meeter_being pointer to the ape
 * @param met_being pointer to another ape
 * @return celebrity value of the met ape
 */
n_int episodic_met_being_celebrity(
    simulated_being *meeter_being,
    simulated_being *met_being )
{
    n_int i, j, celebrity = 0, ctr, aff;
    simulated_iepisodic *meeter_episodic = being_episodic( meeter_being );
    n_byte2 first_name = being_gender_name( met_being );
    n_byte2 family_name = being_family_name( met_being );

    if ( !meeter_episodic )
    {
        return 0;
    }

    /** check all episodic memories of the meeter */
    for ( i = 0; i < EPISODIC_SIZE; i++ )
    {
        aff = ( n_int )( meeter_episodic[i].affect ) - EPISODIC_AFFECT_ZERO;
        if ( aff > 1 )
        {
            aff = 1;
        }
        if ( aff < -1 )
        {
            aff = -1;
        }

        /** check both the meeter and the met ape for each memory */
        for ( j = BEING_MEETER; j <= BEING_MET; j++ )
        {
            ctr = 0;
            /** same first name */
            if ( meeter_episodic[i].first_name[j] == first_name )
            {
                celebrity += aff;
                ctr++;
            }
            /** same family name */
            if ( meeter_episodic[i].family_name[j] == family_name )
            {
                celebrity += aff;
                ctr++;
            }
            /** if both first name and family name match then
            increase the celebrity value further */
            if ( ctr == 2 )
            {
                celebrity += aff * 2;
            }
        }
    }

    /** limit within range */
    if ( celebrity > 16 )
    {
        celebrity = 16;
    }
    if ( celebrity < -16 )
    {
        celebrity = -16;
    }
    return celebrity;
}

/**
 * @brief This returns the percentage of episodic memories or intentions which are first person.
 * Some memories originate from the self and others are acquired from others via chatting.
 * @param local pointer to the ape
 * @param intention 0=episodic memories, 1=intentions
 * @return percentage in the range 0-100
 */
n_int episodic_first_person_memories_percent(
    simulated_being *local,
    n_byte intention )
{
    n_int i, hits = 0, memories = 0;
    simulated_iepisodic *local_episodic = being_episodic( local );
    if ( local_episodic == 0L )
    {
        return 0;
    }

    /** examine all memories */
    for ( i = 0; i < EPISODIC_SIZE; i++ )
    {
        if ( local_episodic[i].event > 0 )
        {
            if ( intention != 0 )
            {
                /** ratio of intentions to other memories */
                if ( local_episodic[i].event >= EVENT_INTENTION )
                {
                    hits++;
                }
            }
            else
            {
                /** ratio of first person memories to other memories */
                if ( being_name_comparison( local, local_episodic[i].first_name[BEING_MEETER], local_episodic[i].family_name[BEING_MEETER] ) )
                {
                    hits++;
                }
            }
            memories++;
        }
    }
    if ( memories > 0 )
    {
        return hits * 100 / memories;
    }
    else
    {
        if ( intention != 0 )
        {
            return 0;
        }
        return 100;
    }
}

/**
 * @brief Returns the index of the the episodic memory which can be overwritten with a new one.
 * @param event The type of event
 * @param affect The affect value associated with the event
 * @param name1 Name of the first ape in the memory (meeter)
 * @param family1 Family name of the first ape in the memory (meeter)
 * @param name2 Name of the second ape in the memory (met)
 * @param family2 Family name of the second ape in the memory (met)
 * @param local Pointer to the ape
 * @return array index of the episodic memory which can be replaced.
 */
static n_int simulated_iepisodic_replace_index(
    being_episodic_event_type event,
    n_int affect,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2,
    simulated_being *local )
{
    /** absolute affect value */
    n_int abs_aff = affect;
    n_int i;
    n_int replace = -1;
    n_int min;
    n_byte event_exists = 0;

    simulated_iepisodic *local_episodic = being_episodic( local );

    if ( !local_episodic )
    {
        return -1;
    }

    /** replace only events with an affect lower then the current */
    abs_aff = ABS( abs_aff );
    min = abs_aff;
    for ( i = 0; i < EPISODIC_SIZE; i++ )
    {
        /** is this the same type of event */
        if ( local_episodic[i].event == event )
        {
            /** is this the same being? */
            if ( ( local_episodic[i].first_name[BEING_MEETER] == name1 ) &&
                    ( local_episodic[i].family_name[BEING_MEETER] == family1 ) )
            {
                /** get absolute affect value */
                n_int aff1 = ABS( ( n_int )( local_episodic[i].affect ) - EPISODIC_AFFECT_ZERO );
                /** does this have the least affect (most forgettable) */
                event_exists = 1;
                if ( aff1 <= min )
                {
                    min = aff1;
                    replace = i;
                }
            }
        }
    }

    if ( event_exists == 0 )
    {
        /** Use any empty memory slots */
        for ( i = 0; i < EPISODIC_SIZE; i++ )
        {
            if ( local_episodic[i].event == 0 )
            {
                return i;
            }
        }

        /** no event of this type was found,
        so search for any event with the lowest affect */
        min = abs_aff;
        for ( i = 0; i < EPISODIC_SIZE; i++ )
        {
            /** get absolute affect value */
            n_int aff1 = ABS( ( n_int )( local_episodic[i].affect ) - EPISODIC_AFFECT_ZERO );
            /** does this have the least affect (most forgettable) */
            if ( aff1 < min )
            {
                min = aff1;
                replace = i;
            }
        }
    }

    return replace;
}

/**
 * @brief Stores an episodic memory
 * @param local Pointer to the being
 * @param event The type of event
 * @param affect Affect value associated with the event
 * @param name1 First name of a being participating in the event
 * @param family1 Family names of a being participating in the event
 * @param name2 First name of a second being participating in the event
 * @param family2 Family names of a second being participating in the event
 * @param arg Any additional argument
 * @param food Type of food
 */
static void episodic_store_full(
    simulated_being *local,
    being_episodic_event_type event,
    n_int affect,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2,
    n_byte2 arg,
    n_byte food )
{
    simulated_iepisodic *local_episodic = being_episodic( local );
    n_int replace;
    n_byte  old_event;
    n_byte4 old_time;
    n_byte4 new_time;

    if ( local_episodic == 0L )
    {
        return;
    }

    if ( local->delta.awake == FULLY_ASLEEP )
    {
        return;
    }

    replace = simulated_iepisodic_replace_index( event, affect, name1, family1, name2, family2, local );

    if ( replace == -1 )
    {
        return;
    }

    old_event = local_episodic[replace].event;
    old_time = local_episodic[replace].space_time.time;

    /** insert the current event into the episodic memory */
    local_episodic[replace].event       = event;
    local_episodic[replace].affect      = ( n_byte2 )( affect + EPISODIC_AFFECT_ZERO );

    spacetime_set( &local_episodic[replace].space_time, being_location( local ) );

    new_time = local_episodic[replace].space_time.time;

    local_episodic[replace].first_name[BEING_MEETER] = name1;
    local_episodic[replace].family_name[BEING_MEETER] = family1;
    local_episodic[replace].first_name[BEING_MET] = name2;
    local_episodic[replace].family_name[BEING_MET] = family2;
    local_episodic[replace].food = food;
    local_episodic[replace].arg = arg;

    if ( ( event == 0 ) || ( event >= EVENTS ) )
    {
        ( void )SHOW_ERROR( "Event outside scope" );
    }

    if ( local_logging )
    {
        if ( ( old_event != event ) /*|| ((old_time+10) < (new_time))*/ ) /**< TODO this may need to be changed */
        {
            n_string_block description = {0};
            n_string_block str = {0};
            n_string_block time = {0};
            n_string_block combination = {0};
            n_int social_event;

            social_event = episode_description( local, replace, description );

            if ( ( local_social == 1 ) && ( social_event == 0 ) )
            {
                return;
            }

            being_name_simple( local, str );
            spacetime_to_string( time );
            io_three_string_combination( combination, time, str, description, 35 );

            ( *local_logging )( combination );
        }
    }
}
/**
 * @brief Remember eating
 * @param local Pointer to the being
 * @param energy Energy obtained from food
 * @param food_type The type of food
 */
void episodic_food( simulated_being *local, n_int energy, n_byte food_type )
{
    episodic_store_full( local, EVENT_EAT, energy,
                         being_gender_name( local ), being_family_name( local ),
                         0, 0, 0, food_type );
}

/**
 * @brief Updates the episodic memory with details about an event
 * @param local Pointer to the ape
 * @param event The type of event
 * @param affect The affect value associated with the event
 * @param name1 Name of the first ape in the memory (meeter)
 * @param family1 Family name of the first ape in the memory (meeter)
 * @param name2 Name of the second ape in the memory (met)
 * @param family2 Family name of the second ape in the memory (met)
 * @param arg Any additional arguments
 */
void episodic_store_memory(
    simulated_being *local,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2,
    n_byte2 arg )
{
    episodic_store_full( local, event, affect, name1, family1, name2, family2, arg, 0 );
}
/**
 * @brief Store an episodic memory about the self
 * @param local Pointer to the being
 * @param event The type of event
 * @param affect An affect value associated with the event
 * @param arg Any additional argument
 */
void episodic_self(
    simulated_being *local,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 arg )
{
    episodic_store_memory( local, event, affect,
                           being_gender_name( local ), being_family_name( local ),
                           0, 0, arg );
}
/**
 * @brief Remember an event which occurred between being in close proximity
 * @param local Pointer to the first being
 * @param other Pointer to the second being
 * @param event The type of event
 * @param affect Affect value associated with the event
 * @param arg Any additional argument
 */
void episodic_close(
    simulated_being *local,
    simulated_being *other,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 arg )
{
    episodic_store_memory(
        local, event, affect,
        being_gender_name( other ), being_family_name( other ),
        0, 0, arg );
}
/**
 * @brief Remember a particular interaction between two beings
 * @param local Pointer to the being
 * @param other Pointer to the being being interacted with
 * @param event The type of event
 * @param affect The affect associated with the interaction
 * @param arg Any additional argument
 */
void episodic_interaction(
    simulated_being *local,
    simulated_being *other,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 arg )
{
    episodic_store_memory(
        local, event, affect,
        being_gender_name( local ), being_family_name( local ),
        being_gender_name( other ), being_family_name( other ), arg );
}

/**
 * @brief Generate an intention.
 * Note that intentions are stored together with episodic memories,
 * with the event type making the difference between a memory about
 * the past and an intention about the future.
 * @param local Pointer to the ape
 * @param episode_index Episodic memory array index to use.
 * @param mins_ahead The number of minutes into the future for which the intention will last.
 * @param args Any additional arguments
 * @return Returns 1 if the update was successful, or 0 otherwise.
 */
n_byte episodic_intention(
    simulated_being *local,
    n_int episode_index,
    n_byte2 mins_ahead,
    n_byte args )
{
    n_byte4 date;
    n_byte4 time;
    n_int   replace;
    n_byte  event;

    simulated_iepisodic *local_episodic = being_episodic( local );

    if ( local_episodic == 0L )
    {
        return 0;
    }

    event = local_episodic[episode_index].event;

    if ( event == 0 )
    {
        return 0;
    }

    time = land_time();
    date = local_episodic[episode_index].space_time.date;
    if ( time >= TIME_DAY_MINUTES )
    {
        /** increment date by one day */
        time %= TIME_DAY_MINUTES;
        date++;
    }

    if ( event >= EVENT_INTENTION )
    {
        /** extend the time of an existing intention */
        local_episodic[episode_index].space_time.time = time;
        local_episodic[episode_index].space_time.date = date;
        local_episodic[episode_index].arg = args;
        /** if this was someone else's intention it now becomes yours */
        local_episodic[episode_index].first_name[BEING_MEETER] = being_gender_name( local );
        local_episodic[episode_index].family_name[BEING_MEETER] = being_family_name( local );
        return 1;
    }

    /** only certain types of events become intentions */
    if ( !( ( event == EVENT_GROOM ) || ( event == EVENT_CHAT ) ) )
    {
        return 0;
    }

    /** find a memory index to replace */
    replace = simulated_iepisodic_replace_index(
                  EVENT_INTENTION + event,
                  ( n_int )( local_episodic[episode_index].affect ) - EPISODIC_AFFECT_ZERO,
                  being_gender_name( local ),
                  being_family_name( local ),
                  local_episodic[episode_index].first_name[BEING_MET],
                  local_episodic[episode_index].family_name[BEING_MET],
                  local );

    if ( replace == -1 )
    {
        return 0;
    }

    if ( replace == episode_index )
    {
        return 0;
    }

    memory_copy( ( n_byte * )&local_episodic[episode_index], ( n_byte * )&local_episodic[replace], sizeof( simulated_iepisodic ) );

    local_episodic[replace].event = EVENT_INTENTION + event;

    local_episodic[replace].space_time.time = time;
    local_episodic[replace].space_time.date = date;

    local_episodic[replace].first_name[BEING_MEETER] = being_gender_name( local );
    local_episodic[replace].family_name[BEING_MEETER] = being_family_name( local );

    local_episodic[replace].arg = args;

    return 1;
}

/**
 * @brief Copy an episodic memory (an anecdote) from one ape to another during chat.
 * @param local Pointer to the ape conveying the anecdote
 * @param other Pointer to the ape to which the anecdote will be copied
 * @return Returns 1 if the copy was successful, 0 otherwise
 */
n_byte episodic_anecdote(
    simulated_being *local,
    simulated_being *other )
{
    simulated_iepisodic *local_episodic = being_episodic( local );
    simulated_iepisodic *other_episodic = being_episodic( other );
    n_int affect;
    n_byte event;
    n_int replace, mult = 1;

    if ( local_episodic == 0L || other_episodic == 0L || local == other )
    {
        return 0;
    }

    affect = ( n_int )( local_episodic[being_attention( local, ATTENTION_EPISODE )].affect ) - EPISODIC_AFFECT_ZERO;
    event = local_episodic[being_attention( local, ATTENTION_EPISODE )].event;

    /** both protagonists must be awake */
    if ( ( event == 0 ) ||
            ( local->delta.awake == FULLY_ASLEEP ) ||
            ( other->delta.awake == FULLY_ASLEEP ) )
    {
        return 0;
    }

    if ( local->delta.awake != FULLY_AWAKE )
    {
        /** more likely to make errors while drowsy */
        mult = 2;
    }

    /** mutate with some probability */
    if ( being_random( local ) <
            ( ANECDOTE_EVENT_MUTATION_RATE +
              ( local->changes.learned_preference[PREFERENCE_ANECDOTE_EVENT_MUTATION] ) * 100 )*mult )
    {
        event = ( n_byte )( being_random( local ) % EVENTS );
    }
    if ( being_random( local ) <
            ( ANECDOTE_AFFECT_MUTATION_RATE +
              ( local->changes.learned_preference[PREFERENCE_ANECDOTE_AFFECT_MUTATION] ) * 100 )*mult )
    {
        /** affect gets exaggerated or downplayed */
        affect = ( affect * ( 64 + ( n_int )( being_random( local ) & 127 ) ) ) / 128;
        /** keep affect within range */
        if ( affect < -32000 )
        {
            affect = -32000;
        }
        if ( affect > 32000 )
        {
            affect = 32000;
        }
    }

    /** find an index within the other episodic memory in which to insert */
    replace = simulated_iepisodic_replace_index(
                  event, affect,
                  local_episodic[being_attention( local, ATTENTION_EPISODE )].first_name[BEING_MEETER],
                  local_episodic[being_attention( local, ATTENTION_EPISODE )].family_name[BEING_MEETER],
                  local_episodic[being_attention( local, ATTENTION_EPISODE )].first_name[BEING_MET],
                  local_episodic[being_attention( local, ATTENTION_EPISODE )].family_name[BEING_MET],
                  local );

    if ( replace == -1 )
    {
        return 0;
    }

    other_episodic[replace] = local_episodic[being_attention( local, ATTENTION_EPISODE )];
    other_episodic[replace].event = event;
    other_episodic[replace].affect = ( n_byte2 )( affect + EPISODIC_AFFECT_ZERO );

    /** other ape pays attention to the incoming anecdote */
    being_set_attention( local, ATTENTION_EPISODE, replace );

    return 1;
}

#else

/**
 * An empty function
 */
void episodic_interaction(
    simulated_being *local,
    simulated_being *other,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 arg )
{

}

/**
 * An empty function
 */
void episodic_store_memory(
    simulated_being *local,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2,
    n_byte2 arg )
{
}

#endif


