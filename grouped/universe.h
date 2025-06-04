/****************************************************************

 universe.h

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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
#include "toolkit.h"
#include "script.h"
#include "sim.h"

#ifndef SIMULATEDAPE_UNIVERSE_H
#define SIMULATEDAPE_UNIVERSE_H

#define  EPISODIC_ON

#define  TERRITORY_ON  /* entity */
#define  BRAINCODE_ON  /* entity */
#define  IMMUNE_ON     /* entity */

#ifdef TARGET_OS_IOS
#undef  BRAIN_ON
#else
#define BRAIN_ON
#endif

#define  FEATURE_SET

#undef  DEBUG_LACK_OF_MOVEMENT

#define SINGLE_BRAIN            (32768)
#define DOUBLE_BRAIN            (SINGLE_BRAIN*2)

#define CHARACTER_WIDTH  (8)
#define CHARACTER_HEIGHT (14)


typedef enum
{
    fc_object_name,
    fc_n_spacetime,
    fc_n_byte,
    fc_n_byte2,
    fc_end
} file_contents;

typedef struct
{
    file_contents     contents;
    n_string          value;
    n_int             number;
} simulated_file_definition;

/*
 This table represents the operator calculation that is used to create the density
 graphs. The first section of the table shows if a component is added "+", subtracted
 "-", not used "." or boolean true "X". This is translated to numbers in the second
 part of the table. This is condensed into the operator number and at the far right
 what these operator sums represent.
 */

static const n_byte    operators[17][7] =
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

#define AGE_IN_DAYS(bei)        (land_date() - being_dob(bei))
#define AGE_IN_YEARS(bei)       (AGE_IN_DAYS(bei)/TIME_YEAR_DAYS)

/* in days, a little young, yes */
#define AGE_OF_MATURITY (30)

/*4*/
typedef enum
{
    VARIABLE_VECT_X = ( VARIABLE_IF + 1 ),
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
    VARIABLE_ENERGY,
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

    VARIABLE_ID_NUMBER,
    VARIABLE_DATE_OF_BIRTH,

    VARIABLE_IS_ERROR,

    VARIABLE_WEATHER,  /* Everything after this value can be both set and get */
    VARIABLE_BRAIN_VALUE, /* This is a special case, all the remainder are stored as variables */

    VARIABLE_VECT_ANGLE,
    VARIABLE_FACING,

    VARIABLE_SPEED,
    VARIABLE_ENERGY_DELTA,

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
    VARIABLE_MEMORY_FIRST_NAME0,
    VARIABLE_MEMORY_FAMILY_NAME_ONE0,
    VARIABLE_MEMORY_FAMILY_NAME_TWO0,
    VARIABLE_MEMORY_FIRST_NAME1,
    VARIABLE_MEMORY_FAMILY_NAME_ONE1,
    VARIABLE_MEMORY_FAMILY_NAME_TWO1,
    VARIABLE_MEMORY_EVENT,
    VARIABLE_MEMORY_AFFECT,

    VARIABLE_BEING /* This is a special case, it is the location where the main code starts */
} SECONDARY_APESCRIPT;

typedef enum
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
} BODY_INVENTORY_TYPES;

typedef enum
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
} RELATIONSHIP_TYPES;

typedef enum
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
} PREFERENCES_MATE;

typedef enum
{
    ATTENTION_EXTERNAL = 0,
    ATTENTION_ACTOR,
    ATTENTION_EPISODE,
    ATTENTION_BODY,
    ATTENTION_RELATIONSHIP,
    ATTENTION_TERRITORY,
    ATTENTION_SIZE
} attention_type;

typedef enum
{
    INTERVAL_MINS = 0,
    INTERVAL_HOURS,
    INTERVAL_DAYS,
    INTERVAL_MONTHS,
    INTERVAL_YEARS,
    INTERVALS
} TIME_INTERVALS;

static const n_int interval_steps[] =
{ 1, TIME_HOUR_MINUTES, TIME_DAY_MINUTES, TIME_MONTH_MINUTES, TIME_YEAR_MINUTES};
static const n_constant_string interval_description[] = { " mins", " hours", " days", " months", " years" };

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
#define MATING_RANGE            METRES_TO_APESPACE(5)

/* Tollerance within which social drive continues to increase */
#define SOCIAL_TOLLERANCE       1

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
#define GET_BODY_FAT(bei) (FAT_MASS(GENE_FRAME(being_genetics(bei)),being_energy(bei)))

/* maximum height of the being (in millimetres) */
#define BEING_MAX_HEIGHT_MM 2000

/* maximum height of the being (in arbitrary units) */
#define BEING_MAX_HEIGHT    65535

/* returns height in real units (mm) */
#define GET_BEING_HEIGHT(bei) (being_height(bei)*BEING_MAX_HEIGHT_MM/BEING_MAX_HEIGHT)

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

#define EVENT_INTENTION     (128)

/* this possible could be genetic */
#define THROW_ACCURACY      (1<<15)
#define WHACK_ACCURACY      (1<<15)

#define PAIR_BOND_THRESHOLD 2  /* minimum level of attraction for mating */

#ifdef FEATURE_SET

#define MAX_FEATURESET_SIZE 16  /* max size of a set of features associated with a social graph entry */

/* The maximum hit counter value for each feature within a set */
#define MAX_FEATURE_FREQUENCY 2048

/* the maximum number of matches of a stereotype */
#define MAX_FEATURESET_OBSERVATIONS 2048

#endif

#define SOCIAL_SIZE         12  /* maximum size of the social network */
#define SOCIAL_SIZE_BEINGS  (SOCIAL_SIZE>>1) /* max number of specific beings within the social graph */
#define EPISODIC_SIZE       12  /* maximum number of episodic memories */

/* ApeScript overrides */
#define OVERRIDE_GOAL        1

/* number of time steps after which a goal is abandoned */
#define GOAL_TIMEOUT        (60*24)

/* how close do we need to get to a goal to say that it has been reached? */
#define GOAL_RADIUS         40000

#define DRIVES_MAX          255  /* maximum value of each drive */

enum drives_definition
{
    DRIVE_HUNGER = 0,
    DRIVE_SOCIAL,
    DRIVE_FATIGUE,
    DRIVE_SEX,
    DRIVES   /* number of biological drives */
};

/** used with social_meet function to specify whether
    the location should be included within the social graph entry */
enum being_meet_location_type
{
    LOCATION_KNOWN = 0,
    LOCATION_UNKNOWN
};

/* Types of entities which it's possible to have a disposition towards */
enum
{
    ENTITY_BEING = 0,
    ENTITY_BEING_GROUP,
    ENTITY_OBJECT,
    ENTITY_TERRITORY
};

typedef struct
{
    n_byte2  value;
    n_byte2  frequency;
    n_byte   type;
} simulated_feature;

#ifdef FEATURE_SET

typedef struct
{
    n_byte2         feature_number;
    simulated_feature   features[MAX_FEATURESET_SIZE];
    n_byte2         observations;
} simulated_featureset;

#endif


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
    n_spacetime space_time;

    n_byte2  first_name[2];
    n_byte2  family_name[2];

    n_byte   attraction;
    n_byte   friend_foe;
    n_byte2  belief;
    n_byte2  familiarity;

    n_byte   relationship;

    n_byte   entity_type;

#ifdef FEATURE_SET
    simulated_featureset classification;
#endif

#ifdef BRAINCODE_ON
    n_byte   braincode[BRAINCODE_SIZE];
#endif
} simulated_isocial;


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
    n_spacetime space_time;
    n_byte2  first_name[2];
    n_byte2  family_name[2];

    n_byte   event;
    n_byte   food;
    n_byte2  affect;
    n_byte2  arg;
} simulated_iepisodic;


/*! @struct
 @field name A number representing the place name
 @field familiarity Number of times that this place has been visited
 @discussion This is the equivalent of a place cell, containing parameters for a particular place on the map
 */
typedef    struct
{
    n_byte  name;
    n_byte2 familiarity;
} simulated_iplace;

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
typedef    struct
{
    n_byte type;
    n_byte position;
    n_byte address;
    n_byte frequency;
    n_byte offset;
    n_byte state;
} simulated_ibrain_probe;

#endif

typedef n_byte4 n_genetics;


#define CHROMOSOMES                 4
#define CHROMOSOME_Y                0

#define IMMUNE_ANTIGENS             8
#define IMMUNE_POPULATION           16

enum
{
    WATCH_NONE = 0,
    WATCH_ALL = 1,
    WATCH_SOCIAL_GRAPH,
    WATCH_EPISODIC,
    WATCH_BRAINCODE,
    WATCH_BRAINPROBES,
    WATCH_APPEARANCE,
    WATCH_SPEECH,
    WATCH_PATHOGENS,
    WATCH_STATES
};

/* ------- ------- -------  SIMULATED APE GENETICS MATRIX  (BETA) ------- ------- ------- */

#define GET_NUCLEOTIDE(gene,num)    ((gene[((num)>>3) & 3]>>((num&7)*2))&3)

/* returns a value for the gene in the range 0-15 */
#define GENE_VAL(gene, num0, num1) ((GET_NUCLEOTIDE(gene,num0) << 2) | GET_NUCLEOTIDE(gene,num1))

/* Genes which regulate the transcription network */
#define GENE_REGULATOR(gene,a,b)            (1+GENE_VAL(gene,(15+(a)),(15+(b))))

#define GENE_VAL_REG(gene,a,b,c,d)          GENE_VAL(gene,GENE_REGULATOR(gene,a,b),GENE_REGULATOR(gene,c,d))
/* this may become redundant since body frame will
 be determined by other genes */
#define GENE_FRAME(gene)                    GENE_VAL_REG(gene, 10, 11, 1, 6)

/* hair length */

#define GENE_HAIR(gene)                     GENE_VAL_REG(gene, 12, 5, 12, 11)
/* Nose shape */

#define GENE_NOSE_SHAPE(gene)               GENE_VAL_REG(gene, 4, 5, 6, 8)

/* Mouth shape */

#define GENE_MOUTH_SHAPE(gene)              GENE_VAL_REG(gene, 9, 5, 8, 15)

/* Pigmentation */

#define GENE_PIGMENTATION(gene)             GENE_VAL_REG(gene, 8, 9, 8, 3)

/* Ear shape */

#define GENE_EAR_SHAPE(gene)                GENE_VAL_REG(gene, 12, 4, 14, 1)

/* Eyebrow shape */

#define GENE_EYEBROW_SHAPE(gene)            GENE_VAL_REG(gene, 9, 10, 8, 4)

/* Eye shape */

#define GENE_EYE_SHAPE(gene)                GENE_VAL_REG(gene, 9, 12, 1, 5)

/* Eye color */

#define GENE_EYE_COLOR(gene)                GENE_VAL_REG(gene, 9, 7, 3, 7)

/* Eye separation */

#define GENE_EYE_SEPARATION(gene)           GENE_VAL_REG(gene, 3, 2, 0, 14)



/* Rate of growth */

/*#define GENE_RATE_OF_GROWTH(gene)           GENE_VAL_REG(gene, 11, 5, 13, 11) not used */

/* Returns a value for a gene corresponding to a body segment
 The number of body segments defined by the constant BONES
 is assumed to be < 18 */
#define GENE_HOX(gene,bodyseg)              GENE_VAL_REG(gene, 2+(bodyseg), 4+(bodyseg), 18-(bodyseg), 21-(bodyseg))

/* Vision */

#define GENE_VISION_INITIAL(gene)           GENE_VAL_REG(gene, 2, 12, 3, 9)
#define GENE_VISION_DELTA(gene)             GENE_VAL_REG(gene, 11, 7, 2, 9)

/* Ear shape */

#define GENE_EAR_SHAPE(gene)                GENE_VAL_REG(gene, 12, 4, 14, 1)

/* Eyebrow shape */

#define GENE_EYEBROW_SHAPE(gene)            GENE_VAL_REG(gene, 9, 10, 8, 4)


#ifdef IMMUNE_ON

/*! @struct
 @field antigens The number of antigens for each shape
 @field shape_antigen The shape of the antigen
 @field antbodies The number of antibodies for each shape
 @field shape_antibody The shape of the antibody
 @field strength immune system strength
 @discussion Immune system parameters
 */
typedef    struct
{
    n_byte antigens[IMMUNE_ANTIGENS];
    n_byte shape_antigen[IMMUNE_ANTIGENS];

    n_byte antibodies[IMMUNE_POPULATION];
    n_byte shape_antibody[IMMUNE_POPULATION];

    n_byte2 random_seed[2];
} simulated_immune_system;

#endif

typedef struct
{
    n_byte2     location[2];
} simulated_idead_body;

#define NUMBER_OF_BODIES (10)

typedef struct
{
    simulated_idead_body bodies[NUMBER_OF_BODIES];
    n_byte2         count;
    n_byte2         location;
} simulated_remains;

typedef struct
{
    n_byte4     date_of_birth;
    n_byte2     generation_min;
    n_byte2     generation_max;
    n_byte2     name[2];
    n_genetics  genetics[CHROMOSOMES];           /* constant */
} simulated_being_constant;

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

enum
{
    METABOLISM_STATE = 0,
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
    ORGAN_STOMACH = 1,
    ORGAN_MUSCLES,
    ORGAN_LIVER,
    ORGAN_KIDNEYS,
    ORGAN_LUNGS,
    ORGAN_PANCREAS_A,
    ORGAN_PANCREAS_B,
    ORGAN_TISSUE,
    ORGANS
};


typedef enum
{
    FULLY_ASLEEP   =   0,
    SLIGHTLY_AWAKE =   1,
    FULLY_AWAKE    =   2
} sleep_state;





typedef struct
{
    n_byte2     location[2];
    n_byte      direction_facing;
    n_byte      velocity[10];
    n_byte2     stored_energy;
    n_byte2     random_seed[2];
    n_byte2     macro_state;
    n_byte      parasites;
    n_byte      honor;
    n_byte      crowding;
    n_byte2     height;
    n_byte2     mass;
    n_byte      posture;
    n_byte2     goal[4];

    n_byte2     social_coord_x;
    n_byte2     social_coord_y;
    n_byte2     social_coord_nx; /* why is this needed? */
    n_byte2     social_coord_ny; /* why is this needed? */

    n_byte /*sleep_state*/ awake;

#ifdef DEBUG_LACK_OF_MOVEMENT
    n_int       total_movement;
#endif
} simulated_being_delta;

#if 0
static simulated_file_definition simulated_being_events_json[]=
    {
        {type_object_name, "simulated_being_events", 1},
        {type_object_name, "social", 1},

        {type_n_spacetime, "space_time", 1},

        {type_n_byte2, "first_name", 2},
        {type_n_byte2, "family_name", 2},

        {type_n_byte, "attraction", 1},
        {type_n_byte, "friend_foe", 1},
        {type_n_byte2, "belief", 1},
        {type_n_byte2, "familiarity", 1},

        {type_n_byte, "relationship", 1},

        {type_n_byte, "entity_type", 1},
        {type_end, "", 1}

    #ifdef FEATURE_SET
        n_byte2         feature_number;
        simulated_feature   features[MAX_FEATURESET_SIZE];

        n_byte2  value;
        n_byte2  frequency;
        n_byte   type;


        n_byte2         observations;

        simulated_featureset classification;
    #endif

    #ifdef BRAINCODE_ON
        n_byte   braincode[BRAINCODE_SIZE];
    #endif

        {type_object_name, "episodic", 1},
        n_spacetime space_time;
        n_byte2  first_name[2];
        n_byte2  family_name[2];

        n_byte   event;
        n_byte   food;
        n_byte2  affect;
        n_byte2  arg;

      {type_end, "", 1}
    };
#endif

typedef struct
{
    simulated_isocial       social[SOCIAL_SIZE];
    simulated_iepisodic   episodic[EPISODIC_SIZE];
#ifdef TERRITORY_ON
    simulated_iplace territory[TERRITORY_DIMENSION * TERRITORY_DIMENSION];
#endif
} simulated_being_events;

typedef struct
{
    n_byte      drives[DRIVES];
    n_byte      shout[SHOUT_BYTES];
    n_byte2     inventory[INVENTORY_SIZE];
    n_byte      learned_preference[PREFERENCES];

    n_byte4     date_of_conception;
    n_genetics  fetal_genetics[CHROMOSOMES];     /* constant */

    n_byte2     father_name[2];
    n_byte2     mother_name[2];

    n_byte2     child_generation_min;
    n_byte2     child_generation_max;
} simulated_being_volatile;

typedef struct
{
#ifdef BRAINCODE_ON
    n_byte braincode_register[BRAINCODE_PSPACE_REGISTERS];
    simulated_ibrain_probe brainprobe[BRAINCODE_PROBES];
#endif
#ifdef BRAIN_ON
    n_byte            brain[DOUBLE_BRAIN];
#endif
    n_byte2     brain_state[6];
    n_byte2     script_overrides;
    n_byte      attention[ATTENTION_SIZE];
} simulated_being_brain;

typedef struct
{
    simulated_being_delta    delta;
    simulated_being_constant constant;
    simulated_being_events   events;
    simulated_being_brain    braindata;
    simulated_being_volatile changes;
#ifdef IMMUNE_ON
    simulated_immune_system immune_system;
#endif
} simulated_being;

typedef void ( being_birth_event )( simulated_being *born, simulated_being *mother, void *sim );
typedef void ( being_death_event )( simulated_being *deceased, void *sim );

#define LARGE_SIM 256

typedef struct
{
    simulated_being   beings[LARGE_SIM];
    simulated_remains remains;
    simulated_being    *select; /* used by gui */
    being_birth_event *ext_birth;
    being_death_event *ext_death;

    n_uint            num;
    n_uint            max;
} simulated_group;

typedef struct
{
    n_uint          real_time;
    n_uint          last_time;
    n_uint          delta_cycles;
    n_uint          count_cycles;

    n_uint          delta_frames;
    n_uint          count_frames;
} simulated_timing;

/* macros defined to ease in the vectorised code */


#define GET_M(bei)      ((bei)->delta.mass)

#define    GET_I(bei)    (being_genetics(bei)[CHROMOSOME_Y])

#define    FIND_SEX(array)        (array&3)
#define    SEX_FEMALE            3

#define    BRAIN_OFFSET(num)    (num)


typedef void ( loop_fn )( simulated_group *group, simulated_being *actual, void *data );
typedef void ( loop_no_sim_fn )( simulated_being *actual, void *data );
typedef void ( loop_no_sim_no_data_fn )( simulated_being *actual );

void loop_no_thread( simulated_group *group, simulated_being *being_not, loop_fn bf_func, void *data );
void loop_being_no_sim( simulated_being *beings, n_uint number_beings, loop_no_sim_fn bf_func, void *data );
void loop_being_no_sim_no_data( simulated_being *beings, n_uint number_beings, loop_no_sim_no_data_fn bf_func );

void loop_being( simulated_group *group, loop_fn bf_func, n_int beings_per_thread );

n_int being_location_x( simulated_being *value );
n_int being_location_y( simulated_being *value );
n_int being_energy( simulated_being *value );
n_genetics *being_genetics( simulated_being *value );
n_int being_dob( simulated_being *value );

typedef void ( console_generic )( void *ptr, n_string ape_name, simulated_being *local_being, n_string result );

typedef void ( line_braincode )( n_string pointer, n_int line );

#ifdef BRAINCODE_ON
void command_populate_braincode( simulated_group *group, line_braincode function );
#endif

n_file *death_record_file_ready( void );
void death_record_file_cleanup( void );
void console_capture_death( simulated_being *deceased, void *sim );


void     *sim_init( KIND_OF_USE kind, n_uint randomise, n_uint offscreen_size, n_uint landbuffer_size );
void      sim_cycle( void );
void      sim_update_output( void );

/* This is the new way. Please continue forwards. */
n_file   *tranfer_out( void );
n_file   *tranfer_out_json( void );
n_int     tranfer_in( n_file *input_file );

n_int     sim_interpret( n_file *input_file );
void      sim_close( void );

simulated_timing *sim_timing( void );
simulated_group   *sim_group( void );

n_uint sim_memory_allocated( n_int max );

void sim_flood( void );

void sim_healthy_carrier( void );

void sim_realtime( n_uint time );

void sim_set_select( simulated_being *select );

n_int sim_new_run_condition( void );

void sim_view_options( n_int px, n_int py );
void sim_rotate( n_int integer_rotation_256 );
void sim_move( n_int rel_vel, n_byte kind );
void sim_change_selected( n_byte forwards );

n_int sim_view_regular( n_int px, n_int py );


void sim_control_set( n_int px, n_int py, n_byte value, n_byte character );
void sim_control_erase( n_int size_x, n_int size_y, n_int max_characters );
void sim_control_regular( n_int px, n_int py );

void sim_terrain( n_int sx );


void sim_set_console_input( n_console_input *local_input_function );
void sim_set_console_output( n_console_output *local_output_function );

void sim_set_output( n_int value );
n_int sim_get_writing_output( void );
n_string sim_output_string( void );

n_int sim_new( void );

void transfer_debug_csv( n_file *fil, n_byte initial );

n_int get_time_interval( n_string str, n_int *number, n_int *interval );

void watch_ape( void *ptr, n_console_output output_function );
void watch_control( void *ptr, n_string beingname, simulated_being *local_being, n_string result );

n_int command_speak( void *ptr, n_string response, n_console_output output_function );
n_int command_alphabet( void *ptr, n_string response, n_console_output output_function );

void command_change_selected( simulated_group *group, n_byte forwards );

n_int command_stop( void *ptr, n_string response, n_console_output output_function );
n_int command_idea( void *ptr, n_string response, n_console_output output_function );
n_int command_being( void *ptr, n_string response, n_console_output output_function );
n_int command_braincode( void *ptr, n_string response, n_console_output output_function );
n_int command_speech( void *ptr, n_string response, n_console_output output_function );

n_int command_relationship_count( n_int friend_type );
simulated_being *command_relationship_being( simulated_group *group, n_int friend_type, n_int location );

n_int command_pathogen_graph( void *ptr, n_string response, n_console_output output_function );
n_int command_social_graph( void *ptr, n_string response, n_console_output output_function );
n_int command_genome( void *ptr, n_string response, n_console_output output_function );
n_int command_appearance( void *ptr, n_string response, n_console_output output_function );
n_int command_stats( void *ptr, n_string response, n_console_output output_function );
n_int command_episodic( void *ptr, n_string response, n_console_output output_function );
n_int command_probes( void *ptr, n_string response, n_console_output output_function );
n_int command_watch( void *ptr, n_string response, n_console_output output_function );
n_int command_logging( void *ptr, n_string response, n_console_output output_function );
n_int command_list( void *ptr, n_string response, n_console_output output_function );
n_int command_memory( void *ptr, n_string response, n_console_output output_function );
n_int command_next( void *ptr, n_string response, n_console_output output_function );
n_int command_previous( void *ptr, n_string response, n_console_output output_function );
n_int command_simulation( void *ptr, n_string response, n_console_output output_function );
n_int command_step( void *ptr, n_string response, n_console_output output_function );
n_int command_run( void *ptr, n_string response, n_console_output output_function );
n_int command_interval( void *ptr, n_string response, n_console_output output_function );
n_int command_reset( void *ptr, n_string response, n_console_output output_function );
n_int command_top( void *ptr, n_string response, n_console_output output_function );
n_int command_epic( void *ptr, n_string response, n_console_output output_function );
n_int command_file( void *ptr, n_string response, n_console_output output_function );
n_int command_event( void *ptr, n_string response, n_console_output output_function );
n_int command_epic( void *ptr, n_string response, n_console_output output_function );

n_int command_debug( void *ptr, n_string response, n_console_output output_function );

n_int console_death( void *ptr, n_string response, n_console_output output_function );

n_int command_save( void *ptr, n_string response, n_console_output output_function );
n_int command_open( void *ptr, n_string response, n_console_output output_function );
n_int command_script( void *ptr, n_string response, n_console_output output_function );

n_int command_quit( void *ptr, n_string response, n_console_output output_function );

#ifndef    _WIN32
n_int sim_thread_console_quit( void );
void  sim_thread_console( void );
#endif

void sim_console( n_string simulation_filename, n_uint randomise );

#ifdef CONSOLE_REQUIRED

const static simulated_console_command control_commands[] =
{
    {&io_help,               "help",           "[(command)]",          "Displays a list of all the commands"},
#ifdef COMMAND_LINE_EXPLICIT
    {&command_reset,         "reset",          "",                     "Reset the simulation"},
    {&command_reset,         "clear"           "",                     ""},

    {&command_open,          "open",           "[file]",               "Load a simulation file"},
    {&command_open,          "load",           "",                     ""},
#endif
    {&command_script,        "script",         "[file]",               "Load an ApeScript simulation file"},
    {&command_save,          "save",           "[file]",               "Save a simulation file"},

    {&command_quit,           "quit",           "",                     "Quits the console"},
    {&command_quit,           "exit",           "",                     ""},
    {&command_quit,           "close",          "",                     ""},

    {&command_stop,          "stop",           "",                     "Stop the simulation during step or run"},

    {&command_speak,         "speak",          "[file]",               "Create an AIFF file of Ape speech"},
    {&command_alphabet,      "alpha",          "[file]",               "Create an AIFF file of Ape alphabet"},
    {&command_file,          "file",           "[(component)]",        "Information on the file format"},
    {&command_run,           "run",            "(time format)|forever", "Simulate for a given number of days or forever"},
    {&command_step,          "step",           "",                     "Run for a single logging interval"},
    {&command_top,           "top",            "",                     "List the top apes"},
    {&command_epic,          "epic",           "",                     "List the most talked about apes"},
    {&command_interval,      "interval",       "(days)",               "Set the simulation logging interval in days"},
    {&command_event,         "event",          "on|social|off",        "Episodic events (all) on, social on or all off"},
    {&command_logging,       "logging",        "on|off",               "Turn logging of images and data on or off"},
    {&command_logging,       "log",            "",                     ""},
    {&command_simulation,    "simulation",     "",                     ""},
    {&command_simulation,    "sim",            "",                     "Show simulation parameters"},
    {&command_watch,         "watch",          "(ape name)|all|off|*", "Watch (specific *) for the current ape"},
    {&command_watch,         "monitor",        "",                     ""},
    {&command_idea,          "idea",           "",                     "Track shared braincode between apes"},
    {&command_being,         "ape",            "",                     "Name of the currently watched ape"},
    {&command_being,         "pwd",            "",                     ""},

    {&command_pathogen_graph, "pathogen",       "(ape name)",      "* Show pathogens for a named ape"},

    {&command_social_graph,  "friends",        "",                     ""},
    {&command_social_graph,  "social",         "",                     ""},
    {&command_social_graph,  "socialgraph",    "",                     ""},
    {&command_social_graph,  "graph",          "(ape name)",           "* Show social graph for a named ape"},
    {&command_braincode,     "braincode",      "(ape name)",           "* Show braincode for a named ape"},
    {&command_speech,        "speech",         "(ape name)",           "* Show speech for a named ape"},

    {&command_episodic,      "episodic",       "(ape name)",           "* Show episodic memory for a named ape"},
    {&command_probes,        "probes",         "(ape name)",           "* Show brain probes for a named ape"},
    {&command_stats,         "stats",          "(ape name)",           "* Show parameters for a named ape"},
    {&command_stats,         "status",         "",                     ""},
    {&command_appearance,    "appearance",     "(ape name)",           "* Show appearance values for a named ape"},
    {&command_appearance,    "physical",       "",                     ""},
    {&command_genome,        "genome",         "(ape name)",           "Show genome for a named ape"},
    {&command_genome,        "genetics",       "",                     ""},
    {&command_list,          "list",           "",                     "List all ape names"},
    {&command_list,          "ls",             "",                     ""},
    {&command_list,          "dir",            "",                     ""},

    {&command_next,          "next",           "",                     "Next ape"},

    {&command_previous,      "previous",       "",                     "Previous ape"},
    {&command_previous,      "prev",           "",                     ""},


    {&command_debug,         "debug",          "",                    "Run debug check"},

    {&command_memory,        "memory",         "",                    "Memory information for the simulation"},

    {0L, 0L},
};

#endif

#endif /* SIMULATEDAPE_UNIVERSE_H */
