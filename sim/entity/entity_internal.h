/****************************************************************

 entity_internal.h

 =============================================================

 Copyright 1996-2016 Tom Barbalet. All rights reserved.

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



enum inventory_type
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
};

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

#define IMMUNE_FIT                            5
#define MIN_ANTIBODIES                        16
#define MIN_ANTIGENS                          8
#define PATHOGEN_TRANSMISSION_PROB            1000
#define PATHOGEN_ENVIRONMENT_PROB             100
#define PATHOGEN_MUTATION_PROB                100
#define ANTIBODY_DEPLETION_PROB               100
#define ANTIBODY_GENERATION_PROB(bei)         (being_energy(bei))

#define RANDOM_PATHOGEN(seed,pathogen_type)   (((seed%(255/PATHOGEN_TRANSMISSION_TOTAL))*PATHOGEN_TRANSMISSION_TOTAL)+pathogen_type)
#define PATHOGEN_SEVERITY(pathogen)           (((pathogen)*(pathogen))>>11)
#define PATHOGEN_TRANSMISSION(pathogen)       ((pathogen)&7)

enum PATHOGEN_TRANSMISSION_METHOD
{
    PATHOGEN_TRANSMISSION_AIR = 0,
    PATHOGEN_TRANSMISSION_SOIL,
    PATHOGEN_TRANSMISSION_SEX,
    PATHOGEN_TRANSMISSION_TOUCH,
    PATHOGEN_TRANSMISSION_FOOD_VEGETABLE,
    PATHOGEN_TRANSMISSION_FOOD_FRUIT,
    PATHOGEN_TRANSMISSION_FOOD_SHELLFISH,
    PATHOGEN_TRANSMISSION_FOOD_SEAWEED,
    PATHOGEN_TRANSMISSION_TOTAL
};

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

/* gestation period in days */
#define GESTATION_DAYS      1

/* number of days after birth that weaning takes place */
#define WEANING_DAYS        14

/* time during which the mother is able to carry a child on her back */
#define CARRYING_DAYS       3

/* after a new ape is born how soon can the mother conceive */
#define CONCEPTION_INHIBITION_DAYS  5

/* energy gained by the child when suckling from the mother per time step */
#define SUCKLING_ENERGY     2

/* maximum separation between mother and child when suckling */
#define SUCKLING_MAX_SEPARATION     METRES_TO_APESPACE(2)

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

/* energy values for different foods */

/* TODO: add EGGS and potentially INSECTS  to food groups */

enum FOOD_KINDS
{
    FOOD_VEGETABLE = 0,
    FOOD_FRUIT,
    FOOD_SHELLFISH,
    FOOD_SEAWEED,
    FOOD_BIRD_EGGS,
    FOOD_LIZARD_EGGS,
    FOOD_TYPES
};

/* maximum energy obtainable from different types of food */

enum energy_types
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
};

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

/* affect (how many cycles it takes to forget particular episodic memory events) */

#define COMPOSITE_AFFECT(enjoyment,interest,surprise, \
anger,disgust,dissmell, \
distress,fear,shame) \
((enjoyment)+(interest)-(anger)-(disgust)-(dissmell)-(distress)-(fear)-(shame))

/* Affect values are composed from primitives */

typedef enum
{
    AFFECT_MATE                 = COMPOSITE_AFFECT(1000,0,0,      0,0,0,       0,0,0),
    AFFECT_BIRTH                = COMPOSITE_AFFECT(1500,0,1000,   0,100,100,   200,200,50),
    AFFECT_CARRYING             = COMPOSITE_AFFECT(300,300,0,     0,0,0,       0,0,0),
    AFFECT_CARRIED              = COMPOSITE_AFFECT(300,300,0,     0,0,0,       0,0,0),
    AFFECT_SUCKLING             = COMPOSITE_AFFECT(500,0,0,       0,0,0,       0,0,0),
    AFFECT_CHAT                 = COMPOSITE_AFFECT(0,100,0,       0,0,0,       0,0,0),
    AFFECT_GROOM                = COMPOSITE_AFFECT(50,50,0,       0,0,0,       0,0,0),
    AFFECT_SEEK_MATE            = COMPOSITE_AFFECT(0,600,0,       0,0,0,       0,0,0),
    AFFECT_SQUABBLE_VICTOR      = COMPOSITE_AFFECT(1100,0,100,    0,0,0,       100,0,0),
    AFFECT_SQUABBLE_VANQUISHED  = COMPOSITE_AFFECT(0,0,100,       200,0,0,     600,100,100),
    AFFECT_WHACKED              = COMPOSITE_AFFECT(0,0,100,       20,0,0,      20,20,40),
    AFFECT_HURL                 = COMPOSITE_AFFECT(0,0,0,         100,0,0,     0,0,0),
    AFFECT_HUGGED               = COMPOSITE_AFFECT(100,0,0,       0,0,0,       0,0,0),
    AFFECT_PRODDED              = COMPOSITE_AFFECT(0,0,0,         0,0,0,       5,0,5),
    AFFECT_RECEIVE              = COMPOSITE_AFFECT(25,25,0,       0,0,0,       0,0,0),
    AFFECT_FISH                 = COMPOSITE_AFFECT(100,100,0,     0,0,0,       0,0,0),
    AFFECT_SMILED               = COMPOSITE_AFFECT(10,0,0,        0,0,0,       0,0,0),
    AFFECT_GLOWER               = COMPOSITE_AFFECT(0,0,10,        0,0,0,       0,10,0)
} affect_type;
/* ------- ------- ------- GENETICS FOR THE NOBLE APES (BETA) ------- ------- ------- */

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

/* nature in the range 0-15 from the genetics
 nurture in the range 0-255 from learned preferences.
 Resulting value is in the range 0-15 */
#define NATURE_NURTURE(nature,nurture)      (((nature) + ((nurture)>>4))>>1)

/* A social drive threshold value above which beings interact */
#define SOCIAL_THRESHOLD(bei)               ((NATURE_NURTURE(GENE_SOCIAL(being_genetics(bei)),bei->changes.learned_preference[PREFERENCE_SOCIAL]))>>1)


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
    n_int loc_x,
    n_int loc_y,
    n_int az,
    n_byte * food_type,
    noble_being * local_being);

n_int food_absorption(noble_being * local, n_int max_energy, n_byte food_type);

void food_values(n_int loc_x,
                 n_int loc_y,
                 n_int *grass, n_int *trees, n_int *bush);

n_int genetics_compare(n_genetics * genetics_a, n_genetics * genetics_b);
void  genetics_set(n_genetics * genetics_a, n_genetics * n_genetics);
void  genetics_zero(n_genetics * genetics_a);

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

n_int being_posture_under(noble_being * value, enum posture_type post);

void being_honor_inc_dec(noble_being * inc, noble_being * dec);

void being_honor_swap(noble_being * victor, noble_being * vanquished);

n_int being_honor_compare(noble_being * first, noble_being * second);

void being_unpack_family(n_byte2 name, n_byte * values);

n_byte2 * being_location(noble_being * value);

void being_remove_parasites(noble_being * value, n_int number_of_parasites);

void being_delta(noble_being * primary, noble_being * secondary, n_vect2 * delta);

void being_facing_towards(noble_being * value, n_vect2 * vector);

n_int being_pregnant(noble_being * value);

n_genetics * being_fetal_genetics(noble_being * value);

n_int being_energy_less_than(noble_being * value, n_int less_than);

void  being_living(noble_being * value);

void  being_inc_drive(noble_being * value, enum drives_definition drive);

void  being_dec_drive(noble_being * value, enum drives_definition drive);

void  being_reset_drive(noble_being * value, enum drives_definition drive);

enum inventory_type being_carried(noble_being * value, enum BODY_INVENTORY_TYPES location);
void being_drop(noble_being * value, enum BODY_INVENTORY_TYPES location);
void being_take(noble_being * value, enum BODY_INVENTORY_TYPES location, enum inventory_type object);

void being_remains(noble_simulation * sim, noble_being * dead);

#endif /* NOBLEAPE_ENTITY_INTERNAL_H */
