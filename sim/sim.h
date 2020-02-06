/****************************************************************

 sim.h

 =============================================================

 Copyright 1996-2020 Tom Barbalet. All rights reserved.

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

/*! \file   sim.h
 *  \brief  This is the interface between the ApeSDK toolkit and what consumes
    the ApeSDK toolkit.
 */

#ifndef _SIM_H_
#define _SIM_H_
/*	Variable Definitions */

#undef   SKELETON_RENDER

#ifndef    _WIN32
#define   ALPHA_WEATHER_DRAW
#undef   METAL_RENDER
#else
#undef   ALPHA_WEATHER_DRAW
#endif

#define FAST_START_UNREALISTIC_INITIAL_WEATHER

#ifdef AUTOMATED

#define FIXED_RANDOM_SIM 0x5261f726

#endif

/*! @define */
#define	SHORT_VERSION_NAME		 "Simulated Ape 0.699 "
#define	FULL_DATE				 __DATE__

/*! @define */
#define	VERSION_NUMBER		     699
#define	COPYRIGHT_DATE		     "Copyright 1996 - 2020 "

#define FULL_VERSION_COPYRIGHT "Copyright Tom Barbalet, 1996-2020."

/*! @define */
#define	SIMULATED_APE_SIGNATURE		    (('N'<< 8) | 'A')
#define	SIMULATED_WAR_SIGNATURE		(('N'<< 8) | 'W')

#define  COPYRIGHT_NAME		    "Tom Barbalet. "
#define  COPYRIGHT_FOLLOW		"All rights reserved."

/*! @struct
@field signature The program signature defined as SIMULATED_APE_SIGNATURE
through the Simulated Ape file handling etc.
@field version The version of the implementation that is the version
number defined as VERSION_NUMBER (thus changes between versions).
@discussion This is a means of tracking the programs and file versions
that are used primarily through the file interface but also through
other implementations over the Simulated Ape development. This includes
Simulated Warfare currently, but potentially could include other software
that accepts the Simulated Ape text based file format.
*/
typedef	struct
{
    n_byte2 signature;
    n_byte2 version;
} n_version;

typedef enum
{
    ET_SIMULATED_APE,
    ET_SIMULATED_APE_GHOST,
    ET_FIERCE_FELINE,
    ET_FIERCE_BIRD_OF_PREY,
}entity_type;



#define    NUM_VIEW    (0)
#define    NUM_TERRAIN (1)
#define    NUM_CONTROL (2)

#define WINDOW_PROCESSING NUM_TERRAIN

#define    DRAW_WINDOW_VIEW    (1)
#define    DRAW_WINDOW_TERRAIN (2)
#define    DRAW_WINDOW_CONTROL (4)

#define    CHECK_DRAW_WINDOW(num, kind) (num & kind)

/* maximum bytes in a braincode program */
#define BRAINCODE_SIZE                    128

/* number of probes which can be applied to the brain */
#define BRAINCODE_PROBES                  (BRAINCODE_SIZE>>3)

#define BRAINCODE_PSPACE_REGISTERS         3
/* maximum frequency of a brain probe */
#define BRAINCODE_MAX_FREQUENCY            16

/* number of bytes per instruction */
#define BRAINCODE_BYTES_PER_INSTRUCTION    3

/* number of instructions which a MVB copies */
#define BRAINCODE_BLOCK_COPY               16

#define BRAINCODE_MAX_ADDRESS              (BRAINCODE_SIZE*2)
#define BRAINCODE_ADDRESS(i)               ((i) % BRAINCODE_MAX_ADDRESS)

enum braincode_locations
{
    BRAINCODE_EXTERNAL = 0,
    BRAINCODE_INTERNAL
};

/* instruction codes */

enum BRAINCODE_COMMANDS
{
    /* data */
    BRAINCODE_DAT0 = 0,
    BRAINCODE_DAT1,

    /* operators */
    BRAINCODE_ADD,
    BRAINCODE_SUB,
    BRAINCODE_MUL,
    BRAINCODE_DIV,
    BRAINCODE_MOD,
    BRAINCODE_MVB,
    BRAINCODE_MOV,
    BRAINCODE_JMP,
    BRAINCODE_CTR,
    BRAINCODE_SWP,
    BRAINCODE_INV,
    BRAINCODE_STP,
    BRAINCODE_LTP,

    /* conditionals */
    BRAINCODE_JMZ,
    BRAINCODE_JMN,
    BRAINCODE_DJN,
    BRAINCODE_AND,
    BRAINCODE_OR,
    BRAINCODE_SEQ,
    BRAINCODE_SNE,
    BRAINCODE_SLT,

    /* sensors */
    BRAINCODE_SEN,
    BRAINCODE_SEN2,
    BRAINCODE_SEN3,

    /* actuators */
    BRAINCODE_ACT,
    BRAINCODE_ACT2,
    BRAINCODE_ACT3,
    BRAINCODE_ANE,

    BRAINCODE_INSTRUCTIONS
};

typedef enum
{
    WEATHER_SEVEN_ERROR = -1,
    WEATHER_SEVEN_SUNNY_DAY = 0,
    WEATHER_SEVEN_CLOUDY_DAY = 1,
    WEATHER_SEVEN_RAINY_DAY = 2,
    WEATHER_SEVEN_CLEAR_NIGHT = 3,
    WEATHER_SEVEN_CLOUDY_NIGHT = 4,
    WEATHER_SEVEN_RAINY_NIGHT = 5,
    WEATHER_SEVEN_DAWN_DUSK = 6
} weather_values;

enum window_information
{
    TERRAIN_WINDOW_WIDTH		= (4096),
    TERRAIN_WINDOW_HEIGHT       = (3072),
    TERRAIN_WINDOW_AREA			= (TERRAIN_WINDOW_WIDTH * TERRAIN_WINDOW_HEIGHT),
    CONTROL_WINDOW_WIDTH        = (2048),
    CONTROL_WINDOW_HEIGHT       = (2048),
    CONTROL_WINDOW_AREA         = (CONTROL_WINDOW_WIDTH * CONTROL_WINDOW_HEIGHT)
};

typedef enum
{
    KIND_PRE_STARTUP = -2,
    KIND_NOTHING_TO_RUN = -1,
    KIND_LOAD_FILE = 0,
    KIND_NEW_SIMULATION,
    KIND_NEW_APES,
    KIND_START_UP,
    KIND_MEMORY_SETUP,
} KIND_OF_USE;

extern n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number);

void compress_buffer(n_byte * input, n_byte * output, n_int n, n_int compressed);
void compress_buffer_run(n_byte * input, n_byte * output, n_int n, n_int compressed, n_int number);

void compress_brain_compressed(n_byte * brain);
void compress_brain_expand(n_byte * brain);

enum color_type
{
    COLOR_BLACK     =   (0),
    COLOR_WHITE     =   (252),
    COLOR_BLUE      =   (253),
    COLOR_RED_DARK  =   (254),
    COLOR_RED       =   (255)
};

#define    NON_INTERPOLATED COLOR_WHITE

#ifdef SIMULATED_PLANET

#define MAP_BITS                      (8)
#define MAP_TITLES                    (6)

#else

#define MAP_BITS                      (9)
#define MAP_TITLES                    (1)

#endif

/*
 750    1624    874
 828    1472    644
 750    1334    584
 640    1136    496
 */


#define MAP_DIMENSION                 (1<<(MAP_BITS))
#define MAP_AREA                      (1<<(2*MAP_BITS))

#define APE_TO_MAP_BIT_RATIO          (6)
#define MAP_TO_TERRITORY_RATIO        (5)

#define APESPACE_TO_MAPSPACE(num)     ((num)>>APE_TO_MAP_BIT_RATIO)
#define MAPSPACE_TO_APESPACE(num)     ((num)<<APE_TO_MAP_BIT_RATIO)

#define MAPSPACE_TO_TERRITORY(num)    ((num)>>MAP_TO_TERRITORY_RATIO)

#define TERRITORY_DIMENSION           MAPSPACE_TO_TERRITORY(MAP_DIMENSION)
#define TERRITORY_AREA                (TERRITORY_DIMENSION*TERRITORY_DIMENSION)

/* dimension of the territory map */

#define APESPACE_TO_TERRITORY(num)    (APESPACE_TO_MAPSPACE(num)>>MAP_TO_TERRITORY_RATIO)


#define HI_RES_MAP_BITS               (MAP_BITS+3)

#define HI_RES_MAP_DIMENSION          (1<<(HI_RES_MAP_BITS))
#define HI_RES_MAP_AREA               (1<<(2*HI_RES_MAP_BITS))

#define MAP_APE_RESOLUTION_SIZE       (MAPSPACE_TO_APESPACE(MAP_DIMENSION))
#define	APESPACE_BOUNDS               ((MAP_APE_RESOLUTION_SIZE)-1)

#define	APESPACE_CONFINED(num)        (n_byte2)((num)>APESPACE_BOUNDS ? APESPACE_BOUNDS : ((num)<0 ? 0 : (num)))

#define LAND_TILE_EDGE                 (256)
#define	POSITIVE_LAND_COORD(num)       ((num+(3*MAP_DIMENSION))&(MAP_DIMENSION-1))
#define	POSITIVE_LAND_COORD_HIRES(num) ((num+(3*HI_RES_MAP_DIMENSION))&(HI_RES_MAP_DIMENSION-1))
#define NUMBER_LAND_TILES              (MAP_DIMENSION/LAND_TILE_EDGE)
#define WEATHER_TO_MAPSPACE(num)       ((num))
#define MAPSPACE_TO_WEATHER(num)       ((num))

#define LAND_DITHER(x,y,z)             (((x+y+z)&15)-(((x&y)|z)&7)-((x|(y&z))&7))

#define	OFFSCREENSIZE                  (MAP_AREA + TERRAIN_WINDOW_AREA + CONTROL_WINDOW_AREA)

#define WEATHER_CLOUD       (32768>>4)

#define	WEATHER_RAIN		(WEATHER_CLOUD * 3)

#define TIME_HOUR_MINUTES           (60)
#define TIME_DAY_MINUTES            (TIME_HOUR_MINUTES * 24)
#define TIME_MONTH_MINUTES          (TIME_DAY_MINUTES * 28)
#define TIME_YEAR_MINUTES           (TIME_MONTH_MINUTES * 13)
#define TIME_YEAR_DAYS              (7 * 52)				/*364 also = 13 * 28 */
#define	TIME_CENTURY_DAYS           (TIME_YEAR_DAYS * 100)

#define LUNAR_ORBIT_MINS            39312

#define	WATER_TEST(pz,w)            ((pz)<(w))

#define	WATER_MAP		      128

#define TIDE_AMPLITUDE_LUNAR  8
#define TIDE_AMPLITUDE_SOLAR  2

#define TIDE_MAX              (WATER_MAP + TIDE_AMPLITUDE_LUNAR + TIDE_AMPLITUDE_SOLAR)

/* Night/Day definitions */
#define IS_NIGHT(num)		((((num)>>5) <  (11))||(((num)>>5) >  (36)))
#define	IS_DAWNDUSK(num)	((((num)>>5) == (11))||(((num)>>5) == (36)))

#define NIGHT_END_POINT     (256)
#define DAWN_END_POINT      (384)
#define DAY_END_POINT       (1152)
#define DUSK_END_POINT      (1184)

#define MAX_MODIFIED_TIME   (238)

#define NIGHT_TIME_DIVISION(time)     ((time)>>4)
#define DAWN_DUSK_TIME_DIVISION(time) ((time)>>3)
#define DAY_TIME_DIVISION(time)       ((time)>>2)

/*
 Night 1184  - 1439
 Night 0 - 351
 Dawn  352 - 383
 Day   384 - 1151
 Dusk  1152 - 1183
 */

void  weather_init(void);
void  weather_wind_vector(n_vect2 * pos, n_vect2 * wind);
n_int weather_pressure(n_int px, n_int py);
void  weather_cycle(void);
void  weather_wind(void);
weather_values weather_seven_values(n_int px, n_int py);

void land_seed_genetics(n_byte2 * local_random);

void land_init(void);
void land_init_high_def(n_byte double_spread);

void  land_clear(KIND_OF_USE kind, n_byte4 start);
void  land_cycle(void);
void  land_vect2(n_vect2 * output, n_int * actual_z, n_vect2 * location);
n_int land_operator_interpolated(n_int locx, n_int locy, n_byte * kind);

n_int land_map_dimension(void);
n_int land_map_bits(void);

void  land_tide(void);
n_int land_location(n_int px, n_int py);
n_byte * land_location_tile(n_int tile);
n_int land_location_vect(n_vect2 * value);

typedef struct
{
    n_byte2     genetics[2];                           /* save-able */
    
    n_byte      topography[2][MAP_AREA];                    /* generated */
    n_c_int     atmosphere[2][ MAP_AREA];                 /* save-able and generate-able */
    n_byte2     delta_pressure[ MAP_AREA];             /* generated */
    
    n_byte2     delta_pressure_highest;
    n_byte2     delta_pressure_lowest;
    
    n_c_int     atmosphere_highest;
    n_c_int     atmosphere_lowest;
    n_int       local_delta;
} n_tile;

typedef struct
{
#ifdef SIMULATED_PLANET
    n_tile tiles[6];
#else
    n_tile tiles[1];
#endif
    n_byte2     genetics[2];                           /* save-able */

    n_int      wind_value_x; /* 6 to 96 */
    n_int      wind_value_y; /* 6 to 96 */
    n_int      wind_aim_x;  /* 6 to 96 */
    n_int      wind_aim_y;  /* 6 to 96 */
    
    n_int      wind_dissipation;
    
    n_byte4     date;                                  /* save-able */
    n_byte2     time;                                  /* save-able */
    
    n_byte      tide_level;                            /* generated */
    
    n_byte      topography_highdef[HI_RES_MAP_AREA * 2]; /* generated */
    n_byte4     highres_tide[HI_RES_MAP_AREA/32];      /* generated */
} n_land;

typedef struct
{
    n_int  x, y;
    n_int  tile;
    n_uint facing;
} n_tile_coordinates;

void tile_wind(n_land * land);
void tile_cycle(n_land * land);

void tile_weather_init(n_land * land);

void tile_land_init(n_land * land);
void tile_land_erase(n_land * land);
void tile_land_random(n_land * land, n_byte2 * random);


void tile_creation(n_byte * map, n_byte2 * random);

n_byte tiles_topography(n_land * land, n_int tile, n_int buffer, n_int lx, n_int ly);
n_byte * tiles_topography_map(n_land * land, n_int tile, n_int buffer);

n_c_int tiles_atmosphere(n_land * land, n_int tile, n_int buffer, n_int lx, n_int ly);
void tile_resolve_coordinates(n_tile_coordinates * coordinates);

void land_color_init(void);
void land_color_time(n_byte2 * color_fit, n_int toggle_tidedaylight);
void land_color_time_8bit(n_byte * color_fit, n_int toggle_tidedaylight);

n_land * land_ptr(void);
n_byte4 land_date(void);
n_byte4 land_time(void);
n_byte2 * land_genetics(void);
n_byte land_tide_level(void);
n_byte * land_topography(void);
n_byte * land_topography_highdef(void);
n_byte4 * land_highres_tide(void);
n_c_int * land_weather(n_int tile);

#define BASH_COLOR_DEFAULT "\033[0m"

#define BASH_COLOR_LIGHT_GREEN "\033[92m"
#define BASH_COLOR_LIGHT_YELLOW "\033[93m"
#define BASH_COLOR_LIGHT_RED "\033[91m"

#define BASH_COLOR_LIGHT_GREY "\033[37m"
#define BASH_COLOR_DARK_GREY "\033[90m"

n_int spacetime_after(n_spacetime * initial, n_spacetime * second);
void  spacetime_copy(n_spacetime * to, n_spacetime * from);
n_int spacetime_before_now(n_spacetime * initial);
void  spacetime_set(n_spacetime * set, n_byte2 * location);

void land_convert_to_map(n_vect2 * value);

#endif /* _SIM_H_ */

