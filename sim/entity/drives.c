/****************************************************************

 drives.c

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

/*! \file   drives.c
 *  \brief  Functions related to biological drives
 */

/*NOBLEMAKE DEL=""*/

#include "entity.h"
#include "entity_internal.h"

/*NOBLEMAKE END=""*/

/** Mate seeking */

#define GENE_MATE_SEEK(gene)                GENE_VAL_REG(gene, 8, 9, 14, 2)

/**
 * @brief Update the hunger drive
 * @param local Pointer to the ape
 */
static void drives_hunger(noble_being * local)
{
    /** if the being is hungry and its hunger drive is not already saturated */
    if (being_energy(local) < BEING_HUNGRY)
    {
        /** increase hunger drive */
        being_inc_drive(local, DRIVE_HUNGER);
        /** decrease sex drive (hunger subsumes sex) */
        being_dec_drive(local, DRIVE_SEX);
    }
    else
    {
        /** otherwise decrease hunger drive */
        being_dec_drive(local, DRIVE_HUNGER);
    }
}

typedef struct
{
    n_int         beings_in_vacinity;
    noble_being * being;
} drives_sociability_data;

static void drives_sociability_loop(noble_simulation * local_sim, noble_being * other, void * data)
{
    drives_sociability_data * dsd = (drives_sociability_data *)data;
    n_int   distance_squared;
    n_vect2 difference_vector;
    /* this needs to be checked against simulation near sight and sound values */
    const n_int apespace_span = APESPACE_TO_MAPSPACE(10);
    being_delta(dsd->being, other, &difference_vector);
    /* los should not be used here, it's too expensive */
    distance_squared = vect2_dot(&difference_vector, &difference_vector, 1, 1);
    if (distance_squared < (apespace_span * apespace_span))
    {
        dsd->beings_in_vacinity++;
    }
}

/**
 * @brief Social drive governs how likely the being is to interact with others.
 * This affects behaviors such as grooming, chatting and mating
 * @param local Pointer to the ape
 * @param beings_in_vicinity The number of other apes in the vicinity
 */
static void drives_sociability(
    noble_being * local,
    noble_simulation * sim)
{
    drives_sociability_data dsd;
    dsd.beings_in_vacinity = 0;
    dsd.being = local;
    being_loop_no_thread(sim, local, drives_sociability_loop, &dsd);
    
    /** if the being is not overcrowded and its social drive is not saturated */
    if (dsd.beings_in_vacinity < local->crowding + SOCIAL_TOLLERANCE)
    {
        /** increase the social drive */
        being_inc_drive(local, DRIVE_SOCIAL);
    }
    else
    {
        /** decrease the social drive */
        being_dec_drive(local, DRIVE_SOCIAL);
    }

    /** Adjust crowding (typical expected number of neighbours). */
    if (dsd.beings_in_vacinity < local->crowding) local->crowding--;
    if (dsd.beings_in_vacinity > local->crowding) local->crowding++;

    /** Minimum expected neighbours */
    if (local->crowding < MIN_CROWDING) local->crowding = MIN_CROWDING;

    /** Maximum expected neighbours.  Beyond this behavioral sink occurs. */
    if (local->crowding > MAX_CROWDING) local->crowding = MAX_CROWDING;
}

/**
 * @brief Updates the sex drive
 * @param local Pointer to the ape
 * @param awake whether the ape is awake
 * @param local_sim Pointer to the simulation
 */
static void drives_sex(
    noble_being * local,
    n_int awake,
    noble_simulation * local_sim)
{
    n_int i,max;
    noble_social * local_social_graph = being_social(local);
    n_int age_in_days = AGE_IN_DAYS(local_sim,local);

#ifdef EPISODIC_ON
    noble_episodic * local_episodic = being_episodic(local);
#endif

    /** is the being mature */
    if (age_in_days > AGE_OF_MATURITY)
    {
        /** is the being awake and its sex drive not saturated */
        if (awake)
        {
            /** increase the sex drive */
            being_inc_drive(local, DRIVE_SEX);

            /** if sex drive is above a mate seeking threshold and
            the being has no current goal */
            if ((being_drive(local, DRIVE_SEX) > THRESHOLD_SEEK_MATE) &&
                    (local->goal[0]==GOAL_NONE))
            {
                /** either search for a preferred mate, or mate randomly */
                if (GENE_MATE_SEEK(being_genetics(local))&1)
                {
                    /** look for a mate */
#ifdef EPISODIC_ON
                    if (!local_episodic) return;

                    /** does the being remember mating in the recent past */
                    for(i=0; i<EPISODIC_SIZE; i++)
                    {
                        if (local_episodic[i].event == EVENT_MATE)
                        {
                            /** not someone else's mate */
                            
                            if (being_name_compartison(local, local_episodic[i].first_name[BEING_MEETER], local_episodic[i].family_name[BEING_MEETER]))
                            {
                                /** set a goal to seek the remembered mate */
                                local->goal[0]=GOAL_MATE;
                                local->goal[1]=local_episodic[i].first_name[BEING_MET];
                                local->goal[2]=local_episodic[i].family_name[BEING_MET];
                                local->goal[3]=GOAL_TIMEOUT;
                                /** remember seeking a mate */
                                episodic_store_memory(
                                    local, EVENT_SEEK_MATE, AFFECT_SEEK_MATE,
                                    local_sim,
                                    being_gender_name(local), being_family_name(local),
                                    local->goal[1], local->goal[2],0);
                                break;
                            }
                        }
                    }
#endif
                    /** if the being is not seeking a remembered mate
                        then examine the social graph for attractive prospects */
                    if (local->goal[0]!=GOAL_MATE)
                    {
                        max = 0;
                        if (!local_social_graph) return;

                        for(i=1; i<SOCIAL_SIZE_BEINGS; i++)
                        {
                            if (!SOCIAL_GRAPH_ENTRY_EMPTY(local_social_graph,i))
                            {
                                if ((local_social_graph[i].attraction) > max)
                                {
                                    /** who are we most attracted to? */
                                    max=local_social_graph[i].attraction;
                                    local->goal[0]=GOAL_MATE;
                                    local->goal[1]=
                                        local_social_graph[i].first_name[BEING_MET];
                                    local->goal[2]=
                                        local_social_graph[i].family_name[BEING_MET];
                                    local->goal[3]=GOAL_TIMEOUT;
                                }
                            }
                        }
                        /** if an attractive mate was found then remember this event */
                        if (local->goal[0]==GOAL_MATE)
                        {
                            episodic_store_memory(
                                local, EVENT_SEEK_MATE, AFFECT_SEEK_MATE,
                                local_sim,
                                being_gender_name(local), being_family_name(local),
                                local->goal[1], local->goal[2],0);
                        }
                    }
                }
            }
            /** during gestation reduce the sex drive */
            if (being_pregnant(local) != 0)
            {
                if (being_drive(local, DRIVE_SEX) >= GESTATION_SEX_DRIVE_DECREMENT)
                {
                    being_dec_drive(local, DRIVE_SEX);
                }
            }
        }
        else
        {
            /** while sleeping reduce sex drive */
            being_dec_drive(local, DRIVE_SEX);
        }
        /** if sex drive falls below the mate seeking threshold and the being
            is seeking a mate, then stop seeking a mate */
        if ((being_drive(local, DRIVE_SEX) < THRESHOLD_SEEK_MATE) &&
                (local->goal[0]==GOAL_MATE))
        {
            local->goal[0]=GOAL_NONE;
        }
    }
}

/**
 * @brief Updates the fatigue drive.  This doesn't do much, but is
 * accessible to the braincode.
 * @param local Pointer to the ape
 */
static void drives_fatigue(
    noble_being * local)
{
    /** if the being is moving fast enough then increase the fatigue drive */
    if (being_speed(local) > FATIGUE_SPEED_THRESHOLD)
    {
        being_inc_drive(local, DRIVE_FATIGUE);
        /** Add extra fatigue when swimming */
        if (being_state(local)&BEING_STATE_SWIMMING)
        {
            being_inc_drive(local, DRIVE_FATIGUE);
        }
        /** As fatigue increases, sex drive decreases */
        being_dec_drive(local, DRIVE_SEX);
    }
    else
    {
        /** When resting fatigue drive decreases */
        being_dec_drive(local, DRIVE_FATIGUE);
    }
}

/**
 * @brief Update biological drives
 * @param local Pointer to the ape
 * @param beings_in_vicinity The number of other apes within the vicinity
 * @param awake Whether the ape is awake
 * @param sim Pointer to the simulation
 */
void drives_cycle(
    noble_being * local,
    n_int awake,
    noble_simulation * sim)
{
    drives_hunger(local);
    drives_sociability(local, sim);
    drives_sex(local, awake, sim);
    drives_fatigue(local);
}
