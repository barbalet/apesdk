/****************************************************************

 episodic.c

 =============================================================

 Copyright 1996-2012 Tom Barbalet. All rights reserved.

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

/*! \file   episodic.c
 *  \brief  This handles episodic memory
 */


/*NOBLEMAKE DEL=""*/

#ifndef	_WIN32
#include "../noble/noble.h"
#include "../universe/universe.h"
#else
#include "..\noble\noble.h"
#include "..\universe\universe.h"
#endif

#include "entity_internal.h"
#include "entity.h"

/*NOBLEMAKE END=""*/

/*! /file episodic.c
 * /brief Functions relating to updates of episodic memories and intentions.
 */

#ifdef EPISODIC_ON

/**
 * If the given episodic memory is an intention, as defined by the event type,
 * then update the learned preferences based upon the intention type.
 * For example, if the ape intends to chat then the chatting preference
 * may be increased which makes chatting more likely.
 * @param local_sim pointer to simulation
 * @param local pointer to the particular ape
 * @param episode_index array index of the episodic memory representing the intention
 */
static void episodic_intention_update(noble_simulation * local_sim, noble_being * local, n_int episode_index)
{
    episodic_memory * local_episodic = GET_EPI(local_sim, local);
    n_byte event;
    n_int learned_preference_index=-1;
    if (local_episodic == 0L)
    {
        return;
    }
    event = local_episodic[episode_index].event - EVENT_INTENTION;
    switch(event)
    {
    case EVENT_CHAT:
    {
        learned_preference_index = PREFERENCE_CHAT;
        break;
    }
    case EVENT_GROOM:
    {
        if ((local_episodic[episode_index].arg&2)!=0)
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
    if (learned_preference_index>-1)
    {
        if ((local_episodic[episode_index].arg&1)!=0)
        {
            if (local->learned_preference[learned_preference_index]<255)
            {
                local->learned_preference[learned_preference_index]++;
            }
        }
        else
        {
            if (local->learned_preference[learned_preference_index]>0)
            {
                local->learned_preference[learned_preference_index]--;
            }
        }
    }
}

/**
 * Update the episodic memories for a given ape.
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
 * @param local_sim pointer to the simulation
 * @param local pointer to the ape
 */
void episodic_cycle(noble_simulation * local_sim, noble_being * local)
{
    n_int i;
    episodic_memory * local_episodic = GET_EPI(local_sim, local);

    if (!local_episodic) return;

    for (i=0; i<EPISODIC_SIZE; i++)
    {
        if (local_episodic[i].event == 0) continue;

        /** remove intentions which are outdated */
        if (local_episodic[i].event >= EVENT_INTENTION)
        {
            /** is this my intention, or someone else's? */
            if ((local_episodic[i].first_name[BEING_MEETER]==GET_NAME_GENDER(local_sim,local)) &&
                    (local_episodic[i].family_name[BEING_MEETER]==GET_NAME_FAMILY2(local_sim,local)))
            {

                if (local_episodic[i].date[0] < local_sim->land->date[0])
                {
                    local_episodic[i].event = 0;
                    continue;
                }
                else
                {
                    if (local_episodic[i].time < local_sim->land->time)
                    {
                        local_episodic[i].event = 0;
                        continue;
                    }
                }
            }
            episodic_intention_update(local_sim,local,i);
        }

        /** fade towards EPISODIC_AFFECT_ZERO */
        if (local_episodic[i].affect < EPISODIC_AFFECT_ZERO)
        {
            /** negative memories fade */
            if (EPISODIC_AFFECT_ZERO - local_episodic[i].affect > 16)
            {
                local_episodic[i].affect+=(1+GENE_NEGATIVE_AFFECT_FADE(GET_G(local)));
            }
            else
            {
                local_episodic[i].affect++;
            }
        }
        else
        {
            if (local_episodic[i].affect > EPISODIC_AFFECT_ZERO)
            {
                /** positive memories fade */
                if (local_episodic[i].affect - EPISODIC_AFFECT_ZERO > 16)
                {
                    local_episodic[i].affect-=(1+GENE_POSITIVE_AFFECT_FADE(GET_G(local)));
                }
                else
                {
                    local_episodic[i].affect--;
                }
            }
        }
    }
}

/**
 * Returns a celebrity factor based upon how many apes within
 * the episodic memory of the given ape have a similar name to the
 * met ape, and their friend or foe values.
 * This means that initial beliefs about other apes are partly
 * a form of stereotyping
 * @param local_sim pointer to the simulation
 * @param meeter_being pointer to the ape
 * @param met_being pointer to another ape
 * @return celebrity value of the met ape
 */
n_int episodic_met_being_celebrity(
    noble_simulation * local_sim,
    noble_being * meeter_being,
    noble_being * met_being)
{
    n_int i,j,celebrity=0,ctr,aff;
    episodic_memory * meeter_episodic = GET_EPI(local_sim, meeter_being);
    n_byte2 first_name = GET_NAME_GENDER(local_sim,met_being);
    n_byte2 family_name = GET_NAME_FAMILY2(local_sim,met_being);

    if (!meeter_episodic) return 0;

    /** check all episodic memories of the meeter */
    for (i=0; i<EPISODIC_SIZE; i++)
    {
        aff = (n_int)(meeter_episodic[i].affect) - EPISODIC_AFFECT_ZERO;
        if (aff>1) aff=1;
        if (aff<-1) aff=-1;

        /** check both the meeter and the met ape for each memory */
        for (j=BEING_MEETER; j<=BEING_MET; j++)
        {
            ctr=0;
            /** same first name */
            if (meeter_episodic[i].first_name[j]==first_name)
            {
                celebrity+=aff;
                ctr++;
            }
            /** same family name */
            if (meeter_episodic[i].family_name[j]==family_name)
            {
                celebrity+=aff;
                ctr++;
            }
            /** if both first name and family name match then
            increase the celebrity value further */
            if (ctr==2)
            {
                celebrity+=aff*2;
            }
        }
    }

    /** limit within range */
    if (celebrity>16) celebrity=16;
    if (celebrity<-16) celebrity=-16;
    return celebrity;
}

/**
 * This returns the percentage of episodic memories or intentions which are first person.
 * Some memories originate from the self and others are acquired from others via chatting.
 * @param local_sim pointer to the simulation
 * @param local pointer to the ape
 * @param intention: 0=episodic memories, 1=intentions
 * @return percentage in the range 0-100
 */
n_int episodic_first_person_memories_percent(
    noble_simulation * local_sim,
    noble_being * local,
    n_byte intention)
{
    n_int i,hits=0,memories=0;
    episodic_memory * local_episodic = GET_EPI(local_sim, local);
    n_byte2 name = GET_NAME_GENDER(local_sim,local);
    n_byte2 family = GET_NAME_FAMILY2(local_sim,local);

    if (local_episodic == 0L)
    {
        return 0;
    }

    /** examine all memories */
    for (i=0; i<EPISODIC_SIZE; i++)
    {
        if (local_episodic[i].event>0)
        {
            if (intention!=0)
            {
                /** ratio of intentions to other memories */
                if (local_episodic[i].event >= EVENT_INTENTION)
                {
                    hits++;
                }
            }
            else
            {
                /** ratio of first person memories to other memories */
                if ((local_episodic[i].first_name[BEING_MEETER] == name) &&
                        (local_episodic[i].family_name[BEING_MEETER] == family))
                {
                    hits++;
                }
            }
            memories++;
        }
    }
    if (memories>0)
    {
        return hits*100/memories;
    }
    else
    {
        if (intention!=0)
        {
            return 0;
        }
        return 100;
    }
}

/**
 * Returns the index of the the episodic memory which can be overwritten with a new one.
 * @param event The type of event
 * @param affect The affect value associated with the event
 * @param name1 Name of the first ape in the memory (meeter)
 * @param family1 Family name of the first ape in the memory (meeter)
 * @param name2 Name of the second ape in the memory (met)
 * @param family2 Family name of the second ape in the memory (met)
 * @param local Pointer to the ape
 * @param local_sim Pointer to the simulation
 * @return array index of the episodic memory which can be replaced.
 */
static n_int episodic_memory_replace_index(
    n_byte event,
    n_int affect,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2,
    noble_being * local,
    noble_simulation * local_sim)
{
    /** absolute affect value */
    n_int abs_aff = affect;
    n_int i;
    n_int replace=-1;
    n_int min;
    n_byte event_exists=0;

    episodic_memory * local_episodic = GET_EPI(local_sim, local);

    if (!local_episodic) return -1;

    /** replace only events with an affect lower then the current */
    abs_aff = ABS(abs_aff);
    min = abs_aff;
    for (i=0; i<EPISODIC_SIZE; i++)
    {
        /** is this the same type of event */
        if (local_episodic[i].event == event)
        {
            /** is this the same being? */
            if ((local_episodic[i].first_name[BEING_MEETER]==name1) &&
                    (local_episodic[i].family_name[BEING_MEETER]==family1))
            {
                /** get absolute affect value */
                n_int aff1 = ABS((n_int)(local_episodic[i].affect)-EPISODIC_AFFECT_ZERO);
                /** does this have the least affect (most forgettable) */
                event_exists = 1;
                if (aff1 <= min)
                {
                    min = aff1;
                    replace = i;
                }
            }
        }
    }

    if (event_exists==0)
    {
        /** Use any empty memory slots */
        for (i=0; i<EPISODIC_SIZE; i++)
        {
            if (local_episodic[i].event == 0)
            {
                return i;
            }
        }

        /** no event of this type was found,
        so search for any event with the lowest affect */
        min = abs_aff;
        for (i=0; i<EPISODIC_SIZE; i++)
        {
            /** get absolute affect value */
            n_int aff1 = ABS((n_int)(local_episodic[i].affect)-EPISODIC_AFFECT_ZERO);
            /** does this have the least affect (most forgettable) */
            if (aff1 < min)
            {
                min = aff1;
                replace = i;
            }
        }
    }

    return replace;
}

/**
 * Updates the episodic memory with details about an event
 * @param local Pointer to the ape
 * @param event The type of event
 * @param affect The affect value associated with the event
 * @param local_sim Pointer to the simulation
 * @param name1 Name of the first ape in the memory (meeter)
 * @param family1 Family name of the first ape in the memory (meeter)
 * @param name2 Name of the second ape in the memory (met)
 * @param family2 Family name of the second ape in the memory (met)
 * @param arg Any additional arguments
 */
void episodic_store_memory(
    noble_being * local,
    n_byte event,
    n_int affect,
    noble_simulation * local_sim,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2,
    n_byte2 arg)
{
    episodic_memory * local_episodic = GET_EPI(local_sim, local);
    n_int replace;

    if (local_episodic == 0L)
    {
        return;
    }

    if (being_awake_local(local_sim, local)==FULLY_ASLEEP) return;

    replace = episodic_memory_replace_index(event,affect,name1,family1,name2,family2,local,local_sim);

    if (replace == -1) return;

    /** insert the current event into the episodic memory */
    local_episodic[replace].event=event;
    local_episodic[replace].affect= (n_byte2)(affect+EPISODIC_AFFECT_ZERO);
    local_episodic[replace].location[0]=GET_X(local);
    local_episodic[replace].location[1]=GET_Y(local);
    local_episodic[replace].time=local_sim->land->time;
    local_episodic[replace].date[0]=local_sim->land->date[0];
    local_episodic[replace].date[1]=local_sim->land->date[1];
    local_episodic[replace].first_name[BEING_MEETER]=name1;
    local_episodic[replace].family_name[BEING_MEETER]=family1;
    local_episodic[replace].first_name[BEING_MET]=name2;
    local_episodic[replace].family_name[BEING_MET]=family2;
    local_episodic[replace].food=0;
    local_episodic[replace].arg=arg;
}

/**
 * Generate an intention.
 * Note that intentions are stored together with episodic memories,
 * with the event type making the difference between a memory about
 * the past and an intention about the future.
 * @param local_sim Pointer to the simulation
 * @param local Pointer to the ape
 * @param episode_index Episodic memory array index to use.
 * @param mins_ahead The number of minutes into the future for which the intention will last.
 * @param args Any additional arguments
 * @return Returns 1 if the update was successful, or 0 otherwise.
 */
n_byte episodic_intention(
    noble_simulation * local_sim,
    noble_being * local,
    n_int episode_index,
    n_byte2 mins_ahead,
    n_byte args)
{
    n_int replace;
    n_byte2 time, date0,date1;
    episodic_memory * local_episodic = GET_EPI(local_sim, local);
    n_byte event;

    if (local_episodic == 0L)
    {
        return 0;
    }

    event = local_episodic[episode_index].event;

    if (event==0) return 0;

    time = local_sim->land->time + mins_ahead;
    date0 = local_episodic[episode_index].date[0];
    date1 = local_episodic[episode_index].date[1];
    if (time >= TIME_DAY_MINUTES)
    {
        /** increment date by one day */
        time %= TIME_DAY_MINUTES;
        date0++;
    }

    if (event >= EVENT_INTENTION)
    {
        /** extend the time of an existing intention */
        local_episodic[episode_index].time = time;
        local_episodic[episode_index].date[0] = date0;
        local_episodic[episode_index].date[1] = date1;
        local_episodic[episode_index].arg = args;
        /** if this was someone else's intention it now becomes yours */
        local_episodic[episode_index].first_name[BEING_MEETER] = GET_NAME_GENDER(local_sim,local);
        local_episodic[episode_index].family_name[BEING_MEETER] = GET_NAME_FAMILY2(local_sim,local);
        return 1;
    }

    /** only certain types of events become intentions */
    if (!((event==EVENT_GROOM) ||
            (event==EVENT_CHAT)))
    {
        return 0;
    }

    /** find a memory index to replace */
    replace = episodic_memory_replace_index(
                  EVENT_INTENTION + event,
                  (n_int)(local_episodic[episode_index].affect)-EPISODIC_AFFECT_ZERO,
                  GET_NAME_GENDER(local_sim,local),
                  GET_NAME_FAMILY2(local_sim,local),
                  local_episodic[episode_index].first_name[BEING_MET],
                  local_episodic[episode_index].family_name[BEING_MET],
                  local, local_sim);

    if (replace == -1) return 0;

    local_episodic[replace] = local_episodic[episode_index];
    local_episodic[replace].event = EVENT_INTENTION + event;
    local_episodic[replace].time = time;
    local_episodic[replace].date[0] = date0;
    local_episodic[replace].date[1] = date1;
    local_episodic[replace].first_name[BEING_MEETER] = GET_NAME_GENDER(local_sim,local);
    local_episodic[replace].family_name[BEING_MEETER] = GET_NAME_FAMILY2(local_sim,local);
    local_episodic[replace].arg = args;

    return 1;
}

/**
 * Copy an episodic memory (an anecdote) from one ape to another during chat.
 * @param local_sim Pointer to the simulation
 * @param local Pointer to the ape conveying the anecdote
 * @param other Pointer to the ape to which the anecdote will be copied
 * @return Returns 1 if the copy was successful, 0 otherwise
 */
n_byte episodic_anecdote(
    noble_simulation * local_sim,
    noble_being * local,
    noble_being * other)
{
    episodic_memory * local_episodic = GET_EPI(local_sim, local);
    episodic_memory * other_episodic = GET_EPI(local_sim, other);
    n_int affect;
    n_byte event;
    n_int replace,mult=1;

    if (local_episodic == 0L || other_episodic == 0L || local == other)
    {
        return 0;
    }

    affect = (n_int)(local_episodic[GET_A(local,ATTENTION_EPISODE)].affect)-EPISODIC_AFFECT_ZERO;
    event = local_episodic[GET_A(local,ATTENTION_EPISODE)].event;

    /** both protagonists must be awake */
    if ((event==0) ||
            (being_awake_local(local_sim, local)==FULLY_ASLEEP) ||
            (being_awake_local(local_sim, other)==FULLY_ASLEEP))
    {
        return 0;
    }

    if (being_awake_local(local_sim, local)!=FULLY_AWAKE)
    {
        /** more likely to make errors while drowsy */
        mult=2;
    }

    /** mutate with some probability */
    if (math_random(local->seed) <
            (ANECDOTE_EVENT_MUTATION_RATE+
             (local->learned_preference[PREFERENCE_ANECDOTE_EVENT_MUTATION])*100)*mult)
    {
        event = (n_byte)(math_random(local->seed) % EVENTS);
    }
    if (math_random(local->seed) <
            (ANECDOTE_AFFECT_MUTATION_RATE+
             (local->learned_preference[PREFERENCE_ANECDOTE_AFFECT_MUTATION])*100)*mult)
    {
        /** affect gets exaggerated or downplayed */
        affect = (affect * (64 + (n_int)(math_random(local->seed) & 127))) / 128;
        /** keep affect within range */
        if (affect<-32000) affect=-32000;
        if (affect>32000) affect=32000;
    }

    /** find an index within the other episodic memory in which to insert */
    replace = episodic_memory_replace_index(
                  event,affect,
                  local_episodic[GET_A(local,ATTENTION_EPISODE)].first_name[BEING_MEETER],
                  local_episodic[GET_A(local,ATTENTION_EPISODE)].family_name[BEING_MEETER],
                  local_episodic[GET_A(local,ATTENTION_EPISODE)].first_name[BEING_MET],
                  local_episodic[GET_A(local,ATTENTION_EPISODE)].family_name[BEING_MET],
                  local,local_sim);

    if (replace==-1) return 0;

    other_episodic[replace] = local_episodic[GET_A(local,ATTENTION_EPISODE)];
    other_episodic[replace].event = event;
    other_episodic[replace].affect = (n_byte2)(affect+EPISODIC_AFFECT_ZERO);

    /** other ape pays attention to the incoming anecdote */
    GET_A(local,ATTENTION_EPISODE) = (n_byte)replace;

    return 1;
}

#else

/**
 * An empty function
 */
void episodic_store_memory(
    noble_being * local,
    n_byte event,
    n_int affect,
    noble_simulation * local_sim,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2,
    n_byte2 arg)
{
}

#endif


