/****************************************************************

 entity.h

 =============================================================

 Copyright 1996-2023 Tom Barbalet. All rights reserved.

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

#ifndef SIMULATEDAPE_ENTITY_H
#define SIMULATEDAPE_ENTITY_H

#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"
#include "../universe/universe.h"

#define    LAND_ON
#define    WEATHER_ON
#define RANDOM_INITIAL_BRAINCODE
#undef  SHORT_NAMES
#undef    FIXED_RANDOM_SIM

enum being_energy
{
    BEING_DEAD   = 0,
    BEING_HUNGRY = ( 10 * 128 ),
    BEING_FULL   = ( BEING_HUNGRY * 3 )
};

#define SOCIAL_RESPECT_NORMAL    127

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
    BEING_STATE_NO_FOOD = 2048,
    BEING_STATES = 13
} being_state_type;

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
} being_episodic_event_type;


/* affect (how many cycles it takes to forget particular episodic memory events) */

#define COMPOSITE_AFFECT(enjoyment,interest,surprise, \
anger,disgust,dissmell, \
distress,fear,shame) \
((enjoyment)+(interest)-(anger)-(disgust)-(dissmell)-(distress)-(fear)-(shame))

typedef enum
{
    AFFECT_MATE                 = COMPOSITE_AFFECT( 1000, 0, 0,      0, 0, 0,       0, 0, 0 ),
    AFFECT_BIRTH                = COMPOSITE_AFFECT( 1500, 0, 1000,   0, 100, 100,   200, 200, 50 ),
    AFFECT_CARRYING             = COMPOSITE_AFFECT( 300, 300, 0,     0, 0, 0,       0, 0, 0 ),
    AFFECT_CARRIED              = COMPOSITE_AFFECT( 300, 300, 0,     0, 0, 0,       0, 0, 0 ),
    AFFECT_SUCKLING             = COMPOSITE_AFFECT( 500, 0, 0,       0, 0, 0,       0, 0, 0 ),
    AFFECT_CHAT                 = COMPOSITE_AFFECT( 0, 100, 0,       0, 0, 0,       0, 0, 0 ),
    AFFECT_GROOM                = COMPOSITE_AFFECT( 50, 50, 0,       0, 0, 0,       0, 0, 0 ),
    AFFECT_SEEK_MATE            = COMPOSITE_AFFECT( 0, 600, 0,       0, 0, 0,       0, 0, 0 ),
    AFFECT_SQUABBLE_VICTOR      = COMPOSITE_AFFECT( 1100, 0, 100,    0, 0, 0,       100, 0, 0 ),
    AFFECT_SQUABBLE_VANQUISHED  = COMPOSITE_AFFECT( 0, 0, 100,       200, 0, 0,     600, 100, 100 ),
    AFFECT_WHACKED              = COMPOSITE_AFFECT( 0, 0, 100,       20, 0, 0,      20, 20, 40 ),
    AFFECT_HURL                 = COMPOSITE_AFFECT( 0, 0, 0,         100, 0, 0,     0, 0, 0 ),
    AFFECT_HUGGED               = COMPOSITE_AFFECT( 100, 0, 0,       0, 0, 0,       0, 0, 0 ),
    AFFECT_PRODDED              = COMPOSITE_AFFECT( 0, 0, 0,         0, 0, 0,       5, 0, 5 ),
    AFFECT_RECEIVE              = COMPOSITE_AFFECT( 25, 25, 0,       0, 0, 0,       0, 0, 0 ),
    AFFECT_FISH                 = COMPOSITE_AFFECT( 100, 100, 0,     0, 0, 0,       0, 0, 0 ),
    AFFECT_SMILED               = COMPOSITE_AFFECT( 10, 0, 0,        0, 0, 0,       0, 0, 0 ),
    AFFECT_GLOWER               = COMPOSITE_AFFECT( 0, 0, 10,        0, 0, 0,       0, 10, 0 ),
    EPISODIC_AFFECT_ZERO = ( 16384 )

} AFFECT_TYPE;

typedef enum
{
    INVENTORY_CHILD = 1,
    INVENTORY_WOUND = 2,
    INVENTORY_GROOMED = 4,
    INVENTORY_BRANCH = 8,
    INVENTORY_ROCK = 16,
    INVENTORY_SHELL = 32,
    INVENTORY_TWIG = 64,
    INVENTORY_NUT = 128,
    INVENTORY_NUT_CRACKED = 256,
    INVENTORY_GRASS = 512,
    INVENTORY_SCRAPER = 1024,
    INVENTORY_SPEAR = 2048,
    INVENTORY_FISH = 4096,
    INVENTORY_BIRD_EGGS = 8192,
    INVENTORY_LIZARD_EGGS = 16384
} inventory_type;


/* energy values for different foods */
/* TODO: add EGGS and potentially INSECTS  to food groups */

typedef enum
{
    FOOD_VEGETABLE = 0,
    FOOD_FRUIT,
    FOOD_SHELLFISH,
    FOOD_SEAWEED,
    FOOD_BIRD_EGGS,
    FOOD_LIZARD_EGGS,
    FOOD_TYPES
} FOOD_KINDS;

/* maximum energy obtainable from different types of food */

typedef enum
{
    ENERGY_GRASS               = 50,
    ENERGY_BUSH                = 100,
    ENERGY_FRUIT               = 100,
    ENERGY_SEAWEED             = 30,
    ENERGY_SHELLFISH           = 300,
    ENERGY_NUT                 = 200,
    ENERGY_FISH                = 600,
    ENERGY_BIRD_EGGS           = 800,
    ENERGY_LIZARD_EGGS         = 1000
} energy_types;

/* gestation period in days */
#define GESTATION_DAYS      1

/* energy gained by the child when suckling from the mother per time step */
#define SUCKLING_ENERGY     2

/* maximum separation between mother and child when suckling */
#define SUCKLING_MAX_SEPARATION     METRES_TO_APESPACE(2)

/* number of days after birth that weaning takes place */
#define WEANING_DAYS        14

/* time during which the mother is able to carry a child on her back */
#define CARRYING_DAYS       3

/* after a new ape is born how soon can the mother conceive */
#define CONCEPTION_INHIBITION_DAYS  5

#define APESPACE_TO_HR_MAPSPACE(num)  ((num)>>3)

/* nature in the range 0-15 from the genetics
 nurture in the range 0-255 from learned preferences.
 Resulting value is in the range 0-15 */
#define NATURE_NURTURE(nature,nurture)      (((nature) + ((nurture)>>4))>>1)


/* A social drive threshold value above which beings interact */
#define SOCIAL_THRESHOLD(bei)               ((NATURE_NURTURE(GENE_SOCIAL(being_genetics(bei)),bei->changes.learned_preference[PREFERENCE_SOCIAL]))>>1)



/* Defines initial braincode instruction type probability*/
#define GENE_BRAINCODE_SENSORS(gene)        GENE_VAL_REG(gene, 11, 14, 4, 7)
#define GENE_BRAINCODE_ACTUATORS(gene)      GENE_VAL_REG(gene, 15, 10, 10, 0)
#define GENE_BRAINCODE_CONDITIONALS(gene)   GENE_VAL_REG(gene, 4, 9, 0, 2)
#define GENE_BRAINCODE_OPERATORS(gene)      GENE_VAL_REG(gene, 12, 1, 6, 12)
#define GENE_BRAINCODE_DATA(gene)           GENE_VAL_REG(gene, 8, 15, 7, 12)

/* Rates at which the affect associated with episodic memories fade over time.
 This makes dysphoria-like memory effects possible, and also regulate how
 long events remain in episodic memory */
#define GENE_NEGATIVE_AFFECT_FADE(gene)     GENE_VAL_REG(gene, 9, 15, 13, 2)
#define GENE_POSITIVE_AFFECT_FADE(gene)     GENE_VAL_REG(gene, 11, 4, 3, 12)

/* preference for social activity, similar to flocking */
#define GENE_SOCIAL(gene)                   GENE_VAL_REG(gene, 22, 2, 13, 9)


/* worst case 1500 + 180 per step */

#define VISIBILITY_MAXIMUM      (2000)

#define VISIBILITY_SPAN    VISIBILITY_MAXIMUM /*(VISIBILITY_MAXIMUM / ((15+16) >> 1))*/

#define    WALK_ON_WATER(pz,w)    (((pz)<w) ? w : (pz))

/** Swim - better or worse at swimming both speed and energy use */

#define GENE_SWIM(gene)                     GENE_VAL_REG(gene, 9, 11, 13, 7)

/** Speed on land - faster v. slower */

#define GENE_SPEED(gene)                    GENE_VAL_REG(gene, 14, 5, 12, 10)

/** Control on random wander - more or less random wander */

#define GENE_STAGGER(gene)                  GENE_VAL_REG(gene, 12, 14, 3, 11)

/** Hill climbing - improved energy use for climbing */

#define GENE_HILL_CLIMB(gene)               GENE_VAL_REG(gene, 4, 6, 5, 2)


/* different types of goal */
typedef enum
{
    GOAL_NONE           = 0,
    GOAL_LOCATION       = 1,
    GOAL_MATE           = 2,
    GOAL_UNKNOWN        = 3 /* add a new goal here when needed */
} goal_types;

typedef struct
{
    simulated_being   *local;
    simulated_isocial *local_social;
    n_uint         opposite_sex_distance;
    n_uint         same_sex_distance;
    simulated_being   *opposite_sex;
    simulated_being   *same_sex;
} being_nearest;

void  being_living( simulated_being *value );

void  being_inc_drive( simulated_being *value, enum drives_definition drive );
void  being_dec_drive( simulated_being *value, enum drives_definition drive );

n_int being_energy_less_than( simulated_being *value, n_int less_than );

void being_turn_away_from_water( simulated_being *value );
void being_facing_init( simulated_being *value );
n_byte being_honor_immune( simulated_being *value );
void being_set_brainstates( simulated_being *value, n_int asleep, n_byte2 val1, n_byte2 val2, n_byte2 val3 );


void being_delta( simulated_being *primary, simulated_being *secondary, n_vect2 *delta );
void being_facing_towards( simulated_being *value, n_vect2 *vector );
n_int being_pregnant( simulated_being *value );
n_int being_female( simulated_being *value );
n_int being_speaking( simulated_being *value );

n_genetics *being_fetal_genetics( simulated_being *value );


n_int social_set_relationship( simulated_group *group, simulated_being *meeter_being, n_byte relationship, simulated_being *met_being );

n_int social_get_relationship( simulated_being *meeter_being,
                               n_byte relationship );

n_int social_network( simulated_group *group, simulated_being *meeter_being, simulated_being *met_being, n_uint distance );

n_byte social_groom( simulated_group *group,
                     simulated_being *meeter_being,
                     simulated_being *met_being,
                     n_uint distance,
                     n_int awake,
                     n_byte2 familiarity );

n_byte2 social_squabble( simulated_being *meeter_being, simulated_being *met_being, n_uint distance, n_int is_female, simulated_group *group );
n_int social_mate( simulated_being *meeter_being, simulated_being *met_being, n_int being_index, n_uint distance, simulated_group *group );
n_int social_chat( simulated_being *meeter_being, simulated_being *met_being, n_int being_index, simulated_group *group );


void episodic_food( simulated_being *local, n_int energy, n_byte food_type );

void immune_init( simulated_immune_system *immune, n_byte2 *local_random );
void immune_seed( simulated_immune_system *immune_mother, simulated_immune_system *immune_child );
void immune_transmit( simulated_immune_system *immune0, simulated_immune_system *immune1, n_byte transmission_type );
n_int immune_response( simulated_immune_system *immune, n_byte honor_immune, n_int being_energy );
void immune_ingest_pathogen( simulated_immune_system *immune, n_byte food_type );


void body_genetics( simulated_being *beings, n_int number, n_genetics *genetics, n_genetics *mother_genetics, n_genetics *father_genetics, n_byte2 *local );

n_int food_eat(
    n_int loc_x,
    n_int loc_y,
    n_int az,
    n_byte *food_type,
    simulated_being *local_being );

void  genetics_set( n_genetics *genetics_a, n_genetics *n_genetics );

void episodic_self(
    simulated_being *local,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 arg );

void episodic_close(
    simulated_being *local,
    simulated_being *other,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 arg );

n_byte being_los_projection( simulated_being *local, n_vect2 *extern_end );

#ifdef BRAIN_ON
n_byte *being_brain( simulated_being *value );
#endif

simulated_iepisodic *being_episodic( simulated_being *value );
simulated_isocial *being_social( simulated_being *value );

n_int being_brainstates( simulated_being *value, n_int awake, n_byte2 *states );

n_byte being_honor( simulated_being *value );

void being_honor_delta( simulated_being *value, n_int delta );

n_byte being_first_name( simulated_being *value );

void being_set_family_name( simulated_being *value, n_byte first, n_byte last );

n_byte being_family_first_name( simulated_being *value );

n_byte being_family_second_name( simulated_being *value );

n_byte2 being_gender_name( simulated_being *value );

n_byte2 being_family_name( simulated_being *value );

void being_name_simple( simulated_being *value, n_string str );

n_byte being_posture( simulated_being *value );

void being_set_posture( simulated_being *value, n_byte post );

n_int being_location_x( simulated_being *value );

n_int being_location_y( simulated_being *value );

n_byte2 *being_location( simulated_being *value );

void being_set_location( simulated_being *value, n_byte2 *from );

#ifdef DEBUG_LACK_OF_MOVEMENT

n_int being_total_movement( simulated_being *value );
void being_add_total_movement( simulated_being *value );
void being_zero_total_movement( simulated_being *value );
void being_register_movement( simulated_being *value, n_string comment_string );

#endif

n_int being_dob( simulated_being *value );

n_byte being_speed( simulated_being *value );

void being_set_speed( simulated_being *value, n_byte sp );

void being_speed_advance( simulated_being *value );

n_int being_ten_minute_distance( simulated_being *value );

void being_facing_vector( simulated_being *value, n_vect2 *vect, n_int divisor );

n_byte being_facing( simulated_being *value );

void being_wander( simulated_being *value, n_int wander );

n_genetics *being_genetics( simulated_being *value );

n_int being_energy( simulated_being *value );

void  being_dead( simulated_being *value );

void   being_energy_delta( simulated_being *value, n_int delta );

n_int being_parasites( simulated_being *value );

void being_set_parasites( simulated_being *value, n_byte parasites );

void being_add_parasites( simulated_being *value );

void being_remove_parasites( simulated_being *value, n_int number_of_parasites );

n_byte   being_drive( simulated_being *value, enum drives_definition drive );
n_byte *being_drives( simulated_being *value );

n_int   being_height( simulated_being *value );

void    being_set_height( simulated_being *value, n_int height );

n_int   being_mass( simulated_being *value );

typedef struct
{
    n_int         max_shout_volume;
    simulated_being *local;
} being_listen_struct;

void being_listen( simulated_group *group, simulated_being *local, void *data );

void being_listen_loop_no_sim( simulated_being *other, void *data );

#ifdef BRAINCODE_ON
n_byte *being_braincode_external( simulated_being *value );
n_byte *being_braincode_internal( simulated_being *value );
#endif

void  being_remove_external_set( n_int value );
n_int being_remove_internal( void );
void being_remove_internal_clear( void );

void   metabolism_vascular_description( n_int index, n_string description );
n_string metabolism_description( n_int index );
n_int  metabolism_vascular_radius( simulated_being *local_being, n_int vessel_index );

void  being_name_byte2( n_byte2 first, n_byte2 family, n_string name );
n_int being_init( simulated_being *beings, n_int number, simulated_being *local, simulated_being *mother,
                  n_byte2 *random_factor );

n_uint being_init_group( simulated_being *beings, n_byte2 *local_random, n_uint count_to, n_uint max );

void being_erase( simulated_being *value );

n_uint being_affect( simulated_being *local, n_byte is_positive );

void   episodic_cycle( simulated_group *group, simulated_being *local, void *data );
void   episodic_cycle_no_sim( simulated_being *local_being );

void   being_cycle_awake( simulated_group *group, simulated_being *local );
void   being_cycle_universal( simulated_being *local );

typedef struct
{
    n_int         beings_in_vacinity;
    simulated_being *being;
} drives_sociability_data;

void drives_sociability_loop_no_sim( simulated_being *other, void *data );

void drives_cycle( simulated_group *group, simulated_being *local_being, void *data );

void drives_fatigue( simulated_being *local );
void drives_hunger( simulated_being *local );

void    being_state_description( n_byte2 state, n_string result );

void    being_set_state( simulated_being *value, being_state_type state );
void    being_add_state( simulated_being *value, being_state_type state );
n_byte2 being_state( simulated_being *value );

n_byte2 being_random( simulated_being *value );

void being_set_goal_mate( simulated_being *local, n_byte2 first_name, n_byte2 family_name );
void being_set_goal_none( simulated_being *local );
void being_set_goal_location( simulated_being *local, n_byte2 lx, n_byte2 ly );

n_int being_check_goal( simulated_being *local, goal_types goal );
void being_goal_cycle( simulated_being *local );

void being_set_select_name( simulated_group *group, n_string name );
n_string being_get_select_name( simulated_group *group );

n_int being_name_comparison( simulated_being *value, n_byte2 gender_name, n_byte2 family_name );

void social_graph_link_name(
    simulated_group *group,
    simulated_being *local_being,
    n_int social_graph_index,
    n_byte met, n_string name );

n_int episodic_first_person_memories_percent(
    simulated_being *local,
    n_byte intention );

void body_genome( n_byte maternal, n_genetics *genome, n_byte *genome_str );

void being_relationship_description( n_int index, n_string description );
n_string being_body_inventory_description( n_int index );

void brain_dialogue(
    simulated_group *group,
    n_byte awake,
    simulated_being *meeter_being,
    simulated_being *met_being,
    n_byte         *bc0,
    n_byte         *bc1,
    n_int being_index );

simulated_being *being_from_name( simulated_group *group, n_string name );

void being_remove( simulated_group *group );

void brain_three_byte_command( n_string string, n_byte *response );

void brain_sentence( n_string string, n_byte *response );

n_int episode_description( simulated_being *local_being,
                           n_int index,
                           n_string description );

void episodic_logging( n_console_output *output_function, n_int social );

n_uint social_respect_mean( simulated_being *local_being );

void social_goals( simulated_being *local );

void being_genetic_wandering( simulated_being *local, being_nearest *nearest );

void being_territory_index( simulated_being *local );
void being_calculate_speed( simulated_being *local, n_int tmp_speed, n_byte2 loc_state );

simulated_being *being_find_name( simulated_group *group, n_byte2 first_gender, n_byte2 family );
void          being_move( simulated_being *local, n_int vel, n_byte kind );
n_byte        being_awake( simulated_being *local );

n_byte being_crowding( simulated_being *value );
void being_crowding_cycle( simulated_being *value, n_int beings_in_vacinity );

void speak_out( n_string filename, n_string paragraph );

void social_conception( simulated_being *female,
                        simulated_being *male,
                        simulated_group *group );

void social_initial_loop( simulated_group *group, simulated_being *local_being, void *data );

void social_secondary_loop_no_sim( simulated_being *local_being );

void being_tidy_loop_no_sim( simulated_being *local_being, void *data );

void being_recalibrate_honor_loop_no_sim( simulated_being *value );

void being_remove_loop1( simulated_group *group, simulated_being *local_being, void *data );

typedef struct
{
    simulated_being *being_count;
    simulated_being *reference;
    n_int         selected_died;
    n_uint        count;
} being_remove_loop2_struct;

void being_remove_loop2( simulated_group *group, simulated_being *local, void *data );

being_remove_loop2_struct *being_remove_initial( simulated_group *group );

void being_remains_init( simulated_remains *remains );

n_int being_index( simulated_group *group, simulated_being *local );

void being_high_res( simulated_being *value, n_vect2 *vector );

void being_space( simulated_being *value, n_vect2 *vector );

void being_clear_attention( simulated_being *value );

void being_set_attention( simulated_being *value, n_int index, n_int attention );

n_byte being_attention( simulated_being *value, n_int index );

n_byte being_basic_line_of_sight( simulated_being *local, n_vect2 *extern_end, n_vect2 *start, n_vect2 *delta, n_vect2 *end );

typedef n_byte ( simulated_being_can_move )( n_vect2 *location, n_vect2 *delta );
void   being_can_move_override( simulated_being_can_move *new_can_move );
n_byte being_can_move( n_vect2 *location, n_vect2 *delta );

typedef void   ( simulated_being_wrap )( n_vect2 *location );
void being_wrap_override( simulated_being_wrap *new_move );
void being_wrap( n_vect2 *location );

typedef void   ( simulated_being_initial_location )( n_vect2 *location, n_byte2 *seed );
void being_initial_location_override( simulated_being_initial_location *new_initial_location );
void being_initial_location( n_vect2 *location, n_byte2 *seed );

typedef n_byte ( simulated_being_line_of_sight )( simulated_being *local, n_vect2 *location );
void being_line_of_sight_override( simulated_being_line_of_sight *new_line_of_sight );
n_byte being_line_of_sight( simulated_being *local, n_vect2 *location );

typedef void ( simulated_being_brain_cycle )( n_byte *local, n_byte2 *constants );
void being_brain_cycle_override( simulated_being_brain_cycle *new_brain_cycle );
void being_brain_cycle( n_byte *local, n_byte2 *constants );


void being_reset_drive( simulated_being *value, enum drives_definition drive );
n_uint being_genetic_comparison( n_genetics *primary, n_genetics *secondary, n_int parse_requirements );
n_int being_move_energy( simulated_being *local_being, n_int *conductance );

#endif /* SIMULATEDAPE_ENTITY_H */
