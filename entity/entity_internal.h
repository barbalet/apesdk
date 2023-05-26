/****************************************************************

 entity_internal.h

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

#ifndef SIMULATEDAPE_ENTITY_INTERNAL_H
#define SIMULATEDAPE_ENTITY_INTERNAL_H


#define CONSUME_E(being, max_energy, food)

/* offsets applied to land operator values */
#define OFFSET_GRASS    40
#define OFFSET_BUSH     14

enum featureset_members
{
    FEATURESET_PIGMENTATION,
    FEATURESET_HAIR,
    FEATURESET_HEIGHT,
    FEATURESET_FAT,
    FEATURESET_EYE_SHAPE,
    FEATURESET_EYE_COLOR,
    FEATURESET_EYE_SEPARATION,
    FEATURESET_NOSE_SHAPE,
    FEATURESET_EAR_SHAPE,
    FEATURESET_EYEBROW_SHAPE,
    FEATURESET_MOUTH_SHAPE,
    FEATURESET_TERRITORY,

    FEATURESET_FEATURE_SET,
    FEATURESET_SIZE
};

/* honor multiplier used to calculate the probability of a female mating */
#define MATING_PROB         12

/* Note that the number here are reduced from realistic values into
 a smaller 30 day maturity range */

/* maximum distance over which a parasite can hob from one being to another */
#define PARASITE_HOP_MAX_DISTANCE  METRES_TO_APESPACE(2)

/* how much energy does a parasite cost the ape per time step */
#define PARASITE_ENERGY_COST       1

/* probability of acquiring a parasite from the environment */
#define PARASITE_ENVIRONMENT       5000

/* multiplying factor used to increase the probability
 if already carrying parasites */
#define PARASITE_BREED             10

/* maximum separation between grooming apes */
#define GROOMING_MAX_SEPARATION    METRES_TO_APESPACE(2)

/* apes must be moving slowly to be able to groom */
#define MAX_SPEED_WHILST_GROOMING  30

/* the probability of social grooming */
#define GROOMING_PROB              10000

/* multiplier used to increase the probability of
 grooming a higher status individual */
#define GROOMING_PROB_HONOR        10

/* number of parasites removed at each grooming session */
#define PARASITES_REMOVED          2

/* speed of running away */
#define SQUABBLE_FLEE_SPEED         20

/* amount of energy consumed during a show of force */
#define SQUABBLE_ENERGY_SHOWFORCE   200

/* amount of energy consumed during an attack */
#define SQUABBLE_ENERGY_ATTACK      500

/* amount of energy lost if hit by a rock */
#define SQUABBLE_ENERGY_ROCK_HURL    100

/* amount of energy lost if hit by a branch */
#define SQUABBLE_ENERGY_BRANCH_WHACK 50

/* friend or foe adjustment made within the social graph after a squabble */
#define SQUABBLE_DISRESPECT         20

/* honor value adjustment after a squabble */
#define SQUABBLE_HONOR_ADJUST       10

/* distance beyond which a squabble only results in a show of force */
#define SQUABBLE_SHOW_FORCE_DISTANCE 10

/* the minimum amount of genetic variation (bases) below which individuals
 are considered to be related */
#define MINIMUM_GENETIC_VARIATION   32



/* Affect values are composed from primitives */

enum mutation_type
{
    MUTATION_MATERNAL = 0,
    MUTATION_PATERNAL,
    MUTATION_MATERNAL_DUPLICATE,
    MUTATION_PATERNAL_DUPLICATE
};

#define DIPLOID(parent1,parent2)    ((parent1)|((parent2)<<16))
#define CHROMOSOME_FROM_MOTHER(ch)  (((ch)>>16)&65535)
#define CHROMOSOME_FROM_FATHER(ch)  ((ch)&65535)

/* probability of mutation */
#define MUTATION_CROSSOVER_PROB     500
#define MUTATION_DELETION_PROB      200
#define MUTATION_TRANSPOSE_PROB     200

enum posture_type
{
    POSTURE_CROUCHING = 100,
    POSTURE_UPRIGHT = 200
};

enum individual_action_type
{
    ACTION_PICKUP = 0,
    ACTION_DRAG,
    ACTION_DROP,
    ACTION_SWAP_HANDS,
    ACTION_BRANDISH,
    ACTION_CHEW,
    ACTION_BASH_OBJECTS,
    ACTION_JAB,
    INDIVIDUAL_ACTIONS
};

enum social_action_type
{
    ACTION_GIVE = 0,
    ACTION_BASH,
    ACTION_HUG,
    ACTION_PROD,
    ACTION_POINT,
    ACTION_TICKLE,
    ACTION_SMILE,
    ACTION_GLOWER,
    ACTION_PAT,
    SOCIAL_ACTIONS
};

n_int food_absorption( simulated_being *local, n_int max_energy, n_byte food_type );

void food_values( n_int loc_x,
                  n_int loc_y,
                  n_int *grass, n_int *trees, n_int *bush );

n_int genetics_compare( n_genetics *genetics_a, n_genetics *genetics_b );
void  genetics_zero( n_genetics *genetics_a );

void social_action( simulated_being *meeter_being, simulated_being *met_being, n_byte action );

n_int get_simulated_isocial( simulated_being *meeter_being, simulated_being *met_being );

n_int episodic_met_being_celebrity(
    simulated_being *meeter_being,
    simulated_being *met_being );

void episodic_store_memory(
    simulated_being *local,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 name1, n_byte2 family1,
    n_byte2 name2, n_byte2 family2, n_byte2 arg );

void episodic_interaction(
    simulated_being *local,
    simulated_being *other,
    being_episodic_event_type event,
    AFFECT_TYPE affect,
    n_byte2 arg );

n_byte episodic_intention(
    simulated_being *local,
    n_int episode_index,
    n_byte2 mins_ahead,
    n_byte args );

n_byte episodic_anecdote(
    simulated_being *local,
    simulated_being *other );

void being_init_braincode( simulated_being *local,
                           simulated_being *other,
                           n_byte friend_foe,
                           n_byte internal );

n_byte get_braincode_instruction( simulated_being *local_being );

n_int being_posture_under( simulated_being *value, enum posture_type post );

void being_honor_inc_dec( simulated_being *inc, simulated_being *dec );

void being_honor_swap( simulated_being *victor, simulated_being *vanquished );

n_int being_honor_compare( simulated_being *first, simulated_being *second );

void being_unpack_family( n_byte2 name, n_byte *values );

void being_remove_parasites( simulated_being *value, n_int number_of_parasites );

inventory_type being_carried( simulated_being *value, BODY_INVENTORY_TYPES location );
void being_drop( simulated_being *value, BODY_INVENTORY_TYPES location );
void being_take( simulated_being *value, BODY_INVENTORY_TYPES location, inventory_type object );

void being_remains( simulated_group *group, simulated_being *dead );

void brain_cycle( n_byte *local, n_byte2 *constants );

#endif /* SIMULATEDAPE_ENTITY_INTERNAL_H */
