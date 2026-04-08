/****************************************************************

 shared.h

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

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

 ****************************************************************/

#ifndef SIMULATEDAPE_SHARED_H
#define SIMULATEDAPE_SHARED_H

#include <stdint.h>

#ifndef _WIN32

#include "./toolkit/toolkit.h"

#else

#include ".\toolkit\toolkit.h"

#endif



typedef enum
{
    SHARED_CYCLE_OK = 0,
    SHARED_CYCLE_QUIT,
    SHARED_CYCLE_DEBUG_OUTPUT,
    SHARED_CYCLE_NEW_APES
} shared_cycle_state;

#define    NUM_VIEW    (0)
#define    NUM_TERRAIN (1)
#define    NUM_CONTROL (2)
#define    NUM_NIL     (3)

enum
{
    NA_MENU_PAUSE = 0,
    NA_MENU_WEATHER,
    NA_MENU_BRAIN,
    NA_MENU_BRAINCODE,
    NA_MENU_TIDEDAYLIGHT,
    NA_MENU_TERRITORY,
    NA_MENU_PREVIOUS_APE,
    NA_MENU_NEXT_APE,
    NA_MENU_CLEAR_ERRORS,
    NA_MENU_FLOOD,
    NA_MENU_HEALTHY_CARRIER,
    NA_MENU_FOLLOW,
    NA_MENU_SOCIAL_WEB
};

void shared_color_8_bit_to_48_bit( n_byte2 *fit );

void shared_dimensions( n_int *dimensions );

n_int shared_init( n_int view, n_uint random );

void shared_close( void );

n_int shared_simulation_started(void);

n_int shared_menu( n_int menuValue );

n_uint shared_max_fps( void );

void shared_rotate( n_double num, n_int wwind );
void shared_delta( n_double delta_x, n_double delta_y, n_int wwind );
void shared_zoom( n_double num, n_int wwind );

void shared_keyReceived( n_int value, n_int localIdentification );
void shared_keyUp( void );

void shared_mouseOption( n_byte option );
void shared_mouseReceived( n_double valX, n_double valY, n_int localIdentification );
void shared_mouseUp( void );
void shared_mouseReceived_ios( n_double valX, n_double valY );

void shared_about( void );

n_byte * shared_draw( n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed );
void shared_draw_ios( n_byte4 *outputBuffer, n_int dim_x, n_int dim_y );

shared_cycle_state shared_cycle( n_uint ticks, n_int localIdentification );
shared_cycle_state shared_cycle_ios( n_uint ticks );

n_byte *shared_legacy_draw( n_byte fIdentification, n_int dim_x, n_int dim_y );

n_int shared_new( n_uint seed );
n_int shared_new_agents( n_uint seed );

n_byte shared_openFileName( n_constant_string cStringFileName, n_int isScript );

void shared_saveFileName( n_constant_string cStringFileName );

void shared_script_debug_handle( n_constant_string cStringFileName );

void shared_process(const char* fileUrl);

n_int shared_being_number( void );
void shared_being_name( n_int number, n_string name );
void shared_being_select( n_int number);

typedef struct
{
    int32_t index;
    float   x;
    float   y;
    float   z;
    float   facing;
    float   energy;
    float   age_days;
    float   height;
    float   mass;
    uint8_t speaking;
    uint8_t female;
    uint8_t pregnant;
    uint8_t pigmentation;
    uint8_t hair;
    uint8_t frame;
    uint8_t eye_color;
    uint8_t eye_shape;
    uint16_t state;
    uint8_t goal_type;
    uint8_t honor;
    uint8_t drive_hunger;
    uint8_t drive_social;
    uint8_t drive_fatigue;
    uint8_t drive_sex;
} shared_immersiveape_being_snapshot;

typedef struct
{
    float   x;
    float   y;
    float   z;
    float   intensity;
    uint8_t food_type;
    uint8_t vegetation;
    uint8_t rain;
    uint8_t reserved;
} shared_immersiveape_food_snapshot;

typedef struct
{
    int32_t has_selection;
    int32_t selected_index;
    uint32_t date;
    uint32_t time;
    uint32_t world_seed;
    float   daylight;
    float   sun_angle;
    float   tide;
    float   water_level;
    uint8_t weather;
    uint8_t nearby_count;
    uint8_t food_count;
    uint8_t reserved0;
    shared_immersiveape_being_snapshot selected;
} shared_immersiveape_scene_snapshot;

int32_t shared_immersiveape_capture_scene(
    shared_immersiveape_scene_snapshot *scene,
    shared_immersiveape_being_snapshot *nearby,
    int32_t max_nearby,
    shared_immersiveape_food_snapshot *food,
    int32_t max_food );

void shared_immersiveape_fill_terrain_patch(
    int32_t center_x,
    int32_t center_y,
    int32_t half_extent,
    int32_t resolution,
    float *heights,
    uint8_t *materials,
    uint8_t *clouds,
    float *water_heights );

#ifndef    _WIN32

n_int sim_thread_console_quit( void );
void sim_thread_console( void );

#endif

#endif /* SIMULATEDAPE_SHARED_H */
