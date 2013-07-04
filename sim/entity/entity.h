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

#undef	FIXED_RANDOM_SIM

#undef	LARGE_SIM

#undef	BRAIN_HASH

#define SOFT_BRAIN_ON

#undef  RANDOM_INITIAL_BRAINCODE


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
    OUTPUT_ACTUATOR
};

#define GENEALOGY_ON

enum GENEALOGY_FORMAT
{
    GENEALOGY_NONE = 0,
    GENEALOGY_GENXML,
    GENEALOGY_GEDCOM,
    GENEALOGY_FORMATS
};

void being_memory(noble_simulation * local, n_byte * buffer, n_uint * location, n_int memory_available);

n_byte * being_brain(noble_being * value);
episodic_memory * being_episodic(noble_being * value);
social_link * being_social(noble_being * value);

n_int being_first_name(noble_being * value);

void being_set_family_name(noble_being * value, n_byte first, n_byte last);

n_int being_family_first_name(noble_being * value);

n_int being_family_second_name(noble_being * value);

n_int being_gender_name(noble_being * value);

n_int being_family_name(noble_being * value);

void being_name_simple(noble_being * value, n_string str);

n_int being_location_x(noble_being * value);

n_int being_location_y(noble_being * value);

n_byte2 * being_location(noble_being * value);

void being_delta(noble_being * primary, noble_being * secondary, n_vect2 * delta);
void being_set_location(noble_being * value, n_byte2 * from);

n_int being_dob(noble_being * value);

n_int being_speed(noble_being * value);
void being_set_speed(noble_being * value, n_byte sp);

void being_facing_towards(noble_being * value, n_vect2 * vector);
void being_facing_vector(noble_being * value, n_vect2 * vect, n_int divisor);

n_int being_facing(noble_being * value);
void being_wander(noble_being * value, n_int wander);

n_genetics * being_genetics(noble_being * value);

n_int being_energy(noble_being * value);
void   being_set_energy(noble_being * value, n_int energy);

void   being_energy_delta(noble_being * value, n_int delta);

n_int   being_drive(noble_being * value, enum drives_definition drive);
void    being_inc_drive(noble_being * value, enum drives_definition drive);
void    being_dec_drive(noble_being * value, enum drives_definition drive);
void    being_reset_drive(noble_being * value, enum drives_definition drive);

n_int   being_height(noble_being * value);
n_int   being_mass(noble_being * value);

void genealogy_log(noble_simulation * sim,n_byte value);
n_int genealogy_save(noble_simulation * sim, n_string filename);
void genealogy_birth(noble_being * child, noble_being * mother, void * sim);
void genealogy_death(noble_being * local_being, void * sim);

typedef void (being_no_return)(noble_simulation * sim, noble_being * actual);

void being_loop_no_return(noble_simulation * sim, being_no_return bnr_func);

n_int  brain_probe_to_location(n_int position);

void   metabolism_vascular_description(n_int index, n_string description);
n_string metabolism_description(n_int index);
n_int  metabolism_vascular_radius(noble_being * local_being, n_int vessel_index);

void   being_name(n_byte female, n_int first, n_byte family0, n_byte family1, n_string name);
n_int being_init(n_land * land, noble_being * beings, n_int number, noble_being * local, noble_being * mother,
                 n_byte2* random_factor);
void being_erase(noble_being * value);

void   being_cycle_awake(noble_simulation * sim, n_uint current_being_index);
void   being_cycle_universal(noble_simulation * sim, noble_being * local, n_byte awake);
void   being_state_description(n_byte2 state, n_string result);
n_uint being_affect(noble_simulation * local_sim, noble_being * local, n_byte is_positive);

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
n_int body_skeleton_points(noble_being * being, n_vect2 * keypoints, n_vect2 *points, n_int max_points,
                           n_int shoulder_angle, n_int elbow_angle, n_int wrist_angle,
                           n_int hip_angle, n_int knee_angle);

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

void brain_hash(n_byte *local_brain, n_byte * local_brain_hash);
void brain_cycle(n_byte * local, n_byte2 * constants);
noble_being * being_from_name(noble_simulation * sim, n_string name);

void being_tidy(noble_simulation * local_sim);
void being_remove(noble_simulation * local_sim);

void brain_three_byte_command(n_string string, n_byte * response);

void brain_sentence(n_string string, n_byte * response);

void braincode_statistics(noble_simulation * sim);

n_int episode_description(noble_simulation * sim,
                          noble_being * local_being,
                          n_int index,
                          n_string description);

void episodic_logging(n_console_output * output_function, n_int social);

n_uint social_respect_mean(noble_simulation * sim, noble_being *local_being);
noble_being * being_find_name(noble_simulation * sim, n_byte2 first_gender, n_byte2 family);
noble_being * being_find_female(noble_simulation * sim, n_genetics * genetics);
void          being_move(noble_being * local, n_int vel, n_byte kind);
n_byte        being_awake(noble_simulation * sim, noble_being * local);

n_byte        being_los(n_land * land, noble_being * local, n_byte2 lx, n_byte2 ly);

void          speak_out(n_string filename, n_string paragraph);

enum
{
    SKELETON_NECK = 0,
    SKELETON_LUMBAR,
    SKELETON_PELVIS,
    SKELETON_LEFT_HIP,
    SKELETON_RIGHT_HIP,
    SKELETON_LEFT_KNEE,
    SKELETON_RIGHT_KNEE,
    SKELETON_LEFT_ANKLE,
    SKELETON_RIGHT_ANKLE,
    SKELETON_LEFT_SHOULDER,
    SKELETON_RIGHT_SHOULDER,
    SKELETON_LEFT_SHOULDER_SOCKET,
    SKELETON_RIGHT_SHOULDER_SOCKET,
    SKELETON_LEFT_ELBOW,
    SKELETON_RIGHT_ELBOW,
    SKELETON_LEFT_WRIST,
    SKELETON_RIGHT_WRIST,
    SKELETON_LEFT_COLLAR,
    SKELETON_RIGHT_COLLAR,
    SKELETON_POINTS
};

#define SKELETON_LUMBAR_VERTIBRA  4
#define SKELETON_LUMBAR_VERTIBRA2 8
#define SKELETON_VERTIBRA_RIBS    10

#endif /* NOBLEAPE_ENTITY_H */
