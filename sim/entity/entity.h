/****************************************************************

 entity.h

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
#ifndef NOBLEAPE_ENTITY_H
#define NOBLEAPE_ENTITY_H
/*NOBLEMAKE END=""*/

#ifndef	_WIN32

#include "../noble/noble.h"
#include "../universe/universe.h"

#else

#include "..\noble\noble.h"
#include "..\universe\universe.h"

#endif

#define	LAND_ON
#define	WEATHER_ON
#undef RANDOM_INITIAL_BRAINCODE

#undef	FIXED_RANDOM_SIM
#undef	LARGE_SIM
#define SOFT_BRAIN_ON


#define EPISODIC_AFFECT_ZERO (16384)

enum being_energy
{
    BEING_DEAD   = 0,
    BEING_HUNGRY = (10*128),
    BEING_FULL   = (BEING_HUNGRY*3)
};

#define SOCIAL_RESPECT_NORMAL	127

enum BRAINPROBE_TYPE
{
    INPUT_SENSOR = 0,
    OUTPUT_ACTUATOR,
    NUMBER_BRAINPROBE_TYPES
};

#define GENEALOGY_ON

enum GENEALOGY_FORMAT
{
    GENEALOGY_NONE = 0,
    GENEALOGY_GENXML,
    GENEALOGY_GEDCOM,
    GENEALOGY_FORMATS
};


/* if you change this you need to change the corresponding definitions in being_state_description */
typedef enum
{
    BEING_STATE_ASLEEP = 0,
    BEING_STATE_AWAKE = 1,
    BEING_STATE_HUNGRY = 2,
    BEING_STATE_SWIMMING = 4,
    BEING_STATE_EATING = 8,
    BEING_STATE_MOVING = 16,
    BEING_STATE_SPEAKING = 32,
    BEING_STATE_SHOUTING = 64,
    BEING_STATE_GROOMING = 128,
    BEING_STATE_SUCKLING = 256,
    BEING_STATE_SHOWFORCE = 512,
    BEING_STATE_ATTACK = 1024,
    BEING_STATES = 12
}being_state_type;

typedef enum
{
    EVENT_EAT = 1,
    EVENT_MATE,
    EVENT_HIT,
    EVENT_HIT_BY,
    EVENT_SWIM,
    EVENT_GROOM,
    EVENT_GROOMED,
    EVENT_CHAT,
    EVENT_SHOUT,
    EVENT_BIRTH,
    EVENT_CARRIED,
    EVENT_CARRIED_BY,
    EVENT_SUCKLED,
    EVENT_SUCKLED_BY,
    EVENT_SEEK_MATE,
    EVENT_WHACKED,
    EVENT_WHACKED_BY,
    EVENT_HURLED,
    EVENT_HURLED_BY,
    EVENT_HUGGED,
    EVENT_HUGGED_BY,
    EVENT_PRODDED,
    EVENT_PRODDED_BY,
    EVENT_DRAG,
    EVENT_BRANDISH,
    EVENT_DROP,
    EVENT_PICKUP,
    EVENT_GIVEN,
    EVENT_GIVEN_BY,
    EVENT_CHEW,
    EVENT_BASH_OBJECTS,
    EVENT_FISH,
    EVENT_SMILED,
    EVENT_SMILED_BY,
    EVENT_GLOWERED,
    EVENT_GLOWERED_BY,
    EVENT_PATTED,
    EVENT_PATTED_BY,
    EVENT_POINT,
    EVENT_POINTED,
    EVENT_TICKLED,
    EVENT_TICKLED_BY,
    EVENTS
}being_episodic_event_type;

void braincode_number_of_instructions(
                                      noble_simulation * sim,
                                      noble_being * local_being,
                                      n_int * no_of_sensors,
                                      n_int * no_of_actuators,
                                      n_int * no_of_operators,
                                      n_int * no_of_conditionals,
                                      n_int * no_of_data);

n_int being_memory(noble_simulation * local, n_byte * buffer, n_uint * location, n_int memory_available);

n_byte * being_brain(noble_being * value);
noble_episodic * being_episodic(noble_being * value);
noble_social * being_social(noble_being * value);

n_int being_brainstates(noble_being * value, n_int awake, n_byte2 * states);

n_byte being_honor(noble_being * value);

void being_honor_inc_dec(noble_being * inc, noble_being * dec);

void being_honor_swap(noble_being * victor, noble_being * vanquished);

n_int being_honor_compare(noble_being * first, noble_being * second);

n_byte being_first_name(noble_being * value);

void being_set_family_name(noble_being * value, n_byte first, n_byte last);

n_byte being_family_first_name(noble_being * value);

n_byte being_family_second_name(noble_being * value);

n_byte2 being_gender_name(noble_being * value);

n_byte2 being_family_name(noble_being * value);

void being_name_simple(noble_being * value, n_string str);

n_byte being_posture(noble_being * value);

void being_set_posture(noble_being * value, n_byte post);

n_int being_posture_under(noble_being * value, enum posture_type post);

n_int being_location_x(noble_being * value);

n_int being_location_y(noble_being * value);

n_byte2 * being_location(noble_being * value);

void being_delta(noble_being * primary, noble_being * secondary, n_vect2 * delta);
void being_set_location(noble_being * value, n_byte2 * from);

n_int being_dob(noble_being * value);

n_byte being_speed(noble_being * value);
void being_set_speed(noble_being * value, n_byte sp);

void being_facing_towards(noble_being * value, n_vect2 * vector);
void being_facing_vector(noble_being * value, n_vect2 * vect, n_int divisor);

n_byte being_facing(noble_being * value);
void being_wander(noble_being * value, n_int wander);

n_genetics * being_genetics(noble_being * value);

n_int being_pregnant(noble_being * value);

n_genetics * being_fetal_genetics(noble_being * value);

n_int being_energy(noble_being * value);
void   being_set_energy(noble_being * value, n_int energy);

void   being_energy_delta(noble_being * value, n_int delta);

n_int   being_drive(noble_being * value, enum drives_definition drive);
void    being_inc_drive(noble_being * value, enum drives_definition drive);
void    being_dec_drive(noble_being * value, enum drives_definition drive);
void    being_reset_drive(noble_being * value, enum drives_definition drive);

n_int   being_height(noble_being * value);
n_int   being_mass(noble_being * value);

enum inventory_type being_carried(noble_being * value, enum BODY_INVENTORY_TYPES location);
void being_drop(noble_being * value, enum BODY_INVENTORY_TYPES location);
void being_take(noble_being * value, enum BODY_INVENTORY_TYPES location, enum inventory_type object);

#ifdef BRAINCODE_ON
n_byte * being_braincode_external(noble_being * value);
n_byte * being_braincode_internal(noble_being * value);
#endif

typedef void (being_loop_fn)(noble_simulation * sim, noble_being * actual, void * data);

void being_loop(noble_simulation * sim, being_loop_fn bl_func, void * data);

void being_loop_not_being(noble_simulation * sim, noble_being * being_not, being_loop_fn bl_func, void * data);

void   metabolism_vascular_description(n_int index, n_string description);
n_string metabolism_description(n_int index);
n_int  metabolism_vascular_radius(noble_being * local_being, n_int vessel_index);

void  being_name_byte2(n_byte2 first, n_byte2 family, n_string name);
n_int being_init(n_land * land, noble_being * beings, n_int number, noble_being * local, noble_being * mother,
                 n_byte2* random_factor);
void being_erase(noble_being * value);

n_uint being_affect(noble_simulation * local_sim, noble_being * local, n_byte is_positive);

void   being_cycle_awake(noble_simulation * sim, noble_being * local);
void   being_cycle_universal(noble_simulation * sim, noble_being * local, n_byte awake);

void    being_state_description(n_byte2 state, n_string result);

void    being_set_state(noble_being * value, being_state_type state);
void    being_add_state(noble_being * value, being_state_type state);
n_byte2 being_state(noble_being * value);

n_byte2 being_random(noble_being * value);

void being_set_select_name(noble_simulation * sim, n_string name);
n_string being_get_select_name(noble_simulation * sim);

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
    n_int being_index);

void brain_cycle(n_byte * local, n_byte2 * constants);
noble_being * being_from_name(noble_simulation * sim, n_string name);

void being_tidy(noble_simulation * local_sim);
void being_remove(noble_simulation * local_sim);

void brain_three_byte_command(n_string string, n_byte * response);

void brain_sentence(n_string string, n_byte * response);

n_int episode_description(noble_simulation * sim,
                          noble_being * local_being,
                          n_int index,
                          n_string description);

void episodic_logging(n_console_output * output_function, n_int social);

n_uint social_respect_mean(noble_simulation * sim, noble_being *local_being);
noble_being * being_find_name(noble_simulation * sim, n_byte2 first_gender, n_byte2 family);
void          being_move(noble_being * local, n_int vel, n_byte kind);
n_byte        being_awake(noble_simulation * sim, noble_being * local);

n_byte        being_los(n_land * land, noble_being * local, n_byte2 lx, n_byte2 ly);

void          speak_out(n_string filename, n_string paragraph);

void social_conception(noble_being * female,
                       noble_being * male,
                       noble_simulation * sim);

#endif /* NOBLEAPE_ENTITY_H */
