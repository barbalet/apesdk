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

typedef struct shared_session shared_session;

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

enum
{
    SHARED_TUTORIAL_EDGE_MIN_X = 0,
    SHARED_TUTORIAL_EDGE_MAX_X,
    SHARED_TUTORIAL_EDGE_MIN_Y,
    SHARED_TUTORIAL_EDGE_MAX_Y
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
void shared_copy_rotate_180( n_byte4 *outputBuffer, const n_byte4 *source, n_int dim_x, n_int dim_y );
void shared_draw_ios( n_byte4 *outputBuffer, n_int dim_x, n_int dim_y );

shared_cycle_state shared_cycle( n_uint ticks, n_int localIdentification );
shared_cycle_state shared_cycle_ios( n_uint ticks );

n_byte *shared_legacy_draw( n_byte fIdentification, n_int dim_x, n_int dim_y );

n_int shared_new( n_uint seed );
n_int shared_new_agents( n_uint seed );

n_byte shared_openFileName( n_constant_string cStringFileName, n_int isScript );

void shared_saveFileName( n_constant_string cStringFileName );

shared_session *shared_session_create( n_uint random );
void shared_session_destroy( shared_session *session );
n_int shared_session_init( shared_session *session, n_int view, n_uint random );
n_int shared_session_simulation_started( const shared_session *session );
shared_cycle_state shared_session_cycle( shared_session *session, n_uint ticks, n_int localIdentification );
shared_cycle_state shared_session_cycle_ios( shared_session *session, n_uint ticks );
n_byte *shared_session_draw( shared_session *session, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed );
void shared_session_draw_ios( shared_session *session, n_byte4 *outputBuffer, n_int dim_x, n_int dim_y );
n_int shared_session_menu( shared_session *session, n_int menuValue );
n_int shared_session_new( shared_session *session, n_uint seed );
n_int shared_session_pause_state( shared_session *session );
n_int shared_session_selected_being_index( shared_session *session );
n_int shared_session_selected_being_facing( shared_session *session );
n_int shared_session_being_number( shared_session *session );
n_int shared_session_selected_being_location( shared_session *session, n_int *x, n_int *y );
void shared_session_rotate( shared_session *session, n_double num, n_int wwind );
n_int shared_session_input_is_active( shared_session *session );
void shared_session_mouseOption( shared_session *session, n_byte option );
void shared_session_mouseReceived( shared_session *session, n_double valX, n_double valY, n_int localIdentification );
void shared_session_mouseUp( shared_session *session );
n_byte shared_session_openFileName( shared_session *session, n_constant_string cStringFileName, n_int isScript );
void shared_session_saveFileName( shared_session *session, n_constant_string cStringFileName );

void shared_script_debug_handle( n_constant_string cStringFileName );

void shared_process(const char* fileUrl);

n_int shared_initial_tutorial_enabled( void );
n_int shared_initial_tutorial_count( void );
n_int shared_initial_tutorial_window( n_int step );
n_int shared_initial_tutorial_anchor_x( n_int step );
n_int shared_initial_tutorial_anchor_y( n_int step );
n_int shared_initial_tutorial_anchor_width( n_int step );
n_int shared_initial_tutorial_anchor_height( n_int step );
n_int shared_initial_tutorial_edge( n_int step );
n_constant_string shared_initial_tutorial_title( n_int step );
n_constant_string shared_initial_tutorial_text( n_int step );

n_int shared_being_number( void );
void shared_being_name( n_int number, n_string name );
void shared_being_select( n_int number);
n_int shared_selected_location( n_int *x, n_int *y );
n_int shared_selected_being_location( n_int *x, n_int *y );

#ifndef    _WIN32

n_int sim_thread_console_quit( void );
void sim_thread_console( void );

#endif

#endif /* SIMULATEDAPE_SHARED_H */
