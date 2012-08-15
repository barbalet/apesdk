/****************************************************************

 entity.h

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

/*NOBLEMAKE DEL=""*/
#ifndef _NOBLEAPE_ENTITY_H_
#define _NOBLEAPE_ENTITY_H_
/*NOBLEMAKE END=""*/

#define	LAND_ON
#define	BRAIN_ON /* currently has to be included episodic doesn't work without it */
#define	WEATHER_ON

#undef	FIXED_RANDOM_SIM

#undef	LARGE_SIM

#undef	BRAIN_HASH

#undef  BRAINCODE_SHOW

#define SOFT_BRAIN_ON

#define EPISODIC_AFFECT_ZERO (16384)

#define BEING_HUNGRY				(10*128)
#define	BEING_FULL 			(BEING_HUNGRY*3)
#define	BEING_STARVE 		(BEING_HUNGRY*2)

#define SOCIAL_RESPECT_NORMAL	127

enum BRAINPROBE_TYPE
{
    INPUT_SENSOR = 0,
    OUTPUT_ACTUATOR
};

n_int  brain_probe_to_location(n_int position);

void   metabolism_vascular_description(n_int index, n_string description);
n_string metabolism_description(n_int index);
n_int  metabolism_vascular_radius(noble_being * local_being, n_int vessel_index);

void   being_name(n_byte female, n_int first, n_byte family0, n_byte family1, n_string name);
n_int  being_init(noble_simulation * sim, noble_being * mother, n_int random_factor, n_byte first_generation);
void   being_cycle_awake(noble_simulation * sim, n_uint current_being_index);
void   being_cycle_asleep(noble_simulation * sim, n_uint current_being_index);
void   being_state_description(n_byte2 state, n_string result);
n_uint being_affect(noble_simulation * local_sim, noble_being * local, n_byte is_positive);

void social_graph_link_name(
    noble_simulation * local_sim,
    noble_being * local_being,
    n_int social_graph_index,
    n_byte met, n_string name);
void sim_social(noble_simulation * local);


n_int episodic_first_person_memories_percent(
    noble_simulation * local_sim,
    noble_being * local,
    n_byte intention);

void being_immune_transmit(noble_being * meeter_being, noble_being * met_being, n_byte transmission_type);
void body_genome(n_byte maternal, n_genetics * genome, n_byte * genome_str);
void being_relationship_description(n_int index, n_string description);
n_string being_body_inventory_description(n_int index);

void brain_dialogue(
    noble_simulation * sim,
    n_byte awake,
    noble_being * meeter_being,
    noble_being * met_being,
    n_byte		* bc0,
    n_byte		* bc1,
    n_int being_index,
    n_byte internal);

void brain_hash(n_byte *local_brain, n_byte * local_brain_hash);
void brain_cycle(n_byte * local, n_byte2 * constants);
noble_being * being_from_name(noble_simulation * sim, n_string name);

void being_tidy(noble_simulation * local_sim);
void being_remove(noble_simulation * local_sim);

void brain_three_byte_command(n_byte * string, n_byte * response);

void braincode_statistics(noble_simulation * sim);

void episode_description(noble_simulation * sim,
                         noble_being * local_being,
                         n_int index,
                         n_string description);

n_uint social_respect_mean(noble_simulation * sim, noble_being *local_being);
noble_being * being_find_name(noble_simulation * sim, n_byte2 first_gender, n_byte2 family);
noble_being * being_find_female(noble_simulation * sim, n_genetics * genetics);
void          being_move(noble_being * local, n_int vel, n_byte kind);
n_byte        being_awake(noble_simulation * sim, n_uint reference);
n_byte        being_los(n_land * land, noble_being * local, n_byte2 lx, n_byte2 ly);


#endif /* _NOBLEAPE_ENTITY_H_ */
