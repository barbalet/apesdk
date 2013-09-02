/****************************************************************

 universe.h

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
#ifndef NOBLEAPE_UNIVERSE_H
#define NOBLEAPE_UNIVERSE_H
/*NOBLEMAKE END=""*/

/*NOBLEMAKE VAR=""*/

/* displays and simulates ape's brain */

#undef  THREADED

#define EPISODIC_ON /* currently has to be included - naming doesn't work without it */
#define PARASITES_ON  /* entity */
#define TERRITORY_ON  /* entity */
#define BRAINCODE_ON  /* entity */
#define IMMUNE_ON     /* entity */
#define METABOLISM_ON /* entity */

/* dimension of the territory map */
#define TERRITORY_DIMENSION           (MAP_DIMENSION>>6)
#define TERRITORY_AREA                (TERRITORY_DIMENSION*TERRITORY_DIMENSION)
#define APESPACE_TO_TERRITORY(num)    (APESPACE_TO_MAPSPACE(num)>>6)

/*
 This table represents the operator calculation that is used to create the density
 graphs. The first section of the table shows if a component is added "+", subtracted
 "-", not used "." or boolean true "X". This is translated to numbers in the second
 part of the table. This is condensed into the operator number and at the far right
 what these operator sums represent.
 */

static const n_byte	operators[17][7] =
{
    /*AHWOUS*/
    "+.....", /* Area */
    ".+....", /* Height */
    "..+...", /* Water */
    "...+..", /* Moving Sun */
    "....+.", /* Total Sun */
    ".....+", /* Salt */
    /*AHWOUS*/
    ".-+.+-", /* Bush */
    "..+.+-", /* Grass */
    "-++.+-", /* Tree */
    "-++.-+", /* Seaweed */
    "+.-.-+", /* Rockpool */
    "+-+-++", /* Beach */
    /*AHWOUS*/
    "..+-.-", /* Insect */
    "..--.-", /* Mouse */
    "..+++-", /* Parrot */
    "-+.-.-", /* Lizard */
    "..+-+-"  /* Eagle */
    /*AHWOUS*/
};

#define AGE_IN_DAYS(sim,bei)        (TIME_IN_DAYS(sim->land->date) - being_dob(bei))
#define AGE_IN_YEARS(sim,bei)       (AGE_IN_DAYS(sim,bei)/TIME_YEAR_DAYS)


/* in days, a little young, yes */
#define AGE_OF_MATURITY (30)

/*4*/
enum SECONDARY_APESCRIPT
{
    VARIABLE_VECT_X = (VARIABLE_IF + 1),
    VARIABLE_VECT_Y,

    VARIABLE_RANDOM,
    VARIABLE_WATER_LEVEL,

    VARIABLE_BIOLOGY_AREA,
    VARIABLE_BIOLOGY_HEIGHT,

    VARIABLE_BIOLOGY_WATER,
    VARIABLE_BIOLOGY_MOVING_SUN,

    VARIABLE_BIOLOGY_TOTAL_SUN,
    VARIABLE_BIOLOGY_SALT,
    VARIABLE_BIOLOGY_BUSH,

    VARIABLE_BIOLOGY_GRASS,
    VARIABLE_BIOLOGY_TREE,

    VARIABLE_BIOLOGY_SEAWEED,
    VARIABLE_BIOLOGY_ROCKPOOL,
    VARIABLE_BIOLOGY_BEACH,

    VARIABLE_BIOLOGY_INSECT,
    VARIABLE_BIOLOGY_MOUSE,

    VARIABLE_BIOLOGY_PARROT,
    VARIABLE_BIOLOGY_LIZARD,

    VARIABLE_BIOLOGY_EAGLE,
    VARIABLE_BIOLOGY_OUTPUT,

    VARIABLE_HUNGRY,
    VARIABLE_LOCATION_Z,

    VARIABLE_TEST_Z,
    VARIABLE_IS_VISIBLE,

    VARIABLE_TIME,
    VARIABLE_DATE,

    VARIABLE_CURRENT_BEING,
    VARIABLE_NUMBER_BEINGS,

    VARIABLE_LOCATION_X,
    VARIABLE_LOCATION_Y,

    VARIABLE_STATE,
    VARIABLE_LISTEN,

    VARIABLE_ID_NUMBER,
    VARIABLE_DATE_OF_BIRTH,

    VARIABLE_WEATHER,  /* Everything after this value can be both set and get */
    VARIABLE_BRAIN_VALUE, /* This is a special case, all the remainder are stored as variables */

    VARIABLE_VECT_ANGLE,
    VARIABLE_FACING,

    VARIABLE_SPEED,
    VARIABLE_ENERGY,

    VARIABLE_HONOR,
    VARIABLE_PARASITES,
    VARIABLE_HEIGHT,

    VARIABLE_FIRST_NAME,
    VARIABLE_FAMILY_NAME_ONE,
    VARIABLE_FAMILY_NAME_TWO,

    VARIABLE_GOAL_TYPE,
    VARIABLE_GOAL_X,
    VARIABLE_GOAL_Y,

    VARIABLE_DRIVE_HUNGER,
    VARIABLE_DRIVE_SOCIAL,
    VARIABLE_DRIVE_FATIGUE,
    VARIABLE_DRIVE_SEX,

    VARIABLE_SPEAK,

    VARIABLE_BRAIN_X,
    VARIABLE_BRAIN_Y,

    VARIABLE_BRAIN_Z,
    VARIABLE_SELECT_BEING,

    VARIABLE_TEST_X,
    VARIABLE_TEST_Y,

    VARIABLE_BIOLOGY_OPERATOR,

    VARIABLE_POSTURE,

    VARIABLE_PREFERENCE_MATE_HEIGHT_MALE,
    VARIABLE_PREFERENCE_MATE_HEIGHT_FEMALE,
    VARIABLE_PREFERENCE_MATE_PIGMENTATION_MALE,
    VARIABLE_PREFERENCE_MATE_PIGMENTATION_FEMALE,
    VARIABLE_PREFERENCE_MATE_HAIR_MALE,
    VARIABLE_PREFERENCE_MATE_HAIR_FEMALE,
    VARIABLE_PREFERENCE_MATE_FRAME_MALE,
    VARIABLE_PREFERENCE_MATE_FRAME_FEMALE,
    VARIABLE_PREFERENCE_GROOM_MALE,
    VARIABLE_PREFERENCE_GROOM_FEMALE,
    VARIABLE_PREFERENCE_ANECDOTE_EVENT_MUTATION,
    VARIABLE_PREFERENCE_ANECDOTE_AFFECT_MUTATION,
    VARIABLE_PREFERENCE_CHAT,

    VARIABLE_ATTENTION_ACTOR_INDEX,
    VARIABLE_ATTENTION_EPISODE_INDEX,
    VARIABLE_ATTENTION_BODY_INDEX,

    VARIABLE_SHOUT_CONTENT,
    VARIABLE_SHOUT_HEARD,
    VARIABLE_SHOUT_CTR,
    VARIABLE_SHOUT_VOLUME,
    VARIABLE_SHOUT_FAMILY0,
    VARIABLE_SHOUT_FAMILY1,

    VARIABLE_SOCIAL_GRAPH_LOCATION_X,
    VARIABLE_SOCIAL_GRAPH_LOCATION_Y,
    VARIABLE_SOCIAL_GRAPH_TIME,
    VARIABLE_SOCIAL_GRAPH_DATE,
    VARIABLE_SOCIAL_GRAPH_CENTURY,
    VARIABLE_SOCIAL_GRAPH_ATTRACTION,
    VARIABLE_SOCIAL_GRAPH_FOF,
    VARIABLE_SOCIAL_GRAPH_FAMILIARITY,
    VARIABLE_MEMORY_FIRST_NAME,
    VARIABLE_MEMORY_FAMILY_NAME_ONE,
    VARIABLE_MEMORY_FAMILY_NAME_TWO,

    VARIABLE_MEMORY_LOCATION_X,
    VARIABLE_MEMORY_LOCATION_Y,
    VARIABLE_MEMORY_TIME,
    VARIABLE_MEMORY_DATE,
    VARIABLE_MEMORY_CENTURY,
    VARIABLE_MEMORY_FIRST_NAME0,
    VARIABLE_MEMORY_FAMILY_NAME_ONE0,
    VARIABLE_MEMORY_FAMILY_NAME_TWO0,
    VARIABLE_MEMORY_FIRST_NAME1,
    VARIABLE_MEMORY_FAMILY_NAME_ONE1,
    VARIABLE_MEMORY_FAMILY_NAME_TWO1,
    VARIABLE_MEMORY_EVENT,
    VARIABLE_MEMORY_AFFECT,

    VARIABLE_BEING /* This is a special case, it is the location where the main code starts */
};

enum
{
    RELATIONSHIP_SELF = 1,
    RELATIONSHIP_MOTHER,
    RELATIONSHIP_FATHER,
    RELATIONSHIP_DAUGHTER,
    RELATIONSHIP_SON,
    RELATIONSHIP_GRANDDAUGHTER,
    RELATIONSHIP_GRANDSON,
    RELATIONSHIP_SISTER,
    RELATIONSHIP_BROTHER,
    RELATIONSHIP_MATERNAL_GRANDMOTHER,
    RELATIONSHIP_MATERNAL_GRANDFATHER,
    RELATIONSHIP_PATERNAL_GRANDMOTHER,
    RELATIONSHIP_PATERNAL_GRANDFATHER,

    OTHER_MOTHER,
    OTHER_FATHER,
    OTHER_DAUGHTER,
    OTHER_SON,
    OTHER_GRANDDAUGHTER,
    OTHER_GRANDSON,
    OTHER_SISTER,
    OTHER_BROTHER,
    OTHER_MATERNAL_GRANDMOTHER,
    OTHER_MATERNAL_GRANDFATHER,
    OTHER_PATERNAL_GRANDMOTHER,
    OTHER_PATERNAL_GRANDFATHER,

    RELATIONSHIPS
};

enum
{
    BEING_STATE_ASLEEP = 0,
    BEING_STATE_AWAKE = 1,
    BEING_STATE_HUNGRY = 2,
    BEING_STATE_SWIMMING = 4,
    BEING_STATE_EATING = 8,
    BEING_STATE_MOVING = 16,
    BEING_STATE_SPEAKING = 32,
    BEING_STATE_SHOUTING = 64,
    BEING_STATE_REPRODUCING = 128,
    BEING_STATE_GROOMING = 256,
    BEING_STATE_SUCKLING = 512,
    BEING_STATE_SHOWFORCE = 1024,
    BEING_STATE_ATTACK = 2048,
    BEING_STATES = 13
};


enum PREFERENCES_MATE
{
    PREFERENCE_MATE_HEIGHT_MALE = 0,
    PREFERENCE_MATE_HEIGHT_FEMALE,
    PREFERENCE_MATE_PIGMENTATION_MALE,
    PREFERENCE_MATE_PIGMENTATION_FEMALE,
    PREFERENCE_MATE_HAIR_MALE,
    PREFERENCE_MATE_HAIR_FEMALE,
    PREFERENCE_MATE_FRAME_MALE,
    PREFERENCE_MATE_FRAME_FEMALE,
    PREFERENCE_GROOM_MALE,
    PREFERENCE_GROOM_FEMALE,
    PREFERENCE_ANECDOTE_EVENT_MUTATION,
    PREFERENCE_ANECDOTE_AFFECT_MUTATION,
    PREFERENCE_CHAT,
    PREFERENCE_SOCIAL,
    PREFERENCES
};

#define IMMUNE_FIT                            5
#define MIN_ANTIBODIES                        16
#define MIN_ANTIGENS                          8
#define IMMUNE_ANTIGENS                       8
#define IMMUNE_POPULATION                     16
#define PATHOGEN_TRANSMISSION_PROB            1000
#define PATHOGEN_ENVIRONMENT_PROB             100
#define PATHOGEN_MUTATION_PROB                100
#define ANTIBODY_DEPLETION_PROB               100
#define ANTIBODY_GENERATION_PROB(bei)         (being_energy(bei))

#define RANDOM_PATHOGEN(seed,pathogen_type)   (((seed&31)*8)+pathogen_type)
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
    PATHOGEN_TRANSMISSION_FOOD_SEAWEED
};

enum
{
    METABOLISM_STATE=0,
    METABOLISM_PROTEIN,
    METABOLISM_STARCH,
    METABOLISM_FAT,
    METABOLISM_SUGAR,
    METABOLISM_WATER,
    METABOLISM_BILE,
    METABOLISM_GLUCOSE,
    METABOLISM_MUSCLE,
    METABOLISM_AMINO_ACIDS,
    METABOLISM_GLUCOGEN,
    METABOLISM_ADRENALIN,
    METABOLISM_GLYCOGEN,
    METABOLISM_AMMONIA,
    METABOLISM_UREA,
    METABOLISM_LACTATE,
    METABOLISM_OXYGEN,
    METABOLISM_CO2,
    METABOLISM_FATTY_ACIDS,
    METABOLISM_TRIGLYCERIDE,
    METABOLISM_ADIPOSE,
    METABOLISM_INSULIN,
    METABOLISM_ADP,
    METABOLISM_ATP,
    METABOLISM_ENERGY,
    METABOLISM_HEAT,
    METABOLISM_PYRUVATE,
    METABOLISM_WASTE,
    METABOLISM_LEPTIN,
    METABOLISM_GHRELIN,
    METABOLISM_PROLACTIN,
    METABOLISM_MILK,
    METABOLISM_HEART_RATE,
    METABOLISM_BREATHING_RATE,
    METABOLISM_THERMOREGULATOR,
    METABOLISM_LUNG_CAPACITY,
    METABOLISM_SIZE
};

enum
{
    ORGAN_STOMACH=1,
    ORGAN_MUSCLES,
    ORGAN_LIVER,
    ORGAN_KIDNEYS,
    ORGAN_LUNGS,
    ORGAN_PANCREAS_A,
    ORGAN_PANCREAS_B,
    ORGAN_TISSUE,
    ORGANS
};

enum
{
    INTERVAL_MINS = 0,
    INTERVAL_HOURS,
    INTERVAL_DAYS,
    INTERVAL_MONTHS,
    INTERVAL_YEARS,
    INTERVALS
};



static const n_int interval_steps[] =
{ 1, TIME_HOUR_MINUTES, TIME_DAY_MINUTES, TIME_MONTH_MINUTES, TIME_YEAR_MINUTES};
static const n_constant_string interval_description[] = { "mins","hours","days","months","years" };


#define METABOLISM_HUNGER_THRESHOLD 1
#define METABOLISM_MAX_PRODUCTS     4
#define METABOLISM_MAX_REACTANTS    4
#define GLUCOSE_THRESHOLD_MAX       500
#define GLUCOSE_THRESHOLD_MIN       200
#define GLUCOSE_THRESHOLD_STARVE    100
#define ADRENALIN_MAX               1000

#define VASCULAR_SIZE               28
#define VASCULAR_SIZE_CORE          19
#define VASCULAR_CONSTRICTION_ZERO  127
#define BLOOD_VISCOSITY             3500000
#define BLOOD_DENSITY               105000
#define CORE_TEMPERATURE            37000

#define VASCULAR_PARASYMPATHETIC    -1
#define VASCULAR_SYMPATHETIC        1

#define RANK_STRESS_ALPHA           200
/* minimum and maximum heart rate in Hz x 1000 */
#define HEART_RATE_MAX(sim,bei)     ((1338333 - AGE_IN_DAYS(sim,bei))/365)
#define HEART_RATE_RESTING          1000

/* minimum and maximum breathing rate in Hz x 1000 */
#define BREATHING_RATE_MIN          167
#define BREATHING_RATE_MAX          300
#define CO2_PANT                    1400

#define INDICATORS_FREQUENCY                  60
#define INDICATORS_BUFFER_SIZE                24

/* nature in the range 0-15 from the genetics
   nurture in the range 0-255 from learned preferences.
   Resulting value is in the range 0-15 */
#define NATURE_NURTURE(nature,nurture)      (((nature) + ((nurture)>>4))>>1)

enum BODY_INVENTORY_TYPES
{
    BODY_HEAD = 0,

    BODY_TEETH,
    BODY_BACK,
    BODY_FRONT,

    BODY_LEFT_HAND,
    BODY_RIGHT_HAND,
    BODY_LEFT_FOOT,
    BODY_RIGHT_FOOT,

    INVENTORY_SIZE
};

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
    INVENTORY_FISH = 4096
};

#define VALUABLE_OBJECT    (INVENTORY_NUT & INVENTORY_SHELL)

enum posture_type
{
    POSTURE_CROUCHING = 100,
    POSTURE_UPRIGHT = 200
};

enum attention_type
{
    ATTENTION_ACTOR,
    ATTENTION_EPISODE,
    ATTENTION_BODY,
    ATTENTION_RELATIONSHIP,
    ATTENTION_TERRITORY,
    ATTENTION_SIZE
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

#define FISHING_PROB  (1<<8)

/* converts a distance in metres to a squared range value */
#define METRES_TO_APESPACE(m)   (m*m*80000)

/* shouting */
#define SHOUT_RANGE      METRES_TO_APESPACE(50)
#define SHOUT_REFRACTORY 10

enum shout_elements
{
    SHOUT_CONTENT = 0,
    SHOUT_HEARD,
    SHOUT_CTR,
    SHOUT_VOLUME,
    SHOUT_FAMILY0,
    SHOUT_FAMILY1,
    SHOUT_BYTES
};

/* threshold for the sex drive, beyond which the being seeks a preferred mate */
#define THRESHOLD_SEEK_MATE     100

/* During gestation sex drive will be decreased by this amount per cycle */
#define GESTATION_SEX_DRIVE_DECREMENT 16

/* Speed beyond which fatigue drive begins to increase */
#define FATIGUE_SPEED_THRESHOLD 8

/* determines how often anecdotes will be mutated */
#define ANECDOTE_EVENT_MUTATION_RATE   5000
#define ANECDOTE_AFFECT_MUTATION_RATE  5000

/* used in the social graph */
enum being_interaction_social
{
    BEING_MEETER = 0,  /* the first person, I/me */
    BEING_MET          /* the second person, You */
};

/* number of days after which social graph entries may be forgotten */
#define SOCIAL_FORGET_DAYS      10

/* Distance within which social communication can take place.
   Note that this is a squared value */
#define SOCIAL_RANGE            METRES_TO_APESPACE(10)

/* range for squabbling */
#define SQUABBLE_RANGE          METRES_TO_APESPACE(5)

/* Distance within which mating can take place */
#define MATING_RANGE            METRES_TO_APESPACE(2)

/* Tollerance within which social drive continues to increase */
#define SOCIAL_TOLLERANCE       0

/* Minimum expected neighbours within the social range */
#define MIN_CROWDING            1

/* Maximum expected neighbours within the social range.
   Beyond this behavioral sink occurs. */
#define MAX_CROWDING            3

/* maximum mass of the being in 10g increments */
#define BEING_MAX_MASS_G        7000

/* maximum body fat in 10g increments */
#define BEING_MAX_MASS_FAT_G    (BEING_MAX_MASS_G>>2)

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

/* calculate the mass of fat in 10g increments */
#define FAT_MASS(frame,energy) (MIN( ((BEING_MAX_MASS_FAT_G*energy*frame)>>(4+12)),BEING_MAX_MASS_FAT_G))

/* returns the amount of body fat in 10g increments */
#define GET_BODY_FAT(bei) (FAT_MASS(GET_FR(bei),being_energy(bei)))

/* maximum height of the being (in millimetres) */
#define BEING_MAX_HEIGHT_MM 2000

/* maximum height of the being (in arbitrary units) */
#define BEING_MAX_HEIGHT    65535

/* returns height in real units (mm) */
#define GET_BEING_HEIGHT(bei) (GET_H(bei)*BEING_MAX_HEIGHT_MM/BEING_MAX_HEIGHT)

/* Minimum lung capacity in cubic centimetres */
#define MIN_LUNG_CAPACITY(bei) (GET_BEING_HEIGHT(bei)*4000/BEING_MAX_HEIGHT_MM)

/* Maximum lung capacity in cubic centimetres */
#define MAX_LUNG_CAPACITY(bei) (GET_BEING_HEIGHT(bei)*6000/BEING_MAX_HEIGHT_MM)

/* the amount of growth for a given energy intake
   Note that if AGE_OF_MATURITY is changed then this may
   also need to be changed to have a similar adult growth */
#define ENERGY_TO_GROWTH(b,e)         ((e>>3)*3)

/* physical characteristics at birth */
#define BIRTH_HEIGHT        2000
#define BIRTH_MASS          100

/* is the given index within a social graph empty? */
#define SOCIAL_GRAPH_ENTRY_EMPTY(graph,index) ((graph[index].first_name[BEING_MET]==0) && (graph[index].family_name[BEING_MET]==0) && (graph[index].relationship<=RELATIONSHIP_SELF))

/* is there a known location for the given social graph entry? */
#define SOCIAL_GRAPH_ENTRY_LOCATION_EXISTS(graph,index) (graph[index].location[0] + graph[index].location[1] > 0)

/* is the given social graph entry a family member? */
#define IS_FAMILY_MEMBER(graph,index) ((graph[index].relationship > RELATIONSHIP_SELF) && (graph[index].relationship < OTHER_MOTHER))

enum EPISODIC_EVENTS
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
};

#define EVENT_INTENTION     (128)

/* this possible could be genetic */
#define THROW_ACCURACY      (1<<15)
#define WHACK_ACCURACY      (1<<15)

#define PAIR_BOND_THRESHOLD 2  /* minimum level of attraction for mating */

#define MAX_FEATURESET_SIZE 16  /* max size of a set of features associated with a social graph entry */
#define SOCIAL_SIZE         12  /* maximum size of the social network */
#define SOCIAL_SIZE_BEINGS  (SOCIAL_SIZE>>1) /* max number of specific beings within the social graph */
#define EPISODIC_SIZE       12  /* maximum number of episodic memories */

/* The maximum hit counter value for each feature within a set */
#define MAX_FEATURE_FREQUENCY 2048

/* the maximum number of matches of a stereotype */
#define MAX_FEATURESET_OBSERVATIONS 2048

/* ApeScript overrides */
#define OVERRIDE_GOAL		1

/* number of time steps after which a goal is abandoned */
#define GOAL_TIMEOUT        (60*24)

/* how close do we need to get to a goal to say that it has been reached? */
#define GOAL_RADIUS         40000

/* different types of goal */
enum goal_types
{
    GOAL_NONE           = 0,
    GOAL_LOCATION       = 1,
    GOAL_MATE           = 2
};

#define DRIVES_MAX          255  /* maximum value of each drive */

enum drives_definition
{
    DRIVE_HUNGER = 0,
    DRIVE_SOCIAL,
    DRIVE_FATIGUE,
    DRIVE_SEX,
    DRIVES   /* number of biological drives */
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

#ifdef PARASITES_ON

/* maximum number of parasites in the range 0-255 */
#define MAX_PARASITES(bei)   ((GENE_HAIR(being_genetics(bei))*255)>>4)

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

#endif

/* ape names */
#define NAMES_SURNAMES              64
#define NAMES_MALE                  256
#define NAMES_FEMALE                256

/* energy values for different foods */

enum FOOD_KINDS
{
    FOOD_VEGETABLE = 0,
    FOOD_FRUIT,
    FOOD_SHELLFISH,
    FOOD_SEAWEED,
    FOOD_TYPES
};

enum
{
    WATCH_NONE = 0,
    WATCH_ALL = 1,
    WATCH_SOCIAL_GRAPH,
    WATCH_EPISODIC,
    WATCH_BRAINCODE,
    WATCH_BRAINPROBES,
    WATCH_VASCULAR,
    WATCH_RESPIRATION,
    WATCH_METABOLISM,
    WATCH_APPEARANCE,
    WATCH_SPEECH,
    WATCH_STATES
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
    ENERGY_FISH                = 600
};

/* a maximum generation number to prevent overflows */
#define MAX_GENERATION              4294967294
enum GENERATION_NUM
{
    GENERATION_MATERNAL = 0,
    GENERATION_PATERNAL,
    GENERATION_FATHER,
    GENERATION_TOTAL
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

enum affect_type
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
};
/* ------- ------- ------- GENETICS FOR THE NOBLE APES (BETA) ------- ------- ------- */

typedef n_c_uint n_genetics;

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

#define GET_NUCLEOTIDE(gene,num)    ((gene[((num)>>3) & 3]>>((num&7)*2))&3)

/* returns a value for the gene in the range 0-15 */
#define GENE_VAL(gene, num0, num1) ((GET_NUCLEOTIDE(gene,num0) << 2) | GET_NUCLEOTIDE(gene,num1))

#define CHROMOSOMES                 4
#define CHROMOSOME_Y                0

/* probability of mutation */
#define MUTATION_CROSSOVER_PROB     500
#define MUTATION_DELETION_PROB      200
#define MUTATION_TRANSPOSE_PROB     200


/* ------- ------- -------  NOBLE APE GENETICS MATRIX  (BETA) ------- ------- ------- */

/* Genes which regulate the transcription network */
#define GENE_REGULATOR(gene,a,b)            (1+GENE_VAL(gene,(15+(a)),(15+(b))))

#define GENE_VAL_REG(gene,a,b,c,d)          GENE_VAL(gene,GENE_REGULATOR(gene,a,b),GENE_REGULATOR(gene,c,d))

/* Rate of growth */

/*#define GENE_RATE_OF_GROWTH(gene)           GENE_VAL_REG(gene, 11, 5, 13, 11) not used */

/* Returns a value for a gene corresponding to a body segment
   The number of body segments defined by the constant BONES
   is assumed to be < 18 */
#define GENE_HOX(gene,bodyseg)              GENE_VAL_REG(gene, 2+(bodyseg), 4+(bodyseg), 18-(bodyseg), 21-(bodyseg))

/* this may become redundant since body frame will
   be determined by other genes */
#define GENE_FRAME(gene)                    GENE_VAL_REG(gene, 10, 11, 1, 6)

/* hair length */

#define GENE_HAIR(gene)                     GENE_VAL_REG(gene, 12, 5, 12, 11)

/* Eye shape */

#define GENE_EYE_SHAPE(gene)                GENE_VAL_REG(gene, 9, 12, 1, 5)

/* Eye color */

#define GENE_EYE_COLOR(gene)                GENE_VAL_REG(gene, 9, 7, 3, 7)

/* Eye separation */

#define GENE_EYE_SEPARATION(gene)           GENE_VAL_REG(gene, 3, 2, 0, 14)

/* Nose shape */

#define GENE_NOSE_SHAPE(gene)               GENE_VAL_REG(gene, 4, 5, 6, 8)

/* Ear shape */

#define GENE_EAR_SHAPE(gene)                GENE_VAL_REG(gene, 12, 4, 14, 1)

/* Eyebrow shape */

#define GENE_EYEBROW_SHAPE(gene)            GENE_VAL_REG(gene, 9, 10, 8, 4)

/* Mouth shape */

#define GENE_MOUTH_SHAPE(gene)              GENE_VAL_REG(gene, 9, 5, 8, 15)

/* Pigmentation */

#define GENE_PIGMENTATION(gene)             GENE_VAL_REG(gene, 8, 9, 8, 3)

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

/* gene which controls the threshold beyond which glucose
   begins to be stored as glycogen or fat */
#define GENE_GLUCOSE_THRESHOLD_MAX(gene)    GENE_VAL_REG(gene, 6, 15, 19, 18)

/* gene which controls the threshold below which glycogen
   and fat begins to be converted to glucose */
#define GENE_GLUCOSE_THRESHOLD_MIN(gene)    GENE_VAL_REG(gene, 16, 14, 10, 7)

/* controls the rate at which glucogen is produced */
#define GENE_GLUCOGEN_RATE(gene)            GENE_VAL_REG(gene, 17, 11, 20, 2)

/* controls the rate at which insulin is produced */
#define GENE_INSULIN_RATE(gene)             GENE_VAL_REG(gene, 13, 7, 9, 11)

/* controls the rate at which adrenalin is produced */
#define GENE_ADRENALIN_RATE(gene)           GENE_VAL_REG(gene, 16, 19, 13, 3)

/* capacity of bladder */
#define GENE_BLADDER_CAPACITY(gene)         GENE_VAL_REG(gene, 5, 15, 2, 7)

#define GENE_WASTE_CAPACITY(gene)           GENE_VAL_REG(gene, 12, 11, 19, 1)

/* rate of suckling */
#define GENE_SUCKLING_RATE(gene)            GENE_VAL_REG(gene, 4, 14, 10, 15)

/* A social drive threshold value above which beings interact */
#define SOCIAL_THRESHOLD(bei)               ((NATURE_NURTURE(GENE_SOCIAL(being_genetics(bei)),bei->learned_preference[PREFERENCE_SOCIAL]))>>1)

/** used with social_meet function to specify whether
    the location should be included within the social graph entry */
enum being_meet_location_type {
    LOCATION_KNOWN = 0,
    LOCATION_UNKNOWN
};

/* Types of entities which it's possible to have a disposition towards */
enum
{
    ENTITY_BEING=0,
    ENTITY_BEING_GROUP,
    ENTITY_OBJECT,
    ENTITY_TERRITORY
};

enum featureset_members {
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

typedef struct
{
    n_byte2  id;
    n_byte2  no_of_features;
    n_byte   feature_type[MAX_FEATURESET_SIZE];
    n_byte2  feature_value[MAX_FEATURESET_SIZE];
    n_byte2  feature_frequency[MAX_FEATURESET_SIZE];
    n_byte2  observations;
} noble_featureset;

/*! @struct
 @discussion This describes a disposition towards other beings or things
 (an edge in the social graph)
 @field entity_type The type of entity encountered
 @field location Location where the entity was last seen
 @field time The time when the social graph entry was last updated
 @field date The date when the social graph entry was last updated
 @field first_name The first name of the entity
 @field family_name The family names of the entity
 @field attraction Attraction/desirability value used for mating
 @field friend_foe Whether friendly or hostile towards this entity
 @field belief Current belief about the state of this entity
 @field familiarity How many times has this entity been encountered
 @field relationship The type of relationship with the entity
 @field classification set of features which belong to this entity
 @field braincode the language machinery associated with this entity
 */
typedef struct
{
    n_byte   entity_type;

    n_byte2  location[2];
    n_byte2  time;
    n_byte2  date[2];

    n_byte2  first_name[2];
    n_byte2  family_name[2];

    n_byte   attraction;
    n_byte   friend_foe;
    n_byte2  belief;
    n_byte2  familiarity;

    n_byte   relationship;

    noble_featureset classification;

#ifdef BRAINCODE_ON
    n_byte   braincode[BRAINCODE_SIZE];
#endif
} social_link;


/*! @struct
 @discussion Describes an episodic memory event
 @field event Type of event
 @field affect Affect value associated with the event
 @field location Location of the event
 @field time Time when the event occurred
 @field date0 First component of the date when the event occurred
 @field date1 Second component of the date when the event occurred
 @field first_name First name of a being associated with the event
 @field food Food item associated with the event
 @field family_name Family name of a being associated with the event
 @field arg Additional argument
 */
typedef struct
{
    n_byte2  location[2];
    n_byte2  time;
    n_byte2  date[2];

    n_byte2  first_name[2];
    n_byte2  family_name[2];

    n_byte   event;
    n_byte   food;
    n_byte2  affect;
    n_byte2  arg;
} episodic_memory;


/*! @struct
 @field name A number representing the place name
 @field familiarity Number of times that this place has been visited
 @discussion This is the equivalent of a place cell, containing parameters for a particular place on the map
 */
typedef	struct
{
    n_byte name;
    n_byte2 familiarity;
}
noble_place;

#ifdef BRAINCODE_ON

/*! @struct
 @field type type of probe (0=read, 1=write)
 @field position position of the probe within the brain
 @field address address within the braincode
 @field frequency frequency of the pulse in time steps (minutes)
 @field offset resting value
 @field frequency current state of the probe
 @discussion A probe which can be attached to the brain to inject or detect signals.
 */
typedef	struct
{
    n_byte type;
    n_byte position;
    n_byte address;
    n_byte frequency;
    n_byte offset;
    n_byte state;
}
noble_brain_probe;

#endif

#ifdef METABOLISM_ON

/*! @struct
 @field parent array index of the parent
 @field constriction Constriction or dilation (mm 127=none)
 @field hardening Reduction in elasticity (Mpa x 1000)
 @field length length (cm x 1000)
 @field thickness thickness (cm x 1000)
 @field radius radius (cm x 1000)
 @field elasticity elasticity (Mpa x 1000)
 @field resistance resistance (Pa.s/ml x 1000)
 @field intertia inertia (Pa.s2/ml x 1000)
 @field compliance compliance (ml/Pa x 10)
 @field flow_rate Rate of flow (133416ml x 1000)
 @field volume volume (133416ml x 1000)
 @field pressure pressure (1mmHg x 1000)
 @field temperature temperature (C x 1000)
 @discussion Vessel object.
 */
typedef	struct
{
    n_byte2 parent;
    n_byte  constriction;
    n_byte  hardening;
    n_byte2 length;
    n_byte2 thickness;
    n_byte2 radius;
    n_byte2 elasticity;
    n_uint resistance;
    n_uint inertia;
    n_uint compliance;
    n_uint flow_rate;
    n_uint volume;
    n_uint pressure;
    n_uint temperature;
}
noble_vessel;

#endif

#ifdef IMMUNE_ON

/*! @struct
 @field antigens The number of antigens for each shape
 @field shape_antigen The shape of the antigen
 @field antbodies The number of antibodies for each shape
 @field shape_antibody The shape of the antibody
 @field strength immune system strength
 @discussion Immune system parameters
 */
typedef	struct
{
    n_byte antigens[IMMUNE_ANTIGENS];
    n_byte shape_antigen[IMMUNE_ANTIGENS];

    n_byte antibodies[IMMUNE_POPULATION];
    n_byte shape_antibody[IMMUNE_POPULATION];
}
noble_immune_system;

#endif


enum indicator_type
{
    IT_POPULATION = 0,
    IT_PARASITES,
    IT_POPULATION_DENSITY,
    IT_POPULATION_DENSITY_END = IT_POPULATION_DENSITY + (8*8),
    IT_AVERAGE_AGE_DAYS,
    IT_AVERAGE_MOBILITY,
    IT_AVERAGE_ENERGY,
    IT_AVERAGE_ENERGY_INPUT,
    IT_AVERAGE_ENERGY_OUTPUT,
    IT_AVERAGE_AMOROUSNESS,
    IT_AVERAGE_COHESION,
    IT_AVERAGE_FAMILIARITY,
    IT_AVERAGE_SOCIAL_LINKS,
    IT_AVERAGE_POSITIVE_AFFECT,
    IT_AVERAGE_NEGATIVE_AFFECT,
    IT_AVERAGE_ANTIGENS,
    IT_AVERAGE_ANTIBODIES,

    IT_AVERAGE_CHAT,
    IT_AVERAGE_SHOUTS,
    IT_AVERAGE_LISTENS,
    IT_AVERAGE_GROOMING,

    IT_AVERAGE_PARASITE_MOBILITY,
    IT_AVERAGE_BRAINPROBE_ACTIVITY,

    IT_DROWNINGS,
    IT_IDEOLOGY_SD,
    IT_GENETICS_SD,
    IT_FAMILY_NAME_SD,

    IT_DRIVES,
    IT_DRIVES_END = IT_DRIVES + DRIVES,

    IT_FOOD,
    IT_FOOD_END = IT_FOOD + FOOD_TYPES,

    IT_AVERAGE_FIRST_PERSON,
    IT_AVERAGE_INTENTIONS,
    IT_AVERAGE_SENSORS,
    IT_AVERAGE_ACTUATORS,
    IT_AVERAGE_OPERATORS,
    IT_AVERAGE_CONDITIONALS,
    IT_AVERAGE_DATA,

    IT_NUMBER_ENTRIES
};

typedef struct
{
    n_byte2	location[2];
    n_byte	direction_facing;
    n_byte	velocity;
    n_byte2	stored_energy;
    n_byte2	date_of_birth[2]; /* constant */
    n_byte2	speak;
    n_byte2 seed[2];
    n_byte2	state;

    /* The basic brain formula is;
     b(t+1) = a*l + b(t)*m + (b(t)-b(t-1))*n;
     the first three values are l, m, n for awake, then l, m, n for asleep */

    n_byte2	brain_state[6];  /* constant for now */

    /* height in arbitrary units
           can be converted to real units with GET_BEING_HEIGHT */
    n_byte2 height;

    /* body mass in kg */
    n_byte2 mass;

    /* flags indicating apescript overrides */
    n_byte2 script_overrides;

    /* shouting */
    n_byte  shout[SHOUT_BYTES];

    /* running average number of conspecifics in the vicinity */
    n_byte crowding;

    /* posture ranging from quadrupedal to bipedal */
    n_byte posture;

    /* inventory for various parts of the body */
    n_byte2 inventory[INVENTORY_SIZE];

#ifdef PARASITES_ON
    n_byte parasites;
#endif
    /* social status value */
    n_byte honor;

    n_byte2	date_of_conception[2]; /* constant */

    /* indexes to current focus of attention*/
    n_byte attention[ATTENTION_SIZE];

    n_genetics mother_genetics[CHROMOSOMES]; /* constant */

    n_byte2 unused1;                             /* constant */

    n_genetics father_genetics[CHROMOSOMES]; /* constant */

    n_byte  father_honor;                       /* constant */
    n_byte2 father_name[2];                     /* constant */

    n_genetics genes[CHROMOSOMES];       /* constant */

    n_byte2     social_x;
    n_byte2     social_y;
    n_byte2     social_nx;
    n_byte2     social_ny;

    n_byte drives[DRIVES];
    n_byte2 goal[4];
    n_byte learned_preference[PREFERENCES];
    /* around here the previous file system breaks */
    /* generation number from the mother and father */
    n_uint generation[GENERATION_TOTAL];            /* constant */
    /* temporary father generation value used during gestation */
    n_uint father_generation;                       /* seems to be unused */

#ifdef TERRITORY_ON
    noble_place territory[TERRITORY_DIMENSION*TERRITORY_DIMENSION];
#endif
#ifdef IMMUNE_ON
    noble_immune_system immune_system;
#endif
#ifdef METABOLISM_ON
    noble_vessel vessel[VASCULAR_SIZE];
    n_byte2 metabolism[METABOLISM_SIZE];
#endif
#ifdef BRAINCODE_ON
    n_byte braincode_register[BRAINCODE_PSPACE_REGISTERS];
    noble_brain_probe brainprobe[BRAINCODE_PROBES];
#endif
    n_byte          * brain;
    social_link     * social;
    episodic_memory * episodic;
}
noble_being;

typedef void (being_birth_event)(noble_being * born, noble_being * mother, void * sim);
typedef void (being_death_event)(noble_being * deceased, void * sim);


#define INDICATOR_ACCESS(sim, index)            sim->indicators_base[(sim->indicator_index * IT_NUMBER_ENTRIES) + (index)]

#define INDICATOR_SET(sim, index, val)          INDICATOR_ACCESS(sim, index) = (val)
#define INDICATOR_ADD(sim, index, val)          INDICATOR_ACCESS(sim, index) += (val)
#define INDICATOR_INC(sim, index)               INDICATOR_ACCESS(sim, index) ++

#define INDICATOR_DIVIDE(sim, index, value)     INDICATOR_ACCESS(sim, index) /= (value)
#define INDICATOR_NORMALIZE(sim, index)         INDICATOR_DIVIDE(sim, index, sim->num)
#define INDICATOR_MULTIPLY(sim, index, value)   INDICATOR_ACCESS(sim, index) *= value

#define INDICATOR_NAME(sim, index)              sim->indicators_name[(index)]
#define INDICATOR_INIT_NAME(sim, index, string) sim.indicators_name[(index)] = string


/*! @struct
 @field land The n_land pointer.
 @field weather The n_weather pointer.
 @field beings The noble_being pointer.
 @field num The number of beings currently active.
 @field max The maximum number of beings that could be active.
 @field select The current selected ape being shown through the GUI.
 @field someone_speaking Is a Noble Ape speaking?
 @discussion This is the highest level of collected Noble Ape Simulation stuff.
 It is primarily used by the GUI layer to address down into the Core layer.
 */
typedef struct
{
    n_land    * land;
    n_weather * weather;
    noble_being   * beings;


    n_uint	        num;
    n_uint	        max;

    n_uint          select;	 /* used by gui */
    n_byte          someone_speaking;

    n_uint          real_time;
    n_uint          last_time;
    n_uint          delta_cycles;
    n_uint          count_cycles;

    n_uint          indicator_index;
    n_uint          indicators_logging;

    n_uint          indicators_base[IT_NUMBER_ENTRIES * INDICATORS_BUFFER_SIZE];
    n_string        indicators_name[IT_NUMBER_ENTRIES];

    being_birth_event * ext_birth;
    being_death_event * ext_death;

    n_byte   *highres;
    n_c_uint *highres_tide;
}
noble_simulation;


/* macros defined to ease in the vectorised code */

#ifdef METABOLISM_ON
#define	GET_MT(bei,index) ((bei)->metabolism[index])
#endif
#define GET_A(bei,index) ((bei)->attention[index])
#define GET_H(bei)      ((bei)->height)
#define GET_M(bei)      ((bei)->mass)
#define GET_FR(bei)     (GENE_FRAME(being_genetics(bei)))

#define	GET_I(bei)	(being_genetics(bei)[CHROMOSOME_Y])
#define	GET_MI(bei)	((bei)->mother_genetics[0])

#define NO_BEINGS_FOUND	(0xffffffff)

#define SINGLE_BRAIN			(32768)
#define DOUBLE_BRAIN			(SINGLE_BRAIN*2)

#define	FIND_SEX(array)		(array&3)
#define	SEX_FEMALE			3

#define	BRAIN_OFFSET(num)	(num)

#define FAMILY_NAME_AND_MOD (63)

#define UNPACK_FAMILY_FIRST_NAME(packed_family_name) (packed_family_name&FAMILY_NAME_AND_MOD)
#define UNPACK_FAMILY_SECOND_NAME(packed_family_name) (packed_family_name>>6)
#define GET_NAME_FAMILY(f0,f1) (f0|(f1<<6))

n_int being_location_x(noble_being * value);
n_int being_location_y(noble_being * value);
n_int being_facing(noble_being * value);
n_int being_energy(noble_being * value);
n_int being_speed(noble_being * value);
n_genetics * being_genetics(noble_being * value);
n_int being_dob(noble_being * value);

typedef void (console_generic)(void *ptr, n_string ape_name, noble_being * local_being, n_string result);

typedef void (line_braincode)(n_string pointer, n_int line);

void console_populate_braincode(noble_simulation * local_sim, line_braincode function);

void *    sim_init(KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size);
void      sim_cycle(void);

/* This is the old way. Soon to be removed. */
n_int     sim_interpret(n_byte * buff, n_uint len);
n_int     sim_filein(n_byte * buff, n_uint len);
n_byte *  sim_fileout(n_uint * len);

/* This is the new way. Please continue forwards. */
n_file *  file_out(void);
n_int     file_in(n_file * input_file);
n_int     file_interpret(n_file * input_file);

void	  sim_close(void);
void	  sim_populations(n_uint	*total, n_uint * female, n_uint * male);

noble_simulation * sim_sim(void);

void sim_flood(void);

void sim_healthy_carrier(void);

void sim_realtime(n_uint time);

void sim_set_select(n_uint number);

void sim_debug_csv(n_file * fil, n_byte initial);

n_int get_time_interval(n_string str, n_int * number, n_int * interval);

void watch_ape(void * ptr, n_console_output output_function);

n_int console_speak(void * ptr, n_string response, n_console_output output_function);
n_int console_alphabet(void * ptr, n_string response, n_console_output output_function);

n_int console_stop(void * ptr, n_string response, n_console_output output_function);
n_int console_idea(void * ptr, n_string response, n_console_output output_function);
n_int console_being(void * ptr, n_string response, n_console_output output_function);
n_int console_braincode(void * ptr, n_string response, n_console_output output_function);
n_int console_speech(void * ptr, n_string response, n_console_output output_function);
n_int console_vascular(void * ptr, n_string response, n_console_output output_function);
n_int console_respiration(void * ptr, n_string response, n_console_output output_function);
n_int console_metabolism(void * ptr, n_string response, n_console_output output_function);
n_int console_social_graph(void * ptr, n_string response, n_console_output output_function);
n_int console_genome(void * ptr, n_string response, n_console_output output_function);
n_int console_appearance(void * ptr, n_string response, n_console_output output_function);
n_int console_stats(void * ptr, n_string response, n_console_output output_function);
n_int console_episodic(void * ptr, n_string response, n_console_output output_function);
n_int console_probes(void * ptr, n_string response, n_console_output output_function);
n_int console_watch(void * ptr, n_string response, n_console_output output_function);
n_int console_logging(void * ptr, n_string response, n_console_output output_function);
n_int console_list(void * ptr, n_string response, n_console_output output_function);
n_int console_simulation(void * ptr, n_string response, n_console_output output_function);
n_int console_step(void * ptr, n_string response, n_console_output output_function);
n_int console_run(void * ptr, n_string response, n_console_output output_function);
n_int console_interval(void * ptr, n_string response, n_console_output output_function);
n_int console_reset(void * ptr, n_string response, n_console_output output_function);
n_int console_top(void * ptr, n_string response, n_console_output output_function);
n_int console_epic(void * ptr, n_string response, n_console_output output_function);
n_int console_file(void * ptr, n_string response, n_console_output output_function);
n_int console_event(void * ptr, n_string response, n_console_output output_function);


n_int console_save(void * ptr, n_string response, n_console_output output_function);
n_int console_open(void * ptr, n_string response, n_console_output output_function);
n_int console_script(void * ptr, n_string response, n_console_output output_function);

n_int console_quit(void * ptr, n_string response, n_console_output output_function);

#ifndef	_WIN32

n_int sim_thread_console_quit(void);
void  sim_thread_console(void);

#endif

void sim_tide_block(n_byte * small_map, n_byte * map, n_c_uint * tide_block);

#ifdef THREADED
void sim_draw_thread_on(void);
void sim_draw_thread_start(void);
void sim_draw_thread_end(void);
#endif

#ifdef BRAIN_HASH

extern n_byte	brain_hash_out[12];
extern n_uint	brain_hash_count;

#endif


#ifdef CONSOLE_REQUIRED

const static noble_console_command control_commands[] =
{
    {&io_help,               "help",           "[(command)]",          "Displays a list of all the commands"},
#ifdef CONSOLE_ONLY
#ifdef COMMAND_LINE_EXPLICIT
    {&console_reset,         "reset",          "",                     "Reset the simulation"},
    {&console_reset,         "clear"           "",                     ""},

    {&console_open,          "open",           "[file]",               "Load a simulation file"},
    {&console_open,          "load",           "",                     ""},
#endif
    {&console_script,        "script",         "[file]",               "Load an ApeScript simulation file"},
    {&console_save,          "save",           "[file]",               "Save a simulation file"},

    {&console_quit,               "quit",           "",                     "Quits the console"},
    {&console_quit,               "exit",           "",                     ""},
    {&console_quit,               "close",          "",                     ""},
#else
    {&cle_load,              "load",           "[file]",               "Load a simulation file"},
    {&cle_script,            "script",         "[file]",               "Load apescript file"},
    {&cle_script,            "apescript",      "",                     ""},
    {&cle_video,             "video",          "ideosphere|genepool <filename>", "Create a video"},
    {&cle_video,             "movie",          "genespace|preferences<filename>","Create a video"},
    {&cle_video,             "film",           "relations <filename>",           "Create a video"},
    {&longterm_quit,         "quit",           "",                     "Quits the console"},
    {&longterm_quit,         "exit",           "",                     ""},
    {&longterm_quit,         "close",          "",                     ""},
    {&cle_reset,             "reset",          "",                     "Reset the simulation"},
    {&cle_reset,             "clear"           "",                     ""},
#endif

    {&console_stop,          "stop",           "",                     "Stop the simulation during step or run"},

    {&console_speak,         "speak",          "[file]",               "Create an AIFF file of Noble Ape speech"},
    {&console_alphabet,      "alpha",          "[file]",               "Create an AIFF file of Noble Ape alphabet"},
    {&console_file,          "file",           "[(component)]",        "Information on the file format"},
    {&console_run,           "run",            "(time format)|forever","Simulate for a given number of days or forever"},
    {&console_step,          "step",           "",                     "Run for a single logging interval"},
    {&console_top,           "top",            "",                     "List the top apes"},
    {&console_epic,          "epic",           "",                     "List the most talked about apes"},
    {&console_interval,      "interval",       "(days)",               "Set the simulation logging interval in days"},
    {&console_event,         "event",          "on|social|off",        "Episodic events (all) on, social on or all off"},
    {&console_logging,       "logging",        "on|off",               "Turn logging of images and data on or off"},
    {&console_logging,       "log",            "",                     ""},
    {&console_simulation,    "simulation",     "",                     ""},
    {&console_simulation,    "sim",            "",                     "Show simulation parameters"},
    {&console_watch,         "watch",          "(ape name)|all|off|*", "Watch (specific *) for the current ape"},
    {&console_watch,         "monitor",        "",                     ""},
    {&console_idea,          "idea",           "",                     "Track shared braincode between apes"},
    {&console_being,         "ape",            "",                     "Name of the currently watched ape"},
    {&console_being,         "pwd",            "",                     ""},
    {&console_social_graph,  "friends",        "",                     ""},
    {&console_social_graph,  "social",         "",                     ""},
    {&console_social_graph,  "socialgraph",    "",                     ""},
    {&console_social_graph,  "graph",          "(ape name)",           "* Show social graph for a named ape"},
    {&console_braincode,     "braincode",      "(ape name)",           "* Show braincode for a named ape"},
    {&console_speech,        "speech",         "(ape name)",           "* Show speech for a named ape"},
    {&console_vascular,      "vascular",       "(ape name)",           "* Show vascular system for a named ape"},
    {&console_respiration,   "respiration",    "(ape name)",           "* Show respiration system for a named ape"},
    {&console_respiration,   "breathing",      "",                     ""},
    {&console_respiration,   "breath",         "",                     ""},
    {&console_metabolism,    "metabolism",     "(ape name)",           "* Show metabolism for a named ape"},
    {&console_metabolism,    "chemistry",      "",                     ""},
    {&console_episodic,      "episodic",       "(ape name)",           "* Show episodic memory for a named ape"},
    {&console_probes,        "probes",         "(ape name)",           "* Show brain probes for a named ape"},
    {&console_stats,         "stats",          "(ape name)",           "* Show parameters for a named ape"},
    {&console_stats,         "status",         "",                     ""},
    {&console_appearance,    "appearance",     "(ape name)",           "* Show appearance values for a named ape"},
    {&console_appearance,    "physical",       "",                     ""},
    {&console_genome,        "genome",         "(ape name)",           "Show genome for a named ape"},
    {&console_genome,        "genetics",       "",                     ""},
    {&console_list,          "list",           "",                     "List all ape names"},
    {&console_list,          "ls",             "",                     ""},
    {&console_list,          "dir",            "",                     ""},

    {0L, 0L},
};

#endif

/*NOBLEMAKE END=""*/
/*NOBLEMAKE DEL=""*/


#endif /* NOBLEAPE_UNIVERSE_H */

/*NOBLEMAKE END=""*/

/*NOBLEMAKE SET="universe_internal.h"*/
/*NOBLEMAKE SET="land.c"*/
/*NOBLEMAKE SET="being.c"*/
/*NOBLEMAKE SET="social.c"*/
/*NOBLEMAKE SET="brain.c"*/
/*NOBLEMAKE SET="sim.c"*/



