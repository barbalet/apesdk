/****************************************************************

 shared.c

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

/* This is intended to dramatically simplify the interface between
     the various platform versions of Simulated Ape */

#include <stdio.h>

#include "app_shell.h"
#include "buffer.h"
#include "gui.h"

#ifndef    _WIN32

#include "../shared.h"

#else

#include "..\shared.h"

#endif

static n_int   simulation_started = 0;
static n_int   shared_session_initializing = 0;
static n_int   shared_session_started_count = 0;
static n_int   shared_default_uses_core = 0;
static n_int   shared_core_memory_ready = 0;

static n_int   mouse_x, mouse_y;
static n_byte  mouse_option, mouse_identification;
static n_byte  mouse_down;

static n_byte  mouse_drag;
static n_int   mouse_drag_x, mouse_drag_y;

static n_byte  key_identification;
static n_byte2 key_value;
static n_byte  key_down;

static ape_buffer outputBuffer = { 0L, 0L, 0 };

static n_int    ios_control_x = 0;
static n_int    ios_control_y = 0;

static n_int    ios_view_x = 0;
static n_int    ios_view_y = 0;

static n_int    ios_dimension_x = 0;
static n_int    ios_dimension_y = 0;

#define	Y_DELTA	36
#define	X_DELTA	20

static n_int toggle_pause = 0;

typedef struct
{
    n_file *snapshot;
    n_int simulation_started;
    n_int selected_index;
    n_int toggle_pause;
    draw_toggle_state draw_state;
} shared_core_context;

struct shared_session
{
    ape_app_shell shell;
    n_uint initial_random;
    shared_core_context core;
};

static shared_core_context shared_default_core;
static n_byte shared_default_core_ready = 0;
static shared_core_context *shared_active_core = 0L;

static void shared_core_context_release_snapshot( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    io_file_free( &context->snapshot );
}

static void shared_core_context_reset_values( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    context->simulation_started = 0;
    context->selected_index = -1;
    context->toggle_pause = 0;
    draw_toggle_state_default( &context->draw_state );
}

static void shared_default_core_ensure( void )
{
    if ( shared_default_core_ready )
    {
        return;
    }

    memory_erase( ( n_byte * )&shared_default_core, sizeof( shared_default_core ) );
    shared_core_context_reset_values( &shared_default_core );
    shared_default_core_ready = 1;
}

static n_int shared_current_selected_index( void )
{
    simulated_group *group = sim_group();

    if ( ( group == 0L ) || ( group->select == 0L ) )
    {
        return -1;
    }

    return being_index( group, group->select );
}

static void shared_select_index( n_int selected_index )
{
    simulated_group *group = sim_group();

    if ( ( group == 0L ) || ( selected_index < 0 ) || ( selected_index >= ( n_int )group->num ) )
    {
        return;
    }

    sim_set_select( &( group->beings[selected_index] ) );
}

static void shared_core_context_sync_from_globals( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    context->simulation_started = simulation_started;
    context->toggle_pause = toggle_pause;
    context->selected_index = shared_current_selected_index();
    draw_toggle_state_get( &context->draw_state );
}

static void shared_core_context_apply_globals( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    simulation_started = context->simulation_started;
    toggle_pause = context->toggle_pause;
    draw_toggle_state_set( &context->draw_state );
}

static void shared_core_context_capture( shared_core_context *context )
{
    n_file *snapshot;

    if ( context == 0L )
    {
        return;
    }

    shared_core_context_sync_from_globals( context );
    if ( context->simulation_started == 0 )
    {
        return;
    }

    if ( sim_new() )
    {
        sim_cycle();
        sim_update_output();
        context->selected_index = shared_current_selected_index();
    }

    snapshot = tranfer_out();
    if ( snapshot != 0L )
    {
        shared_core_context_release_snapshot( context );
        context->snapshot = snapshot;
    }
}

static n_int shared_core_context_restore( shared_core_context *context )
{
    if ( context == 0L )
    {
        return 0;
    }

    if ( context->simulation_started )
    {
        n_file *snapshot = 0L;

        if ( context->snapshot == 0L )
        {
            return 0;
        }

        snapshot = io_file_duplicate( context->snapshot );
        if ( snapshot == 0L )
        {
            return 0;
        }

        if ( sim_state_restore( snapshot, context->selected_index ) != 0 )
        {
            io_file_free( &snapshot );
            return 0;
        }
        io_file_free( &snapshot );
        shared_core_context_apply_globals( context );
        shared_select_index( context->selected_index );
        return 1;
    }

    shared_core_context_apply_globals( context );
    return 1;
}

static n_int shared_core_activate( shared_core_context *context )
{
    shared_default_core_ensure();

    if ( context == 0L )
    {
        return 0;
    }

    if ( shared_active_core == context )
    {
        return 1;
    }

    if ( shared_active_core != 0L )
    {
        shared_core_context_capture( shared_active_core );
    }

    if ( context->simulation_started )
    {
        if ( shared_core_context_restore( context ) == 0 )
        {
            return 0;
        }
    }
    else
    {
        shared_core_context_apply_globals( context );
    }

    shared_active_core = context;
    return 1;
}

static void shared_core_context_became_current( shared_core_context *context )
{
    if ( context == 0L )
    {
        return;
    }

    shared_core_context_release_snapshot( context );
    shared_active_core = context;
    shared_core_context_sync_from_globals( context );
}

static n_int shared_core_started_count( void )
{
    n_int count = shared_session_started_count;

    shared_default_core_ensure();
    if ( shared_default_core.simulation_started )
    {
        count++;
    }

    return count;
}

static void shared_core_close_if_unused( void )
{
    if ( ( shared_core_memory_ready == 0 ) || ( shared_core_started_count() != 0 ) )
    {
        return;
    }

    sim_close();
    shared_core_memory_ready = 0;
    simulation_started = 0;
    shared_active_core = 0L;
}

static n_int control_toggle_pause( n_byte actual_toggle )
{
    if ( io_command_line_execution() )
    {
        command_stop( 0L, "", io_console_out );
    }

    if ( actual_toggle )
    {
        toggle_pause ^= 1;
    }
    else
    {
        toggle_pause = 1;
    }

    return toggle_pause;
}

/* do mouse down interaction over a window, at a point, with keys possibly pressed */

extern n_byte	check_about;
extern n_uint	tilt_z;

static n_int control_mouse_was_down = 0;

static n_byte control_mouse_down( ape_app_shell_mouse *session_mouse )
{
    if ( session_mouse != 0L )
    {
        return session_mouse->down;
    }
    return mouse_down;
}

static n_byte control_mouse_drag( ape_app_shell_mouse *session_mouse )
{
    if ( session_mouse != 0L )
    {
        return session_mouse->drag;
    }
    return mouse_drag;
}

static void control_mouse_set_drag( ape_app_shell_mouse *session_mouse, n_int px, n_int py )
{
    if ( session_mouse != 0L )
    {
        session_mouse->drag_x = px;
        session_mouse->drag_y = py;
        session_mouse->drag = 1;
        return;
    }

    mouse_drag_x = px;
    mouse_drag_y = py;
    mouse_drag = 1;
}

static void control_mouse( ape_app_shell_mouse *session_mouse, n_byte wwind, n_int px, n_int py, n_byte option )
{
    if ( wwind == NUM_CONTROL )
    {
        n_int *was_down = ( session_mouse != 0L ) ? &session_mouse->control_was_down : &control_mouse_was_down;

        if ( !control_mouse_down( session_mouse ) && *was_down )
        {
            sim_control_regular( px, py );
        }
        *was_down = control_mouse_down( session_mouse );
    }

    if ( wwind == NUM_VIEW )
    {
        if ( option )
        {
            sim_view_options( px, py );
        }
        else
        {
            if ( sim_view_regular( px, py ) )
            {
                if ( control_mouse_drag( session_mouse ) == 0 )
                {
                    control_mouse_set_drag( session_mouse, px, py );
                }
            }
        }
    }
    if ( wwind == NUM_TERRAIN )
    {
        n_int upper_x, upper_y;

        check_about = 0;
        draw_terrain_coord( &upper_x, &upper_y );

        if ( option != 0 )
        {
            if ( ( px > ( upper_x / 4 ) ) && ( px < ( ( upper_x * 3 ) / 4 ) ) )
            {
                if ( py < ( upper_y / 4 ) )
                {
                    tilt_z = ( ( tilt_z + 1 ) & 255 );
                }
                else if ( py < ( ( upper_y * 3 ) / 4 ) )
                {
                    tilt_z = ( ( tilt_z + 255 ) & 255 );
                }
            }
        }
        else
        {
            n_int sx = px - ( ( upper_x ) >> 1 );
            sim_terrain( sx );
        }
    }
}

/* do key down in which window */
static void control_key( n_byte wwind, n_byte2 num )
{
    if ( ( num > 27 ) && ( num < 32 ) ) /* 28, 29, 30, 31 */
    {
        if ( wwind != NUM_VIEW )
        {
            if ( ( num - 28 ) < 2 )
            {
                sim_rotate( 0 - ( 1 - ( ( num - 28 ) << 1 ) ) );
            }
            else
            {
                sim_move( ( 160 - ( ( num - 28 ) << 6 ) ), 1 );
            }
        }
        else
        {
            sim_move( ( 31 - num ), 0 );
        }
    }
    if ( ( num > 2075 ) && ( num < 2078 ) )
    {
        sim_rotate( 0 - ( -1 + ( ( 2077 - num ) << 1 ) ) );
    }
    if ( ( num > 2077 ) && ( num < 2080 ) )
    {
        sim_change_selected( num == 2078 );
    }
}

#ifdef SCRIPT_DEBUG

static n_int script_entry = 1;

static n_int shared_script_debug_ready( void )
{
    if ( script_entry == 0 )
    {
        return 0;
    }
    if ( scdebug_file_ready() != 0L )
    {
        script_entry = 0;
        return 1;
    }
    return 0;
}

void shared_script_debug_handle( n_constant_string cStringFileName )
{
    if ( cStringFileName )
    {
        n_file   *outputfile = scdebug_file_ready();
        io_disk_write( outputfile, cStringFileName );
    }
    scdebug_file_cleanup();
    script_entry = 1;
}

#else

void shared_script_debug_handle( n_constant_string cStringFileName )
{

}

#endif

static void *control_init( KIND_OF_USE kind, n_uint randomise )
{
    void *sim_return = 0L;

    ( void )draw_error( 0L, 0L, 0 );
    draw_undraw_clear();

    sim_return = sim_init( kind, randomise, OFFSCREENSIZE, VIEWWINDOW( 0 ) );
    sim_set_output( 1 );
    if ( sim_return )
    {
        return draw_offscreen( sim_return );
    }
    return 0;
}

void shared_dimensions( n_int *dimensions )
{
    dimensions[0] = 3;
    dimensions[1] = 512;
    dimensions[2] = 512;
    dimensions[3] = 1;
}

static shared_cycle_state shared_cycle_core( n_uint ticks, n_int localIdentification, ape_app_shell *shell )
{
    shared_cycle_state return_value = SHARED_CYCLE_OK;
    ape_app_shell_mouse *session_mouse = ( shell != 0L ) ? &shell->mouse : 0L;
    ape_app_shell_key *session_key = ( shell != 0L ) ? &shell->key : 0L;
    n_byte current_mouse_identification = ( session_mouse != 0L ) ? session_mouse->identification : mouse_identification;
    n_byte current_mouse_down = ( session_mouse != 0L ) ? session_mouse->down : mouse_down;
    n_byte current_mouse_option = ( session_mouse != 0L ) ? session_mouse->option : mouse_option;
    n_int current_mouse_x = ( session_mouse != 0L ) ? session_mouse->x : mouse_x;
    n_int current_mouse_y = ( session_mouse != 0L ) ? session_mouse->y : mouse_y;
    n_byte current_key_down = ( session_key != 0L ) ? session_key->down : key_down;
    n_byte current_key_identification = ( session_key != 0L ) ? session_key->identification : key_identification;
    n_byte2 current_key_value = ( session_key != 0L ) ? session_key->value : key_value;

    if ( simulation_started == 0 )
    {
        return return_value;
    }

    ticks = ticks & 67108863; /* 71 58 27 88 */
    ticks *= 60;
#ifndef	_WIN32
    sim_thread_console();
#endif
    if ( current_mouse_identification == localIdentification )
    {
        if ( localIdentification == NUM_CONTROL )
        {
            control_mouse( session_mouse, current_mouse_identification, current_mouse_x, current_mouse_y, current_mouse_option );
        }
        else if ( current_mouse_down == 1 )
        {
            if ( localIdentification == NUM_VIEW )
            {
#if (MAP_BITS == 8)
                control_mouse( session_mouse, current_mouse_identification, current_mouse_x / 2, current_mouse_y / 2, current_mouse_option );
#else
                control_mouse( session_mouse, current_mouse_identification, current_mouse_x, current_mouse_y, current_mouse_option );
#endif
            }
            else
            {
                control_mouse( session_mouse, current_mouse_identification, current_mouse_x, current_mouse_y, current_mouse_option );
            }
        }
    }

    if ( ( current_key_down == 1 ) && ( current_key_identification == localIdentification ) )
    {
        if ( ( current_key_identification == NUM_VIEW ) || ( current_key_identification == NUM_TERRAIN ) )
        {
            control_key( current_key_identification, current_key_value );
        }
    }
    if ( localIdentification == WINDOW_PROCESSING )
    {
        sim_realtime( ticks );
        if ( ( io_command_line_execution() != 1 ) && ( !toggle_pause ) )
        {
            sim_cycle();
            sim_update_output();
        }
#ifndef    _WIN32
        if ( sim_new_run_condition() )
        {
            return_value = SHARED_CYCLE_NEW_APES;
        }
#endif
#ifdef SCRIPT_DEBUG
        if ( shared_script_debug_ready() )
        {
            return_value = SHARED_CYCLE_DEBUG_OUTPUT;
        }
#endif
#ifndef	_WIN32
        if ( sim_thread_console_quit() )
        {
            return_value = SHARED_CYCLE_QUIT;
        }
#endif
    }    
    return return_value;
}

shared_cycle_state shared_cycle( n_uint ticks, n_int localIdentification )
{
    shared_cycle_state state;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return SHARED_CYCLE_OK;
    }

    state = shared_cycle_core( ticks, localIdentification, 0L );
    shared_core_context_sync_from_globals( &shared_default_core );
    return state;
}

static n_int shared_init_core( n_int view, n_uint random )
{
    key_down = 0;
    mouse_down = 0;
    mouse_drag = 0;
    if ( view == WINDOW_PROCESSING )
    {
        KIND_OF_USE init_type = shared_core_memory_ready ? KIND_NEW_SIMULATION : KIND_START_UP;

        if ( control_init( init_type, random ) == 0L )
        {
            return SHOW_ERROR( "Initialization failed lack of memory" );
        }
        shared_core_memory_ready = 1;
        simulation_started = 1;
        if ( shared_session_initializing == 0 )
        {
            shared_default_uses_core = 1;
        }
    }
    return view;
}

n_int shared_init( n_int view, n_uint random )
{
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_init_core( view, random );
    if ( result == view )
    {
        shared_default_core.simulation_started = simulation_started;
        shared_default_core.selected_index = shared_current_selected_index();
        shared_core_context_became_current( &shared_default_core );
    }
    return result;
}

void shared_close( void )
{
    shared_default_core_ensure();

    if ( shared_default_core.simulation_started == 0 )
    {
        shared_core_close_if_unused();
        return;
    }

    if ( shared_active_core != &shared_default_core )
    {
        ( void )shared_core_activate( &shared_default_core );
    }

    simulation_started = 0;
    shared_default_uses_core = 0;
    shared_default_core.simulation_started = 0;
    shared_default_core.selected_index = -1;
    shared_core_context_release_snapshot( &shared_default_core );
    ape_buffer_free( &outputBuffer );

    if ( shared_active_core == &shared_default_core )
    {
        shared_active_core = 0L;
    }

    shared_core_close_if_unused();
}

n_int shared_simulation_started(void)
{
    shared_default_core_ensure();
    if ( shared_active_core == &shared_default_core )
    {
        shared_core_context_sync_from_globals( &shared_default_core );
    }
    return shared_default_core.simulation_started;
}

void shared_keyReceived( n_int value, n_int fIdentification )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    key_down = 1;
    key_value = value;
    key_identification = fIdentification;
}

void shared_keyUp( void )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    key_down = 0;
}

void shared_mouseOption( n_byte option )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    mouse_option = option;
}

static void shared_mouse_coordinates( n_double valX, n_double valY, n_int fIdentification, n_int *x, n_int *y )
{
    if ( ( x == 0L ) || ( y == 0L ) )
    {
        return;
    }

    if ( fIdentification == NUM_VIEW )
    {
        n_vect2 *being_location = draw_selected_location();
        if ( being_location != 0L )
        {
            *x = ( n_int )( valX + 256 + being_location->x ) & 511;
            *y = ( n_int )( valY + 256 + being_location->y ) & 511;
            return;
        }
    }

    *x = ( n_int )valX;
    *y = ( n_int )valY;
}

void shared_mouseReceived( n_double valX, n_double valY, n_int fIdentification )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    mouse_down = 1;
    mouse_identification = fIdentification;
    shared_mouse_coordinates( valX, valY, fIdentification, &mouse_x, &mouse_y );
}

void shared_mouseReceived_ios( n_double valX, n_double valY )
{
    n_int new_rotation_vertical = ( valX < valY );

    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );

    if (new_rotation_vertical) printf("port\n"); else printf("land\n");
    printf("mouse %f x %f\n", valX, valY);
    printf("contr %ld x %ld\n", ios_control_x, ios_control_y);
    printf("view  %ld x %ld\n", ios_view_x, ios_view_y);
    printf("dimen %ld x %ld\n", ios_dimension_x, ios_dimension_y);
    printf("------------\n");

    mouse_x = ( n_int )valX;
    mouse_y = ( n_int )valY;
}


void shared_mouseUp( void )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    mouse_down = 0;
    mouse_drag = 0;
}

void shared_about( void )
{
    draw_about();
}

static void shared_clearErrors( void )
{
    ( void )draw_error( 0L, 0L, 0 );
}


static n_int shared_new_type( KIND_OF_USE type, n_uint seed )
{
    static  n_int NewBlock = 0;

    if ( NewBlock )
    {
        return 0;
    }
    NewBlock = 1;
    ( void )control_init( type, seed );
    NewBlock = 0;
    return 0;
}

n_int shared_new( n_uint seed )
{
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_new_type( KIND_NEW_SIMULATION, seed );
    shared_core_context_became_current( &shared_default_core );
    return result;
}

n_int shared_new_agents( n_uint seed )
{
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_new_type( KIND_NEW_APES, seed );
    shared_core_context_became_current( &shared_default_core );
    return result;
}

static n_byte shared_openFileName_core( n_constant_string cStringFileName, n_int isScript )
{
    ( void )control_toggle_pause( 0 );

    if ( isScript )
    {
        return ( command_script( 0L, (n_string) cStringFileName, 0L ) == 0 );
    }
    return ( command_open( 0L, (n_string) cStringFileName, 0L ) == 0 );
}

n_byte shared_openFileName( n_constant_string cStringFileName, n_int isScript )
{
    n_byte result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return 0;
    }

    result = shared_openFileName_core( cStringFileName, isScript );
    shared_core_context_became_current( &shared_default_core );
    return result;
}

static void shared_saveFileName_core( n_constant_string cStringFileName )
{
    ( void )control_toggle_pause( 0 );
    ( void )command_save( 0L, (n_string) cStringFileName, 0L );
}

void shared_saveFileName( n_constant_string cStringFileName )
{
    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return;
    }

    shared_saveFileName_core( cStringFileName );
    shared_core_context_became_current( &shared_default_core );
}

void shared_delta( n_double delta_x, n_double delta_y, n_int wwind )
{

}

void shared_zoom( n_double num, n_int wwind )
{

}

void shared_rotate( n_double num, n_int wwind )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );

    if ( wwind == NUM_TERRAIN )
    {
        n_int integer_rotation_256 = ( n_int )( ( num * 256 ) / 360 );
        sim_rotate( integer_rotation_256 );

    }

    shared_core_context_sync_from_globals( &shared_default_core );
}

n_uint shared_max_fps( void )
{
    return 60;
}

static n_int shared_menu_core( n_int menuVal )
{
    switch ( menuVal )
    {
    case NA_MENU_PAUSE:
        return control_toggle_pause( 1 );
    case NA_MENU_WEATHER:
        return draw_toggle_weather();
    case NA_MENU_BRAIN:
        return draw_toggle_brain();
    case NA_MENU_BRAINCODE:
        return draw_toggle_braincode();
    case NA_MENU_TERRITORY:
        return draw_toggle_territory();
    case NA_MENU_TIDEDAYLIGHT:
        return draw_toggle_tide_daylight();
    case NA_MENU_PREVIOUS_APE:
        ( void )control_key( 0, 2079 );
        return 0;
    case NA_MENU_NEXT_APE:
        ( void )control_key( 0, 2078 );
        return 0;
    case NA_MENU_CLEAR_ERRORS:
        ( void )draw_error( 0L, 0L, 0 );
        return 0;
    case NA_MENU_FLOOD:
        sim_flood();
        return 0;
    case NA_MENU_HEALTHY_CARRIER:
        sim_healthy_carrier();
        return 0;
    case NA_MENU_FOLLOW:
        return draw_toggle_follow();
    case NA_MENU_SOCIAL_WEB:
        return draw_toggle_social_web();
    }
    return -1;
}

n_int shared_menu( n_int menuVal )
{
    n_int result;

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return -1;
    }

    result = shared_menu_core( menuVal );
    shared_core_context_sync_from_globals( &shared_default_core );
    return result;
}

n_byte *shared_legacy_draw( n_byte fIdentification, n_int dim_x, n_int dim_y )
{
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );

    if ( fIdentification == NUM_TERRAIN )
    {
        draw_window( dim_x, dim_y );
    }
    draw_cycle( 0, DRAW_WINDOW_VIEW | DRAW_WINDOW_CONTROL | DRAW_WINDOW_TERRAIN );

    return draw_pointer( fIdentification );
}

void shared_color_8_bit_to_48_bit( n_byte2 *fit )
{
    land_color_time( fit, 1 );
}

n_int shared_being_number( void )
{
    simulated_group *group = sim_group();
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
    if ( group )
    {
        return group->num;
    }
    return 0;
}

void shared_being_name( n_int number, n_string name )
{
    simulated_group *group = sim_group();
    simulated_being *being = 0L;
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
    if ( group )
    {
        if (number < group->num)
        {
            being = &(group->beings[number]);
        }
    }
    being_name_simple(being, name);
}

void shared_being_select( n_int number)
{
    simulated_group *group = sim_group();
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
    if ( group )
    {
        if (number < group->num)
        {
            sim_set_select(&group->beings[number]);
        }
    }
    shared_core_context_sync_from_globals( &shared_default_core );
}

n_int shared_selected_location( n_int *x, n_int *y )
{
    n_vect2 *being_location = draw_selected_location();
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    being_location = draw_selected_location();
    if ( ( being_location == 0L ) || ( x == 0L ) || ( y == 0L ) )
    {
        return 0;
    }

    *x = being_location->x;
    *y = being_location->y;
    return 1;
}

n_int shared_selected_being_location( n_int *x, n_int *y )
{
    simulated_group *group = sim_group();
    shared_default_core_ensure();
    ( void )shared_core_activate( &shared_default_core );
    group = sim_group();
    if ( ( group == 0L ) || ( group->select == 0L ) || ( x == 0L ) || ( y == 0L ) )
    {
        return 0;
    }

    *x = being_location_x( group->select );
    *y = being_location_y( group->select );
    return 1;
}

static n_byte4         colorLookUp[256][256];
static n_uint          old_hash = 0;

static void shared_color_update( void )
{
    n_int           loop = 0;
    n_int           loopColors = 0;
    n_byte          fit[256 * 3];
    n_int           cloud = 1;
    n_uint          new_hash;

    land_color_time_8bit( fit, draw_toggle_tide_daylight_value() );

    new_hash = math_hash( fit, 256 * 3 );

    if ( new_hash != old_hash )
    {
        old_hash = new_hash;
        while ( loopColors < 256 )
        {
            n_byte *juxtapose = ( n_byte * )&colorLookUp[0][loopColors];
            n_byte red = fit[loop++];
            n_byte green = fit[loop++];
            n_byte blue = fit[loop++];
            n_byte alpha = 0;


            juxtapose[0] = alpha;
            juxtapose[1] = red;
            juxtapose[2] = green;
            juxtapose[3] = blue;
            
            loopColors++;
        }
        while ( cloud < 256 )
        {
            n_int negCloud = 256 - cloud;
            loopColors = 0;
            while ( loopColors < 256 )
            {
                n_byte *juxtapose  = ( n_byte * )&colorLookUp[cloud][loopColors];
                n_byte *juxtapose0 = ( n_byte * )&colorLookUp[0][loopColors];
                juxtapose[0] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[0] ) >> 8 ) );
                juxtapose[1] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[1] ) >> 8 ) );
                juxtapose[2] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[2] ) >> 8 ) );
                juxtapose[3] = ( n_byte )( cloud + ( ( negCloud * juxtapose0[3] ) >> 8 ) );
                loopColors++;
            }
            cloud++;
        }
    }
}

static n_byte * shared_output_buffer(n_int width, n_int height)
{
    return ape_buffer_require( &outputBuffer, width, height );
}

#define Y_POINT(ly, dim_y) (ly)

static void shared_bitcopy_view(n_byte *outputBuffer, n_int dim_x, n_int dim_y,
                                 n_int offset_x, n_int offset_y, n_int multi_x)
{
    n_byte *index = draw_pointer(NUM_VIEW);
    n_byte *local_weather = draw_weather_grayscale();
    n_vect2 *being_location = draw_selected_location();
    n_byte4 *outputBuffer4 = (n_byte4 *)outputBuffer;

    if (index == 0L || local_weather == 0L || being_location == 0L)
    {
        return;
    }

    n_int base_x = (256 + being_location->x + dim_x) % dim_x;
    n_int base_y = (256 + being_location->y + dim_y) % dim_y;

    for (n_int ly = 0; ly < dim_y; ly++)
    {
        n_int point_y = (Y_POINT(ly, dim_y) + base_y) % dim_y;
        n_int row_offset = point_y * dim_x;
        n_byte *index_row = &index[row_offset];
        n_byte *weather_row = &local_weather[row_offset];
        n_int out_offset = (ly + offset_y) * multi_x + offset_x;

        for (n_int lx = 0; lx < dim_x; lx++)
        {
            n_int point_x = (lx + base_x) % dim_x;
            n_byte value = index_row[point_x];
            n_byte cloud = weather_row[point_x];
            n_byte4 *color_row = (n_byte4 *)colorLookUp[cloud];
            outputBuffer4[out_offset + lx] = color_row[value];
        }
    }
}

static void shared_bitcopy_view_ios(n_byte *outputBuffer, n_int dimension,
                                    n_int offset_x, n_int offset_y, n_int multi_x)
{
    n_byte *index = draw_pointer(NUM_VIEW);
    n_byte *local_weather = draw_weather_grayscale();
    n_vect2 *being_location = draw_selected_location();
    n_byte4 *outputBuffer4 = (n_byte4 *)outputBuffer;

    n_int map_dim_x = MAP_DIMENSION;
    n_int map_dim_y = MAP_DIMENSION;

    if (index == 0L || local_weather == 0L || being_location == 0L)
    {
        return;
    }

    if (dimension > MAP_DIMENSION)
    {
        dimension = MAP_DIMENSION;
    }

    n_int point_start = (MAP_DIMENSION - dimension) / 2;
    if (point_start < 0) point_start = 0;

    n_int base_x = (256 + being_location->x + map_dim_x) % map_dim_x;
    n_int base_y = (256 + being_location->y + map_dim_y) % map_dim_y;

    for (n_int screen_y = 0; screen_y < dimension; screen_y++)
    {
        n_int ly = point_start + screen_y;
        n_int point_y = (Y_POINT(ly, map_dim_y) + base_y) % map_dim_y;
        n_int row_offset = point_y * map_dim_x;
        n_byte *index_row = &index[row_offset];
        n_byte *weather_row = &local_weather[row_offset];
        n_int out_offset = (ly + offset_y) * multi_x + offset_x;

        for (n_int screen_x = 0; screen_x < dimension; screen_x++)
        {
            n_int lx = point_start + screen_x;
            n_int point_x = (lx + base_x) % map_dim_x;
            n_byte value = index_row[point_x];
            n_byte cloud = weather_row[point_x];
            n_byte4 *color_row = (n_byte4 *)colorLookUp[cloud];
            outputBuffer4[out_offset + screen_x] = color_row[value];
        }
    }
}


static void shared_bitcopy(n_byte *outputBuffer, n_int dim_x, n_int dim_y,
                           n_int offset_x, n_int offset_y, n_int multi_x, n_int fIdentification)
{
    n_byte *index = draw_pointer(fIdentification);
    n_byte4 *outputBuffer4 = (n_byte4 *)outputBuffer;

    if (index == 0L)
    {
        return;
    }

    n_byte4 *color_row = (n_byte4 *)colorLookUp[0];

    for (n_int ly = 0; ly < dim_y; ly++)
    {
        n_byte *index_row = &index[Y_POINT(ly, dim_y) * dim_x];
        n_int out_offset = (ly + offset_y) * multi_x + offset_x;

        for (n_int lx = 0; lx < dim_x; lx++)
        {
            n_byte value = index_row[lx];
            outputBuffer4[out_offset + lx] = color_row[value];
        }
    }
}

static void shared_draw_into_buffer( n_byte *drawBuffer, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    if ( drawBuffer == 0L )
    {
        return;
    }

#ifdef	_WIN32
    {
        n_byte *index = draw_pointer( fIdentification );
        memory_copy( index, drawBuffer, dim_x * dim_y );
    }
#else

    draw_window( dim_x, dim_y );
    draw_cycle( size_changed, ( n_byte )( 1 << fIdentification ) );
    shared_color_update();

#ifdef ALPHA_WEATHER_DRAW
    if ( fIdentification == NUM_VIEW )
    { // fix here for MacOS 26
        dim_y -= 4;

        shared_bitcopy_view( drawBuffer, dim_x, dim_y, 0, 0, dim_x );
    }
    else
#endif
    {
        shared_bitcopy( drawBuffer, dim_x, dim_y, 0, 0, dim_x, fIdentification );
    }
#endif
}

n_byte * shared_draw( n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    n_byte *drawBuffer = shared_output_buffer( dim_x, dim_y );

    shared_default_core_ensure();
    if ( shared_core_activate( &shared_default_core ) == 0 )
    {
        return drawBuffer;
    }

    if ( simulation_started == 0 )
    {
        SHOW_ERROR( "draw - simulation not started" );
        return drawBuffer;
    }

    shared_draw_into_buffer( drawBuffer, fIdentification, dim_x, dim_y, size_changed );

    shared_core_context_sync_from_globals( &shared_default_core );
    return drawBuffer;
}

void shared_copy_rotate_180( n_byte4 *outputBuffer, const n_byte4 *source, n_int dim_x, n_int dim_y )
{
    if ( ( outputBuffer == 0L ) || ( source == 0L ) )
    {
        return;
    }

    for ( n_int ly = 0; ly < dim_y; ly++ )
    {
        n_int row_offset = ly * dim_x;
        n_int source_row_offset = ( dim_y - 1 - ly ) * dim_x;

        for ( n_int lx = 0; lx < dim_x; lx++ )
        {
            outputBuffer[row_offset + lx] = source[source_row_offset + ( dim_x - 1 - lx )];
        }
    }
}

static n_int shared_session_init_callback( void *context, n_int view, n_uint random )
{
    ( void )context;
    return ( shared_init_core( view, random ) == view );
}

static shared_cycle_state shared_session_cycle_callback( void *context, n_uint ticks, n_int identification )
{
    shared_session *session = ( shared_session * )context;
    return shared_cycle_core( ticks, identification, ( session != 0L ) ? &session->shell : 0L );
}

static void shared_session_draw_callback( void *context, n_byte *drawBuffer, n_int identification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    ( void )context;
    shared_draw_into_buffer( drawBuffer, identification, dim_x, dim_y, size_changed );
}

shared_session *shared_session_create( n_uint random )
{
    shared_session *session = ( shared_session * )memory_new( sizeof( shared_session ) );

    if ( session != 0L )
    {
        memory_erase( ( n_byte * )session, sizeof( shared_session ) );
        session->initial_random = random;
        ape_app_shell_reset( &session->shell );
        shared_core_context_reset_values( &session->core );
    }

    return session;
}

void shared_session_destroy( shared_session *session )
{
    if ( session == 0L )
    {
        return;
    }

    if ( ape_app_shell_simulation_started( &session->shell ) )
    {
        if ( shared_session_started_count > 0 )
        {
            shared_session_started_count--;
        }
    }

    if ( shared_active_core == &session->core )
    {
        shared_active_core = 0L;
    }

    ape_app_shell_shutdown( &session->shell, 0L, 0L );
    session->core.simulation_started = 0;
    shared_core_context_release_snapshot( &session->core );
    shared_core_close_if_unused();

    memory_free( ( void ** )&session );
}

n_int shared_session_init( shared_session *session, n_int view, n_uint random )
{
    n_int was_started;

    if ( session == 0L )
    {
        return -1;
    }

    was_started = ape_app_shell_simulation_started( &session->shell );
    session->initial_random = random;

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    shared_session_initializing++;
    ( void )ape_app_shell_init( &session->shell, view, random, shared_session_init_callback, session );
    shared_session_initializing--;

    if ( ape_app_shell_simulation_started( &session->shell ) )
    {
        if ( was_started == 0 )
        {
            shared_session_started_count++;
        }
        session->core.simulation_started = simulation_started;
        session->core.selected_index = shared_current_selected_index();
        shared_core_context_became_current( &session->core );
        return view;
    }

    if ( was_started && ( shared_session_started_count > 0 ) )
    {
        shared_session_started_count--;
    }
    session->core.simulation_started = 0;

    return -1;
}

n_int shared_session_simulation_started( const shared_session *session )
{
    if ( session == 0L )
    {
        return 0;
    }

    return ape_app_shell_simulation_started( &session->shell );
}

shared_cycle_state shared_session_cycle( shared_session *session, n_uint ticks, n_int localIdentification )
{
    shared_cycle_state state;

    if ( session == 0L )
    {
        return SHARED_CYCLE_OK;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return SHARED_CYCLE_OK;
    }

    state = ape_app_shell_cycle( &session->shell, ticks, localIdentification, shared_session_cycle_callback, session );
    shared_core_context_sync_from_globals( &session->core );
    return state;
}

shared_cycle_state shared_session_cycle_ios( shared_session *session, n_uint ticks )
{
    return shared_session_cycle( session, ticks, WINDOW_PROCESSING );
}

n_byte *shared_session_draw( shared_session *session, n_int fIdentification, n_int dim_x, n_int dim_y, n_byte size_changed )
{
    n_byte *buffer;

    if ( session == 0L )
    {
        return 0L;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0L;
    }

    buffer = ape_app_shell_draw( &session->shell, fIdentification, dim_x, dim_y, size_changed, shared_session_draw_callback, session );
    shared_core_context_sync_from_globals( &session->core );
    return buffer;
}

void shared_session_draw_ios( shared_session *session, n_byte4 *outputBuffer, n_int dim_x, n_int dim_y )
{
    n_byte *buffer;

    if ( outputBuffer == 0L )
    {
        return;
    }

    buffer = shared_session_draw( session, NUM_TERRAIN, dim_x, dim_y, 0 );
    if ( buffer == 0L )
    {
        return;
    }

    shared_copy_rotate_180( outputBuffer, ( const n_byte4 * )buffer, dim_x, dim_y );
}

static n_byte shared_session_is_started( shared_session *session )
{
    return ( n_byte )( ( session != 0L ) && ape_app_shell_simulation_started( &session->shell ) );
}

n_int shared_session_menu( shared_session *session, n_int menuValue )
{
    n_int result;

    if ( shared_session_is_started( session ) == 0 )
    {
        return -1;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    result = shared_menu_core( menuValue );
    shared_core_context_sync_from_globals( &session->core );
    return result;
}

n_int shared_session_new( shared_session *session, n_uint seed )
{
    n_int result;

    if ( shared_session_is_started( session ) == 0 )
    {
        return -1;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    result = shared_new_type( KIND_NEW_SIMULATION, seed );
    shared_core_context_became_current( &session->core );
    return result;
}

n_int shared_session_pause_state( shared_session *session )
{
    if ( session == 0L )
    {
        return 0;
    }

    return session->core.toggle_pause;
}

n_int shared_session_selected_being_index( shared_session *session )
{
    if ( session == 0L )
    {
        return -1;
    }

    return session->core.selected_index;
}

n_int shared_session_selected_being_facing( shared_session *session )
{
    simulated_group *group;
    n_int facing;

    if ( shared_session_is_started( session ) == 0 )
    {
        return -1;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return -1;
    }

    group = sim_group();
    if ( ( group == 0L ) || ( group->select == 0L ) )
    {
        return -1;
    }

    facing = ( n_int )being_facing( group->select );
    shared_core_context_sync_from_globals( &session->core );
    return facing;
}

n_int shared_session_being_number( shared_session *session )
{
    simulated_group *group;

    if ( shared_session_is_started( session ) == 0 )
    {
        return 0;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0;
    }

    group = sim_group();
    shared_core_context_sync_from_globals( &session->core );
    if ( group == 0L )
    {
        return 0;
    }
    return group->num;
}

n_int shared_session_selected_being_location( shared_session *session, n_int *x, n_int *y )
{
    simulated_group *group;

    if ( ( shared_session_is_started( session ) == 0 ) || ( x == 0L ) || ( y == 0L ) )
    {
        return 0;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0;
    }

    group = sim_group();
    if ( ( group == 0L ) || ( group->select == 0L ) )
    {
        return 0;
    }

    *x = being_location_x( group->select );
    *y = being_location_y( group->select );
    shared_core_context_sync_from_globals( &session->core );
    return 1;
}

void shared_session_rotate( shared_session *session, n_double num, n_int wwind )
{
    if ( shared_session_is_started( session ) == 0 )
    {
        return;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return;
    }

    shared_rotate( num, wwind );
    shared_core_context_sync_from_globals( &session->core );
}

n_int shared_session_input_is_active( shared_session *session )
{
    if ( session == 0L )
    {
        return 0;
    }

    return ( session->shell.key.down != 0 ) ||
           ( session->shell.mouse.down != 0 ) ||
           ( session->shell.mouse.drag != 0 );
}

void shared_session_mouseOption( shared_session *session, n_byte option )
{
    if ( session == 0L )
    {
        return;
    }

    ape_app_shell_mouse_option( &session->shell, option );
}

void shared_session_mouseReceived( shared_session *session, n_double valX, n_double valY, n_int localIdentification )
{
    n_int session_x = 0;
    n_int session_y = 0;

    if ( session == 0L )
    {
        return;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return;
    }

    shared_mouse_coordinates( valX, valY, localIdentification, &session_x, &session_y );
    ape_app_shell_mouse_received( &session->shell, session_x, session_y, localIdentification );
    shared_core_context_sync_from_globals( &session->core );
}

void shared_session_mouseUp( shared_session *session )
{
    if ( session == 0L )
    {
        return;
    }

    ape_app_shell_mouse_up( &session->shell );
}

n_byte shared_session_openFileName( shared_session *session, n_constant_string cStringFileName, n_int isScript )
{
    n_byte result;

    if ( shared_session_is_started( session ) == 0 )
    {
        return 0;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return 0;
    }

    result = shared_openFileName_core( cStringFileName, isScript );
    shared_core_context_became_current( &session->core );
    return result;
}

void shared_session_saveFileName( shared_session *session, n_constant_string cStringFileName )
{
    if ( shared_session_is_started( session ) == 0 )
    {
        return;
    }

    if ( shared_core_activate( &session->core ) == 0 )
    {
        return;
    }

    shared_saveFileName_core( cStringFileName );
    shared_core_context_sync_from_globals( &session->core );
}

void shared_draw_ios( n_byte4 *outputBuffer, n_int dim_x, n_int dim_y )
{
    n_byte *buffer;

    if ( outputBuffer == 0L )
    {
        return;
    }

    buffer = shared_draw( NUM_TERRAIN, dim_x, dim_y, 0 );
    if ( buffer == 0L )
    {
        return;
    }

    shared_copy_rotate_180( outputBuffer, ( const n_byte4 * )buffer, dim_x, dim_y );
}

shared_cycle_state shared_cycle_ios( n_uint ticks )
{
    return shared_cycle( ticks, WINDOW_PROCESSING );
}
