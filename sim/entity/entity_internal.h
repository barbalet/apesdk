/****************************************************************

 entity_internal.h

 =============================================================

 Copyright 1996-2014 Tom Barbalet. All rights reserved.

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

#ifndef NOBLEAPE_ENTITY_INTERNAL_H
#define NOBLEAPE_ENTITY_INTERNAL_H

enum sleep_state
{
    FULLY_ASLEEP   =   0,
    SLIGHTLY_AWAKE =   1,
    FULLY_AWAKE    =   2
};

#define CONSUME_E(being, max_energy, food)

/* offsets applied to land operator values */
#define OFFSET_GRASS    40
#define OFFSET_BUSH     14

typedef struct
{
    noble_being  * local;
    noble_social * local_social;
    n_uint         opposite_sex_distance;
    n_uint         same_sex_distance;
    noble_being  * opposite_sex;
    noble_being  * same_sex;
} being_nearest;

void body_genetics(noble_being * beings, n_int number, n_genetics * genetics, n_genetics * mother_genetics, n_genetics * father_genetics, n_byte2 * local);

n_int food_eat(
    n_land * local_land,
    n_int loc_x,
    n_int loc_y,
    n_int az,
    n_byte * food_type,
    noble_being * local_being);

n_int food_absorption(noble_being * local, n_int max_energy, n_byte food_type);

void food_values(n_land * local_land,
                 n_int loc_x,
                 n_int loc_y,
                 n_int *grass, n_int *trees, n_int *bush);

n_int genetics_compare(n_genetics * genetics_a, n_genetics * genetics_b);
void  genetics_set(n_genetics * genetics_a, n_genetics * n_genetics);
void  genetics_zero(n_genetics * genetics_a);

void metabolism_init(noble_being * local_being);
void metabolism_cycle(noble_simulation * local_sim, noble_being * local_being);
void metabolism_vascular_response(noble_simulation * local_sim, noble_being * local_being, n_int response);
void metabolism_eat(noble_being * local_being, n_byte food_type);
void metabolism_suckle(noble_simulation * sim,noble_being * child, noble_being * mother);

void social_action(noble_simulation * sim, noble_being * meeter_being, noble_being * met_being, n_byte action);
n_int social_network(noble_simulation *sim, noble_being * meeter_being, noble_being * met_being, n_int distance);

n_int social_set_relationship(noble_simulation * sim, noble_being * meeter_being, n_byte relationship,noble_being * met_being);

n_int social_get_relationship(noble_simulation * sim,
                              noble_being * meeter_being,
                              n_byte relationship);

n_byte social_groom(noble_simulation * sim,
                    noble_being * meeter_being,
                    noble_being * met_being,
                    n_int distance,
                    n_int awake,
                    n_byte2 familiarity);

n_byte2 social_squabble(noble_being * meeter_being, noble_being * met_being, n_uint distance, n_int is_female, noble_simulation * sim);
n_int social_mate(noble_being * meeter_being, noble_being * met_being, n_int being_index, n_int distance, noble_simulation * sim);
n_int social_chat(noble_being * meeter_being, noble_being * met_being, n_int being_index, noble_simulation * sim);
void social_goals(noble_being * local);
n_int get_noble_social(noble_being * meeter_being, noble_being * met_being, noble_simulation * sim);

n_int episodic_met_being_celebrity(
    noble_simulation * local_sim,
    noble_being * meeter_being,
    noble_being * met_being);

void episodic_store_memory(
    noble_simulation * local_sim,
    noble_being * local,
    being_episodic_event_type event,
    affect_type affect,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2, n_byte2 arg);

void episodic_food(noble_simulation * local_sim, noble_being * local, n_int energy, n_byte food_type);

void episodic_self(
    noble_simulation * local_sim,
    noble_being * local,
    being_episodic_event_type event,
    affect_type affect,
    n_byte2 arg);

void episodic_close(
    noble_simulation * local_sim,
    noble_being * local,
    noble_being * other,
    being_episodic_event_type event,
    affect_type affect,
    n_byte2 arg);

void episodic_interaction(
    noble_simulation * local_sim,
    noble_being * local,
    noble_being * other,
    being_episodic_event_type event,
    affect_type affect,
    n_byte2 arg);

n_byte episodic_intention(
    noble_simulation * local_sim,
    noble_being * local,
    n_int episode_index,
    n_byte2 mins_ahead,
    n_byte args);

n_byte episodic_anecdote(
    noble_simulation * local_sim,
    noble_being * local,
    noble_being * other);

void being_init_braincode(noble_being * local,
                          noble_being * other,
                          n_byte friend_foe,
                          n_byte internal);

n_byte get_braincode_instruction(noble_being * local_being);

void being_ingest_pathogen(noble_being * local, n_byte food_type);

#endif /* NOBLEAPE_ENTITY_INTERNAL_H */
