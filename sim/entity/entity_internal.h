/****************************************************************

 entity_internal.h

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
#ifndef _NOBLEAPE_ENTITY_INTERNAL_H_
#define _NOBLEAPE_ENTITY_INTERNAL_H_
/*NOBLEMAKE END=""*/

#define VISUAL_DISTANCE_SQUARED (4000*4000)

#define FULLY_ASLEEP    0
#define SLIGHTLY_AWAKE  1
#define FULLY_AWAKE     2

#define CONSUME_E(being,max_energy,food)  ((max_energy)*(1+food_absorption(food,being))>>3)
#define CONSUME(being,max_energy,food)  (GET_E(being)+=(n_byte2)CONSUME_E(being,max_energy,food))

/* offsets applied to land operator values */
#define OFFSET_GRASS    40
#define OFFSET_BUSH     14

void body_genome_random(noble_simulation * sim, noble_being * local, n_byte2 * local_random);
void body_genetics(noble_simulation * sim, noble_being * local, noble_being * mother, n_byte2 * local_random);

n_int food_eat(
    n_land * local_land,
    n_weather * local_weather,
    n_int loc_x,
    n_int loc_y,
    n_int az,
    n_byte * food_type,
    noble_being * local_being);

n_int food_absorption(n_byte food_type, noble_being * local);

void food_values(n_land * local_land, n_weather * local_weather,
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


void social_action(
    noble_simulation * sim,
    noble_being * meeter_being,
    noble_being * met_being,
    n_byte action);
n_int social_network(noble_being * meeter_being, noble_being * met_being, n_int distance, noble_simulation *sim);
n_int social_set_relationship(noble_being * meeter_being,n_byte relationship,noble_being * met_being,noble_simulation * sim);
n_int social_get_relationship(noble_being * meeter_being,
                              n_byte relationship,
                              noble_simulation * sim);
n_byte social_groom(noble_being * meeter_being, noble_being * met_being, n_int distance, n_int awake, n_byte2 familiarity, noble_simulation * sim);
n_byte2 social_squabble(noble_being * meeter_being, noble_being * met_being, n_uint distance, n_int is_female, noble_simulation * sim);
n_int social_mate(noble_being * meeter_being, noble_being * met_being, n_byte2 * today, n_int being_index, n_int distance, noble_simulation * sim);
n_int social_chat(noble_being * meeter_being, noble_being * met_being, n_int being_index, noble_simulation * sim);
n_int social_goals(noble_being * local, n_int loc_f);
n_int get_social_link(noble_being * meeter_being, noble_being * met_being, noble_simulation * sim);

n_int episodic_met_being_celebrity(
    noble_simulation * local_sim,
    noble_being * meeter_being,
    noble_being * met_being);
void episodic_store_memory(
    noble_being * local, n_byte event, n_int affect,
    noble_simulation * local_sim,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2, n_byte2 arg);

void episodic_food(noble_simulation * local_sim, noble_being * local, n_int energy, n_byte food_type);

void episodic_self(
                   noble_simulation * local_sim,
                   noble_being * local,
                   n_byte event,
                   n_int affect,
                   n_byte2 arg);

void episodic_close(
                    noble_simulation * local_sim,
                    noble_being * local,
                    noble_being * other,
                    n_byte event,
                    n_int affect,
                    n_byte2 arg);

void episodic_interaction(
                          noble_simulation * local_sim,
                          noble_being * local,
                          noble_being * other,
                          n_byte event,
                          n_int affect,
                          n_byte2 arg);

n_byte episodic_intention(
    noble_simulation * local_sim,
    noble_being * local,
    n_int episode_index,
    n_byte2 mins_ahead,
    n_byte args);

void episodic_cycle(noble_simulation * local_sim, noble_being * local);
n_byte episodic_anecdote(
    noble_simulation * local_sim,
    noble_being * local,
    noble_being * other);

void being_init_braincode(noble_simulation * sim,
                          noble_being * local,
                          noble_being * other,
                          n_int random_factor,
                          n_byte friend_foe,
                          n_byte internal);

void drives_cycle(noble_being * local, n_byte social, n_int awake, noble_simulation * sim);

n_byte get_braincode_instruction(noble_being * local_being);

void being_ingest_pathogen(noble_being * local, n_byte food_type);

void watch_speech(void *ptr, n_string beingname, noble_being * local, n_string result);

#endif /* _NOBLEAPE_ENTITY_INTERNAL_H_ */
